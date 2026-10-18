bool InitMSS27(void) {
  bool error = false;
  int pin;

  Serial.println("Init MSS RX");
  if (!mss_27.begin_I2C(0x27)) {
    Serial.println("Port Expander at address 0x27 is not responding");
    error = true;
  } else {
    //setPIN direction
    for (pin = 0; pin < 16; pin++) {
      mss_27.pinMode(pin, INPUT_PULLUP);
      if (debug) {
        Serial.print("Input pin:");
        Serial.println(pin);
      }
    }
  }
  return error;
}



bool InitMSS25(void) {
  bool error = false;
  int pin;

  Serial.println("Init MSS drive");
  if (!mss_25.begin_I2C(0x25)) {
    Serial.println("Port Expander at address 0x25 is not responding");
    error = true;
  } else {
    //setPIN direction
    //This might be done with one register write??
    for (pin = 0; pin < 16; pin++) {
      if (pin < 12) {
        mss_25.pinMode(pin, OUTPUT);
        if (debug) {
          Serial.print("Output pin:");
          Serial.println(pin);
        }
      } else {
        mss_25.pinMode(pin, INPUT_PULLUP);
        if (debug) {
          Serial.print("Input pin:");
          Serial.println(pin);
        }
      }
    }
  }
  mss_25.writeGPIOAB(0x0000);
  return error;
}
// ---------- MSS decode / compact status screen ----------
bool MSSActive(uint16_t gpio, uint8_t bit) {
  return (gpio & (1U << bit)) == 0;
}

int DecodeMSSAspect(uint16_t gpio, uint8_t occBit, uint8_t aprBit, uint8_t advBit) {
  // Exclusive, most-restrictive indication wins:
  // OCC > APR > ADV > CLEAR
  if (MSSActive(gpio, occBit)) return occ;
  else if (MSSActive(gpio, aprBit)) return apr;
  else if (MSSActive(gpio, advBit)) return adv;
  else return clear;
}

void PrintHex16(uint16_t v) {
  if (v < 0x1000) Serial.print('0');
  if (v < 0x0100) Serial.print('0');
  if (v < 0x0010) Serial.print('0');
  Serial.print(v, HEX);
}

const char *AspectName(int status) {
  switch (status) {
    case clear: return "GREEN";
    case adv:   return "FLASH YEL";
    case apr:   return "YELLOW";
    case occ:   return "RED";
    default:    return "?";
  }
}

const char *CascadeName(bool aprOut, bool advOut) {
  if (aprOut) return "APR";
  if (advOut) return "ADV";
  return "NONE";
}

void PrintLogic(bool asserted) {
  Serial.print(asserted ? '1' : '0');
}

void PrintDebugScreen(uint16_t gpio27, uint16_t gpio25, bool diverge,
                      int pointsUpperStatus, int pointsLowerStatus,
                      int mainStatus, int divergeStatus) {
  static bool initialized = false;
  static uint16_t old27 = 0;
  static uint16_t old25 = 0;
  static bool oldDiverge = false;
  static int oldPU = -1, oldPL = -1, oldM = -1, oldD = -1;

  // Ignore unused 0x25 input bits when deciding whether to redraw.
  // Keep all output bits plus the route-select bit.
  const uint16_t gpio25Relevant = gpio25 & (0x0FFF | (1U << DIVERGE_INPUT_BIT));
  const uint16_t old25Relevant  = old25  & (0x0FFF | (1U << DIVERGE_INPUT_BIT));

  bool changed = !initialized ||
                 gpio27 != old27 ||
                 gpio25Relevant != old25Relevant ||
                 diverge != oldDiverge ||
                 pointsUpperStatus != oldPU ||
                 pointsLowerStatus != oldPL ||
                 mainStatus != oldM ||
                 divergeStatus != oldD;

  if (!changed) return;

  initialized = true;
  old27 = gpio27;
  old25 = gpio25;
  oldDiverge = diverge;
  oldPU = pointsUpperStatus;
  oldPL = pointsLowerStatus;
  oldM = mainStatus;
  oldD = divergeStatus;

  const bool pOcc = MSSActive(gpio27, P_OCC_IN);
  const bool pApr = MSSActive(gpio27, P_APR_IN);
  const bool pAdv = MSSActive(gpio27, P_ADV_IN);
  const bool mOcc = MSSActive(gpio27, M_OCC_IN);
  const bool mApr = MSSActive(gpio27, M_APR_IN);
  const bool mAdv = MSSActive(gpio27, M_ADV_IN);
  const bool dOcc = MSSActive(gpio27, D_OCC_IN);
  const bool dApr = MSSActive(gpio27, D_APR_IN);
  const bool dAdv = MSSActive(gpio27, D_ADV_IN);

  // U4 driver outputs are active HIGH internally.
  const bool pAprOut = (gpio25 & (1U << P_APR_OUT)) != 0;
  const bool pAdvOut = (gpio25 & (1U << P_ADV_OUT)) != 0;
  const bool mAprOut = (gpio25 & (1U << M_APR_OUT)) != 0;
  const bool mAdvOut = (gpio25 & (1U << M_ADV_OUT)) != 0;
  const bool dAprOut = (gpio25 & (1U << D_APR_OUT)) != 0;
  const bool dAdvOut = (gpio25 & (1U << D_ADV_OUT)) != 0;

  // ANSI clear-screen + cursor-home. Terminals that do not interpret ANSI
  // will still receive only one compact snapshot per state change.
  Serial.print(F("\033[2J\033[H"));
  Serial.println(F("MSS COMPLEX CASCADE"));
  Serial.println(F("=========================="));
  Serial.print(F("ROUTE: "));
  Serial.println(diverge ? F("DIVERGE") : F("MAIN"));
  Serial.println();

  Serial.println(F("             POINTS   MAIN   DIVERGE"));
  Serial.print(F("OCC             ")); PrintLogic(pOcc); Serial.print(F("       ")); PrintLogic(mOcc); Serial.print(F("       ")); PrintLogic(dOcc); Serial.println();
  Serial.print(F("APR-IN          ")); PrintLogic(pApr); Serial.print(F("       ")); PrintLogic(mApr); Serial.print(F("       ")); PrintLogic(dApr); Serial.println();
  Serial.print(F("ADV-IN          ")); PrintLogic(pAdv); Serial.print(F("       ")); PrintLogic(mAdv); Serial.print(F("       ")); PrintLogic(dAdv); Serial.println();
  Serial.println();

  Serial.println(F("MASTS"));
  Serial.print(F("P-UPPER       ")); Serial.println(AspectName(pointsUpperStatus));
  Serial.print(F("P-LOWER       ")); Serial.println(AspectName(pointsLowerStatus));
  Serial.print(F("MAIN          ")); Serial.println(AspectName(mainStatus));
  Serial.print(F("DIVERGE       ")); Serial.println(AspectName(divergeStatus));
  Serial.println();

  Serial.println(F("CASCADE OUT"));
  Serial.print(F("POINTS        ")); Serial.println(CascadeName(pAprOut, pAdvOut));
  Serial.print(F("MAIN          ")); Serial.println(CascadeName(mAprOut, mAdvOut));
  Serial.print(F("DIVERGE       ")); Serial.println(CascadeName(dAprOut, dAdvOut));
  Serial.println();

  Serial.println(F("RAW"));
  Serial.print(F("IN  0x27: 0x")); PrintHex16(gpio27); Serial.println();
  Serial.print(F("OUT 0x25: 0x")); PrintHex16(gpio25); Serial.println();
  Serial.println(F("=========================="));
}

