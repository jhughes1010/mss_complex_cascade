void PrintOnChangeDiverge(bool diverge) {
  static bool divergePrior = false;
  if (diverge != divergePrior) {
    Serial.print("Diverge: ");
    Serial.println(diverge);
  }
  divergePrior = diverge;
}

void PrintOnChangeBDLocal(bool bd_points, bool bd_main, bool bd_diverge) {
  static bool p_points = false;
  static bool p_main = false;
  static bool p_diverge = false;
  if (p_points != bd_points | p_main != bd_main | p_diverge != bd_diverge) {
    Serial.print("Local BD values: ");
    Serial.print(bd_points);
    Serial.print(bd_main);
    Serial.println(bd_diverge);
  }
  p_points = bd_points;
  p_main = bd_main;
  p_diverge = bd_diverge;
}

bool readOptical(int addr, long currentTime, long activeTime) {
  bool status;
  static bool priorStatus = false;
  status = timeWindowIn(currentTime, activeTime, 10000);
  if (status) {
    if (status != priorStatus) {
      Serial.println("Optical valid capture");
    }
  } else {
    if (status != priorStatus) {
      Serial.println("Optical ignore capture");
    }
  }
  priorStatus = status;
  return false;
}