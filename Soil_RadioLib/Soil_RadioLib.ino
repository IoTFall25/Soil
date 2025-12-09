#include <RadioLib.h>
#include <TaskScheduler.h>
#include "sensor_readings.pb.h"
#include "lorafeather_pins.h"
#include "readsoil.h"
#include "Adafruit_EEPROM_I2C.h"



#define SENSOR_1_PIN A0
#define SENSOR_2_PIN A1
#define SENSOR_3_PIN A2
#define BATTERY_PIN A3

#define RFM95_FREQ 915.0
#define SEND_DURATION 100*TASK_SECOND

// Uncomment this to use serial print debugging
//#define SERIAL_DEBUG
Adafruit_EEPROM_I2C i2ceeprom;
uint8_t MY_ADDR;
// Note: pin defines pulled out into header
RFM95 radio = new Module(RFM95_CS, RFM95_INT, RFM95_RST);

struct message {
  uint8_t myaddr = MY_ADDR;
  uint8_t message_id = 0;
  uint8_t data[64];
};

#define HEADER_LEN 2

// Variables for sending
message tosend;
ReadingSlug slug;
pb_ostream_t pbout;

void debug(String label, String msg);
void debugflash(int del);
void read_and_send();

Scheduler ts;
Task sendtask(SEND_DURATION, TASK_FOREVER, read_and_send, &ts, true);

// TaskScheduler setup

void setup() {
  // Ignore this, it's just for making sure stuff is working
  #ifdef SERIAL_DEBUG
  Serial.begin(115200);
  #endif
  //find our address
  while(!Serial) delay(1);
    if (i2ceeprom.begin(0x50)) {
    MY_ADDR = i2ceeprom.read(0x00);
  } 
  else {
    MY_ADDR = 64; // fallback
    Serial.print("USING FALLBACK ADDR");

  }
  pinMode(13, OUTPUT);
  // ---
  int status = radio.begin(RFM95_FREQ);
  // Any call to debug might be better shown on the screen or 
  debug("Radio begin code", String(status));
  // To avoid serial and external boards, use LED on pin 13
  if (status != 0) debugflash(100);
  // Initialize the ReadingSlug with the reading types
  tosend.myaddr = MY_ADDR;


  //PROTOBUF FIELDS
  slug.has_r1 = true;
  slug.r1.type = ReadingType_REL_HUMID;

  slug.has_r2 = true;
  slug.r2.type = ReadingType_REL_HUMID;

  slug.has_r3 = true;
  slug.r3.type = ReadingType_REL_HUMID;


}

void loop() {
  // put your main code here, to run repeatedly:
  ts.execute();
}

void debug(String label, String msg)
{
  #ifdef SERIAL_DEBUG
  Serial.print(label+" ");
  Serial.println(msg);
  #endif
}
void debugflash(int del)
{
  while(true)
    {
      digitalWrite(13, HIGH);
      delay(del);
      digitalWrite(13, LOW);
      delay(del);
    }
}

void read_and_send()
{

  float soil1, soil2, soil3;
  // read stuff here
  read_soil_rh(SENSOR_1_PIN, soil1);
  read_soil_rh(SENSOR_2_PIN, soil2);
  read_soil_rh(SENSOR_3_PIN, soil3);

  slug.r1.value = soil1;
  slug.r2.value = soil2;
  slug.r3.value = soil3;

  //Debug tests
  Serial.println("Soil at " + String(MY_ADDR) + ": " + String(slug.r1.value));
  Serial.println("Soil at " + String(MY_ADDR) + ": " + String(slug.r2.value));
  Serial.println("Soil at " + String(MY_ADDR) + ": " + String(slug.r3.value));
  Serial.print("Battery OK: "); Serial.println(slug.has_power);
  Serial.print("My Address: "); Serial.println(MY_ADDR);

  // Increment message counter
  tosend.message_id++;

// Build packet buffer
  uint8_t buffer[66];   // 2-byte header + protobuf (max 64)

// Header bytes
  buffer[0] = MY_ADDR;            // sensor address
  buffer[1] = tosend.message_id;  // incrementing counter

// Encode protobuf AFTER the first 2 bytes
  pb_ostream_t s = pb_ostream_from_buffer(buffer + 2, 64);

  if (!pb_encode(&s, ReadingSlug_fields, &slug)) {
    debug("PB encode error", "Failed");
   return;
}

  int bytes_to_send = s.bytes_written + 2;

  // Send the packet
  int status = radio.transmit(buffer, bytes_to_send);
  debug("Bytes sent", String(bytes_to_send));
  debug("Transmit status", String(status));


}
