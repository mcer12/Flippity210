/*

  IDE SETTINGS:
  UART: Enabled
  USB: Enabled (disable for low memory chips)
  Optimize: Fastest + LTO
  Upload method: SWD (or HID if you flashed HID bootloader beforehand)


  FW SIZE
  STM32F103C6T6 (32k flash):
  sketch uses 98% of memory with USB support disabled with either DISPLAY_COUNT_LIMIT set to 2 or some animations disabled
  STM32F103C8T6 (64k flash):
  sketch uses 51% of memory with USB support disabled
  sketch uses 78% of memory with USB support & flashing via HID bootloader


  COMMANDS

  Action (first byte)
  0x20 - update flip dots
  0x21 - update leds
  0x22 - update both
  0x23 - set speed
  0x24 - set animation
  0x25 - set brightness (leds only)
  0x26 - toggle leds power

  Display refresh speed (only for flip dots) 0x23
  SPEED_HIGH 0x01
  SPEED_MED 0x02
  SPEED_LOW 0x03
  SPEED_VERY_LOW 0x04 // low speeds look bad with forced update, but interesting without it
  SPEED_EXTREMELY_LOW 0x05

  Brightness (only for leds) 0x25
  0x01 - low
  0x02 - med
  0x03 - high

  Animation 0x24

  FLIPPITY210_ANIM_SLIDE_RIGHT 0x01
  FLIPPITY210_ANIM_SLIDE_LEFT 0x02
  FLIPPITY210_ANIM_SLIDE_FROM_TOP 0x03
  FLIPPITY210_ANIM_SLIDE_FROM_BOTTOM 0x04
  FLIPPITY210_ANIM_DISSOLVE 0x05
  FLIPPITY210_ANIM_SLIDE_RIGHT_FORCED 0x06
  FLIPPITY210_ANIM_SLIDE_LEFT_FORCED 0x07
  FLIPPITY210_ANIM_SLIDE_FROM_TOP_FORCED 0x08
  FLIPPITY210_ANIM_SLIDE_FROM_BOTTOM_FORCED 0x09

  Leds power 0x26
  0x11 - on
  0x10 - off

  Data:
  3 bytes per display

  Status response (5 bytes):

  status (ready / busy)
  speed
  brightness
  animation
  led power

*/

//#define USE_SERIAL // eats alot of memory, use for debug
#define ENABLE_ANIMATION_DISSOLVE // this is very badly implemented so removing it frees quite alot of space
#define ENABLE_ANIMATION_SLIDE_FROM_BOTTOM
#define ENABLE_ANIMATION_SLIDE_FROM_TOP
#define ENABLE_ANIMATION_SLIDE_LEFT
#define DISPLAY_COUNT_LIMIT 2 // normally it's 4 but if you use STM32 with less flash memory, lowering can free tiny bit of space

#ifndef USE_SERIAL
#define TIMER_INTERRUPT_DEBUG 0
#define _TIMERINTERRUPT_LOGLEVEL_ 0
#endif


#include <Wire.h>
#include "STM32TimerInterrupt.h"

// Daisy chaining selector
#define DAISY_A PB4
#define DAISY_B PB3

// Rows shift register pins
#define ROWS_DATA PB15
#define ROWS_LATCH PB14
#define ROWS_CLK PB13
#define SINK_EN PA_9
#define SOURCE_EN PA_8

// Cols address
#define COLS_ENABLE PB_12
#define PANEL_ADDR_B0 2
#define PANEL_ADDR_B1 1
#define PANEL_ADDR_B2 0
#define GROUP_ADDR_B0 PA_15
#define GROUP_ADDR_B1 7
#define GROUP_ADDR_B2 6
#define COL_ADDR_B0 5
#define COL_ADDR_B1 4
#define COL_ADDR_B2 3

// Leds pins
#define LEDS_EN PB_5

#define LEDS_ADD1 PA_1
#define LEDS_ADD2 PB_11
#define LEDS_A1 PA_3
#define LEDS_A2 PA_10
#define LEDS_A0 PA_4

