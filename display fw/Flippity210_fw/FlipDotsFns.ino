/*
  void setLocalPixel(int x, int y, bool value, int display) {
  bitWrite(dotsBuffer[display][y], x, value);
  }

  void setPixel(int x, int y, bool value) {
  int displayNumber = x / columnCount;
  x = x % columnCount;
  setLocalPixel(x, y, value, displayNumber);
  }
  void blankDisplay(uint8_t dispNumber) {
  for (int i = 0; i < rowCount; i++) {
    dotsBuffer[dispNumber][i] = 0;
  }
  }

  void fillDisplay(uint8_t dispNumber) {
  for (int i = 0; i < rowCount; i++) {
    dotsBuffer[dispNumber][i] = UINT32_MAX;
  }
  }

  void invertDisplay(uint8_t dispNumber) {

  for (int i = 0; i < rowCount; i++) {
    dotsBuffer[dispNumber][i] = ~dotsBuffer[dispNumber][i];
  }

  }

  void blankAll() {
  for (int dispNumber; dispNumber < displaysCount; dispNumber++) {
    for (int i = 0; i < rowCount; i++) {
      dotsBuffer[dispNumber][i] = 0;
    }
  }
  }
*/
/*
  void fillAll() {
  for (int dispNumber; dispNumber < displaysCount; dispNumber++) {
    for (int i = 0; i < rowCount; i++) {
      dotsBuffer[dispNumber][i] = UINT32_MAX;
    }
  }
  }

  void invertAll() {
  for (int dispNumber; dispNumber < displaysCount; dispNumber++) {
    for (int i = 0; i < rowCount; i++) {
      dotsBuffer[dispNumber][i] = ~dotsBuffer[dispNumber][i];
    }
  }
  }


  void draw5x9(int x, int y, uint8_t symbol[9][5]) {
  for (uint8_t row = 0; row < 9; row++) {
    for (uint8_t col = 0; col < 5; col++) {
      setPixel(col + x, row + y, symbol[row][col]);
    }
  }
  }
*/

// Init pins safely, make sure they start up low
// this needs to be called first in setup
void initFlipDots() {
  pinMode(pinNametoDigitalPin(SINK_EN), OUTPUT);
  pinMode(pinNametoDigitalPin(SOURCE_EN), OUTPUT);
  pinMode(ROWS_DATA, OUTPUT);
  pinMode(ROWS_CLK, OUTPUT);
  pinMode(ROWS_LATCH, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);

  digitalWriteFast(SINK_EN, 0);
  digitalWriteFast(SOURCE_EN, 0);
  digitalWrite(ROWS_DATA, 0);
  digitalWrite(ROWS_LATCH, 0);
  digitalWrite(ROWS_CLK, 0);

  pinMode(pinNametoDigitalPin(COLS_ENABLE), OUTPUT);
  //pinMode(pinNametoDigitalPin(PANEL_ADDR_B0), OUTPUT);
  //pinMode(pinNametoDigitalPin(PANEL_ADDR_B1), OUTPUT);
  //pinMode(pinNametoDigitalPin(PANEL_ADDR_B2), OUTPUT);
  pinMode(pinNametoDigitalPin(GROUP_ADDR_B0), OUTPUT);

  digitalWriteFast(COLS_ENABLE, 0);
  //digitalWriteFast(PANEL_ADDR_B0, 1);
  //digitalWriteFast(PANEL_ADDR_B1, 0);
  //digitalWriteFast(PANEL_ADDR_B2, 0);
  digitalWriteFast(GROUP_ADDR_B0, 0);

}

void dotsSelectDisplay(uint8_t displayNumber) {
  shiftSetValue(bytes, PANEL_ADDR_B0, bitRead(displayNumber, 0));
  shiftSetValue(bytes, PANEL_ADDR_B1, bitRead(displayNumber, 1));
  shiftSetValue(bytes, PANEL_ADDR_B2, bitRead(displayNumber, 2));
}

