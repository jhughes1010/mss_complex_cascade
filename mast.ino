// MSS Complex Cascade - mast lighting
// U7 MCP23017 @ 0x26
// Schematic mapping: G/Y/R per head, active LOW.

bool InitMastLighting(void) {
  bool error = false;

  Serial.println("Init LED drive");

  if (!mast.begin_I2C(0x26)) {
    Serial.println("Port Expander at address 0x26 is not responding");
    error = true;
  } else {
    for (int pin = 0; pin < 16; pin++) {
      mast.pinMode(pin, OUTPUT);
      mast.digitalWrite(pin, HIGH);   // active LOW: all LEDs off
    }
  }

  return error;
}

void LightShow(void) {
  // Retained for project compatibility, but not used in normal startup.
  for (int state = 0; state < 4; state++) {
    Serial.print(".");
    for (int signal = 0; signal < 4; signal++) {
      SetSignal(signal, state);
    }
    delay(1000);
  }
  Serial.println(" Start");
}

void SetSignal(int signalHead, int status) {
  int green_bit;
  int yellow_bit;
  int red_bit;
  static int priorStatus[4] = { -1, -1, -1, -1 };
  static bool priorAdvBlink[4] = { false, false, false, false };

  switch (signalHead) {
    case points_upper:
      green_bit = 0;
      yellow_bit = 1;
      red_bit = 2;
      break;

    case points_lower:
      green_bit = 3;
      yellow_bit = 4;
      red_bit = 5;
      break;

    case main:
      green_bit = 6;
      yellow_bit = 7;
      red_bit = 8;
      break;

    case dvg:
      green_bit = 9;
      yellow_bit = 10;
      red_bit = 11;
      break;

    default:
      return;
  }

  // ADV flashes yellow. Other aspects are steady.
  bool advBlink = ((millis() / 500UL) & 1U) != 0;

  // Rewrite when the aspect changes, or when ADV changes flash phase.
  if (status != priorStatus[signalHead] ||
      (status == adv && advBlink != priorAdvBlink[signalHead])) {

    // Exclusive aspect: turn all three lamps OFF first.
    mast.digitalWrite(green_bit, HIGH);
    mast.digitalWrite(yellow_bit, HIGH);
    mast.digitalWrite(red_bit, HIGH);

    if (status == clear) {
      mast.digitalWrite(green_bit, LOW);
    } else if (status == adv) {
      if (advBlink) {
        mast.digitalWrite(yellow_bit, LOW);
      }
    } else if (status == apr) {
      mast.digitalWrite(yellow_bit, LOW);
    } else if (status == occ) {
      mast.digitalWrite(red_bit, LOW);
    }
  }

  priorStatus[signalHead] = status;
  priorAdvBlink[signalHead] = advBlink;
}
