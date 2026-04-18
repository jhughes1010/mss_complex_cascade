bool InitMastLighting(void) {
  bool error = false;
  int pin;

  Serial.println("Init LED drive");

  if (!mast.begin_I2C(0x26)) {
    Serial.println("Port Expander at address 0x26 is not responding");
    error = true;
  } else {
    //setPIN direction
    for (pin = 0; pin < 16; pin++) {
      mast.pinMode(pin, OUTPUT);
      //Serial.print("Output pin:");
      //Serial.println(pin);
    }
  }
  return error;
}

void LightShow(void) {
  int state, signal;
  for (state = 3; state >= 0; state--) {
    Serial.print(".");
    for (signal = 0; signal < 4; signal++) {
      SetSignal(signal, state);
    }
    delay(1500);
  }
  Serial.println(" Start");
}


void SetSignal(int signalHead, int status) {
  int green_bit, red_bit, yellow_bit;
  int addr;
  bool adv_blink;
  static int priorStatus = -1;


  adv_blink = (millis() / 1000) % 2;

  switch (signalHead) {
    case points_upper:
      addr = 0x26;
      green_bit = 0;
      yellow_bit = 1;
      red_bit = 2;
      break;

    case points_lower:
      addr = 0x26;
      green_bit = 7;
      yellow_bit = 6;
      red_bit = 5;
      break;

    case main:
      addr = 0x26;
      green_bit = 7;
      yellow_bit = 6;
      red_bit = 5;
      break;

    case diverge:
      addr = 0x26;
      green_bit = 7;
      yellow_bit = 6;
      red_bit = 5;
      break;
  }
  if (status != priorStatus || status == adv) {
    if (status == clear) {
      mast.digitalWrite(green_bit, LOW);
    } else if (status == adv) {
      if (adv_blink) {
        mast.digitalWrite(yellow_bit, LOW);
      } else {
        mast.digitalWrite(yellow_bit, HIGH);
      }
    } else if (status == apr) {
      mast.digitalWrite(yellow_bit, LOW);
    } else if (status == occ) {
      mast.digitalWrite(red_bit, LOW);
    } else {
      //dark
    }
  }
  priorStatus = status;
}