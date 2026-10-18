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
// ---------- Phase 1 MSS decode / traffic monitor ----------
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
  if(v<0x1000) Serial.print('0'); if(v<0x0100) Serial.print('0');
  if(v<0x0010) Serial.print('0'); Serial.print(v,HEX);
}

void InitMSSTrafficMonitor() { ReportMSSTraffic(mss_27.readGPIOAB(),mss_25.readGPIOAB()); }

void PrintMSSEvent(const char *section,const char *fn,bool asserted) {
  Serial.print(F("MSS IN   ")); Serial.print(section); Serial.print(F("   "));
  Serial.print(fn); Serial.print(F("   "));
  Serial.println(asserted ? F("ASSERTED") : F("CLEARED"));
}

void ReportBit(uint16_t changed,uint16_t gpio,uint8_t bit,const char *section,const char *fn) {
  if(changed & (1U<<bit)) PrintMSSEvent(section,fn,MSSActive(gpio,bit));
}

void ReportMSSTraffic(uint16_t gpio27,uint16_t gpio25) {
  static bool init=false; static uint16_t old27,old25;
  if(!init) { old27=gpio27; old25=gpio25; init=true; return; }
  uint16_t changed27=gpio27^old27;
  uint16_t changed25=(gpio25^old25)&0xF000;
  if(changed27) {
    Serial.print(F("MSS IN RAW 0x27: 0x")); PrintHex16(gpio27); Serial.println();
    ReportBit(changed27,gpio27,P_OCC_IN,"POINTS","OCC");
    ReportBit(changed27,gpio27,P_APR_IN,"POINTS","APR-IN");
    ReportBit(changed27,gpio27,P_ADV_IN,"POINTS","ADV-IN");
    ReportBit(changed27,gpio27,M_OCC_IN,"MAIN","OCC");
    ReportBit(changed27,gpio27,M_APR_IN,"MAIN","APR-IN");
    ReportBit(changed27,gpio27,M_ADV_IN,"MAIN","ADV-IN");
    ReportBit(changed27,gpio27,D_OCC_IN,"DIVERGE","OCC");
    ReportBit(changed27,gpio27,D_APR_IN,"DIVERGE","APR-IN");
    ReportBit(changed27,gpio27,D_ADV_IN,"DIVERGE","ADV-IN");
  }
  if(changed25&(1U<<DIVERGE_INPUT_BIT)) {
    Serial.print(F("MSS IN RAW 0x25: 0x")); PrintHex16(gpio25); Serial.println();
    PrintMSSEvent("ROUTE","DIVERGE",MSSActive(gpio25,DIVERGE_INPUT_BIT));
  }
  old27=gpio27; old25=gpio25;
}
