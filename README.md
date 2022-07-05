This is a temperature controller using an ATtiny44 microcontroller. It monitors the current temperature with a
thermistor and opens and closes a relay based upon the current temperature with the intended purpose of controlling a
heat lamp for a chicken coop.

It does not currently use a rotary encoder for changing high/low threshold values as I need to figure out what I am
going to do with the PB3 reset pin, either I will add a bootloader and change the fuse to turn PB3 into a gpio, or I
will consider changing to a 16 pin ATtiny instead. Otherwise, there's always high voltage programming.

Breadboard prototype:
<<<<<<< HEAD:README.md
![Breadboard prototype]https://github.com/kmkaczor/TemperatureControllerAttiny/blob/main/attiny.jpg]
=======
[image](https://github.com/kmkaczor/TemperatureControllerAttiny/blob/main/attiny.jpg]
>>>>>>> 96f66fdd3b5ff7d187a7590b652584cf7767dc4e:README
