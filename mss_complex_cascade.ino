//MSS complex cascade
//James Hughes
//03-27-2026

enum { points_upper,
       points_lower,
       main,
       diverge };




void setup() {
  Serial.begin(9600);
  //set i2c port expander DDR

  //set LCD i2c port expander
}

void loop() {
  long currentTime = 0;
  static long activeTime = 0;

  currentTime = millis();
  //Read inputs i2C
  //Read inputs optical
  //Process inputs
  //Set masts
  //SetPoints(0);
  //SetMain(0);
  SetMast(points_upper, 0);
  SetMast(points_upper, 0);
  SetMast(main, 0);
  SetMast(diverge, 0);
}
