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