//--------------------------
#include <PixySPI_SS.h>
#include <PixyI2C.h>
#include <Pixy.h>
#include <TPixy.h>
#include <PixyUART.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
//-----------------------
struct dataBlock{
  int left,right,fwd,back,ckw,ccw;
}
typedef struct dataBlock DataOut;

Pixy pixy;

#define headlight_data 19
#define left_data 20 //changeme to a good pin
#define right_data 21 //changeme to a good pin
////////////////////////////////////////////////#leds, led data pin
Adafruit_NeoPixel headlight = Adafruit_NeoPixel(19, headlight_data, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel leftIndicator = Adafruit_NeoPixel(19, left_data, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel rightIndicator = Adafruit_NeoPixel(19, right_data, NEO_GRB + NEO_KHZ800);
//----
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
//====
int workingData[5];
int storageData[10];
int outputData[6];//fwd,fast,lft,rgt,ckw,ccw------fwd+fast=far away

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
Wire.begin(10);
Wire.onRequest(i2cCommand);
}

void loop() {
headlightSet(0,19,1,blue100);

int numDet = pixy.getBlocks();
for(int j=0; j<=numDet; j++){
  getPixyData(j);
}
int distResult = calculateDistance();
switch distResult{
  case 0:
    outputData[0] = 1;
    outputData[1] = 1;
    break;
  case 1:
    outputData[0] = 1;
    outputData[1] = 0;
    break;
  case 2:
    outputData[0] = 0;
    outputData[1] = 0;
    break;
  case 3:
    outputData[0] = 0;
    outputData[1] = 0;
    break;
}
int LRResult = calculatePosition();
switch LRResult{
  case 0:
    outputData[3] = 1;
    outputData[4] = 0;
    break;
  case 1:
    outputData[3] = 0;
    outputData[4] = 1;
    break;
  case 2:
    outputData[3] = 0;
    outputData[4] = 0;
    break;
}
ckwCcwDummy();

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

  int hitAWidth = storageData[3];
  int hitAHeight = storageData[4];
  int hitBWidth = storageData[8];
  int hitBHeight = storageData[9];

  if((hitAWidth < midwidth && hitAHeight < midHeight) && (hitBWidth < midwidth && hitBHeight < midHeight)){
    rightLightset(0,19,1,red100R);
    leftLightSet(0,19,1,red100L);
    return 0;
  }else if((hitAWidth > (2*midWidth) && hitAHeight > (2*midWidth)) && (hitBWidth > (2*midWidth) && hitBHeight > (2*midWidth))){
    rightLightset(0,19,1,green100R);
    leftLightSet(0,19,1,green100L);
    return 2;
  }else if(((hitAWidth < (2*midWidth) && hitAWidth > midWidth) && (hitAHeight < (2*midHeight) && hitAHeight > midHeight)) && ((hitBWidth < (2*midWidth) && hitBWidth > midWidth) && (hitBHeight < (2*midHeight) && hitBHeight > midHeight))){
    rightLightset(0,19,1,yellow100R);
    leftLightSet(0,19,1,yellow100L);
    return 1;
  }else{
    blinkStrip(0,19,2,red100R,2,250);
    blinkStrip(1,19,2,yellow100R,2,250);
    blinkStrip(0,19,2,red100L,1,250);
    blinkStrip(1,19,2,yellow100L,1,250);
    return 3;
  }
}

int calculatePosition(){
  int hitAX = storageData[1];
  //int hitAY = storageData[2];
  int hitBX = storageData[6];
  //int hitBY = storageData[7];
  int deadzone = 20;//slop area, in pixels, that won't trigger the leds. TUNE ME
  uint32_t prevColorL = leftIndicator.getPixelColor(5);
  uint32_t prevColorR = rightIndicator.getPixelColor(5);
//80k+160
  if ((hitAX > 80-deadzone) || (hitBX > 240-deadzone)){//or'd to hopefully account for just 1 visible tape
    //move left
    blinkStrip(0,19,1,prevColorL,1,500);
    return 0;
  }else if((hitAX < 80+deadzone) || (hitBX < 240+deadzone)){
    //move right
    blinkStrip(0,19,1,prevColorR,2,500);
    return 1;
  }else{
  //no l/r
  return 2;
  }
}
void ckwCcwDummy(){
  outputData[4] = 0;
  outputData[5] = 0;
}
void blinkStrip(int startled, int endled, int countby, uint32_t color, int segment, long blinkTime){
  //segment numbers: 0--headlight, 1--left, 2--right
//---------------
  blinkInterval = blinkTime;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= blinkInterval) {
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
void i2cCommand(){
 Wire.send(outputData,6);
}
