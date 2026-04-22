//MSS complex cascade
//James Hughes
//03-27-2026

#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <Adafruit_RGBLCDShield.h>

Adafruit_MCP23X17 mast;
Adafruit_MCP23X17 mss_27;
Adafruit_MCP23X17 mss_25;


enum { points_upper,
       points_lower,
       main,
       dvg };

enum { clear,
       adv,
       apr,
       occ };

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();



void setup() {
  bool error = false;
  int state, head;
  Serial.begin(9600);
  Serial.println("MSS Complex Cascade");
  pinMode(LED_BUILTIN, OUTPUT);
  delay(2000);
  Serial.println("Intentional 5s delay complete");

  //set i2c port expander DDR and pullups
  error = InitMastLighting();
  //error |= InitMSS27();
  error |= InitMSS25();

  //set LCD i2c port expander

  //ckeck if setup failed
  while (error) {
    heartbeat(250);
  }
  LightShow();
  SetSignal(points_lower, clear);
  SetSignal(dvg, clear);

  darkAll();
  //LCD
  LCDInit();
  delay(2000);
}

void loop() {
  int GPIO25, GPIO27;
  bool diverge = false, optical;
  long currentTime = 0;
  static long activeTime = 0;
  int upper_lamp, lower_lamp, main_lamp, diverge_lamp;

  /*while(1){
    Serial.println(".");
  }*/

  currentTime = millis();
  //Read inputs i2C
  GPIO25 = mss_25.readGPIOAB();
  diverge = !(GPIO25 & 0x8000);
  //GPIO27 = 0xffff - mss_27.readGPIOAB();

  //Read inputs optical
  //activeTime = currentTime;
  optical = readOptical(0x50, currentTime, activeTime);

  //Process inputs
  //diverge = heartbeat(2000);
  PrintOnChange(diverge);
  LCDStatus(diverge);
  LCDSigStatus();

  //force override lighting based on diverge
  if (diverge) {
    upper_lamp = occ;
    lower_lamp = clear;
    main_lamp = occ;
    diverge_lamp = clear;

  } else {
    upper_lamp = clear;
    lower_lamp = occ;
    main_lamp = clear;
    diverge_lamp = occ;
  }
  //Set masts
  SetSignal(points_upper, upper_lamp);
  SetSignal(points_lower, lower_lamp);
  SetSignal(main, main_lamp);
  SetSignal(dvg, diverge_lamp);
}
