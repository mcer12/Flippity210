
#ifndef Flippity210_h

#define Flippity210_h

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>

#define FLIPPITY210_SCREEN_RES_W 28
#define FLIPPITY210_SCREEN_RES_H 19
#define FLIPPITY210_BYTES_PER_ROW ((FLIPPITY210_SCREEN_RES_W + 7) / 8)

#define FLIPPITY210_BLACK 0   ///< Draw 'off' pixels
#define FLIPPITY210_WHITE 1   ///< Draw 'on' pixels

#define FLIPPITY210_ADDR_MASTER_DEFAULT 0x42
#define FLIPPITY210_ADDR_SLAVE_DEFAULT 0x08

#define FLIPPITY210_COMMAND_UPDATE_BOTH 0x20
#define FLIPPITY210_COMMAND_UPDATE_LEDS 0x21
#define FLIPPITY210_COMMAND_UPDATE_DOTS 0x22
#define FLIPPITY210_COMMAND_SET_SPEED 0x23
#define FLIPPITY210_COMMAND_SET_ANIMATION 0x24
#define FLIPPITY210_COMMAND_SET_BRIGHTNESS 0x25
#define FLIPPITY210_COMMAND_TOGGLE_LED_POWER 0x26

#define FLIPPITY210_SPEED_HIGH 0x01
#define FLIPPITY210_SPEED_MED 0x02
#define FLIPPITY210_SPEED_LOW 0x03
#define FLIPPITY210_SPEED_VERY_LOW 0x04
#define FLIPPITY210_SPEED_EXTREMELY_LOW 0x05
#define FLIPPITY210_SPEED_LOWEST 0x06

#define FLIPPITY210_BRI_LOW 0x01
#define FLIPPITY210_BRI_MED 0x02
#define FLIPPITY210_BRI_HIGH 0x03

#define FLIPPITY210_ANIM_SLIDE_RIGHT 0x01
#define FLIPPITY210_ANIM_SLIDE_LEFT 0x02
#define FLIPPITY210_ANIM_SLIDE_FROM_TOP 0x03
#define FLIPPITY210_ANIM_SLIDE_FROM_BOTTOM 0x04
#define FLIPPITY210_ANIM_DISSOLVE 0x05
#define FLIPPITY210_ANIM_SLIDE_RIGHT_FORCED 0x06
#define FLIPPITY210_ANIM_SLIDE_LEFT_FORCED 0x07
#define FLIPPITY210_ANIM_SLIDE_FROM_TOP_FORCED 0x08
#define FLIPPITY210_ANIM_SLIDE_FROM_BOTTOM_FORCED 0x09

#define FLIPPITY210_LED_PWR_ON 0x11
#define FLIPPITY210_LED_PWR_OFF 0x10

#define FLIPPITY210_STATUS_READY 0x11
#define FLIPPITY210_STATUS_BUSY 0x10

#define flippity210_swap(a, b) (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b))) ///< No-temp-var swap operation

class Flippity210 : public Adafruit_GFX {

public:

Flippity210(uint8_t displays = 1, int16_t sda = -1, int16_t scl = -1, uint8_t addr = FLIPPITY210_ADDR_SLAVE_DEFAULT, TwoWire *twi = &Wire);

bool begin(void);
void drawPixel(int16_t x, int16_t y, uint16_t color);
void clearDisplay(uint8_t display);
void fillDisplay(uint8_t display);
void invertDisplay(uint8_t display);
void clear(void);
void fill(void);
void invert(void);
void update(void);
void updateLeds(void);
void updateDots(void);
void setBrightness(uint8_t b);
void setAnimationSpeed(uint8_t b);
void setAnimation(uint8_t b);
void setLedsPower(bool on);
uint8_t *getBuffer(void);

void command(uint8_t command, uint8_t data);
uint8_t *getStatus(void);
bool getBusy(void);

protected:

TwoWire *wire;
uint8_t *buffer;
uint8_t _status[5];
uint8_t _displays;
uint8_t _addr;
int16_t _sda;
int16_t _scl;
uint8_t _pixel_skip;

bool sendBuffer(uint8_t command, uint8_t *data, bool async);

};

#endif
