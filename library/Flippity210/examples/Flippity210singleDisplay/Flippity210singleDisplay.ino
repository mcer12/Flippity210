/*
  Basic demo for Flippity210 library
  See https://github.com/mcer12/Flippity210/wiki/Arduino-library
  Requires fonts from https://github.com/robjen/GFX_fonts
  Just download the .h files and put them in the sketch folder.
*/


#include <Wire.h>    // I2C library
#include <Adafruit_GFX.h>    // Core graphics library
#include "Flippity210.h"
#include "Font5x7Fixed.h"
#include "Font4x5Fixed.h"

/*
  Default pinout (changeable on some platforms)
  Platform: SDA, SCL
  ESP8266: 4, 5 (don't mistake with D4, D5)
  ESP8266 (onboard): 0, 2
  ESP32: 21, 22
  Atmega328/168: A4, A5
  RP2040: 4, 5
*/

int displays = 1; // number of daisy chained displays, it should match jumper on the flippity board.
Flippity210 display(displays, 4, 5); // Explicitly set I2C pins, doesn't work on atmega
//Flippity210 display(displays);

unsigned int delayMs = 3000;

void setup() {
  // Enable serial
  Serial.begin(115200);
  //delay(5000);
  Serial.println("");
  Serial.println("Starting up");

  display.begin();
  Serial.println("Started");

  display.setRotation(0);
  display.setTextColor(FLIPPITY210_WHITE);

  display.setAnimation(FLIPPITY210_ANIM_DISSOLVE);
  //display.setAnimationSpeed(FLIPPITY210_SPEED_VERY_LOW);
  //display.setAnimationSpeed(FLIPPITY210_SPEED_MED);
  Serial.println("Animations set");

  display.fill();
  Serial.println("Filled");
  display.update();
  delay(1000);
  display.clear();
  display.update();

  Serial.println("Updated dots");

  delay(5000);

  display.clear();
  display.setFont(&Font5x7Fixed);
  display.setCursor(0, 10);
  display.println(F("Hello"));
  display.setCursor(0, 18);
  display.println(F("community!"));
  display.update();
  Serial.println("text set");

  delay(5000);

  display.clear();
  display.setTextColor(FLIPPITY210_WHITE);
  //display.setFont(&Font5x7Fixed);
  display.setCursor(0, 10);
  display.println(F("Finally made"));
  display.setCursor(0, 18);
  display.println(F("the library"));
  display.update();
  Serial.println("text set");

  delay(5000);

  display.clear();
  display.setTextColor(FLIPPITY210_WHITE);
  //display.setFont(&Font5x7Fixed);
  display.setCursor(0, 10);
  display.println(F("And it"));
  display.setCursor(0, 18);
  display.println(F("supports"));
  display.update();
  Serial.println("text set");

  delay(5000);


  display.clear();
  display.setTextColor(FLIPPITY210_WHITE);
  //display.setFont(&Font5x7Fixed);
  display.setCursor(0, 10);
  display.println(F("Adafruit"));
  display.setCursor(0, 18);
  display.println(F("GFX"));
  display.update();
  Serial.println("text set");

  delay(5000);


  display.clear();
  display.setTextColor(FLIPPITY210_WHITE);
  //display.setFont(&Font5x7Fixed);
  display.setCursor(0, 10);
  display.println(F("Supports"));
  display.setCursor(0, 18);
  display.println(F("transitions"));
  display.update();
  Serial.println("text set");

  display.setAnimationSpeed(FLIPPITY210_SPEED_HIGH);

  delay(5000);

  display.setAnimation(FLIPPITY210_ANIM_SLIDE_RIGHT);
  display.fill();
  display.update();

  display.setAnimation(FLIPPITY210_ANIM_SLIDE_RIGHT);
  display.clear();
  display.update();

  display.setAnimation(FLIPPITY210_ANIM_SLIDE_LEFT);
  display.fill();
  display.update();

  display.setAnimation(FLIPPITY210_ANIM_SLIDE_LEFT);
  display.clear();
  display.update();

  display.setAnimation(FLIPPITY210_ANIM_SLIDE_FROM_TOP);
  display.fill();
  display.update();

  display.setAnimation(FLIPPITY210_ANIM_SLIDE_FROM_TOP);
  display.clear();
  display.update();

  display.setAnimation(FLIPPITY210_ANIM_SLIDE_FROM_BOTTOM);
  display.fill();
  display.update();

  display.setAnimation(FLIPPITY210_ANIM_SLIDE_FROM_BOTTOM);
  display.clear();
  display.setTextColor(FLIPPITY210_WHITE);
  //display.setFont(&Font5x7Fixed);
  display.setCursor(0, 10);
  display.println(F("And more :)"));
  display.setCursor(0, 18);
  display.println(F(""));
  display.update();



  delay(5000);
  //flippity210.invert();
  //printBuffer();

}

void loop() {
  // Wait
  display.setAnimation(FLIPPITY210_ANIM_DISSOLVE);

  //display.setAnimation(FLIPPITY210_ANIM_SLIDE_LEFT);

  //display.setAnimation(FLIPPITY210_ANIM_DISSOLVE);
  display.fill();
  display.update();
  delay(delayMs);

  display.clear();
  display.update();
  delay(delayMs);

  //display.setAnimation(FLIPPITY210_ANIM_SLIDE_RIGHT);

  //display.setAnimation(FLIPPITY210_ANIM_DISSOLVE);
  display.fill();
  display.update();
  delay(delayMs);

  display.clear();
  display.update();
  delay(delayMs);

  //display.setAnimation(FLIPPITY210_ANIM_SLIDE_FROM_TOP);

  //display.setAnimation(FLIPPITY210_ANIM_DISSOLVE);
  display.fill();
  display.update();
  delay(delayMs);

  display.clear();
  display.update();
  delay(delayMs);

  //display.setAnimation(FLIPPITY210_ANIM_SLIDE_FROM_BOTTOM);

  //display.setAnimation(FLIPPITY210_ANIM_DISSOLVE);
  display.fill();
  display.update();
  delay(delayMs);

  display.clear();
  display.update();
  delay(delayMs);

}
