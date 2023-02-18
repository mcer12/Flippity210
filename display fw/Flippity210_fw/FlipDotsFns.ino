void setLocalPixel(int x, int y, bool value, int display) {
  bitWrite(dotsBuffer[display][y], x, value);
}

void setPixel(int x, int y, bool value) {
  int displayNumber = x / columnCount;
  x = x % columnCount;
  setLocalPixel(x, y, value, displayNumber);
}

/* wont fit in STM32F103C6T6
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
  for (int dispNumber; dispNumber < displaysLimit; dispNumber++) {
    for (int i = 0; i < rowCount; i++) {
      dotsBuffer[dispNumber][i] = 0;
    }
  }
}

void fillAll() {
  for (int dispNumber; dispNumber < displaysLimit; dispNumber++) {
    for (int i = 0; i < rowCount; i++) {
      dotsBuffer[dispNumber][i] = UINT32_MAX;
    }
  }
}

void invertAll() {
  for (int dispNumber; dispNumber < displaysLimit; dispNumber++) {
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

  digitalWriteFast(SINK_EN, 0);
  digitalWriteFast(SOURCE_EN, 0);
  digitalWrite(ROWS_DATA, 0);
  digitalWrite(ROWS_LATCH, 0);
  digitalWrite(ROWS_CLK, 0);

  pinMode(pinNametoDigitalPin(COLS_ENABLE), OUTPUT);
  pinMode(pinNametoDigitalPin(PANEL_ADDR_B0), OUTPUT);
  pinMode(pinNametoDigitalPin(PANEL_ADDR_B1), OUTPUT);
  pinMode(pinNametoDigitalPin(PANEL_ADDR_B2), OUTPUT);

  digitalWriteFast(COLS_ENABLE, 0);
  digitalWriteFast(PANEL_ADDR_B0, 1);
  digitalWriteFast(PANEL_ADDR_B1, 0);
  digitalWriteFast(PANEL_ADDR_B2, 0);
}

void dotsSelectDisplay(uint8_t displayNumber) {
  digitalWriteFast(PANEL_ADDR_B0, bitRead(displayNumber, 0));
  digitalWriteFast(PANEL_ADDR_B1, bitRead(displayNumber, 1));
  digitalWriteFast(PANEL_ADDR_B2, bitRead(displayNumber, 2));
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
  shiftSetValue(bytes, GROUP_ADDR_B0, 0); // 1 = 24V, 0 = GND

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
    delayMicroseconds(500);
    digitalWriteFast(COLS_ENABLE, 0);
    digitalWriteFast(SOURCE_EN, 0);
    delayMicroseconds(flipDotUpdateDelay);
  }

}

void dotsUnflipCol(uint8_t dispNumber, uint8_t colNumber, bool forceChange) {

  dotsSelectCol(colNumber);
  shiftSetValue(bytes, GROUP_ADDR_B0, 1); // 1 = 24V, 0 = GND

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
    delayMicroseconds(500);
    digitalWriteFast(COLS_ENABLE, 0);
    digitalWriteFast(SINK_EN, 0);
    delayMicroseconds(flipDotUpdateDelay);
  }
}

void updateCol(uint8_t dispNumber, uint8_t colNumber, bool forceChange) {
  dotsUnflipCol(dispNumber, colNumber, forceChange);
  dotsFlipCol(dispNumber, colNumber, forceChange);

}

void dotsFlipRow(uint8_t row, bool forceChange) {
  shiftSetValue(bytes, GROUP_ADDR_B0, 0); // 1 = 24V, 0 = GND

  for (int dispNumber = 0; dispNumber < displaysLimit; dispNumber++) {
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
      delayMicroseconds(500);
      digitalWriteFast(COLS_ENABLE, 0);
      digitalWriteFast(SOURCE_EN, 0);
      delayMicroseconds(flipDotUpdateDelay);
    }
  }
}

void dotsUnflipRow(uint8_t row, bool forceChange) {
  shiftSetValue(bytes, GROUP_ADDR_B0, 1); // 1 = 24V, 0 = GND

  for (int dispNumber = 0; dispNumber < displaysLimit; dispNumber++) {
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
      delayMicroseconds(500);
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
}
void setDisplaySpeed(byte speed) {
  displayData[1] = speed;
  
  if (speed == PAYLOAD_SPEED_HIGH) {
    flipDotUpdateDelay = 100;
  }
  else if (speed == PAYLOAD_SPEED_MED) {
    flipDotUpdateDelay = 1000;
  }
  else if (speed == PAYLOAD_SPEED_LOW) {
    flipDotUpdateDelay = 2000;
  }
  else if (speed == PAYLOAD_SPEED_VERY_LOW) {
    flipDotUpdateDelay = 4000;
  }
  else if (speed == PAYLOAD_SPEED_EXTREMELY_LOW) {
    flipDotUpdateDelay = 10000;
  }
}
void setDisplayBri(byte bri) {
  displayData[2] = bri;
}
void setDisplayAnimation(byte anim) {
  displayData[3] = anim;
}

void refreshDisplays() {
  setDisplayStatus(PAYLOAD_STATUS_BUSY);

#ifdef USE_SERIAL
  Serial.println("[DISPLAY] Updating...");
#endif

  unsigned long millisUpdate = millis();

  if (displayData[3] == PAYLOAD_ANIM_FAST) {
    for (int dispNumber = 0; dispNumber < displaysLimit; dispNumber++) {
      dotsSelectDisplay(dispNumber);
      for (int col = 0; col < columnCount; col++) {
        updateCol(dispNumber, col, false);
      }
    }
  }
  else if (displayData[3] == PAYLOAD_ANIM_FAST_F) {
    for (int dispNumber = 0; dispNumber < displaysLimit; dispNumber++) {
      dotsSelectDisplay(dispNumber);
      for (int col = 0; col < columnCount; col++) {
        updateCol(dispNumber, col, true);
      }
    }
  }
  else if (displayData[3] == PAYLOAD_ANIM_FAST_INV) {
    for (int dispNumber = 0; dispNumber < displaysLimit; dispNumber++) {
      dotsSelectDisplay(displaysLimit - 1 - dispNumber);
      for (int col = 0; col < columnCount; col++) {
        updateCol(displaysLimit - 1 - dispNumber, columnCount - 1 - col, false);
      }
    }
  }
  else if (displayData[3] == PAYLOAD_ANIM_FAST_INV_F) {
    for (int dispNumber = 0; dispNumber < displaysLimit; dispNumber++) {
      dotsSelectDisplay(displaysLimit - 1 - dispNumber);
      for (int col = 0; col < columnCount; col++) {
        updateCol(displaysLimit - 1 - dispNumber, columnCount - 1 - col, true);
      }
    }
  }
  else if (displayData[3] == PAYLOAD_ANIM_FAST_VER) {
    for (int row = 0; row < rowCount; row++) {
      updateRow(row, false);
    }
  }
  else if (displayData[3] == PAYLOAD_ANIM_FAST_VER_F) {
    for (int row = 0; row < rowCount; row++) {
      updateRow(row, true);
    }
  }
  else if (displayData[3] == PAYLOAD_ANIM_FAST_VER_INV) {
    for (int row = 0; row < rowCount; row++) {
      updateRow(rowCount - 1 - row, false);
    }
  }
  else if (displayData[3] == PAYLOAD_ANIM_FAST_VER_INV_F) {
    for (int row = 0; row < rowCount; row++) {
      updateRow(rowCount - 1 - row, true);
    }
  }

  for (int dispNumber = 0; dispNumber < displaysLimit; dispNumber++) {
    dotsSelectDisplay(dispNumber);
    for (int i = 0; i < rowCount; i++) {
      dotsActive[dispNumber][i] = dotsBuffer[dispNumber][i];
    }
  }
#ifdef USE_SERIAL
  Serial.print("[DISPLAY] Done, time: ");
  Serial.println(millis() - millisUpdate);
#endif
  
  setDisplayStatus(PAYLOAD_STATUS_READY);
}
