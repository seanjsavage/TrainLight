
#include "neopixel.h"

int activityLed  = D7;
unsigned int updateTimeout = 0;    // next time to contact the server
 
#define PIXEL_PIN D0
#define PIXEL_COUNT 60
#define PIXEL_TYPE WS2812B
#define FAR 10
#define CLOSE 5
#define CLOSER 2

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

uint32_t closestColor = strip.Color(254,240,0);  // Yellow
uint32_t closerColor = strip.Color(0,240,0);  // Bright Green
uint32_t closeColor = strip.Color(0,60,0);  // Dark Green
uint32_t farColor = strip.Color(0,0,0);  // Off

void setup() {
	Time.zone(+1.0); // +1.0 Winter/ +2.0 Summer// TODO: detect DST 
	Serial.begin(9600);
	pinMode(activityLed, OUTPUT);

	strip.begin();
	strip.show(); // Initialize all pixels to 'off'
	strip.setBrightness(20);
		
	Spark.function("parse_values", parse_values);

}

void loop() {

	// setup led
	RGB.control(true);
	RGB.brightness(5);

//  Color zones: TL, TR, RT, RB, BR, BL, LB, LT. (Arranged clockwise, starting from upper left.)
//	That means: 22 Southbound, 24 S, N-Judah W, 6/71 W, 24 N, 22 N, 6/71 E, N-Judah E 

//	To test locally:
//	parse_values("TL=1&TR=9&RT=1&RB=3&BR=12&BL=1&LB=4&LT=5&");

	if (updateTimeout > millis()) {
		// keep the same color while waiting
		return;
	}

	digitalWrite(activityLed, HIGH); // indicate activity

	// check again in 5 seconds:
	updateTimeout = millis() + 5000;
	digitalWrite(activityLed, LOW);
}

// Set Transit Lites according to 8-zone square. Inputs a color each of the 8 zones, starting top left of the square and going clockwise. Colors them the specified colors, then waits ('wait2' seconds)

void colorTransitLites(String TL, String TR, String RT, String RB, String BR, String BL, String LB, String LT) {
  int wait2 = 1; // Number of seconds to wait between each loop.
  uint16_t i;
  for(i=0; i<7; i++) {
    distanceToColor(i, LB);
  }
  for(i=7; i<15; i++) {
    distanceToColor(i, LT);
  }
  for(i=15; i<23; i++) {
    distanceToColor(i, TL);
  }
  for(i=23; i<30; i++) {
    distanceToColor(i, TR);
  }
  for(i=30; i<38; i++) {
    distanceToColor(i, RT);
  }
  for(i=38; i<45; i++) {
    distanceToColor(i, RB);
  }
  for(i=45; i<52; i++) {
    distanceToColor(i, BR);
  }
  for(i=52; i<60; i++) {
    distanceToColor(i, BL);
  }
  strip.show();
  delay(wait2*1000);
}

void distanceToColor(uint16_t j, String distance) {
  if(distance == "CLOSEST") {
    strip.setPixelColor(j, closestColor);
  } else if(distance == "CLOSER") {
    strip.setPixelColor(j, closerColor);
  } else if(distance == "CLOSE") {
    strip.setPixelColor(j, closeColor);
  } else {
    strip.setPixelColor(j, farColor);
  }
}

String shortDescr(String &descr) {
	if(descr.startsWith("moderate ")){
		 descr.remove(3,5);
		 return descr;
	}
	return descr;
}

int parse_values(String val){
	int values[8];
	String distances[8];
	char c;
	short i = 0,target=0;
	
	while(val[i]){
		c = val[i++];
		if(c >= '0' && c <= '9'){
			distances[target]+=c;
		}
		else{
			values[target] = atoi(distances[target].c_str());
			target++;
		}
	}
	values[target] = atoi(distances[target].c_str());
	
	for(i = 0; i < 8; i++)
	{
		if(values[i] >= FAR || values[i] <= 0)
			distances[i] = "FAR";
		else if(values[i] >= CLOSE)
			distances[i] = "CLOSE";
		else if(values[i] >= CLOSER)
			distances[i] = "CLOSER";
		else if(values[i] > 0)
			distances[i] = "CLOSEST";
	}

	colorTransitLites(distances[0],distances[1],distances[2],distances[3],distances[4],distances[5],distances[6],distances[7]);
	return 0;
}
