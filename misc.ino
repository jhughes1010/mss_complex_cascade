void PrintOnChange(bool diverge) {
  static bool divergePrior = false;
  if (diverge != divergePrior) {
    Serial.print("Diverge: ");
    Serial.println(diverge);
    divergePrior = diverge;
  }
}