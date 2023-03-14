/*
#ifdef __AVR__
#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_MBED_RP2040)|| defined(ARDUINO_ARCH_RP2040)
#include <pgmspace.h>
#else
#define pgm_read_byte(addr)                                                    \
  (*(const unsigned char *)(addr)) ///< PROGMEM workaround for non-AVR
#endif
*/

#if !defined(__ARM_ARCH) && !defined(ENERGIA) && !defined(ESP8266) &&          \
    !defined(ESP32) && !defined(__arc__)
#include <util/delay.h>
#endif

#include "Flippity210.h"

Flippity210::Flippity210(uint8_t displays, int16_t sda, int16_t scl, uint8_t addr, TwoWire *twi) : Adafruit_GFX(displays * FLIPPITY210_SCREEN_RES_W, FLIPPITY210_SCREEN_RES_H), wire(twi ? twi : &Wire)
{

	_displays = displays;
	_addr = addr;
	_sda = sda;
	_scl = scl;
	_pixel_skip = (FLIPPITY210_BYTES_PER_ROW * 8) - FLIPPITY210_SCREEN_RES_W;
}

bool Flippity210::begin() {
 if ((!buffer) && !(buffer = (uint8_t*)malloc(FLIPPITY210_BYTES_PER_ROW * FLIPPITY210_SCREEN_RES_H * _displays))){
 return false;
 }

if(_sda >= 0 && _scl >= 0){
#if defined(ARDUINO_ARDUINO_NANO33BLE) || defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040) || defined(STM32F1xx)
    wire->setSDA(_sda);
    wire->setSCL(_scl);
#elif defined(ESP32)
    wire->setPins(_sda, _scl);
#elif defined(ESP8266)
    wire->begin(_sda, _scl);
#endif

} else{
#if defined(ESP8266)
    wire->begin();
#endif
}
#if !defined(ESP8266)
  wire->begin();
#endif

    clear();

    return true;
}

void Flippity210::drawPixel(int16_t x, int16_t y, uint16_t color) {

if(x >= width() || y >= height()) return;

bool val = color;
    switch (getRotation()) {
    case 1:
      y = height() - y - 1;
    flippity210_swap(x,y);
      break;
    case 2:
      x = width() - x - 1;
      y = height() - y - 1;
      break;
    case 3:
      x = width() - x - 1;
flippity210_swap(x,y);
      break;
    }

x = x + (_pixel_skip * (x / FLIPPITY210_SCREEN_RES_W));
  bitWrite(buffer[(x / 8) + (y * FLIPPITY210_BYTES_PER_ROW * _displays )], x % 8 , val);

  //bitWrite(buffer, (x + (y * ((FLIPPITY210_SCREEN_RES_W + 7) / 8 ) * 8 * (display + 1))), val);
}
/*
void Flippity210::writePixel(int16_t x, int16_t y, uint16_t color) {
bool val = color;
  bitWrite(buffer[(x / 8) + (y * FLIPPITY210_BYTES_PER_ROW * _displays )], x % 8 , val);

  //bitWrite(buffer, (x + (y * ((FLIPPITY210_SCREEN_RES_W + 7) / 8 ) * 8 * (display + 1))), val);
}
*/


void Flippity210::clearDisplay(uint8_t display) {
  for (int x = 0; x < FLIPPITY210_SCREEN_RES_W; x++) {
      for (int y = 0; y < FLIPPITY210_SCREEN_RES_H; y++) {
        bitWrite(buffer[(display + 1) * x], y, 0);
      }
  }
}


void Flippity210::fillDisplay(uint8_t display) {
  for (int x = 0; x < FLIPPITY210_SCREEN_RES_W; x++) {
      for (int y = 0; y < FLIPPITY210_SCREEN_RES_H; y++) {
        bitWrite(buffer[(display + 1) * x], y, 1);
      }
  }
}

void Flippity210::invertDisplay(uint8_t display) {
  for (int x = 0; x < FLIPPITY210_SCREEN_RES_W; x++) {
      for (int y = 0; y < FLIPPITY210_SCREEN_RES_H; y++) {
        bitWrite(buffer[(display + 1) * x], y, !bitRead(buffer[(display + 1) * x], y));
      }
  }
}