#define LEDS_LATCH PA_6
#define LEDS_CLK PA_5
#define LEDS_D0 PA_7
#define LEDS_D1 PB_0
#define LEDS_D2 PB_1
#define LEDS_D3 PB_10
#define LEDS_D4 PA_2
#define LEDS_D5 PA_0
#define LEDS_D6 PB_9
#define LEDS_D7 PB_8

#define DOTS_POWERON_TIME 600 // Microseconds. Sets time to keep each dot powered to flip correctly. 500uS-1000uS, more will overheat the drivers! Recommended 500uS or more for 24V, 800uS or more for 12V
#define LEDS_INTERVAL_MICROS_LOW_BRI 100 // Microseconds. Speed of switching leds rows
#define LEDS_INTERVAL_MICROS_MED_BRI 155 // Microseconds. Speed of switching leds rows
#define LEDS_INTERVAL_MICROS_HIGH_BRI 300 // Microseconds. Speed of switching leds rows

#define I2C_ADDR 0x08

/* I2C COMMANDS */
#define FLIPPITY210_SCREEN_RES_W 28
#define FLIPPITY210_SCREEN_RES_H 19

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

uint8_t displayData[5] {
  FLIPPITY210_STATUS_READY, // status (ready / busy)
  FLIPPITY210_SPEED_HIGH, // speed
  FLIPPITY210_BRI_HIGH, // brightness
  FLIPPITY210_ANIM_SLIDE_RIGHT, // animation
  FLIPPITY210_LED_PWR_OFF, // led power
};

uint8_t displaysCount; // set by onboard jumper, with maximum of 4 displays. If you want to use more than 4 displays, edit toggleDaisyChainCount() function.

// Flip dots vars
const uint8_t registersCount = 7;
const uint8_t columnCount = 28;
const uint8_t rowCount = 19;
const uint8_t rows[rowCount][2] = {
  {11, 19}, //0 0
  {15, 20}, //1 4
  {14, 21}, //2 3
  {13, 22}, //3 2
  {12, 23}, //4 1
  {29, 38}, //5 11
  {28, 39}, //6 8
  {27, 35}, //7 7
  {31, 36}, //8 6
  {26, 34}, //9 9
  {25, 33}, //10 10
  {30, 37}, //11 5
  {44, 55}, //12 15
  {45, 54}, //13 17
  {41, 49}, //14 14
  {47, 52}, //15 12
  {42, 50}, //16 16
  {46, 53}, //17 13
  {43, 51}, //18 18
};
volatile uint32_t dotsActive[DISPLAY_COUNT_LIMIT][rowCount]; // currently active dots
volatile uint32_t dotsBuffer[DISPLAY_COUNT_LIMIT][rowCount]; // buffer for flip dots to change
volatile uint8_t bytes[registersCount];
volatile bool receivingData = false;
volatile bool dataReady = false;
volatile unsigned flipDotUpdateDelay = 100; // microseconds
volatile unsigned int dotsToFlipUnflip[2]; // dots to flip, dots to unflip

// Leds vars
const uint8_t ledsRegistersCount = 4;
volatile uint32_t ledsBuffer[DISPLAY_COUNT_LIMIT][rowCount]; // buffer for LEDs
volatile uint8_t bytesLeds[DISPLAY_COUNT_LIMIT][ledsRegistersCount];
volatile uint8_t row;
volatile uint8_t toggle;

STM32Timer ledsTimer(TIM1);

// the setup function runs once when you press reset or power the board
void setup() {

  pinMode(DAISY_A, INPUT_PULLUP);
  pinMode(DAISY_B, INPUT_PULLUP);

  toggleDaisyChainCount(); // set the number of displays according to jumper selection
  initFlipDots(); // needs to be called first in setup
  initLeds();

  setDisplayStatus(FLIPPITY210_STATUS_BUSY);

  delay(300); // give ESP some time to boot, needed only with ESP-01

  i2cInit();

#ifdef USE_SERIAL
  Serial.begin(115200);
  Serial.println("Started");
  delay(10);
#endif

  // Initialize the display
  //toggleLeds(FLIPPITY210_LED_PWR_ON); // LEDs off by default
  setDisplayStatus(FLIPPITY210_STATUS_READY);

}

// the loop function runs over and over again forever
void loop() {

  if (!receivingData && dataReady) {
    dataReady = false;
    refreshDisplays();
  }

}
