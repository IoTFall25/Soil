#include <RadioLib.h>
#include <TaskScheduler.h>
#include "sensor_readings.pb.h"
#include "Adafruit_EEPROM_I2C.h"
#include "readsoil.h"
#include "lorafeather_pins.h"


//Hardware 
Adafruit_EEPROM_I2C i2ceeprom;
uint8_t MY_ADDR;

#define SENSOR_1_PIN A0
#define SENSOR_2_PIN A1
#define SENSOR_3_PIN A2
#define BATTERY_PIN A3

#define LED 13
#define RFM95_FREQ 915.0

// Setuping up RadioLib 
RFM95 radio = new Module(16, 21, 17);   // CS, INT, RST

// Message formatting
struct message {
  uint8_t myaddr;
  uint8_t message_id;
  uint8_t data[64];
};
message tosend;

// Protobuf container
ReadingSlug slug;
pb_ostream_t pbout;

#define SECOND 1000
#define MINUTE (60 * SECOND)
void sendMessage();
Task radioTask(10 * MINUTE, TASK_FOREVER, &sendMessage);
Scheduler taskManager;

// Setup
void setup() {
  Serial.begin(115200);
  while(!Serial) delay(1);

  // Load board address from EEPROM
  if (i2ceeprom.begin(0x50)) {
    MY_ADDR = i2ceeprom.read(0x00);
  } else {
    MY_ADDR = 64;
    Serial.println("Using fallback address");
  }

  pinMode(LED, OUTPUT);

  // Start RadioLib
  int status = radio.begin(RFM95_FREQ);
  if (status != RADIOLIB_ERR_NONE) {
    Serial.print("Radio init failed: "); Serial.println(status);
    while(true);
  }

  // Set up protobuf fields
  slug.has_r1 = true;
  slug.r1.type = ReadingType_REL_HUMID;

  slug.has_r2 = true;
  slug.r2.type = ReadingType_REL_HUMID;

  slug.has_r3 = true;
  slug.r3.type = ReadingType_REL_HUMID;

  taskManager.addTask(radioTask);
  radioTask.enable();
}

void loop() {
  taskManager.execute();
}

// Main send function
void sendMessage() {
  digitalWrite(LED, HIGH);

  float soil1, soil2, soil3;

  read_soil_rh(SENSOR_1_PIN, soil1);
  read_soil_rh(SENSOR_2_PIN, soil2);
  read_soil_rh(SENSOR_3_PIN, soil3);

  slug.r1.value = soil1;
  slug.r2.value = soil2;
  slug.r3.value = soil3;

  slug.has_power = (analogRead(BATTERY_PIN) >= 200);

  Serial.print("Soil1: "); Serial.println(slug.r1.value);
  Serial.print("Soil2: "); Serial.println(slug.r2.value);
  Serial.print("Soil3: "); Serial.println(slug.r3.value);
  Serial.print("Battery OK: "); Serial.println(slug.has_power);

  // Encode protobuf
  pbout = pb_ostream_from_buffer(tosend.data, sizeof(tosend.data));
  if (!pb_encode(&pbout, ReadingSlug_fields, &slug)) {
    Serial.println("PB encode failed!");
    digitalWrite(LED, LOW);
    return;
  }

  Serial.print("Encoded bytes: ");
  Serial.println(pbout.bytes_written);

  // Fill header
  tosend.myaddr = MY_ADDR;
  tosend.message_id++;

  // Compute packet size
  uint8_t total_len =
      sizeof(tosend.myaddr) +
      sizeof(tosend.message_id) +
      pbout.bytes_written;

  // Send via RadioLib
  int err = radio.transmit((uint8_t *)&tosend, total_len);
  Serial.print("Transmit status: ");
  Serial.println(err);

  digitalWrite(LED, LOW);
}
