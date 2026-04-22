void PrintOnChange(bool diverge) {
  static bool divergePrior = false;
  if (diverge != divergePrior) {
    Serial.print("Diverge: ");
    Serial.println(diverge);
  }
  divergePrior = diverge;
}

bool readOptical(int addr, long currentTime, long activeTime) {
  bool status;
  if (timeWindowIn(currentTime, activeTime, 30000)) {
    Serial.println("Optical valid");
  }
  return false;
}