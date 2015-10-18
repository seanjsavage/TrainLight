# To run in cron every 30 seconds, the "-q" removes debug messages:
# * * * * * cd [directory-containing-parser]/trainlightparser-rb && ruby trainlight_parser.rb -q


require 'pp'
require 'json'


if !ENV['SPARK_ACCESS_TOKEN']
  STDERR.puts "You need to specify the environment variable SPARK_ACCESS_TOKEN."
  exit
end

class TrainLightScript
  def initialize(debug)
    @debug = debug
  end
  
  def sources  
    # :time_interval: How long it takes to walk from TrainLight to each stop, in minutes. 
    #                 These values are subtracted from the raw feed predictions to form calibrated 
    #                 predictions – and if that results in a value < 0 minutes for a prediction, 
    #                 it will skip that value and use the next bus/train arriving for that stop instead.
    return [
      # 22 Southbound
      {:id =>"TL", :url =>"http://skeduapp.com:3000/agencies/sf-muni/routes/22/stops/4005/predictions", :time_interval => 7},

      # 24 Southbound
      {:id =>"TR", :url =>"http://skeduapp.com:3000/agencies/sf-muni/routes/24/stops/4330/predictions", :time_interval => 2},

      # N Westbound
      {:id =>"RT", :url =>"http://skeduapp.com:3000/agencies/sf-muni/routes/N/stops/7252/predictions", :time_interval => 5},

      # 6/71 Westbound
      {:id =>"RB", :url =>"http://skeduapp.com:3000/agencies/sf-muni/stops/14950/predictions", :time_interval => 7},

      # 24 Northbounnd
      {:id =>"BR", :url =>"http://skeduapp.com:3000/agencies/sf-muni/routes/24/stops/4331/predictions", :time_interval => 2},

      # 22 Northbound
      {:id =>"BL", :url =>"http://skeduapp.com:3000/agencies/sf-muni/routes/22/stops/4006/predictions", :time_interval => 8},

      # 6/71 Eastbound
      {:id =>"LB", :url =>"http://skeduapp.com:3000/agencies/sf-muni/stops/14951/predictions", :time_interval => 6},

      # N Eastbound
      {:id =>"LT", :url =>"http://skeduapp.com:3000/agencies/sf-muni/routes/N/stops/7318/predictions", :time_interval => 4}
    ]
  end
  
  def parse_minutes_away(json_data, time_interval)
    nearest = 999
    json_data.each do |line_data|
      route = line_data['route']

      line_data['values'].each do |value|
        predicted_minutes = value['minutes'].to_i
        minutes = predicted_minutes - time_interval
        debug_log "route #{route}: predicted=#{predicted_minutes} :: adjusted=#{minutes}"
        if minutes > 0 && minutes < nearest
          nearest = minutes
        end
      end
    end
    return nearest
  end

  def debug_log(msg)
    puts msg if @debug
  end

  def run
    minutes = [] # will be filled with the calculated minutes (with walk time subtracted) for each line

    sources.each do |source|
      identifier = source[:id]
      string_data = `curl -s #{source[:url]}`
      json_data = JSON.parse(string_data)
      minutes_away = parse_minutes_away(json_data, source[:time_interval])
      debug_log "#{identifier}: #{minutes_away}"
      minutes << minutes_away
    end

    post_url = "https://api.spark.io/v1/devices/53ff6e066667574818382567/parse_values?access_token=#{ENV['SPARK_ACCESS_TOKEN']}"
    post_value_string = minutes.join('_') + '_' # if you don't actually need the trailing underscore, can get rid of the + part here
    debug_log "t=#{post_value_string}"

    curl_cmd = "curl -s -X POST -d \"t=#{post_value_string}\" #{post_url}"
    debug_log curl_cmd

    post_response = `#{curl_cmd}`
    debug_log "Response: #{post_response}"
  end
end

script = TrainLightScript.new(ARGV.first != '-q')


# run once, wait 30 seconds, run again
script.run
sleep 30
script.run
