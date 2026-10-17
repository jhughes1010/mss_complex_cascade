// MSS interface
// Phase 1: MSS status inputs/outputs only.
// Local block detectors and optical detectors are intentionally not used yet.
//
// MSS convention at the RJ45/status-line interface:
//   LOW  = ASSERTED
//   HIGH = CLEARED
//
// The MCP23017 output drives turn on NPN open-collector stages. Therefore a
// HIGH at U4 asserts the corresponding MSS status line LOW.

struct MSSInputDef {
  const char *section;
  const char *function;
  uint8_t pin;
  bool state;
};

struct MSSOutputDef {
  const char *section;
  const char *function;
  uint8_t pin;
  bool state;
};

// U5 / 0x27. All MSS status inputs are active LOW.
// U3/U6 CD4050 outputs land on U5 as A0..A7 and B0.
MSSInputDef mssInputs[] = {
  { "POINTS",  "OCC",     0, false },
  { "POINTS",  "APR-IN",  1, false },
  { "POINTS",  "ADV-IN",  2, false },
  { "MAIN",    "OCC",     3, false },
  { "MAIN",    "APR-IN",  4, false },
  { "MAIN",    "ADV-IN",  5, false },
  { "DIVERGE", "OCC",     6, false },
  { "DIVERGE", "APR-IN",  7, false },
  { "DIVERGE", "ADV-IN",  8, false }
};

const uint8_t MSS_INPUT_COUNT = sizeof(mssInputs) / sizeof(mssInputs[0]);

// U4 / 0x25 assertion drivers.
// A logical true means ASSERTED at the MSS interface (status line pulled LOW).
MSSOutputDef mssOutputs[] = {
  { "POINTS",  "OCC",      0, false },
  { "POINTS",  "APR-OUT",  1, false },
  { "POINTS",  "ADV-OUT",  2, false },
  { "POINTS",  "DIVERGE",  3, false },
  { "MAIN",    "OCC",      4, false },
  { "MAIN",    "APR-OUT",  5, false },
  { "MAIN",    "ADV-OUT",  6, false },
  { "MAIN",    "DIVERGE",  7, false },
  { "DIVERGE", "OCC",      8, false },
  { "DIVERGE", "APR-OUT",  9, false },
  { "DIVERGE", "ADV-OUT", 10, false },
  { "DIVERGE", "DIVERGE", 11, false }
};

const uint8_t MSS_OUTPUT_COUNT = sizeof(mssOutputs) / sizeof(mssOutputs[0]);

uint16_t lastMSS27Raw = 0xFFFF;

void PrintRawGPIO(const __FlashStringHelper *label, uint16_t value) {
  Serial.print(label);
  Serial.print(F("0x"));
  if (value < 0x1000) Serial.print('0');
  if (value < 0x0100) Serial.print('0');
  if (value < 0x0010) Serial.print('0');
  Serial.println(value, HEX);
}

bool InitMSS27(void) {
  bool error = false;

  Serial.println(F("Init MSS inputs"));
  if (!mss_27.begin_I2C(0x27)) {
    Serial.println(F("Port Expander at address 0x27 is not responding"));
    error = true;
  } else {
    for (uint8_t pin = 0; pin < 16; pin++) {
      mss_27.pinMode(pin, INPUT_PULLUP);
    }
  }
  return error;
}

bool InitMSS25(void) {
  bool error = false;

  Serial.println(F("Init MSS outputs"));
  if (!mss_25.begin_I2C(0x25)) {
    Serial.println(F("Port Expander at address 0x25 is not responding"));
    error = true;
  } else {
    for (uint8_t pin = 0; pin < 12; pin++) {
      mss_25.pinMode(pin, OUTPUT);
    }
    for (uint8_t pin = 12; pin < 16; pin++) {
      mss_25.pinMode(pin, INPUT_PULLUP);
    }

    // Driver LOW = transistor off = MSS status line released/cleared.
    mss_25.writeGPIOAB(0x0000);
  }
  return error;
}

void InitMSSState(void) {
  // Capture initial states silently. Startup is not an MSS transition.
  uint16_t raw = mss_27.readGPIOAB();
  lastMSS27Raw = raw;
  PrintRawGPIO(F("MSS IN   RAW 0x27: "), raw);

  for (uint8_t i = 0; i < MSS_INPUT_COUNT; i++) {
    mssInputs[i].state = ((raw & (1U << mssInputs[i].pin)) == 0);
  }

  for (uint8_t i = 0; i < MSS_OUTPUT_COUNT; i++) {
    mssOutputs[i].state = false;
  }
}

void PrintMSSChange(const __FlashStringHelper *direction,
                    const char *section,
                    const char *function,
                    bool asserted) {
  Serial.print(F("MSS "));
  Serial.print(direction);
  Serial.print(F("  "));
  Serial.print(section);
  Serial.print(F("  "));
  Serial.print(function);
  Serial.print(F("  "));
  Serial.println(asserted ? F("ASSERTED") : F("CLEARED"));
}

void UpdateMSSInputs(void) {
  uint16_t raw = mss_27.readGPIOAB();

  // Low-level troubleshooting: print the complete MCP23017 GPIO word only
  // when any input changes. This is the uninverted electrical value.
  if (raw != lastMSS27Raw) {
    PrintRawGPIO(F("MSS IN   RAW 0x27: "), raw);
    lastMSS27Raw = raw;
  }

  for (uint8_t i = 0; i < MSS_INPUT_COUNT; i++) {
    // Every MSS status input is active LOW.
    bool newState = ((raw & (1U << mssInputs[i].pin)) == 0);

    if (newState != mssInputs[i].state) {
      mssInputs[i].state = newState;
      PrintMSSChange(F("IN "), mssInputs[i].section,
                     mssInputs[i].function, newState);
    }
  }
}

bool GetMSSInput(uint8_t index) {
  if (index >= MSS_INPUT_COUNT) return false;
  return mssInputs[index].state;
}

void SetMSSOutput(uint8_t index, bool asserted) {
  if (index >= MSS_OUTPUT_COUNT) return;
  if (mssOutputs[index].state == asserted) return;

  mssOutputs[index].state = asserted;

  // HIGH turns on the NPN driver, pulling the external MSS line LOW.
  mss_25.digitalWrite(mssOutputs[index].pin, asserted ? HIGH : LOW);

  PrintMSSChange(F("OUT"), mssOutputs[index].section,
                 mssOutputs[index].function, asserted);
}