// ---------- MSS cascade output routing ----------
// U4 / 0x25 driver bits. A HIGH on a -DR output turns on the
// transistor and asserts the corresponding external MSS line LOW.
enum MSS25OutputBit {
  P_OCC_OUT = 0,
  P_APR_OUT = 1,
  P_ADV_OUT = 2,
  P_DIVERGE_OUT = 3,
  M_OCC_OUT = 4,
  M_APR_OUT = 5,
  M_ADV_OUT = 6,
  M_DIVERGE_OUT = 7,
  D_OCC_OUT = 8,
  D_APR_OUT = 9,
  D_ADV_OUT = 10,
  D_DIVERGE_OUT = 11
};

void SetMSSDriver(uint8_t bit, bool asserted) {
  // Driver is active HIGH internally; transistor makes MSS bus active LOW.
  mss_25.digitalWrite(bit, asserted ? HIGH : LOW);
}

void UpdateCascadeOutputs(uint16_t gpio27, bool diverge) {
  // Inputs are active LOW. Cascade rule is exclusive:
  //   OCC    -> APR-OUT on the opposite side
  //   APR-IN -> ADV-OUT on the opposite side
  //   ADV-IN -> no further propagation
  //   CLEAR  -> no restrictive output

  const bool pOcc = MSSActive(gpio27, P_OCC_IN);
  const bool pApr = MSSActive(gpio27, P_APR_IN);

  const bool mOcc = MSSActive(gpio27, M_OCC_IN);
  const bool mApr = MSSActive(gpio27, M_APR_IN);

  const bool dOcc = MSSActive(gpio27, D_OCC_IN);
  const bool dApr = MSSActive(gpio27, D_APR_IN);

  // Start with every APR/ADV cascade output released.
  bool pAprOut = false;
  bool pAdvOut = false;
  bool mAprOut = false;
  bool mAdvOut = false;
  bool dAprOut = false;
  bool dAdvOut = false;

  if (!diverge) {
    // MAIN route selected.
    // POINTS -> MAIN
    if (pOcc)       mAprOut = true;
    else if (pApr)  mAdvOut = true;

    // MAIN -> POINTS
    if (mOcc)       pAprOut = true;
    else if (mApr)  pAdvOut = true;
  } else {
    // DIVERGE route selected.
    // POINTS -> DIVERGE
    if (pOcc)       dAprOut = true;
    else if (pApr)  dAdvOut = true;

    // DIVERGE -> POINTS
    if (dOcc)       pAprOut = true;
    else if (dApr)  pAdvOut = true;
  }

  SetMSSDriver(P_APR_OUT, pAprOut);
  SetMSSDriver(P_ADV_OUT, pAdvOut);
  SetMSSDriver(M_APR_OUT, mAprOut);
  SetMSSDriver(M_ADV_OUT, mAdvOut);
  SetMSSDriver(D_APR_OUT, dAprOut);
  SetMSSDriver(D_ADV_OUT, dAdvOut);
}
