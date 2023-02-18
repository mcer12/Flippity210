void shiftSetValue(volatile byte *data, uint8_t pin, bool value) {
  (value) ? bitSet(data[pin / 8], pin % 8) : bitClear(data[pin / 8], pin % 8);
}


/*
  void ICACHE_RAM_ATTR shiftSetAll(bool value) {
  for (int i = 0; i < registersCount * 8; i++) {
    //(value) ? bitSet(bytes[pinsToRegisterMap[i]], pinsToBitsMap[i]) : bitClear(bytes[pinsToRegisterMap[i]], pinsToBitsMap[i]);
    (value) ? bitSet(bytes[i / 8], i % 8) : bitClear(bytes[i / 8], i % 8);
  }
  }
*/
void shiftOutFast(PinName dataPin, PinName clockPin, bool MSBFIRST, uint8_t command)
{
  for (int i = 0; i < 8; i++)
  {
    bool output = false;
    if (MSBFIRST)
    {
      output = command & 0b10000000;
      command = command << 1;
    }
    else
    {
      output = command & 0b00000001;
      command = command >> 1;
    }
    digitalWriteFast(dataPin, output);
    digitalWriteFast(clockPin, true);
    delayMicroseconds(1);
    digitalWriteFast(clockPin, false);
    delayMicroseconds(1);
  }
}

void ledsWriteBytesToPin(PinName dataPin, volatile byte *data) {
  digitalWriteFast(LEDS_LATCH, 0);
  for (int i = 0; i < ledsRegistersCount; i++) {
    //shiftOut(pinNametoDigitalPin(LEDS_D0), pinNametoDigitalPin(LEDS_CLK), MSBFIRST, data[ledsRegistersCount - 1 - i]);
    //shiftOut(pinNametoDigitalPin(LEDS_D1), pinNametoDigitalPin(LEDS_CLK), MSBFIRST, data[ledsRegistersCount - 1 - i]);
    shiftOutFast(dataPin, LEDS_CLK, MSBFIRST, data[ledsRegistersCount - 1 - i]);
  }
  digitalWriteFast(LEDS_LATCH, 1);
}


void shiftOutFast4(PinName dataPin1, PinName dataPin2, PinName dataPin3, PinName dataPin4, byte command1, byte command2, byte command3, byte command4, PinName clockPin,  bool MSBFIRST)
{
  for (int i = 0; i < 8; i++)
  {
    bool output1 = false;
    bool output2 = false;
    bool output3 = false;
    bool output4 = false;
    
    if (MSBFIRST)
    {
      output1 = command1 & 0b10000000;
      command1 = command1 << 1;
      output2 = command2 & 0b10000000;
      command2 = command2 << 1;
      output3 = command3 & 0b10000000;
      command3 = command3 << 1;
      output4 = command4 & 0b10000000;
      command4 = command4 << 1;
    }
    else
    {
      output1 = command1 & 0b00000001;
      command1 = command1 >> 1;
      output2 = command2 & 0b00000001;
      command2 = command2 >> 1;
      output3 = command3 & 0b00000001;
      command3 = command3 >> 1;
      output4 = command4 & 0b00000001;
      command4 = command4 >> 1;
    }
    digitalWriteFast(dataPin1, output1);
    digitalWriteFast(dataPin2, output2);
    digitalWriteFast(dataPin3, output3);
    digitalWriteFast(dataPin4, output4);
    digitalWriteFast(clockPin, true);
    delayMicroseconds(1);
    digitalWriteFast(clockPin, false);
    delayMicroseconds(1);
  }
}

void ledsWriteBytesToPins(PinName dataPin1, PinName dataPin2, PinName dataPin3, PinName dataPin4, volatile byte *data1, volatile byte *data2, volatile byte *data3, volatile byte *data4) {
  digitalWriteFast(LEDS_LATCH, 0);
  for (int i = 0; i < ledsRegistersCount; i++) {
    shiftOutFast4(dataPin1, dataPin2, dataPin3, dataPin4, data1[ledsRegistersCount - 1 - i], data2[ledsRegistersCount - 1 - i], data3[ledsRegistersCount - 1 - i], data4[ledsRegistersCount - 1 - i], LEDS_CLK, MSBFIRST);
  }
  digitalWriteFast(LEDS_LATCH, 1);
}

void spi2WriteBytes(volatile byte *data) {

  //SPI_2.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

  digitalWrite(ROWS_LATCH, 0);
  for (int i = 0; i < registersCount; i++) {
    //SPI_2.transfer(data[registersCount - 1 - i]);
    shiftOut(ROWS_DATA, ROWS_CLK, MSBFIRST, data[registersCount - 1 - i]);
  }
  //SPI_2.endTransaction();
  digitalWrite(ROWS_LATCH, 1);
}


void ledsWriteBytes(volatile byte *data) {

#ifdef USE_SPI
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
  for (int i = 0; i < ledsRegistersCount; i++) {
    SPI.transfer(data[ledsRegistersCount - 1 - i]);
  }
  SPI.endTransaction();
#endif

#ifndef USE_SPI
  digitalWriteFast(LEDS_LATCH, 0);
  for (int i = 0; i < ledsRegistersCount; i++) {
    //shiftOut(pinNametoDigitalPin(LEDS_D0), pinNametoDigitalPin(LEDS_CLK), MSBFIRST, data[ledsRegistersCount - 1 - i]);
    //shiftOut(pinNametoDigitalPin(LEDS_D1), pinNametoDigitalPin(LEDS_CLK), MSBFIRST, data[ledsRegistersCount - 1 - i]);
    shiftOutFast(LEDS_D1, LEDS_CLK, MSBFIRST, data[ledsRegistersCount - 1 - i]);
  }
  digitalWriteFast(LEDS_LATCH, 1);
#endif
}
