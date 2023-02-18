/*

  DISPLAY FIRMWARE

  COMMANDS

  Action (first byte)
  0x20 - update flip dots
  0x21 - update leds
  0x22 - update both
  0x23 - set speed
  0x24 - set animation
  0x25 - set brightness (leds only)
  0x26 - toggle leds power
  0x27 - set number of daisy-chained displays

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
  0x01 - default simple scan FAST (with less pixels to change, it gets faster)
  0x02 - inverted fast simple scan
  0x03 - top to bottom simple scan
  0x04 - bottom to top simple scan
  0x01 - default simple scan FORCED FLIP (force flip all dots -> equal speed)
  0x02 - inverted fast simple scan
  0x03 - top to bottom simple scan
  0x04 - bottom to top simple scan
  0x05 - wave left-right 1
  0x06 - wave left-right 2
  0x07 - wave right-left 1
  0x08 - wave right-left 2

  Leds power 0x26
  0x10 - off
  0x11 - on

  Data:
  3 bytes per display

  Status response (5 bytes):
  
  status (ready / busy)
  speed
  brightness
  animation
  led power
  
*/
//#define USE_SPI // to be removed
#define USE_SERIAL // eats alot of memory, use for debug

#ifndef USE_SERIAL
#define TIMER_INTERRUPT_DEBUG 0
#define _TIMERINTERRUPT_LOGLEVEL_ 0
#endif

#ifdef USE_SPI
#include <SPI.h>
#endif

#include <Wire.h>
#include "STM32TimerInterrupt.h"

// Rows shift register pins
#define ROWS_DATA PB15
#define ROWS_LATCH PB14
#define ROWS_CLK PB13
#define SINK_EN PA_9
#define SOURCE_EN PA_8

// Cols address
#define COLS_ENABLE PB_12
#define PANEL_ADDR_B0 PB_4
#define PANEL_ADDR_B1 PB_3
#define PANEL_ADDR_B2 PA_15
#define GROUP_ADDR_B0 2
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

#define LEDS_INTERVAL_MICROS 300 // speed of switching leds rows

#define I2C_ADDR 0x08

/* I2C COMMANDS */
#define COMMAND_UPDATE_BOTH 0x20
#define COMMAND_UPDATE_LEDS 0x21
#define COMMAND_UPDATE_DOTS 0x22
#define COMMAND_SET_SPEED 0x23
#define COMMAND_SET_ANIMATION 0x24
#define COMMAND_SET_BRIGHTNESS 0x25
#define COMMAND_TOGGLE_LED_POWER 0x26

#define PAYLOAD_SPEED_HIGH 0x01
#define PAYLOAD_SPEED_MED 0x02
#define PAYLOAD_SPEED_LOW 0x03
#define PAYLOAD_SPEED_VERY_LOW 0x04
#define PAYLOAD_SPEED_EXTREMELY_LOW 0x05

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

#define PAYLOAD_LED_PWR_ON 0x11
#define PAYLOAD_LED_PWR_OFF 0x10

#define PAYLOAD_STATUS_READY 0x11
#define PAYLOAD_STATUS_BUSY 0x10

byte displayData[5] {
  PAYLOAD_STATUS_READY, // status (ready / busy)
  PAYLOAD_SPEED_HIGH, // speed
  PAYLOAD_BRI_HIGH, // brightness
  PAYLOAD_ANIM_FAST, // animation
  PAYLOAD_LED_PWR_OFF, // led power
};

const uint8_t displaysLimit = 3; // maximum number of displays that can be daisychained
uint8_t displaysToUpdate = 0;

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
volatile uint32_t dotsActive[displaysLimit][rowCount]; // currently active dots
volatile uint32_t dotsBuffer[displaysLimit][rowCount]; // buffer for flip dots to change
volatile uint8_t bytes[registersCount];
volatile bool receivingData = false;
volatile bool dataReady = false;
volatile int flipDotUpdateDelay = 100; // microseconds

// Leds vars
const uint8_t ledsRegistersCount = 4;
volatile uint32_t ledsBuffer[displaysLimit][rowCount]; // buffer for LEDs
volatile uint8_t bytesLeds[displaysLimit][ledsRegistersCount];
volatile uint8_t row;
volatile uint8_t toggle;

STM32Timer ledsTimer(TIM1);

// the setup function runs once when you press reset or power the board
void setup() {

  initFlipDots(); // needs to be called first in setup
  initLeds();

  setDisplayStatus(PAYLOAD_STATUS_BUSY);

  i2cInit();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);

#ifdef USE_SERIAL
  Serial.begin(115200);
#endif
  // Initialize the display

  /*
    for (int i = 0; i < 50; i++) {
      fillDisplay(0);
      fillDisplay(1);
      refreshDisplays(true);
      blankDisplay(0);
      blankDisplay(1);
      refreshDisplays(true);

      Projekt zabírá 25684 bytů (39%)  úložného místa pro program. Maximum je 65536 bytů.
Globální proměnné zabírají 1920 bytů (23%)  dynamické paměti, 6272 bytů zůstává pro lokální proměnné. Maximum je 8192 bytů.




Projekt zabírá 25684 bytů (39%)  úložného místa pro program. Maximum je 65536 bytů.
Globální proměnné zabírají 1920 bytů (23%)  dynamické paměti, 6272 bytů zůstává pro lokální proměnné. Maximum je 8192 bytů.

    }
  */

  toggleLeds(1);
  setDisplayStatus(PAYLOAD_STATUS_READY);
}

// the loop function runs over and over again forever
void loop() {
  //Serial.println("loop");

  //delay(1);

  if (!receivingData && dataReady) {
    dataReady = false;
    refreshDisplays();
  }

  /*
    draw5x9(1, 0, lettersSquare[0]);
    draw5x9(8, 0, lettersSquare[1]);
    draw5x9(15, 0, lettersSquare[2]);
    draw5x9(22, 0, lettersSquare[3]);

    draw5x9(1, 10, lettersSquare[4]);
    draw5x9(8, 10, lettersSquare[5]);
    draw5x9(15, 10, lettersSquare[6]);
    draw5x9(22, 10, lettersSquare[7]);

    refreshDisplay(false);
    delay(2000);

    draw5x9(1, 0, lettersSquare[8]);
    draw5x9(8, 0, lettersSquare[9]);
    draw5x9(15, 0, lettersSquare[10]);
    draw5x9(22, 0, lettersSquare[11]);

    draw5x9(1, 10, lettersSquare[12]);
    draw5x9(8, 10, lettersSquare[13]);
    draw5x9(15, 10, lettersSquare[14]);
    draw5x9(22, 10, lettersSquare[15]);

    refreshDisplay(false);
    delay(2000);


    invertDisplay();

    refreshDisplay(false);
    delay(2000);

    blankDisplay();

    refreshDisplay(false);
    delay(2000);
  */

}
