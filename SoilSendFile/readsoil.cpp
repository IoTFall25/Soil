#include "readsoil.h"
#include <Arduino.h>

void read_soil_rh(int pin, int & rh) {
  const int DRY = 869;  
  const int WET = 169;

  int reading = analogRead(pin);

  // Clamp
  if (reading > DRY) reading = DRY;
  if (reading < WET) reading = WET;

  // Normalize 0–1
  float norm = float(DRY - reading) / float(DRY - WET);

  // Convert to 0–100
  rh = int(norm * 100);
}