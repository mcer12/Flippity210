void i2cInit() {
  //Wire.setClock(400000);
  Wire.begin(I2C_ADDR); // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent); // register event
}

unsigned char reverse(unsigned char b) {
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}

void receiveEvent(int howMany)
{
  int32_t bigNum;
  byte command;
  byte wireBuffer[(displaysCount * 4 * rowCount) + 1]; // command byte + 1 byte or 4 bytes per display
  byte wireBufferFlipped[(displaysCount * 4 * rowCount) + 1];
  int currentDisplay = 0;
  int iterator = 0;
  receivingData = true;

  while (Wire.available()) // loop through all but the last
  {
    wireBuffer[iterator] = Wire.read();    // receive byte as an integer
    //Serial.println(wireBuffer[iterator]);         // print the integer
    iterator++;
  }
  //Serial.print("Action: ");
  //Serial.println(wireBuffer[0]);

  if (wireBuffer[0] == FLIPPITY210_COMMAND_SET_SPEED) {
    setDisplaySpeed(wireBuffer[1]);
  }
  if (wireBuffer[0] == FLIPPITY210_COMMAND_SET_ANIMATION) {
    setDisplayAnimation(wireBuffer[1]);
  }
  if (wireBuffer[0] == FLIPPITY210_COMMAND_SET_BRIGHTNESS) {
    setDisplayBri(wireBuffer[1]);
  }
  if (wireBuffer[0] == FLIPPITY210_COMMAND_TOGGLE_LED_POWER) {
    toggleLeds(wireBuffer[1]);
  }

  if (wireBuffer[0] == FLIPPITY210_COMMAND_UPDATE_BOTH || wireBuffer[0] == FLIPPITY210_COMMAND_UPDATE_LEDS || wireBuffer[0] == FLIPPITY210_COMMAND_UPDATE_DOTS) {
    //Serial.print("display: ");
    //Serial.print(wireBuffer[1]);
    //Serial.print(", row: ");
    //Serial.println(wireBuffer[2]);

    bigNum = wireBuffer[6];
    bigNum = (bigNum << 8) | wireBuffer[5];
    bigNum = (bigNum << 8) | wireBuffer[4];
    bigNum = (bigNum << 8) | wireBuffer[3];

    if (wireBuffer[0] == FLIPPITY210_COMMAND_UPDATE_BOTH) {
      dotsBuffer[wireBuffer[1]][wireBuffer[2]] = bigNum; // save to buffer
      ledsBuffer[wireBuffer[1]][wireBuffer[2]] = bigNum; // save to buffer
    }
    if (wireBuffer[0] == FLIPPITY210_COMMAND_UPDATE_LEDS) {
      ledsBuffer[wireBuffer[1]][wireBuffer[2]] = bigNum; // save to buffer
    }
    if (wireBuffer[0] == FLIPPITY210_COMMAND_UPDATE_DOTS) {
      dotsBuffer[wireBuffer[1]][wireBuffer[2]] = bigNum; // save to buffer
    }
    /*
        Serial.print("data: ");
        Serial.print(wireBuffer[3]);
        Serial.print(" ");
        Serial.print(wireBuffer[4]);
        Serial.print(" ");
        Serial.print(wireBuffer[5]);
        Serial.print(" ");
        Serial.print(wireBuffer[6]);
        Serial.println("");
    */
    if (wireBuffer[2] == rowCount - 1 && wireBuffer[1] == 1) {
      receivingData = false;
      dataReady = true;
    }
  } else {
    receivingData = false;
    dataReady = true;
  }
}

void requestEvent()
{
  Wire.write(displayData, 5);
  /*
    Serial.print("Sent status: ");
    for (int i = 0; i < 5; i++) {
    Serial.print(displayData[i]);
    }
    Serial.println("");
  */
}