void dotsSelectCol(uint8_t colNumber) {
  shiftSetValue(bytes, GROUP_ADDR_B1, bitRead(colNumber, 0)); // 30
  shiftSetValue(bytes, GROUP_ADDR_B2, bitRead(colNumber, 1)); // 31
  shiftSetValue(bytes, COL_ADDR_B0, bitRead(colNumber, 2)); // 32
  shiftSetValue(bytes, COL_ADDR_B1, bitRead(colNumber, 3)); // 33
  shiftSetValue(bytes, COL_ADDR_B2, bitRead(colNumber, 4)); // 34
}


void dotsFlipCol(uint8_t dispNumber, uint8_t colNumber, bool forceChange) {

  dotsSelectCol(colNumber);
  digitalWriteFast(GROUP_ADDR_B0, 0); // 1 = 24V, 0 = GND

  for (int row = 0; row < rowCount; row++) {

    if (!bitRead(dotsBuffer[dispNumber][row], colNumber)) continue;
    if (!forceChange && bitRead(dotsBuffer[dispNumber][row], colNumber) == bitRead(dotsActive[dispNumber][row], colNumber)) continue; // if the value is the same as active, skip for faster refresh

    for (int ii = 0; ii < rowCount; ii++) { // reset all to zero because only one row can be flipped at a time!!!!! Otherwise magic smoke.
      shiftSetValue(bytes, rows[ii][0], 0);
      shiftSetValue(bytes, rows[ii][1], 0);
    }
    shiftSetValue(bytes, rows[row][1], bitRead(dotsBuffer[dispNumber][row], colNumber));

    spi2WriteBytes(bytes);

    digitalWriteFast(SOURCE_EN, 1);
    digitalWriteFast(COLS_ENABLE, 1);
    delayMicroseconds(DOTS_POWERON_TIME);
    digitalWriteFast(COLS_ENABLE, 0);
    digitalWriteFast(SOURCE_EN, 0);
    delayMicroseconds(flipDotUpdateDelay);
  }

}

void dotsUnflipCol(uint8_t dispNumber, uint8_t colNumber, bool forceChange) {

  dotsSelectCol(colNumber);
  digitalWriteFast(GROUP_ADDR_B0, 1); // 1 = 24V, 0 = GND

  for (int row = 0; row < rowCount; row++) {

    if (bitRead(dotsBuffer[dispNumber][row], colNumber)) continue;
    if (!forceChange && bitRead(dotsBuffer[dispNumber][row], colNumber) == bitRead(dotsActive[dispNumber][row], colNumber)) continue; // if the value is the same as active, skip for faster refresh

    for (int ii = 0; ii < rowCount; ii++) { // reset all to zero because only one row can be flipped at a time!!!!! Otherwise magic smoke.
      shiftSetValue(bytes, rows[ii][0], 0);
      shiftSetValue(bytes, rows[ii][1], 0);
    }
    shiftSetValue(bytes, rows[row][0], !bitRead(dotsBuffer[dispNumber][row], colNumber));

    spi2WriteBytes(bytes);

    digitalWriteFast(SINK_EN, 1);
    digitalWriteFast(COLS_ENABLE, 1);
    delayMicroseconds(DOTS_POWERON_TIME);
    digitalWriteFast(COLS_ENABLE, 0);
    digitalWriteFast(SINK_EN, 0);
    delayMicroseconds(flipDotUpdateDelay);
  }
}

void updateCol(uint8_t dispNumber, uint8_t colNumber, bool forceChange) {
  dotsUnflipCol(dispNumber, colNumber, forceChange);
  dotsFlipCol(dispNumber, colNumber, forceChange);

}

#if defined(ENABLE_ANIMATION_DISSOLVE)

void getNumPixelsToFlipUnflip() {
  dotsToFlipUnflip[0] = 0; // Dots to flip
  dotsToFlipUnflip[1] = 0; // Dots to unflip

  for (int dispNumber = 0; dispNumber < displaysCount; dispNumber++) {
    for (int row = 0; row < rowCount; row++) {
      for (int colNumber = 0; colNumber < columnCount; colNumber++) {
        if (bitRead(dotsBuffer[dispNumber][row], colNumber) && bitRead(dotsBuffer[dispNumber][row], colNumber) != bitRead(dotsActive[dispNumber][row], colNumber)) {
          dotsToFlipUnflip[0]++;
        }
        if (!bitRead(dotsBuffer[dispNumber][row], colNumber) && bitRead(dotsBuffer[dispNumber][row], colNumber) != bitRead(dotsActive[dispNumber][row], colNumber)) {
          dotsToFlipUnflip[1]++;
        }
      }
    }
  }
}


