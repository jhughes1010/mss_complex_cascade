// MSS Complex Cascade - MSS priority/aspect control
// James Hughes - 2026-10-18
#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <Adafruit_RGBLCDShield.h>

Adafruit_MCP23X17 mast, mss_27, mss_25;
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
#define debug 0
#define BD_POINTS 2
#define BD_MAIN 3
#define BD_DIVERGE 4

enum { points_upper, points_lower, main, dvg };
enum { clear, adv, apr, occ };

// U5 / 0x27. All MSS inputs active LOW.
enum MSS27Bit {
  P_OCC_IN=0, P_APR_IN=1, P_ADV_IN=2,
  M_OCC_IN=3, M_APR_IN=4, M_ADV_IN=5,
  D_OCC_IN=6, D_APR_IN=7, D_ADV_IN=8
};
#define DIVERGE_INPUT_BIT 14   // U4 / 0x25 GPB6, active LOW

void setup() {
  bool error=false;
  Serial.begin(9600);
  Serial.println(F("MSS Complex Cascade"));
  Serial.println(F("MSS priority/aspect control - 2026-10-18"));
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BD_POINTS, INPUT_PULLUP);
  pinMode(BD_MAIN, INPUT_PULLUP);
  pinMode(BD_DIVERGE, INPUT_PULLUP);
  error=InitMastLighting();
  error|=InitMSS27();
  error|=InitMSS25();
  LCDInit();
  while(error) heartbeat(250);
}

void loop() {
  heartbeat(2000);
  uint16_t gpio27=mss_27.readGPIOAB();
  uint16_t gpio25=mss_25.readGPIOAB();
  bool diverge=MSSActive(gpio25,DIVERGE_INPUT_BIT);

  // Independently propagate MSS restrictions through the selected route.
  UpdateCascadeOutputs(gpio27, diverge);

  int pointsUpperStatus;
  int pointsLowerStatus;
  int mainStatus;
  int divergeStatus;

  // MAIN alignment logic currently defined:
  //   P-UPPER protects MAIN, so it reflects MAIN-side MSS status.
  //   MAIN mast protects POINTS, so it reflects POINTS-side MSS status.
  //   P-LOWER and DIVERGE mast are RED.
  // Diverging alignment is intentionally fail-safe RED until defined.
  if(diverge) {
    pointsUpperStatus=occ;
    pointsLowerStatus=occ;
    mainStatus=occ;
    divergeStatus=occ;
  } else {
    pointsUpperStatus=DecodeMSSAspect(gpio27,M_OCC_IN,M_APR_IN,M_ADV_IN);
    pointsLowerStatus=occ;
    mainStatus=DecodeMSSAspect(gpio27,P_OCC_IN,P_APR_IN,P_ADV_IN);
    divergeStatus=occ;
  }

  SetSignal(points_upper,pointsUpperStatus);
  SetSignal(points_lower,pointsLowerStatus);
  SetSignal(main,mainStatus);
  SetSignal(dvg,divergeStatus);

  // Read U4 again after cascade outputs have been written so the screen
  // reports the actual current output state.
  gpio25=mss_25.readGPIOAB();
  PrintDebugScreen(gpio27,gpio25,diverge,
                   pointsUpperStatus,pointsLowerStatus,
                   mainStatus,divergeStatus);
}
