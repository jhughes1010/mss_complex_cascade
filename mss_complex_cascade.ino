//MSS complex cascade
//James Hughes
//03-27-2026

#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <Adafruit_RGBLCDShield.h>

Adafruit_MCP23X17 mast;
Adafruit_MCP23X17 mss_27;
Adafruit_MCP23X17 mss_25;

#define debug 0

#define BD_POINTS 2
#define BD_MAIN 3
#define BD_DIVERGE 4


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
  pinMode(BD_POINTS, INPUT_PULLUP);
  pinMode(BD_MAIN, INPUT_PULLUP);
  pinMode(BD_DIVERGE, INPUT_PULLUP);

  //delay(2000);
  //Serial.println("Intentional 5s delay complete");

  //set i2c port expander DDR and pullups
  error = InitMastLighting();
  error |= InitMSS27();
  error |= InitMSS25();

  //set LCD i2c port expander
  LCDInit();
  delay(2000);

  //ckeck if setup failed
  while (error) {
    heartbeat(250);
  }
  LightShow();
  SetSignal(points_lower, clear);
  SetSignal(dvg, clear);

  darkAll();
}

void loop() {
  int GPIO25, GPIO27;
  bool diverge = false, optical;
  bool bd_points, bd_main, bd_diverge;
  static bool dark = false, opticalActive = false;
  long currentTime = 0;
  static long activeTime = 0, activeTimeOptical = 0;
  int upper_lamp, lower_lamp, main_lamp, diverge_lamp;

  currentTime = millis();
  heartbeat(2000);


  //Read Block detect local
  bd_points = !digitalRead(BD_POINTS);
  bd_main = !digitalRead(BD_MAIN);
  bd_diverge = !digitalRead(BD_DIVERGE);
  //Read inputs i2C
  GPIO25 = mss_25.readGPIOAB();
  diverge = !(GPIO25 & 0x4000);
  GPIO27 = 0xffff - mss_27.readGPIOAB();
  //Serial.println(GPIO27, HEX);

  //Read inputs optical

  //update active time if needed
  if (GPIO27 | bd_points | bd_main | bd_diverge) {
    activeTime = currentTime;
    dark = false;
  }
  if (timeWindowOut(currentTime,activeTime,180000)){
    //dark=true;
    //Serial.println("dark");

  }

  optical = readOptical(0x50, currentTime, activeTime);
  PrintOnChangeBDLocal(bd_points, bd_main, bd_diverge);
  PrintOnChangeDiverge(diverge);

  //Process inputs
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
  if (!dark) {
    SetSignal(points_upper, upper_lamp);
    SetSignal(points_lower, lower_lamp);
    SetSignal(main, main_lamp);
    SetSignal(dvg, diverge_lamp);
  } else {
    darkAll();
  }

  //testing 25 drive
  /*  int gpioreg = 0x0001;
  int x;

  for (x = 0; x < 12; x++) {
    delay(500);
    mss_25.writeGPIOAB(gpioreg);
    gpioreg = gpioreg << 1;
  }*/
}
