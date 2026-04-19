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
  darkAll();
  return error;
}

void LightShow(void) {
  int state, signal;
  for (state = 0; state < 4; state++) {
    Serial.print(".");
    for (signal = 0; signal < 4; signal++) {
      SetSignal(signal, state);
    }
    delay(1000);
    darkAll();
  }
  Serial.println(" Start");
}


void SetSignal(int signalHead, int status) {
  int green_bit, red_bit, yellow_bit;
  int addr;
  bool adv_blink;
  static int priorStatus[4] = { -1, -1, -1, -1 };

  //Serial.print(signalHead);
  //Serial.print(":");
  //Serial.println(status);
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
      green_bit = 3;
      yellow_bit = 4;
      red_bit = 5;
      break;

    case main:
      addr = 0x26;
      green_bit = 6;
      yellow_bit = 7;
      red_bit = 8;
      break;

    case dvg:
      addr = 0x26;
      green_bit = 9;
      yellow_bit = 10;
      red_bit = 11;
      break;
  }
  if (status != priorStatus[signalHead] || status == adv) {
    dark(signalHead);
    if (status == clear) {
      //Serial.println("Clear");
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
      //Serial.println("Occupied");

      mast.digitalWrite(red_bit, LOW);
    } else {
      //dark
    }
  }
  priorStatus[signalHead] = status;
}

void darkAll(void) {
  mast.writeGPIOAB(0xffff);
}
void dark(int signal) {
  int GPIO, mask;
  mask = 7 << (signal * 3);
  GPIO = mast.readGPIOAB();
  GPIO = GPIO | mask;
  mast.writeGPIOAB(GPIO);
}