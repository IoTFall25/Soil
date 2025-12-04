#include "sensor_readings.pb.h"
#include <RadioLib.h>
RFM95 radio = new Module(RFM95_CS, RFM95_INT, RFM95_RST);
#include <TaskScheduler.h>
#include "readsoil.h"

// --- Addresses + Pins ---
//#define MY_ADDR 64
#include "Adafruit_EEPROM_I2C.h"
Adafruit_EEPROM_I2C i2ceeprom;
uint8_t MY_ADDR;

#define BASESTATION_ADDR 0

#define RFM95_CS 16
#define RFM95_INT 21
#define RFM95_RST 17

#define SENSOR_1_PIN A0
#define SENSOR_2_PIN A1
#define SENSOR_3_PIN A2
#define BATTERY_PIN A3

#define LED 13
#define RFM95_FREQ 915.0

// --- Radio ---
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// EXACT SAME STRUCT AS MESH
struct message {
    uint8_t myaddr = MY_ADDR;
    uint8_t message_id = 0;
    uint8_t hop_count = 0;
    uint8_t data[64];
};

//variables for sending
message tosend;
ReadingSlug slug;
pb_ostream_t pbout;

// --- Task Timing ---
#define SECOND 1000
#define MINUTE 60 * SECOND

void sendMessage();
Task radioTask(10 * MINUTE, TASK_FOREVER, &sendMessage);
Scheduler taskManager;

void setup() {
  Serial.begin(115200);
  while(!Serial) delay(1);

  // get EEPROM address (board ID)
  if (i2ceeprom.begin(0x50)) {
    MY_ADDR = i2ceeprom.read(0x00);
  } 
  else {
    MY_ADDR = 64; // fallback
    Serial.print("USING FALLBACK ADDR");

  }

  pinMode(LED, OUTPUT);

  // --- Reset radio ---
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH); delay(100);
  digitalWrite(RFM95_RST, LOW);  delay(100);
  digitalWrite(RFM95_RST, HIGH); delay(100);

  if (!rf95.init()) while (1);
  if (!rf95.setFrequency(RFM95_FREQ)) while (1);

  rf95.setTxPower(23, false);

  // --- Protobuf fields ---
  slug.has_r1 = true;
  slug.r1.type = ReadingType_SoilM;
  slug.r1.value = soil_value1;

  slug.has_r2 = true;
  slug.r2.type = ReadingType_SoilM;
  slug.r2.value = soil_value2;

  slug.has_r3 = true;
  slug.r3.type = ReadingType_SoilM;
  slug.r3.value = soil_value3;

  taskManager.addTask(radioTask);
  radioTask.enable();
}

void loop() {
  taskManager.execute();
}

void sendMessage() {
  digitalWrite(LED, HIGH);

  float soil_valueS1, soil_valueS2, soil_valueS3;

  // Soil readings
  read_soil_rh(SENSOR_1_PIN, soil_valueS1);
  slug.r1.value = soil_valueS1;

  read_soil_rh(SENSOR_2_PIN, soil_valueS2);
  slug.r2.value = soil_valueS2;

  read_soil_rh(SENSOR_3_PIN, soil_valueS3);
  slug.r3.value = soil_valueS3;

  // Battery
  slug.has_power = (analogRead(BATTERY_PIN) >= 200);

  //Debug tests
  Serial.print("Soil1: "); Serial.println(slug.r1.value);
  Serial.print("Soil2: "); Serial.println(slug.r2.value);
  Serial.print("Soil3: "); Serial.println(slug.r3.value);
  Serial.print("Battery OK: "); Serial.println(slug.has_power);
  Serial.print("My Address: "); Serial.println(MY_ADDR);

  // Encode protobuf into tosend.data
  pbout = pb_ostream_from_buffer(tosend.data, sizeof(tosend.data));
  if (!pb_encode(&pbout, ReadingSlug_fields, &slug)) {
    digitalWrite(LED, LOW);
    return;
  }
  //debug
  Serial.print("Bytes Encoded: "); Serial.println(pbout.bytes_written);


  // Update header
  tosend.myaddr = MY_ADDR;
  tosend.message_id++;
  tosend.hop_count = 0;

  // Packet length
  uint8_t total_len =
      sizeof(tosend.myaddr) +
      sizeof(tosend.message_id) +
      sizeof(tosend.hop_count) +
      pbout.bytes_written;

  // Send it
  //FLASHING LED ON SEND TO INDICATE TRANSMITTING
  digitalWrite(LED, HIGH);
  rf95.send((uint8_t*)&tosend, total_len);
  rf95.waitPacketSent();
  //fllash led
  digitalWrite(LED, LOW);
}