void Flippity210::clear() {
  for (int i = 0; i < FLIPPITY210_BYTES_PER_ROW * FLIPPITY210_SCREEN_RES_H * _displays; i++) {
        buffer[i] = 0;
  }
}

void Flippity210::fill() {
  for (int i = 0; i < FLIPPITY210_BYTES_PER_ROW * FLIPPITY210_SCREEN_RES_H * _displays; i++) {
        buffer[i] = 0xff;
  }
}

void Flippity210::invert() {
      for (int i = 0; i < FLIPPITY210_BYTES_PER_ROW * FLIPPITY210_SCREEN_RES_H * _displays; i++) {
            buffer[i] = ~buffer[i];
      }
}


void Flippity210::setBrightness(uint8_t bri) {
  command(FLIPPITY210_COMMAND_SET_BRIGHTNESS, bri);
}

void Flippity210::setAnimationSpeed(uint8_t speed) {
  command(FLIPPITY210_COMMAND_SET_SPEED, speed);
}

void Flippity210::setAnimation(uint8_t animation) {
  command(FLIPPITY210_COMMAND_SET_ANIMATION, animation);
}

void Flippity210::setLedsPower(bool powerOn) {
  if (powerOn) {
    command(FLIPPITY210_COMMAND_TOGGLE_LED_POWER, FLIPPITY210_LED_PWR_ON);
  } else {
    command(FLIPPITY210_COMMAND_TOGGLE_LED_POWER, FLIPPITY210_LED_PWR_OFF);
  }
}

void Flippity210::update() {
  sendBuffer(FLIPPITY210_COMMAND_UPDATE_BOTH, buffer, false);
}

void Flippity210::updateDots() {
  sendBuffer(FLIPPITY210_COMMAND_UPDATE_DOTS, buffer, false);
}

void Flippity210::updateLeds() {
  sendBuffer(FLIPPITY210_COMMAND_UPDATE_LEDS, buffer, false);
}



void Flippity210::command(uint8_t command, uint8_t data) {
  wire->beginTransmission(_addr); // transmit to device #8
  wire->write(command); // sending the designator
  wire->write(data); // sending the buffer in 4 chunks
  wire->endTransmission();    // stop transmitting
}

bool Flippity210::sendBuffer(uint8_t command, uint8_t *data, bool async) {

  if (!async) {
    int i = 0;
    while (getBusy()) {
      //Serial.print(".");
      delay(20);
      i++;
      if (i > 300) {
        //Serial.println("i2c busy timeout");
        return false;
      }
    }
    //Serial.println("");
  }


  uint8_t payload[FLIPPITY210_BYTES_PER_ROW];


    for (int row = 0; row < FLIPPITY210_SCREEN_RES_H; row++) {
        for(int display = 0; display < _displays; display++){
            int startByte = (FLIPPITY210_BYTES_PER_ROW * display) + (row * FLIPPITY210_BYTES_PER_ROW * _displays);

              wire->beginTransmission(_addr); // transmit to device #8
              wire->write(command); // sending the designator
              wire->write(display); // display number
              wire->write(row); // row number

            for (int b = 0; b < FLIPPITY210_BYTES_PER_ROW; b++) {
                  payload[b] = buffer[startByte + b];
            }

          wire->write(payload, FLIPPITY210_BYTES_PER_ROW); // sending the designator
          wire->endTransmission();    // stop transmitting
        }
    }
  //delay(1); // needed to prevent display glitches
  return true;
}

uint8_t *Flippity210::getStatus() {
  uint8_t iterator = 0;
  uint8_t size = 5;
  wire->requestFrom(_addr, size);    // request 5 bytes

  while (wire->available()) { // slave may send less than requested
    _status[iterator] = wire->read(); // receive a byte as character
    iterator++;
  }
  return _status;
}

bool Flippity210::getBusy() {
  uint8_t *value = getStatus();
  if(value[0] == FLIPPITY210_STATUS_READY) return false;
  else return true;
  return false;
}

uint8_t *Flippity210::getBuffer() {
  return buffer;
}