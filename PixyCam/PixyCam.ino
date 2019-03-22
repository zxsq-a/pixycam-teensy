#include <PixySPI_SS.h>
#include <PixyI2C.h>
#include <Pixy.h>
#include <TPixy.h>
#include <PixyUART.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
struct binPair {
  int a,b;
};
typedef struct binPair BinPair;
Pixy pixy;
#define led_data 19
#define kEqn 80k+160 //simplified from (((k+1)*160)-(k*80))
Adafruit_NeoPixel ledstrip = Adafruit_NeoPixel(19, led_data, NEO_GRB + NEO_KHZ800);
void setup() {
  // put your setup code here, to run once:
ledstrip.begin();
ledstrip.show();
}

void loop() {
  // put your main code here, to run repeatedly:
//unit32_t green50 = ledstrip.Color(0, 128, 0);
uint32_t green100 = ledstrip.Color(0, 255, 0);
uint32_t green50 = ledstrip.Color(0, 128, 0);
uint32_t blue50 = ledstrip.Color(0, 0, 128);
uint32_t yellow100 = ledstrip.Color(255, 255, 0);
uint32_t red100 = ledstrip.Color(255, 0, 0);


for (int i=0; i<19; i++){
  ledstrip.setPixelColor(i, 255,255,0);
  ledstrip.show();
}
/* when ready to drop, green; when near but not there yellow; when far, red*/
int numDet = pixy.getBlocks(); //# of hits detected
int dataRecieved [3] [5];
    for (int j=0; j <= numDet; j++){
      dataRecieved[j] [0] = pixy.blocks[j].signature;
      dataRecieved[j] [1] = pixy.blocks[j].x;
      dataRecieved[j] [2] = pixy.blocks[j].y;
      dataRecieved[j] [3] = pixy.blocks[j].width;
      dataRecieved[j] [4] = pixy.blocks[j].height;
    }
 
}

BinPair CalcDist(int widthIn, int heightIn){
  int midWidth = 80;
  int midHeight = 50; //arbitrary values for a middle (YELLOW) distance: CALIBRATE THESE
BinPair dataOut;
   if (widthIn < midWidth && heightIn < midHeight){
    //red lights, far dist data etc TUNE ME
    for (int i=0; i<19; i++){
      ledstrip.setPixelColor(i, red100);
      ledstrip.show();//change later to control signal leds
    }
    dataOut.a = 1;
    dataOut.b = 0;
   }else if(widthIn > (2*midWidth) && heightIn > (2*midHeight)){
    //GreenLights etc TUNE ME
    for (int i=0; i<19; i++){
      ledstrip.setPixelColor(i, green100);
      ledstrip.show();//change later to control signal leds
    }
    dataOut.a = 0;
    dataOut.b = 0;
   } else if ((widthIn > midwith && widthIn < (2*midWidth)) && (heightIn > midHeight && heightIn < (2*midHeight)){
    //YellowLights TUNE ME
    for (int i=0; i<19; i++){
      ledstrip.setPixelColor(i, yellow100);
      ledstrip.show();//change later to control signal leds
    }
    dataOut.a = 1;
    dataOut.b = 0;
   }else{
    //SAFETY FAIL CASE
    //YELLOW+RED
    //fwd/bck data should be 0
    for (int i=0; i<19; i+2){
      ledstrip.setPixelColor(i, red100);
      ledstrip.show();//change later to control signal leds
    }
    for (int i=1; i<19; i+2){
      ledstrip.setPixelColor(i, yellow100);
      ledstrip.show();//change later to control signal leds
    }
    dataOut.a = 0;
    dataOut.b = 0;
   }
   return dataOut;
}
int CalcLR(int numObj, int xcoord, int ycoord){
  //int goalX = 80; values incorporated in #define kEqn
  //int goalY = 150; ignoring y coord for now
  
for (int k = 0; k < 2; k++){
  if((pixy.blocks[k].x > kEqn) && (pixy.blocks[k+1].x > kEqn)){
    //move Left if both blocks are too high of x value          
  }else if ((pixy.blocks[k].x < kEqn) && (pixy.blocks[k+1].x < kEqn)){
    //move right #####ADD THRESHOLD DEADZONE####
  }else{
    //do nothing/failsafe
  }
}
  
}
void dataConstruct(int fwd, int bck, int lft, int rgt, int ckw, int ccw){
  //add parity bit(s)
  //  bit numbers|-0-|-1-|-2-|-3-|-4-|-5-|-6-|-7-|
  //data contains|pbA|fwd|bck|lft|rgt|ckw|ccw|pbB|
  //expected data|-1-|-1-|-0-|-1-|-0-|-1-|-1-|-1-|
  //pbA: fwd|bck&lft|rgt&(~(ckw^ccw)) (err if not = 1 or if ckw and ccw do not match (ditch this if angles get done.)
  //pbB: PBa^fwd^bck^lft^rgt^ckw^ccw should be 1 always (^ is bitwise xor)
  //int aParit = 0b00000000;
  //int fwdBit = 0b00000000;
  //int bckBit = 0b00000000;
  //int lftBit = 0b00000000;
  //int rgtBit = 0b00000000;
  //int ckwBit = 0b00000000;
  //int ccwBit = 0b00000000;
  //int bParit = 0b00000000;

  // binary int = (conditional) ? trueResult : falseResult
  int fwdBit = (fwd == 1) ? 0b01000000 : 0b00000000;
  int bckBit = (bck == 1) ? 0b00100000 : 0b00000000;
  int lftBit = (lft == 1) ? 0b00010000 : 0b00000000;
  int rgtBit = (rgt == 1) ? 0b00001000 : 0b00000000;
  int ckwBit = (ckw == 1) ? 0b00000100 : 0b00000000;
  int ccwBit = (ccw == 1) ? 0b00000010 : 0b00000000;
  int aParit = (((fwdBit | bckBit) & (lftBit | rgtBit)) & (~(ckwBit ^ ccwBit))) ? 0b10000000 : 0b00000000;
  int bParit = (aParit ^ fwdBit ^ bckBit ^ lftBit ^ rgtBit ^ ckwBit ^ ccwBit) ? 0b00000001 : 0b00000000;

  int dataByte = (aParit | fwdBit | bckBit | lftBit | rgtBit | ckwBit | ccwBit | bParit);
}
//send data via i2c

//...