void dissolveDots() {
  getNumPixelsToFlipUnflip();

  unsigned int randToFlip = 0;
  unsigned int randToUnflip = 0;
  unsigned int iterator = 0;
  bool breakOuterLoop = false;

  digitalWriteFast(GROUP_ADDR_B0, 0); // 1 = 24V, 0 = GND

  for (int i = dotsToFlipUnflip[0]; i > 0; i--) {
    randToFlip = random(0, i) + 1;
    breakOuterLoop = false;
    iterator = 0;

    for (int dispNumber = 0; dispNumber < displaysCount; dispNumber++) {
      if (breakOuterLoop) break;

      for (int row = 0; row < rowCount; row++) {
        if (breakOuterLoop) break;

        for (int colNumber = 0; colNumber < columnCount; colNumber++) {
          if (breakOuterLoop) break;

          if (!bitRead(dotsBuffer[dispNumber][row], colNumber)) continue;
          if (bitRead(dotsBuffer[dispNumber][row], colNumber) == bitRead(dotsActive[dispNumber][row], colNumber)) continue; // if the value is the same as active, skip for faster refresh

          iterator++;

          if (iterator != randToFlip) continue;
          if (iterator == randToFlip) {
            breakOuterLoop = true;
            bitWrite(dotsActive[dispNumber][row], colNumber, 1);
          }

          dotsSelectDisplay(dispNumber);
          dotsSelectCol(colNumber);

          for (int ii = 0; ii < rowCount; ii++) { // reset all to zero because only one row can be flipped at a time!!!!! Otherwise magic smoke.
            shiftSetValue(bytes, rows[ii][0], 0);
            shiftSetValue(bytes, rows[ii][1], 0);
          }
          shiftSetValue(bytes, rows[row][1], bitRead(dotsBuffer[dispNumber][row], colNumber));

          spi2WriteBytes(bytes);

          digitalWriteFast(SOURCE_EN, 1);
          digitalWriteFast(COLS_ENABLE, 1);
          delayMicroseconds(DOTS_POWERON_TIME);
          digitalWriteFast(COLS_ENABLE, 0);
          digitalWriteFast(SOURCE_EN, 0);
          delayMicroseconds(flipDotUpdateDelay);
        }
      }
    }
  }

  digitalWriteFast(GROUP_ADDR_B0, 1); // 1 = 24V, 0 = GND

  for (int i = dotsToFlipUnflip[1]; i > 0; i--) {
    randToUnflip = random(0, i) + 1;
    breakOuterLoop = false;
    iterator = 0;

    for (int dispNumber = 0; dispNumber < displaysCount; dispNumber++) {

      if (breakOuterLoop) break;

      for (int row = 0; row < rowCount; row++) {
        if (breakOuterLoop) break;

        for (int colNumber = 0; colNumber < columnCount; colNumber++) {
          if (breakOuterLoop) break;
          if (bitRead(dotsBuffer[dispNumber][row], colNumber)) continue;
          if (bitRead(dotsBuffer[dispNumber][row], colNumber) == bitRead(dotsActive[dispNumber][row], colNumber)) continue; // if the value is the same as active, skip for faster refresh

          iterator++;

          if (iterator != randToUnflip) continue;
          if (iterator == randToUnflip) {
            breakOuterLoop = true;
            bitWrite(dotsActive[dispNumber][row], colNumber, 0);
          }

          dotsSelectDisplay(dispNumber);
          dotsSelectCol(colNumber);

          for (int ii = 0; ii < rowCount; ii++) { // reset all to zero because only one row can be flipped at a time!!!!! Otherwise magic smoke.
            shiftSetValue(bytes, rows[ii][0], 0);
            shiftSetValue(bytes, rows[ii][1], 0);
          }
          shiftSetValue(bytes, rows[row][0], !bitRead(dotsBuffer[dispNumber][row], colNumber));

          spi2WriteBytes(bytes);

          digitalWriteFast(SINK_EN, 1);
          digitalWriteFast(COLS_ENABLE, 1);
          delayMicroseconds(DOTS_POWERON_TIME);
          digitalWriteFast(COLS_ENABLE, 0);
          digitalWriteFast(SINK_EN, 0);
          delayMicroseconds(flipDotUpdateDelay);
        }
      }
    }

  }

}

