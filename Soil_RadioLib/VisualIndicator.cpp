#include <Arduino.h>              // gives pinMode, digitalWrite, delay, Serial, etc.
#include <Adafruit_GFX.h>         // needed for graphics
#include <Adafruit_SSD1306.h>     // needed for oled object + defines like SSD1306_WHITE
#include <Fonts/FreeSans9pt7b.h>
#include "VisualIndicator.h" 
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


Adafruit_SSD1306 oled(128, 64, &Wire, -1);   // DEFINITION 

#define TEMP_HIGH_THRESHOLD 95.0	// Fahrenheit
#define HUMID_HIGH_THRESHOLD 85.0	// Percent
#define LEDPINRED 9
#define LEDPINGREEN 5
#define LEDPINYELLOW 6
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

const unsigned char sproutBitmap[] PROGMEM = {
  0b00000000,0b00000000,
  0b00000011,0b11000000,
  0b00000111,0b11100000,
  0b00001100,0b00110000,
  0b00011000,0b00011000,
  0b00011000,0b00011000,
  0b00001100,0b00110000,
  0b00000111,0b11100000,
  0b00000011,0b11000000,
  0b00000001,0b10000000,
  0b00000001,0b10000000,
  0b00000001,0b10000000,
  0b00000001,0b10000000,
  0b00000001,0b10000000,
  0b00000000,0b00000000,
  0b00000000,0b00000000
};

void setupVI() {
  pinMode(LEDPINRED, OUTPUT);
  pinMode(LEDPINGREEN, OUTPUT);
  pinMode(LEDPINYELLOW, OUTPUT);

  digitalWrite(LEDPINRED, LOW);
  digitalWrite(LEDPINGREEN, LOW);
  digitalWrite(LEDPINYELLOW, LOW);

  // Initialize OLED
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    //Serial.println("OLED not found");
    while (1);
  }

  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setFont(&FreeSans9pt7b);  // Use larger font
  oled.setCursor(0, 20);         // Adjust Y-position for larger font
  oled.println("Initializing...");
  oled.clearDisplay();
  oled.display();
}

// void GHMeasurementsToOLED(float hum, float light, float temp) {
//   //Greenhouses
//   oled.setFont(NULL);
//   oled.clearDisplay();
//   oled.setCursor(0, 25);
//   oled.print("Temp: ");
//   oled.print((temp * 9/5 + 32), 2);
//   oled.print("F");
//   oled.print("\nHumidity: ");
//   oled.print(hum, 2);
//   oled.println("%");
//   oled.println("\nLight: ");
//   oled.print(light, 2);
//   oled.print("lux");
//   oled.display();
//   digitalWrite(LEDPINGREEN, HIGH);
//   oled.setFont(&FreeSans9pt7b);
// }
// void FMeasurementsToOLED(float temp, float pH) {
//   //Fish
//   oled.setFont(NULL);
//   oled.clearDisplay();
//   oled.setCursor(0, 25);
//   oled.print("Temp: ");
//   oled.print((temp * 9/5 + 32), 2);
//   oled.print("F");
//   oled.print("\nPH: ");
//   oled.print(pH, 2);
//   oled.display();
//   digitalWrite(LEDPINGREEN, HIGH);
//   oled.setFont(&FreeSans9pt7b);
// }
void SMeasurementsToOLED(float soil1, float soil2, float soil3) {
  //soil
  oled.setFont(NULL);
  oled.clearDisplay();
  oled.setCursor(0, 25);
  oled.print("Humidity1: ");
  oled.print(soil1, 2);
  oled.print("%");
  oled.print("Humidity2: ");
  oled.print(soil2, 2);
  oled.print("%");
  oled.print("Humidity3: ");
  oled.print(soil3, 2);
  oled.print("%");
  oled.display();
  digitalWrite(LEDPINGREEN, HIGH);
  oled.setFont(&FreeSans9pt7b);
}
// void HMeasurementsToOLED(float temp, float pH) {
//   //Hydroponics
//   oled.setFont(NULL);
//   oled.clearDisplay();
//   oled.setCursor(0, 25);
//   oled.print("Temp: ");
//   oled.print((temp * 9/5 + 32), 2);
//   oled.print("F");
//   oled.print("\nPH: ");
//   oled.print(pH, 2);
//   oled.display();
//   digitalWrite(LEDPINGREEN, HIGH);
//   oled.setFont(&FreeSans9pt7b);
// }

void sendingVI() {
  int num2 = 0;
  while (num2 != 15) {
    digitalWrite(LEDPINGREEN, HIGH);
    delay(100);
    digitalWrite(LEDPINGREEN, LOW);
    delay(100);
    num2 += 1;
    oled.clearDisplay();
    oled.setCursor(0, 25);  // Adjusted for larger font
    oled.println("Sending...");
    oled.display();
  }
}

void hardwareFail(int status) {
  //Flashing yellow light
  if(status == -12){
    oled.clearDisplay();
    oled.setCursor(0, 25);  // Adjusted for larger font
    oled.println("Invalid Frequency");
    oled.display();
  }
  else if(status == -25){
    oled.clearDisplay();
    oled.setCursor(0, 25);  // Adjusted for larger font
    oled.println("Functionality Unsupported");
    oled.display();
  }
  else if(status == -104){
    oled.clearDisplay();
    oled.setCursor(0, 25);  // Adjusted for larger font
    oled.println("Bandwidth value invalid");
    oled.display();
  }
  int num = 0;
  while (num != 10) {
    digitalWrite(LEDPINYELLOW, HIGH);
    delay(100);
    digitalWrite(LEDPINYELLOW, LOW);
    delay(100);
    num += 1;
  }
}

void sendingFail() {
  digitalWrite(LEDPINRED, HIGH);
}

void greenhouseAlert(const char* message, float temp, float humid) {
  if (temp > TEMP_HIGH_THRESHOLD) {
		oled.clearDisplay();
	  oled.setFont(NULL);
	  oled.setCursor(0, 25);
	  oled.println("WARNING: Temp High");
	  oled.display();
		// Trigger red LED if dangerous:
		digitalWrite(LEDPINRED, HIGH);
		delay(2000);
		digitalWrite(LEDPINRED, LOW);
	}
  if (humid > TEMP_HIGH_THRESHOLD) {
		oled.clearDisplay();
	  oled.setFont(NULL);
	  oled.setCursor(0, 25);
	  oled.println("WARNING: Humidity High");
	  oled.display();
	}
	
	// Flash YELLOW for warnings
	for (int i = 0; i < 10; i++) {
		digitalWrite(LEDPINYELLOW, HIGH);
		delay(200);
		digitalWrite(LEDPINYELLOW, LOW);
		delay(200);
	}
}
