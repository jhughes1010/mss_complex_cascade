void SetMast(int mast, int status) {
  int green_bit, red_bit, yellow_bit;
  int addr;
  bool adv_blink;

  adv_blink = (millis() / 1000) % 2;

  switch (mast) {
    case points_upper:
      addr = 0x27;
      green_bit = 7;
      yellow_bit = 6;
      red_bit = 5;
      break;

    case points_lower:
      addr = 0x27;
      green_bit = 7;
      yellow_bit = 6;
      red_bit = 5;
      break;

    case main:
      addr = 0x27;
      green_bit = 7;
      yellow_bit = 6;
      red_bit = 5;
      break;

    case diverge:
      addr = 0x27;
      green_bit = 7;
      yellow_bit = 6;
      red_bit = 5;
      break;
  }

  if (status == clear) {
    //i2c_write(mast,green_bit);
  } else if (status == adv) {
    if (adv_blink) {
      //i2c_write(mast,green_bit);
    } else {  
      //dark
    }
  } else if (status == apr) {
    //i2c_write(mast,green_bit);
  } else if (status == occ) {
    //i2c_write(mast,green_bit);
  } else {
    //dark
  }
}