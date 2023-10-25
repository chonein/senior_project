#include <RH_RF69.h>
#include <SPI.h>

#include "custom_button.h"

#define RF69_FREQ 915.0

// First 3 here are boards w/radio BUILT-IN. Boards using FeatherWing follow.
#define RFM69_CS 8
#define RFM69_INT 3
#define RFM69_RST 4
#define LED 13

// battery pin
#define VBATPIN A7

const uint8_t BUTTON_GREEN_CLICK = 0x10;
const uint8_t BUTTON_RED_CLICK = 0x20;
#define BATTERY_FLAG 0x30

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

int16_t packetnum = 0; // packet counter, we increment per xmission

void setup() {
  Serial.begin(115200);
  // while (!Serial)
  //   delay(1); // Wait for Serial Console (comment out line if no computer)
  init_btn();

  pinMode(LED, OUTPUT);
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  Serial.println("Feather RFM69 TX Test!");
  Serial.println();

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

  if (!rf69.init()) {
    Serial.println("RFM69 radio init failed");
    while (1)
      ;
  }
  Serial.println("RFM69 radio init OK!");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for
  // low power module) No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println("setFrequency failed");
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power
  // with the ishighpowermodule flag set like this:
  rf69.setTxPower(
      14, true); // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the server
  //   uint8_t key[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
  //                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  //   rf69.setEncryptionKey(key);

  Serial.print("RFM69 radio @");
  Serial.print((int)RF69_FREQ);
  Serial.println(" MHz");
}

void loop() {
  static char buff[RH_RF69_MAX_MESSAGE_LEN];
  static uint32_t time_checkpoint = 0;
  //   delay(1000); // Wait 1 second between transmits, could also 'sleep' here!

  if (millis() - time_checkpoint > 200) {
    static uint32_t prevState = LOW;
    uint32_t newState = prevState == LOW ? HIGH : LOW;
    digitalWrite(LED, newState);
    prevState = newState;
    time_checkpoint = millis();
  }

  sendBatteryPeriodically();

  if (Serial.available()) {
    // read in buff. keep byte available for null byte
    int inputChar = Serial.read();
    if (inputChar == 'g') {
      sendFlag(BUTTON_GREEN_CLICK);
    } else if (inputChar == 'r') {
      sendFlag(BUTTON_RED_CLICK);
    }
    rf69.waitPacketSent();
  }
  if (get_btn()) {
    sendFlag(BUTTON_GREEN_CLICK);
  }

  // Now wait for a reply
  uint8_t len = sizeof(buff);

  if (rf69.available()) {
    // Should be a reply message for us now
    if (rf69.recv((uint8_t *)buff, &len)) {
      Serial.print("Got a reply: ");
      Serial.println((char *)buff);
      Blink(LED, 50, 3); // blink LED 3 times, 50ms between blinks
    } else {
      Serial.println("Receive failed");
    }
  }
}

void Blink(byte pin, byte delay_ms, byte loops) {
  while (loops--) {
    digitalWrite(pin, HIGH);
    delay(delay_ms);
    digitalWrite(pin, LOW);
    delay(delay_ms);
  }
}

void sendFlag(uint8_t flag) { rf69.send(&flag, 1); }

void sendBatteryPeriodically() {
  static uint32_t last_sent_time = 0;

  if (millis() - last_sent_time > 30000) {
    sendBattery();
    last_sent_time = millis();
  }
}

void sendBattery() {
  float measuredbat = getBattery();
  uint8_t batteryPacket[1 + sizeof(measuredbat)];
  batteryPacket[0] = BATTERY_FLAG;
  memcpy(batteryPacket + 1, &measuredbat, sizeof(measuredbat));
  rf69.send(batteryPacket, sizeof(batteryPacket));
}

float getBattery() {
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  return measuredvbat;
  // Serial.print("VBat: " ); Serial.println(measuredvbat);
}