#endif





/*
void getNumPixelsToFlipUnflip() {
  dotsToFlipUnflip[0] = 0; // Dots to flip
  dotsToFlipUnflip[1] = 0; // Dots to unflip

  for (int dispNumber = 0; dispNumber < displaysCount; dispNumber++) {
    for (int row = 0; row < rowCount; row++) {
      for (int colNumber = 0; colNumber < columnCount; colNumber++) {
        if (bitRead(dotsBuffer[dispNumber][row], colNumber) && bitRead(dotsBuffer[dispNumber][row], colNumber) != bitRead(dotsActive[dispNumber][row], colNumber)) {
          dotsToFlipUnflip[0]++;
        }
        if (!bitRead(dotsBuffer[dispNumber][row], colNumber) && bitRead(dotsBuffer[dispNumber][row], colNumber) != bitRead(dotsActive[dispNumber][row], colNumber)) {
          dotsToFlipUnflip[1]++;
        }
      }
    }
  }
}



void dissolveToggle(bool source) {
  digitalWriteFast(GROUP_ADDR_B0, !source); // 1 = 24V, 0 = GND

  unsigned int randToFlipUnflip = 0;
  unsigned int iterator = 0;
  bool breakOuterLoop = false;

  for (int i = dotsToFlipUnflip[1]; i > 0; i--) {
    randToFlipUnflip = random(0, i) + 1;
    breakOuterLoop = false;
    iterator = 0;

    for (int dispNumber = 0; dispNumber < displaysCount; dispNumber++) {

      if (breakOuterLoop) break;

      for (int row = 0; row < rowCount; row++) {
        if (breakOuterLoop) break;

        for (int colNumber = 0; colNumber < columnCount; colNumber++) {
          if (breakOuterLoop) break;

          if (source) {
            if (!bitRead(dotsBuffer[dispNumber][row], colNumber)) continue;
          } else {
            if (bitRead(dotsBuffer[dispNumber][row], colNumber)) continue;
          }

          if (bitRead(dotsBuffer[dispNumber][row], colNumber) == bitRead(dotsActive[dispNumber][row], colNumber)) continue; // if the value is the same as active, skip for faster refresh

          iterator++;

          if (iterator != randToFlipUnflip) continue;
          if (iterator == randToFlipUnflip) {
            breakOuterLoop = true;
            bitWrite(dotsActive[dispNumber][row], colNumber, source);
          }

          dotsSelectDisplay(dispNumber);
          dotsSelectCol(colNumber);

          for (int ii = 0; ii < rowCount; ii++) { // reset all to zero because only one row can be flipped at a time!!!!! Otherwise magic smoke.
            shiftSetValue(bytes, rows[ii][0], 0);
            shiftSetValue(bytes, rows[ii][1], 0);
          }
          if (source) {
            shiftSetValue(bytes, rows[row][1], bitRead(dotsBuffer[dispNumber][row], colNumber));
          } else {
            shiftSetValue(bytes, rows[row][0], !bitRead(dotsBuffer[dispNumber][row], colNumber));
          }

          spi2WriteBytes(bytes);

          digitalWriteFast(source ? SOURCE_EN : SINK_EN, 1);
          digitalWriteFast(COLS_ENABLE, 1);
          delayMicroseconds(DOTS_POWERON_TIME);
          digitalWriteFast(COLS_ENABLE, 0);
          digitalWriteFast(source ? SOURCE_EN : SINK_EN, 0);
          delayMicroseconds(flipDotUpdateDelay);
        }
      }
    }
  }
}



void dissolveDots() {
  getNumPixelsToFlipUnflip();

  unsigned int randToFlip = 0;
  unsigned int randToUnflip = 0;
  unsigned int iterator = 0;
  bool breakOuterLoop = false;


  dissolveToggle(1);

  dissolveToggle(0);

}
*/





















