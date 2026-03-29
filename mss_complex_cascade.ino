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
  bool error = true;
  int state, mast;
  Serial.begin(9600);
  //set i2c port expander DDR and pullups

  //set LCD i2c port expander

  //ckeck if setup failed
  while (error) {
    heartbeat(200);
  }
  for (state = 3; state >= 0; state--) {
    for (mast = 0; mast < 4; mast++) {
      SetMast(points_upper, state);
    }
    delay(750);
  }
}

void loop() {
  long currentTime = 0;
  static long activeTime = 0;
  int upper, lower, main_lamp, diverge_lamp;

  heartbeat(1000);
  currentTime = millis();
  //Read inputs i2C

  //Read inputs optical

  //Process inputs

  //Set masts
  SetMast(points_upper, upper);
  SetMast(points_upper, lower);
  SetMast(main, main_lamp);
  SetMast(diverge, diverge_lamp);
}
