This is a temperature controller using an ATtiny44 microcontroller running at 1 Mhz, although other AVR microcontrollers will probably work with minimal code editing. 

It monitors the current temperature with a thermistor and opens and closes a relay based upon the current temperature with the intended purpose of controlling a heat lamp for a chicken coop. Once the temperature minimum is reached, it turns on the relay until the temperature maximum is reached -- both minimum and maximum values can be adjusted with a rotary encoder. Adjusting the minimum temperature can be done by rotating the rotary encoder and one can toggle between adjusting low and high with the rotary encoder SW button. The seven segment display is inverted, that is, the decimal points are at the top instead of the bottom, and the decimal points are used to display whether one is adjusting low or high minimum temperature -- leftmost decimal point is low, right-most decimal point is high.


In the future I may allow temperature scale adjustment but, for now, it uses only fahrenheit.

Breadboard prototype:
![Breadboard prototype](https://github.com/kmkaczor/TemperatureControllerAttiny/blob/main/attiny.jpg)
