

/*
  #define COMMAND_UPDATE_BOTH 0x20
  #define COMMAND_UPDATE_LEDS 0x21
  #define COMMAND_UPDATE_DOTS 0x22
  #define COMMAND_SET_SPEED 0x23
  #define COMMAND_SET_ANIMATION 0x24
  #define COMMAND_SET_BRIGHTNESS 0x25
  #define COMMAND_TOGGLE_LED_POWER 0x26

    #define CLOSING_BYTE 0x250

  #define PAYLOAD_SPEED_LOW 0x01
  #define PAYLOAD_SPEED_MED 0x02
  #define PAYLOAD_SPEED_HIGH 0x03

  #define PAYLOAD_BRI_LOW 0x01
  #define PAYLOAD_BRI_MED 0x02
  #define PAYLOAD_BRI_HIGH 0x03

  #define PAYLOAD_ANIM_FAST 0x01
  #define PAYLOAD_ANIM_FAST_INV 0x02
  #define PAYLOAD_ANIM_FAST_VER 0x03
  #define PAYLOAD_ANIM_FAST_VER_INV 0x04
  #define PAYLOAD_ANIM_FAST_F 0x05
  #define PAYLOAD_ANIM_FAST_INV_F 0x06
  #define PAYLOAD_ANIM_FAST_VER_F 0x07
  #define PAYLOAD_ANIM_FAST_VER_INV_F 0x08

  #define PAYLOAD_PWR_ON 0x11
  #define PAYLOAD_PWR_OFF 0x10

  #define PAYLOAD_STATUS_READY 0x11
  #define PAYLOAD_STATUS_BUSY 0x10
*/


void i2cInit() {
  //Wire.setClock(400000);
  Wire.begin(I2C_ADDR); // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent); // register event
}

void receiveEvent(int howMany)
{
  int32_t bigNum;
  byte command;
  byte wireBuffer[(displaysLimit * 4 * rowCount) + 1]; // command byte + 1 byte or 4 bytes per display
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

  if (wireBuffer[0] == COMMAND_SET_SPEED) {
    setDisplaySpeed(wireBuffer[1]);
  }
  if (wireBuffer[0] == COMMAND_SET_ANIMATION) {
    setDisplayAnimation(wireBuffer[1]);
  }
  if (wireBuffer[0] == COMMAND_SET_BRIGHTNESS) {
    setDisplayBri(wireBuffer[1]);
  }
  if (wireBuffer[0] == COMMAND_TOGGLE_LED_POWER) {
    setDisplayBri(wireBuffer[1]);
  }

  if (wireBuffer[0] == COMMAND_UPDATE_BOTH || wireBuffer[0] == COMMAND_UPDATE_LEDS || wireBuffer[0] == COMMAND_UPDATE_DOTS) {
    //Serial.print("displays to update: ");
    //Serial.println(displaysToUpdate);

    bigNum = wireBuffer[3];
    bigNum = (bigNum << 8) | wireBuffer[4];
    bigNum = (bigNum << 8) | wireBuffer[5];
    bigNum = (bigNum << 8) | wireBuffer[6];

    if (wireBuffer[0] == COMMAND_UPDATE_BOTH) {
      dotsBuffer[wireBuffer[1]][wireBuffer[2]] = bigNum; // save to buffer
      ledsBuffer[wireBuffer[1]][wireBuffer[2]] = bigNum; // save to buffer
    }
    if (wireBuffer[0] == COMMAND_UPDATE_LEDS) {
      ledsBuffer[wireBuffer[1]][wireBuffer[2]] = bigNum; // save to buffer
    }
    if (wireBuffer[0] == COMMAND_UPDATE_DOTS) {
      dotsBuffer[wireBuffer[1]][wireBuffer[2]] = bigNum; // save to buffer
    }

    //Serial.print("data: ");
    //Serial.println(bigNum);

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