void dotsFlipRow(uint8_t row, bool forceChange) {
  digitalWriteFast(GROUP_ADDR_B0, 0); // 1 = 24V, 0 = GND

  for (int dispNumber = 0; dispNumber < displaysCount; dispNumber++) {
    dotsSelectDisplay(dispNumber);

    for (int colNumber = 0; colNumber < columnCount; colNumber++) {
      dotsSelectCol(colNumber);

      if (!bitRead(dotsBuffer[dispNumber][row], colNumber)) continue;
      if (!forceChange && bitRead(dotsBuffer[dispNumber][row], colNumber) == bitRead(dotsActive[dispNumber][row], colNumber)) continue; // if the value is the same as active, skip for faster refresh

      for (int ii = 0; ii < rowCount; ii++) { // reset all to zero because only one row can be flipped at a time!!!!! Otherwise magic smoke.
        shiftSetValue(bytes, rows[ii][0], 0);
        shiftSetValue(bytes, rows[ii][1], 0);
      }
      shiftSetValue(bytes, rows[row][1], bitRead(dotsBuffer[dispNumber][row], colNumber));

      spi2WriteBytes(bytes);

      digitalWriteFast(SOURCE_EN, 1);
      digitalWriteFast(COLS_ENABLE, 1);
      delayMicroseconds(DOTS_POWERON_TIME);
      digitalWriteFast(COLS_ENABLE, 0);
      digitalWriteFast(SOURCE_EN, 0);
      delayMicroseconds(flipDotUpdateDelay);
    }
  }
}

void dotsUnflipRow(uint8_t row, bool forceChange) {
  digitalWriteFast(GROUP_ADDR_B0, 1); // 1 = 24V, 0 = GND

  for (int dispNumber = 0; dispNumber < displaysCount; dispNumber++) {
    dotsSelectDisplay(dispNumber);

    for (int colNumber = 0; colNumber < columnCount; colNumber++) {
      dotsSelectCol(colNumber);

      if (bitRead(dotsBuffer[dispNumber][row], colNumber)) continue;
      if (!forceChange && bitRead(dotsBuffer[dispNumber][row], colNumber) == bitRead(dotsActive[dispNumber][row], colNumber)) continue; // if the value is the same as active, skip for faster refresh

      for (int ii = 0; ii < rowCount; ii++) { // reset all to zero because only one row can be flipped at a time!!!!! Otherwise magic smoke.
        shiftSetValue(bytes, rows[ii][0], 0);
        shiftSetValue(bytes, rows[ii][1], 0);
      }
      shiftSetValue(bytes, rows[row][0], !bitRead(dotsBuffer[dispNumber][row], colNumber));

      spi2WriteBytes(bytes);

      digitalWriteFast(SINK_EN, 1);
      digitalWriteFast(COLS_ENABLE, 1);
      delayMicroseconds(DOTS_POWERON_TIME);
      digitalWriteFast(COLS_ENABLE, 0);
      digitalWriteFast(SINK_EN, 0);
      delayMicroseconds(flipDotUpdateDelay);
    }
  }
}

void updateRow(uint8_t rowNumber, bool forceChange) {
  dotsUnflipRow(rowNumber, forceChange);
  dotsFlipRow(rowNumber, forceChange);
}

void setDisplayStatus(byte busy) {
  displayData[0] = busy;
  if (busy == FLIPPITY210_STATUS_READY) {
    digitalWrite(LED_BUILTIN, 1); // extinguish when ready
  } else {
    digitalWrite(LED_BUILTIN, 0); // lit up when busy
  }

}
void setDisplaySpeed(byte speed) {
  displayData[1] = speed;

  if (speed == FLIPPITY210_SPEED_HIGH) {
    flipDotUpdateDelay = 100;
  }
  else if (speed == FLIPPITY210_SPEED_MED) {
    flipDotUpdateDelay = 1000;
  }
  else if (speed == FLIPPITY210_SPEED_LOW) {
    flipDotUpdateDelay = 2000;
  }
  else if (speed == FLIPPITY210_SPEED_VERY_LOW) {
    flipDotUpdateDelay = 4000;
  }
  else if (speed == FLIPPITY210_SPEED_EXTREMELY_LOW) {
    flipDotUpdateDelay = 10000;
  }
  else if (speed == FLIPPITY210_SPEED_LOWEST) {
    flipDotUpdateDelay = 20000;
  }

}

