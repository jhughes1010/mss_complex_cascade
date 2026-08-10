void LCDInit(void) {
  lcd.begin(16, 2);
  lcd.setBacklight(0x02);
  lcd.print("Complex Cascade");
  lcd.setCursor(0, 1);
  lcd.print("Ver 2026-4-21");
}

void LCDStatus(bool diverge) {
  static bool divergePrior = false;
  if (diverge != divergePrior) {
    lcd.clear();
    if (diverge) {
      lcd.setCursor(15, 1);
      lcd.print("D");
    }
  }
  divergePrior = diverge;
}

void LCDSigStatus(void) {
  static int mode = 0, modePrior = -1;

  if (lcd.readButtons()) {
    mode++;
    mode = mode % 3;
    delay(500);
  }
  if (mode != modePrior) {
    switch (mode) {
      case 0:
        lcd.clear();
        lcd.setBacklight(0x07);
        lcd.setCursor(0, 0);
        lcd.print("Points U/L");
        break;

        break;
      case 1:
        lcd.clear();
        lcd.setBacklight(0x06);
        lcd.setCursor(0, 0);
        lcd.print("Main");
        break;

      case 2:
        lcd.clear();
        lcd.setBacklight(0x05);
        lcd.setCursor(0, 0);
        lcd.print("Diverge");
        break;
    }
  }
  modePrior = mode;
}