// MSS complex cascade
// James Hughes
// Phase 1 MSS I/O recode - 2026-10-17

#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <Adafruit_RGBLCDShield.h>

Adafruit_MCP23X17 mast;
Adafruit_MCP23X17 mss_27;
Adafruit_MCP23X17 mss_25;

// Hardware is present on the PCB, but these detector systems are not
// physically installed and are intentionally excluded from Phase 1 logic.
#define ENABLE_LOCAL_BLOCK_DETECTORS 0
#define ENABLE_OPTICAL_DETECTORS     0

#define BD_POINTS  2
#define BD_MAIN    3
#define BD_DIVERGE 4

enum { points_upper,
       points_lower,
       main,
       dvg };

enum { clear,
       adv,
       apr,
       occ };

// Indices into the Phase 1 MSS input table in mss.ino.
enum MSSInputIndex {
  MSS_P_OCC_IN,
  MSS_P_APR_IN,
  MSS_P_ADV_IN,
  MSS_M_OCC_IN,
  MSS_M_APR_IN,
  MSS_M_ADV_IN,
  MSS_D_OCC_IN,
  MSS_D_APR_IN,
  MSS_D_ADV_IN
};

// Indices into the MSS output table in mss.ino.
enum MSSOutputIndex {
  MSS_P_OCC,
  MSS_P_APR_OUT,
  MSS_P_ADV_OUT,
  MSS_P_DIVERGE,
  MSS_M_OCC,
  MSS_M_APR_OUT,
  MSS_M_ADV_OUT,
  MSS_M_DIVERGE,
  MSS_D_OCC,
  MSS_D_APR_OUT,
  MSS_D_ADV_OUT,
  MSS_D_DIVERGE
};

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

void setup() {
  bool error = false;

  Serial.begin(9600);
  Serial.println(F("MSS Complex Cascade Controller"));
  Serial.println(F("Phase 1 MSS I/O - 2026-10-17"));

  pinMode(LED_BUILTIN, OUTPUT);

#if ENABLE_LOCAL_BLOCK_DETECTORS
  pinMode(BD_POINTS, INPUT_PULLUP);
  pinMode(BD_MAIN, INPUT_PULLUP);
  pinMode(BD_DIVERGE, INPUT_PULLUP);
#endif

  error = InitMastLighting();
  error |= InitMSS27();
  error |= InitMSS25();

  LCDInit();

  while (error) {
    heartbeat(250);
  }

  // Establish the current MSS state without generating fake startup changes.
  InitMSSState();

  Serial.println(F("MSS I/O initialized"));
  Serial.println(F("-------------------"));

  // Keep the existing mast hardware initialization for now. Cascade behavior
  // will be connected to the MSS state machine in the next phase.
  SetSignal(points_lower, clear);
  SetSignal(dvg, clear);
  darkAll();
}

void loop() {
  heartbeat(2000);

  // Phase 1: watch the nine external MSS status inputs (OCC, APR-IN, ADV-IN
  // for Points/Main/Diverge). Serial output occurs only on a state change.
  UpdateMSSInputs();

  // No local block detector or optical detector processing yet.
  // No cascade output logic is applied yet. All MSS outputs remain clear
  // unless future state-machine code calls SetMSSOutput().

  LCDSigStatus();
}
