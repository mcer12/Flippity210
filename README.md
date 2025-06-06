# Flippity210
Control board and library for BUSE BS210 flip dot displays. Might also work with BS110 (non-led version)

![alt text](https://github.com/mcer12/Flippity210/raw/refs/heads/master/resources/wall_clock.webp)  

- This project enables you to control these beautiful flip-dot displays via I2C the same way you use I2C oled displays for example. 
- The library supports Adafruit GFX which enables you to draw fonts and geometry.
- Custom PCB and schematic is provided in the repository and replaces the original control board.
- Optional onboard ESP8266 for easy development
- Supports up to 4 daisy-chained displays with the default firmware, up to 8 in theory with SW changes.

## What's needed to make it work:
- Assembled custom board (see repository wiki page and schematic notes)
- 24V adapter (see repository wiki)
- Arduino IDE to flash the board and optionally STM32 Cube Programmer to flash the USB bootloader
- Arduino compatible board to control the display via I2c. Tested with Arduino (uno, nano, pro mini), Raspberry Pi Pico, ESP8266 and ESP32 - pinouts included in the example sketch
- See project wiki https://github.com/mcer12/Flippity210/wiki

## Status
- PCB: Finished (V3), V3 version has integrated ESP8266 so it doesn't require the daughter board anymore.
- Library: Finished
- 3D printed enclosure: https://www.printables.com/model/468031-flippity210-flip-dot-display-case
