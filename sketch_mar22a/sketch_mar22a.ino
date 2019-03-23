#include <PixySPI_SS.h>
#include <PixyI2C.h>
#include <Pixy.h>
#include <TPixy.h>
#include <PixyUART.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>

struct dataBlock{
  int left,right,fwd,back,ckw,ccw;
}
typedef struct dataBlock DataOut;

Pixy pixy;

#define headlight_data 19
#define left_data 20 //changeme to a good pin
#define right_data 21 //changeme to a good pin
#define kEqn 80k+160
////////////////////////////////////////////////#leds, led data pin
Adafruit_NeoPixel headlight = Adafruit_NeoPixel(19, led_data, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel leftIndicator = Adafruit_NeoPixel(19, led_data, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel rightIndicator = Adafruit_NeoPixel(19, led_data, NEO_GRB + NEO_KHZ800);

uint32_t green100L = leftIndicator.Color(0, 255, 0);
uint32_t yellow100L = leftIndicator.Color(255, 255, 0);
uint32_t red100L = leftIndicator.Color(255, 0, 0);
uint32_t allOffL = leftIndicator.Color(0, 0, 0);
uint32_t green100R = rightIndicator.Color(0, 255, 0);
uint32_t yellow100R = rightIndicator.Color(255, 255, 0);
uint32_t red100R = rightIndicator.Color(255, 0, 0);
uint32_t allOffR = rightIndicator.Color(0, 0, 0);
uint32_t blue100 = headlight.Color(0, 0, 255);
uint32_t allOffH = headlight.Color(0, 0, 0);

int workingData[5];
int storageData[10];
int outputData[6];//fwd,bck,lft,rgt,ckw,ccw

int blinkState = 0;
unsigned long previousMillis = 0;
long blinkInterval = 500;


void setup() {
headlight.begin();
headlight.show();
leftIndicator.begin();
leftIndicator.show();
rightIndicator.begin();
rightIndicator.show();

}

void loop() {
headlightSet(0,19,1,blue100);

int numDet = pixy.getBlocks();
for(int j=0; j<=numDet; j++){
  getPixyData(j);
}
}

void headlightSet(int startled, int endled, int countby, uint32_t color){
  for (int i=startled; i<endled; i+countby){
    headlight.setPixelColor(i, color);
    headlight.show();
  }
}

void leftLightSet(int startled, int endled, int countby, uint32_t color){
  for (int i=startled; i<endled; i+countby){
    leftIndicator.setPixelColor(i, color);
    leftIndicator.show();
  }
}

void rightLightSet(int startled, int endled, int countby, uint32_t color){
  for (int i=startled; i<endled; i+countby){
    rightIndicator.setPixelColor(i, color);
    rightIndicator.show();
  }
}

void getPixyData(int pixyHit){
  workingData[0] = pixy.blocks[pixyHit].signature;
  workingData[1] = pixy.blocks[pixyHit].x;
  workingData[2] = pixy.blocks[pixyHit].y;
  workingData[3] = pixy.blocks[pixyHit].width;
  workingData[4] = pixy.blocks[pixyHit].height;

  if(pixyHit = 0){
    storageData[0] = workingData[0];
    storageData[1] = workingData[1];
    storageData[2] = workingData[2];
    storageData[3] = workingData[3];
    storageData[4] = workingData[4];
  }else{
    storageData[5] = workingData[0];
    storageData[6] = workingData[1];
    storageData[7] = workingData[2];
    storageData[8] = workingData[3];
    storageData[9] = workingData[4];
  }
  
}

int calculateDistance(){//call after storing both sets
  int midWidth = 80; //arbitrary constants, tune to set distances
  int midHeight = 50;
  
  int hitAX = storageData[1];
  int hitAY = storageData[2];
  int hitBX = storageData[6];
  int hitBY = storageData[7];

  if((hitAX < midwidth && hitAY < midHeight) && (hitBX < midwidth && hitBY < midHeight)){
    rightLightset(0,19,1,red100R);
    leftLightSet(0,19,1,red100L);
    return 0;
  }else if((hitAX > (2*midWidth) && hitAY > (2*midWidth)) && (hitBX > (2*midWidth) && hitBY > (2*midWidth))){
    rightLightset(0,19,1,green100R);
    leftLightSet(0,19,1,green100L);
    return 2;
  }else if(((hitAX < (2*midWidth) && hitAX > midWidth) && (hitAY < (2*midHeight) && hitAY > midHeight)) && ((hitBX < (2*midWidth) && hitBX > midWidth) && (hitBY < (2*midHeight) && hitBY > midHeight))){
    rightLightset(0,19,1,yellow100R);
    leftLightSet(0,19,1,yellow100L);
    return 1;
  }else{
    
  }
}

void blinkStrip(int startled, int endled, int countby, uint32_t color, int segment, long blinkTime){
  //segment numbers: 0--headlight, 1--left, 2--right
//---------------
  blinkInterval = blinkTime;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
  if (blinkState == 0) {
      blinkState = 1;
    } else {
      blinkState = 0;
    }
  
//-------------
  switch segment{
    case 0:
      if(blinkState = 1){
        headlightSet(startled, endled, countby, color);
      }else{
        headlightSet(startled, endled, countby, allOffH);
      }
      break;
    case 1:
      if(blinkState = 1){
        leftLightSet(startled, endled, countby, color);
      }else{
        leftLightSet(startled, endled, countby, allOffH);
      }
      break;
    case 2:
    if(blinkState = 1){
        rightLightSet(startled, endled, countby, color);
      }else{
        rightLightSet(startled, endled, countby, allOffH);
      }
      break;
  }
  }
}//end of blinkStrip
