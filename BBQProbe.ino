/*
  esp8266 / esp-12e / Wemos D1 mini Thermometer -> 600°C
  using MAX6675
  https://github.com/adafruit/MAX6675-library/blob/master/max6675.cpp (not used here)
  https://github.com/mcleng/MAX6675-Library/blob/master/MAX6675.cpp

  MAX6675:
  ----------------------------------------------------
  SO      D6      #12   //MISO  GPIO12
  CS      D8      #15   //10k Pull-down, SS GPIO15
  CLK     D5      #14   //SCK   GPIO14
  VCC     3V3     3V3   // with esp822 do not use 5V !!!
  GND     GND     GND

  Blync:
    - create new auth 
    - create Gauge element
    - select virtual pin, e.g. V10
*/

//#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#include <SimpleTimer.h>    // https://github.com/jfturcot/SimpleTimer
SimpleTimer timer; 

//for LED status
#include <Ticker.h>
Ticker ticker;

#define BLYNC_PIN  V10

//#define WEMOS
#ifdef WEMOS
int thermoSO  =   D6; // MISO  GPIO12
int thermoCS  =   D8; // 10k Pull-down, SS GPIO15
int thermoCLK =   D5; // SCK   GPIO14
int LED_PIN   =   0;  // tbd
#else
int thermoSO  =   12;
int thermoCS  =   15;
int thermoCLK =   14;
int LED_PIN   =   4;  
#endif


// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "xxxxxxxxxxxxxxxxxxxxxxxxx";  // Put your Auth Token here. (see Step 3 above)

/////////////////////////////////////////////////////////////////////////////////
// from http://forum.arduino.cc/index.php?topic=44262.msg320447#msg320447
// by   http://forum.arduino.cc/index.php?action=profile;u=6729
char *ftoa(char *a, double f, int precision) {
  long p[] = {0, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000};
  char *ret = a;
  long heiltal = (long)f;
  itoa(heiltal, a, 10);
  while (*a != '\0') a++;
  *a++ = '.';
  long desimal = abs((long)((f - heiltal) * p[precision]));
  itoa(desimal, a, 10);
  return ret;
} // ftoa()

///////////////////////////////////////////////////////////////////////////
//   MAX6675
//   Units to readout temp (0 = raw, 1 = ˚C, 2 = ˚F)
//   https://github.com/mcleng/MAX6675-Library/blob/master/MAX6675.cpp 
///////////////////////////////////////////////////////////////////////////
#define _delay_ms(ms) delayMicroseconds((ms) * 1000)
float readMAX6675(uint8_t units) { 
  uint16_t value = 0;
  uint8_t error_tc = 0;
  float temp = 0.0;
  /*
    Initiate a temperature conversion. According to MAX's tech notes FAQ's
    for the chip, Line going high initiates a conversion, which means, we
    need to clock the chip low to high to initiate the conversion, then wait
    for the conversion to be complete before trying to read the data from
    the chip.
  */
  digitalWrite(thermoCS, LOW);
  _delay_ms(2);
  digitalWrite(thermoCS, HIGH);
  _delay_ms(220);
  /* Read the chip and return the raw temperature value */
  /*
    Bring CS pin low to allow us to read the data from
    the conversion process
  */
  digitalWrite(thermoCS, LOW);
  /* Cycle the clock for dummy bit 15 */
  digitalWrite(thermoCLK, HIGH);
  _delay_ms(1);
  digitalWrite(thermoCLK, LOW);
  /*
    Read bits 14-3 from MAX6675 for the Temp. Loop for each bit reading
    the value and storing the final value in 'temp'
  */
  for (int i = 11; i >= 0; i--) {
    digitalWrite(thermoCLK, HIGH);
    value += digitalRead(thermoSO) << i;
    digitalWrite(thermoCLK, LOW);
  }
  /* Read the TC Input inp to check for TC Errors */
  digitalWrite(thermoCLK, HIGH);
  error_tc = digitalRead(thermoSO);
  digitalWrite(thermoCLK, LOW);
  /*
    Read the last two bits from the chip, faliure to do so will result
    in erratic readings from the chip.
  */
  for (int i = 1; i >= 0; i--) {
    digitalWrite(thermoCLK, HIGH);
    _delay_ms(1);
    digitalWrite(thermoCLK, LOW);
  }
  // Disable Device
  digitalWrite(thermoCS, HIGH);
  /*
    Keep in mind that the temp that was just read is on the digital scale
    from 0˚C to 1023.75˚C at a resolution of 2^12.  We now need to convert
    to an actual readable temperature (this drove me nuts until I figured
    this out!).  Now multiply by 0.25.  I tried to avoid float math but
    it is tough to do a good conversion to ˚F.  THe final value is converted
    to an int and returned at x10 power.
    2 = temp in deg F
    1 = temp in deg C
    0 = raw chip value 0-4095
  */
  if (units == 2) {
    temp = (value * 0.25) * 9.0 / 5.0 + 32.0;
  } else if (units == 1) {
    temp = (value * 0.25);
  } else {
    temp = value;
  }
  /* Output negative of CS_pin if there is a TC error, otherwise return 'temp' */
  if (error_tc != 0) {
    return -thermoCS;
  } else {
    return temp;
  }
}
///////////////////////////////////////////////////////////////////////////
//   READINGS
///////////////////////////////////////////////////////////////////////////
void sendUptime() {
  float t = readMAX6675(1); // (0 = raw, 1 = ˚C, 2 = ˚F)
  //Serial.print("MAX6675 temp: "); Serial.println(t);
  Blynk.virtualWrite(BLYNC_PIN, t); // virtual pin
}
///////////////////////////////////////////////////////////////////////////
//   Ticker
///////////////////////////////////////////////////////////////////////////
void tick() {
  //toggle state
  int state = digitalRead(LED_PIN);  // get the current state of GPIO1 pin
  digitalWrite(LED_PIN, !state);     // set pin to the opposite state
}
///////////////////////////////////////////////////////////////////////////
//   SETUP
///////////////////////////////////////////////////////////////////////////
void setup() {
  //Serial.begin(115200);
  Blynk.begin(auth, "ssid", "pass");

  // blink LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  
  // MAX6675
  pinMode(thermoCLK, OUTPUT);
  pinMode(thermoSO, INPUT); // INPUT !!!
  pinMode(thermoCS, OUTPUT);
  digitalWrite(thermoCS, HIGH);
  // wait for MAX chip to stabilize
  delay(1000);

  // Setup a function to be called every second
  timer.setInterval(1000L, sendUptime);

  ticker.attach(0.5, tick);
}

void loop() {
  Blynk.run();
  timer.run();
}