void setDisplayAnimation(byte anim) {
  displayData[3] = anim;
}

void refreshDisplays() {
  setDisplayStatus(FLIPPITY210_STATUS_BUSY);

#ifdef USE_SERIAL
  Serial.println("[DISPLAY] Updating...");
#endif

  unsigned long millisUpdate = millis();

  if (displayData[3] == FLIPPITY210_ANIM_SLIDE_RIGHT_FORCED) {
    for (int dispNumber = 0; dispNumber < displaysCount; dispNumber++) {
      dotsSelectDisplay(dispNumber);
      for (int col = 0; col < columnCount; col++) {
        updateCol(dispNumber, col, true);
      }
    }
  }
#ifdef ENABLE_ANIMATION_SLIDE_LEFT
  else if (displayData[3] == FLIPPITY210_ANIM_SLIDE_LEFT) {
    for (int dispNumber = 0; dispNumber < displaysCount; dispNumber++) {
      dotsSelectDisplay(displaysCount - 1 - dispNumber);
      for (int col = 0; col < columnCount; col++) {
        updateCol(displaysCount - 1 - dispNumber, columnCount - 1 - col, false);
      }
    }
  }
  else if (displayData[3] == FLIPPITY210_ANIM_SLIDE_LEFT_FORCED) {
    for (int dispNumber = 0; dispNumber < displaysCount; dispNumber++) {
      dotsSelectDisplay(displaysCount - 1 - dispNumber);
      for (int col = 0; col < columnCount; col++) {
        updateCol(displaysCount - 1 - dispNumber, columnCount - 1 - col, true);
      }
    }
  }
#endif
#ifdef ENABLE_ANIMATION_SLIDE_FROM_TOP
  else if (displayData[3] == FLIPPITY210_ANIM_SLIDE_FROM_TOP) {
    for (int row = 0; row < rowCount; row++) {
      updateRow(row, false);
    }
  }
  else if (displayData[3] == FLIPPITY210_ANIM_SLIDE_FROM_TOP_FORCED) {
    for (int row = 0; row < rowCount; row++) {
      updateRow(row, true);
    }
  }
#endif
#ifdef ENABLE_ANIMATION_SLIDE_FROM_BOTTOM
  else if (displayData[3] == FLIPPITY210_ANIM_SLIDE_FROM_BOTTOM) {
    for (int row = 0; row < rowCount; row++) {
      updateRow(rowCount - 1 - row, false);
    }
  }
  else if (displayData[3] == FLIPPITY210_ANIM_SLIDE_FROM_BOTTOM_FORCED) {
    for (int row = 0; row < rowCount; row++) {
      updateRow(rowCount - 1 - row, true);
    }
  }
#endif
#ifdef ENABLE_ANIMATION_DISSOLVE
  else if (displayData[3] == FLIPPITY210_ANIM_DISSOLVE) {
    dissolveDots();
  }
#endif

  else { // default animation FLIPPITY210_ANIM_SLIDE_RIGHT
    for (int dispNumber = 0; dispNumber < displaysCount; dispNumber++) {
      dotsSelectDisplay(dispNumber);
      for (int col = 0; col < columnCount; col++) {
        updateCol(dispNumber, col, false);
      }
    }
  }

  for (int dispNumber = 0; dispNumber < displaysCount; dispNumber++) {
    dotsSelectDisplay(dispNumber);
    for (int i = 0; i < rowCount; i++) {
      dotsActive[dispNumber][i] = dotsBuffer[dispNumber][i];
    }
  }

#ifdef USE_SERIAL
  Serial.print("[DISPLAY] Done, time: ");
  Serial.println(millis() - millisUpdate);
#endif

  setDisplayStatus(FLIPPITY210_STATUS_READY);
}
