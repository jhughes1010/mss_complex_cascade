//InWindow

//OutWindow


void heartbeat(int time) {
  bool status;
  status = (millis() / time) % 2;
  digitalWrite(LED_BUILTIN, status);
}

//Returns true if compareTime is inside the compareWindow
bool timeWindowIn(long currentTime, long compareTime, long compareWindow) {
  bool status = false;
  if (currentTime - compareTime < compareWindow) {
    status = true;
  }
  return status;
}

//Returns true if compareTime is outside the compareWindow
bool timeWindowOut(long currentTime, long compareTime, long compareWindow) {
  bool status = false;
  if (currentTime - compareTime > compareWindow) {
    status = true;
  }
  return status;
}
