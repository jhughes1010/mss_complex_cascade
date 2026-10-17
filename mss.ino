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
  const __FlashStringHelper *section;
  const __FlashStringHelper *function;
  uint8_t pin;
  bool state;
};

struct MSSOutputDef {
  const __FlashStringHelper *section;
  const __FlashStringHelper *function;
  uint8_t pin;
  bool state;
};

// U5 / 0x27. All MSS status inputs are active LOW.
// U3/U6 CD4050 outputs land on U5 as A0..A7 and B0.
MSSInputDef mssInputs[] = {
  { F("POINTS"),  F("OCC"),     0, false },
  { F("POINTS"),  F("APR-IN"),  1, false },
  { F("POINTS"),  F("ADV-IN"),  2, false },
  { F("MAIN"),    F("OCC"),     3, false },
  { F("MAIN"),    F("APR-IN"),  4, false },
  { F("MAIN"),    F("ADV-IN"),  5, false },
  { F("DIVERGE"), F("OCC"),     6, false },
  { F("DIVERGE"), F("APR-IN"),  7, false },
  { F("DIVERGE"), F("ADV-IN"),  8, false }
};

const uint8_t MSS_INPUT_COUNT = sizeof(mssInputs) / sizeof(mssInputs[0]);

// U4 / 0x25 assertion drivers.
// A logical true means ASSERTED at the MSS interface (status line pulled LOW).
MSSOutputDef mssOutputs[] = {
  { F("POINTS"),  F("OCC"),      0, false },
  { F("POINTS"),  F("APR-OUT"),  1, false },
  { F("POINTS"),  F("ADV-OUT"),  2, false },
  { F("POINTS"),  F("DIVERGE"),  3, false },
  { F("MAIN"),    F("OCC"),      4, false },
  { F("MAIN"),    F("APR-OUT"),  5, false },
  { F("MAIN"),    F("ADV-OUT"),  6, false },
  { F("MAIN"),    F("DIVERGE"),  7, false },
  { F("DIVERGE"), F("OCC"),      8, false },
  { F("DIVERGE"), F("APR-OUT"),  9, false },
  { F("DIVERGE"), F("ADV-OUT"), 10, false },
  { F("DIVERGE"), F("DIVERGE"), 11, false }
};

const uint8_t MSS_OUTPUT_COUNT = sizeof(mssOutputs) / sizeof(mssOutputs[0]);

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
  for (uint8_t i = 0; i < MSS_INPUT_COUNT; i++) {
    mssInputs[i].state = ((raw & (1U << mssInputs[i].pin)) == 0);
  }

  for (uint8_t i = 0; i < MSS_OUTPUT_COUNT; i++) {
    mssOutputs[i].state = false;
  }
}

void PrintMSSChange(const __FlashStringHelper *direction,
                    const __FlashStringHelper *section,
                    const __FlashStringHelper *function,
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
