/***************************************************
 DFRobot Gravity: Analog TDS Sensor/Meter
 <https://www.dfrobot.com/wiki/index.php/Gravity:_Analog_TDS_Sensor_/_Meter_For_Arduino_SKU:_SEN0244>
 
 ***************************************************
 This sample code shows how to read the tds value and calibrate it with the standard buffer solution.
 707ppm(1413us/cm)@25^c standard buffer solution is recommended.
 
 Created 2018-1-3
 By Jason <jason.ling@dfrobot.com@dfrobot.com>
 
 GNU Lesser General Public License.
 See <http://www.gnu.org/licenses/> for details.
 All above must be included in any redistribution.
 ****************************************************/
 
 /***********Notice and Trouble shooting***************
 1. This code is tested on Arduino Uno with Arduino IDE 1.0.5 r2 and 1.8.2.
 2. Calibration CMD:
     enter -> enter the calibration mode
     cal:tds value -> calibrate with the known tds value(25^c). e.g.cal:707
     exit -> save the parameters and exit the calibration mode
 ****************************************************/

#include <Arduino.h>
#include "GravityTDS.h"
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

// create a pixel strand with 1 pixel on PIN_NEOPIXEL
Adafruit_NeoPixel pixels(1, PIN_NEOPIXEL);

GravityTDS gravityTds;

const size_t sampleCount = 40;
const size_t sampleMS = 10;

float temperature = 20;

int cmp_float(const void *, const void *);

void setup()
{
    Serial.begin(9600);
    pixels.begin();

    gravityTds.setPin(A10);
    gravityTds.setAref(3.3);
    gravityTds.setAdcRange(1 << 12);
    gravityTds.begin();
    //temperature = readTemperature();  //add your temperature sensor and read it
    gravityTds.setTemperature(temperature);  // set the temperature and execute temperature compensation

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
    display.setTextColor(SSD1306_WHITE);
}

void loop()
{
    gravityTds.update();

    float tdsSamples[sampleCount];
    for (int i = 0; i < sampleCount; i++) {
        tdsSamples[i] = gravityTds.getTdsValue();
        delay(sampleMS);
    }

    // Use median.
    qsort(tdsSamples, sampleCount, sizeof(float), cmp_float);
    float tdsValue = tdsSamples[sampleCount / 2];

    if (tdsValue < 1.0) {
        // Very low: green
        pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    } else if (tdsValue < 6.0) {
        // Below replace threshold: yellow
        pixels.setPixelColor(0, pixels.Color(128, 128, 0));
    } else {
        // High: red
        pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    }

    pixels.show();

    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(4);
    display.print(tdsValue, 0);

    display.setCursor(80, 0);
    display.setTextSize(2);
    display.print(" ppm");
    display.display();

    Serial.print(tdsValue, 0);
    Serial.println(" ppm");
}

int cmp_float(const void *a, const void *b)
{
    const float *A = (float*) a;
    const float *B = (float*) b;
    if (*A < *B) return -1;
    if (*A > *B) return 1;
    return 0;
}
