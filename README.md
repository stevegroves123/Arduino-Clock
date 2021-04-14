# Arduino-Clock
  Clock using Arduino software

Ok - Lets start with the basics I'm no programmer.

Ths code needs to be cleaned up and there are one or two (many) changes needed to make this software "nice" looking, but all the functions work.  Feel free to copy, improve and distribute this code.

I used an Xiao and a 128x64 oled, with the addition of three buttons this clock shows an analog clock face of the time.  When you press the enter button you get access to a menu that shows various user interactions; set an alarm (flash the display), turn on all OLED pixels (light), run a test, turn off all pixels (display off) and show a digital clock with date. 

The main display is built using the U8G2 user_interface functions  - without these I would never have attempted something so complex (for me) as this...(see https://github.com/olikraus/U8g2_Arduino) 

Hardware:

OLED display - Any i2C oled, but an SPI could be used if you change the code. I used a 128x64 yellow/blue oled - but it needs to be changed.

Microcontroiller - Check out Cool Components for the Xiao:  https://coolcomponents.co.uk/products/seeeduino-xiao-arduino-microcontroller-samd21-cortex-m0?_pos=2&_sid=62fc70a32&_ss=r 

Three small press-to-make buttons, one for "enter" and one each for up and down.
![Analog Clock](https://user-images.githubusercontent.com/10800904/114734309-0278c580-9d3c-11eb-9325-3f1f3a40ac5f.jpg)
In the picture the Xiao fits behind the display so all you see is the type-c usb port.  At the moment the power comes from the usb port, however, a small LiPo or LiOn battery would easily power it.
