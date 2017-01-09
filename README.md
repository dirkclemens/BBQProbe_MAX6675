# BBQProbe_MAX6675
Measure temperatures using MAX 6675 and esp8266 / WEMOS D1 mini

Temperatures -> 600°C using MAX6675

[https://github.com/adafruit/MAX6675-library/blob/master/max6675.cpp]() (not used here)
[https://github.com/mcleng/MAX6675-Library/blob/master/MAX6675.cpp]()

MAX6675:
----------------------------------------------------
	MAX6675		WEMOS	ESP12
	SO      	D6      #12   //MISO  GPIO12
	CS     		D8      #15   //10k Pull-down, SS GPIO15
	CLK     	D5      #14   //SCK   GPIO14
	VCC     	3V3     3V3   // with esp822 do not use 5V !!!
	GND     	GND     GND


Configure Blynk ([http://www.blynk.cc]()):
----------------------------------------------------

* create new auth 
* create Gauge element
* select virtual pin, e.g. V10
