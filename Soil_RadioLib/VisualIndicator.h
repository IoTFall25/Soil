#ifndef VISUALINDICATOR_H
#define VISUALINDICATOR_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 oled;

void setupVI();
//void GHMeasurementsToOLED(float hum, float light, float temp);
//void FMeasurementsToOLED(float temp, float pH);
void SMeasurementsToOLED(float soil1, float soil2, float soil3);
void hardwareFail();
void sendingVI();
void sendingFail();
void greenhouseAlert();

#endif