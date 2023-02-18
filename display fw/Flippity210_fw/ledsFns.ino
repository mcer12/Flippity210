void initLeds() {
  pinMode(pinNametoDigitalPin(LEDS_EN), OUTPUT);
  pinMode(pinNametoDigitalPin(LEDS_ADD1), OUTPUT);
  pinMode(pinNametoDigitalPin(LEDS_ADD2), OUTPUT);
  pinMode(pinNametoDigitalPin(LEDS_A0), OUTPUT);
  pinMode(pinNametoDigitalPin(LEDS_A1), OUTPUT);
  pinMode(pinNametoDigitalPin(LEDS_A2), OUTPUT);
  pinMode(pinNametoDigitalPin(LEDS_D0), OUTPUT);
  pinMode(pinNametoDigitalPin(LEDS_D1), OUTPUT);
  pinMode(pinNametoDigitalPin(LEDS_D2), OUTPUT);
  pinMode(pinNametoDigitalPin(LEDS_D3), OUTPUT);
  pinMode(pinNametoDigitalPin(LEDS_D4), OUTPUT);
  pinMode(pinNametoDigitalPin(LEDS_D5), OUTPUT);
  pinMode(pinNametoDigitalPin(LEDS_D6), OUTPUT);
  pinMode(pinNametoDigitalPin(LEDS_D7), OUTPUT);
  pinMode(pinNametoDigitalPin(LEDS_LATCH), OUTPUT);
  pinMode(pinNametoDigitalPin(LEDS_CLK), OUTPUT);

  digitalWriteFast(LEDS_ADD1, false);
  digitalWriteFast(LEDS_ADD2, false);
  digitalWriteFast(LEDS_A0, false);
  digitalWriteFast(LEDS_A1, false);
  digitalWriteFast(LEDS_A2, false);
  digitalWriteFast(LEDS_D0, false);
  digitalWriteFast(LEDS_D1, false);
  digitalWriteFast(LEDS_D2, false);
  digitalWriteFast(LEDS_D3, false);
  digitalWriteFast(LEDS_D4, false);
  digitalWriteFast(LEDS_D5, false);
  digitalWriteFast(LEDS_D6, false);
  digitalWriteFast(LEDS_D7, false);
  digitalWriteFast(LEDS_LATCH, false);
  digitalWriteFast(LEDS_CLK, false);



#ifdef USE_SPI
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE1);
  SPI.setClockDivider(SPI_CLOCK_DIV16);
#endif

  ledsTimer.attachInterruptInterval(LEDS_INTERVAL_MICROS, ledsTimerHandler);

}

void toggleLeds(byte state) {
  if (state == PAYLOAD_LED_PWR_ON) {
    digitalWriteFast(LEDS_EN, false); // LEDs enable is inverted
  }
}

void selectLedsRow(byte rowNumber) {

  // This is very time critical
  // we need to set values first
  // reading bits in between GPIO manipulation can cause glitches
  uint8_t values[] = {
    bitRead(rowNumber, 0),
    bitRead(rowNumber, 1),
    bitRead(rowNumber, 2),
    bitRead(rowNumber, 3),
    bitRead(rowNumber, 4),
    bitRead(rowNumber, 5)
  };

  digitalWriteFast(LEDS_ADD2, values[3]);
  digitalWriteFast(LEDS_ADD1, values[4]);
  digitalWriteFast(LEDS_A0, values[0]);
  digitalWriteFast(LEDS_A1, values[1]);
  digitalWriteFast(LEDS_A2, values[2]);
}

void ledsTimerHandler()
{
  /*
      for (int i = 0; i < ledsRegistersCount; i++) { // reset all to zero because only one row can be flipped at a time!!!!! Otherwise magic smoke.
        //shiftSetValue(bytesLeds, i, 1);
        bytesLeds[0] = 1;
      }
  */

  toggle++;
  if (toggle > 1) toggle = 0;

  if (toggle == 0) {

    for (int dispNumber; dispNumber < displaysLimit; dispNumber++) {
      for (int col = 0; col < columnCount; col++) {
        shiftSetValue(bytesLeds[dispNumber], col, bitRead(ledsBuffer[dispNumber][rowCount - 1 - row], col));
      }
    }
    ledsWriteBytesToPins(LEDS_D0, LEDS_D1, LEDS_D2, LEDS_D3, bytesLeds[0], bytesLeds[1], bytesLeds[2], bytesLeds[3]); // Display 1 and 2

    selectLedsRow(row);
    row++;
    if (row == rowCount) row = 0;

  }
  if (toggle == 1) {
    selectLedsRow(20); // pin 7 of the multiplexer
  }

}
