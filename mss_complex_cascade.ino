//MSS complex cascade
//James Hughes
//03-27-2026

#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <Adafruit_RGBLCDShield.h>


enum { points_upper,
       points_lower,
       main,
       diverge };

enum { clear,
       adv,
       apr,
       occ };




void setup() {
  Serial.begin(9600);
  //set i2c port expander DDR and pullups

  //set LCD i2c port expander
}

void loop() {
  long currentTime = 0;
  static long activeTime = 0;

  currentTime = millis();
  //Read inputs i2C

  //Read inputs optical

  //Process inputs

  //Set masts
  SetMast(points_upper, 0);
  SetMast(points_upper, 0);
  SetMast(main, 0);
  SetMast(diverge, 0);
}
