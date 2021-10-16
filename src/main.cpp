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

Adafruit_SSD1306 display(128, 32, &Wire);

// Address the QT Py's built-in NeoPixel.
Adafruit_NeoPixel pixels(1, PIN_NEOPIXEL);

GravityTDS gravityTds;

// Count of samples to use in each displayed value.
const size_t sampleCount = 2750;

// Take the mean of this many samples from each side of the median.
const size_t aroundMedian = 100;

int cmp_float(const void *, const void *);

void setup()
{
    // Blue pixel during startup
    pixels.begin();
    pixels.setBrightness(10);
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));

    Serial.begin(9600);

    gravityTds.setPin(A10);
    gravityTds.setAref(3.3);
    gravityTds.setAdcRange(1 << 12);
    gravityTds.begin();

    gravityTds.setTemperature(20);

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    // Address 0x3C for 128x32 display.
    // TODO: Why is this not enough to ensure the display works?
    while (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        // Assume display just received power and try again.
        delay(10);
    }

    display.setTextColor(SSD1306_WHITE);
}

void loop()
{
    //float temperature = 20;
    //gravityTds.setTemperature(temperature);

    float tdsSamples[sampleCount];
    
    for (size_t i = 0; i < sampleCount; i++) {
        gravityTds.update();
        tdsSamples[i] = gravityTds.getTdsValue();
    }

    // Use mean of values around median.
    qsort(tdsSamples, sampleCount, sizeof(float), cmp_float);

    const size_t middleIndex = sampleCount / 2;
    float tdsSum = 0;
    for (size_t i = middleIndex - aroundMedian; i < middleIndex + aroundMedian; i++) {
        tdsSum += tdsSamples[i];
    }
    float tdsValue = tdsSum / (aroundMedian * 2);

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

    Serial.print(millis());
    Serial.print(' ');
    Serial.print(tdsValue, 3);
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
