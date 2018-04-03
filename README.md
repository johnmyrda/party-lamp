# party-lamp

#### Code

This project is built with Arduino IDE using the following libraries:

[FastLED](http://fastled.io/)

[Bounce2](https://github.com/thomasfredericks/Bounce2)

[EEPROM](https://www.arduino.cc/en/Reference/EEPROM) <-- This is a built in library

The current pattern and color temperature are saved in EEPROM so the lamp boots to the same state it was in when it is turned on and off.

I had flickering issues when mapping the brightness dial (1024 values) to brightness values (128 values). I solved the flickering issues by creating a brightness update function that maps the potentiometer to 8 possible brightness values. The flickering can still occur if the potentiometer is left in just the right spot, but this is rare and easy to avoid.

All of the pattern functions are non-blocking so that the buttons and brightness dial remain responsive.

#### Hardware

I gutted a cheap [desk lamp from Amazon](https://www.amazon.com/gp/product/B00G7QRLYE) and put an [Arduino Pro Micro](https://www.sparkfun.com/products/12640) and a USB battery pack in the base. 

I created a custom light bulb out of PVC pipe and WS2812b LEDs.

The Pattern Change/Color Temp change switch is a momentary push button switch, the Party Mode/Lamp Mode switch uses a Toggle button a red LED, and brightness is controllable with a potentiometer.
