#include "readsoil.h"
#include <Arduino.h>

void read_soil_rh(int pin, float & rh) {
  void read_soil_rh(int pin, float & rh) {
  const int DRY = 869;  
  const int WET = 169;

  int reading = analogRead(pin);

  // Clamp input
  if (reading > DRY) reading = DRY;
  if (reading < WET) reading = WET;

  float norm = float(DRY - reading) / float(DRY - WET);
  rh = norm * 100.0f;

  // Clamp output
  if (rh < 0) rh = 0;
  if (rh > 100) rh = 100;
}