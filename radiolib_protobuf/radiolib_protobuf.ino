#include <RadioLib.h>
#include <TaskScheduler.h>
#include "sensor_readings.pb.h"
#include "lorafeather_pins.h"

#define RFM95_FREQ 915.0
#define SEND_DURATION 2*TASK_SECOND

// Uncomment this to use serial print debugging
//#define SERIAL_DEBUG
#include "Adafruit_EEPROM_I2C.h"
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
  tosend.myaddr = 10;

  float soil_valueS1, soil_valueS2, soil_valueS3;

  //PROTOBUF FIELDS
  slug.has_power = true;
  slug.has_r1 = true;
  slug.r1.type = ReadingType_REL_HUMID;
  slug.r1.value = soil_valueS1;

  slug.has_r2 = true;
  slug.r2.type = ReadingType_REL_HUMID;
  slug.r2.value = soil_valueS2;

  slug.has_r3 = true;
  slug.r3.type = ReadingType_REL_HUMID;
  slug.r3.value = soil_valueS3;

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

  float soil_valueS1, soil_valueS2, soil_valueS3;
  // read stuff here
  read_soil_rh(SENSOR_1_PIN, soil_valueS1);
  slug.r1.value = soil_valueS1;

  read_soil_rh(SENSOR_2_PIN, soil_valueS2);
  slug.r2.value = soil_valueS2;

  read_soil_rh(SENSOR_3_PIN, soil_valueS3);
  slug.r3.value = soil_valueS3;
  //Debug tests
  Serial.print("Soil1: "); Serial.println(slug.r1.value);
  Serial.print("Soil2: "); Serial.println(slug.r2.value);
  Serial.print("Soil3: "); Serial.println(slug.r3.value);
  Serial.print("Battery OK: "); Serial.println(slug.has_power);
  Serial.print("My Address: "); Serial.println(MY_ADDR);

  tosend.message_id++;
  pbout = pb_ostream_from_buffer(tosend.data, 64);
  pb_encode(&pbout, ReadingSlug_fields, &slug);
  
  debug("PB encoded size", String(pbout.bytes_written));

  int status = radio.transmit((uint8_t *)&tosend, pbout.bytes_written+HEADER_LEN);
  debug("Bytes sent", String(pbout.bytes_written+HEADER_LEN));
  debug("Transmit status code", String(status));

}
