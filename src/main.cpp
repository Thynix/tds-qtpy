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
    pixels.setPixelColor(0, pixels.Color(0, 0, 255));
    pixels.show();

    Serial.begin(9600);

    gravityTds.setPin(A10);
    gravityTds.setAref(3.3);
    gravityTds.setAdcRange(1 << 12);
    gravityTds.begin();

    gravityTds.setTemperature(20);

    // Hope the voltage is stable and the display will be ready.
    delay(1000);

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    // Address 0x3C for 128x32 display.
    // If begin() returns false, there were argument problems. The library
    // doesn't actually check if the display is responding.
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        // Blink red and blue on failure.
        while (true) {
            pixels.setPixelColor(0, pixels.Color(255, 0, 0));
            pixels.show();
            delay(200);

            pixels.setPixelColor(0, pixels.Color(0, 0, 255));
            pixels.show();
            delay(200);
        }
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

    // ZeroWater marketing suggests < 6
    //
    // WHO guidelines - it's just taste, not health unless very low.
    //
    //     > The final report, published in 1980 as an internal working document (3), concluded that “not
    //     > only does completely demineralised water (distillate) have unsatisfactory organoleptic
    //     > properities, but it also has a definite adverse influence on the animal and human organism”.
    //
    // - https://www.who.int/water_sanitation_health/dwq/nutrientschap12.pdf
    //
    //     > Reliable data on possible health effects associated with the ingestion of TDS in drinking-
    //     > water are not available. The results of early epidemiological studies suggest that even low
    //     > concentrations of TDS in drinking-water may have beneficial effects, although adverse effects
    //     > have been reported in two limited investigations.
    //
    // mg/L:
    //
    // < 300   | Excellent
    // < 600   | Good
    // < 900   | Fair
    // < 1,200 | Poor
    // More    | Unacceptable
    //
    // - https://www.who.int/water_sanitation_health/dwq/chemicals/tds.pdf
    // 
    // Note that ppm is almost exactly equal to mg/L: conversion factor 0.998859.
    //
    // - https://www.easycalculation.com/unit-conversion/ppm-mgl-conversion.php
    if (tdsValue < 6.0) {
        // Very low: yellow
        pixels.setPixelColor(0, pixels.Color(128, 128, 0));
    } else if (tdsValue < 300) {
        // Excellent: green
        pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    } else {
        // Noticably high: red
        pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    }

    pixels.show();

    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(4);
    display.printf("%03d", (int) round(tdsValue));

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
