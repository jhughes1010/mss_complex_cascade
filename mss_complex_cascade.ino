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
       diverge };

enum { clear,
       adv,
       apr,
       occ };




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
}

void loop() {
  int GPIO;
  bool diverge = false;
  long currentTime = 0;
  static long activeTime = 0;
  int upper_lamp, lower_lamp, main_lamp, diverge_lamp;

  heartbeat(2000);
  currentTime = millis();
  //Read inputs i2C
  GPIO = mss_25.readGPIOAB();
  diverge = !(GPIO & 0x8000);

  //Read inputs optical
  //optical = readOptical(A0, currentTime);

  //Process inputs
  PrintOnChange(diverge);
  //Serial.print("Diverge: ");
  //Serial.println(diverge);
delay(1000);
  //force override lighting based on diverge
  if (diverge) {
    upper_lamp = occ;
    main_lamp = occ;
  } else {
    lower_lamp = occ;
    diverge_lamp = occ;
  }
  //Set masts
  SetSignal(points_upper, upper_lamp);
  SetSignal(points_upper, lower_lamp);
  SetSignal(main, main_lamp);
  SetSignal(diverge, diverge_lamp);
}
