#include <RH_RF69.h>
#include <SPI.h>

// #include "custom_button.h"
#include "custom_button2.h"

#define RF69_FREQ 915.0

// First 3 here are boards w/radio BUILT-IN. Boards using FeatherWing follow.
#define RFM69_CS 8
#define RFM69_INT 3
#define RFM69_RST 4
#define LED 13

// battery pin
#define VBATPIN A7

#define BATTERY_FLAG 0xA0

#define RED_BUTTON_PACKET 0
#define GREEN_BUTTON_PACKET 1
#define TWO_BUTTON_PACKET 2

#define PACKET_HOLD_BIT_POS 6
#define PACKET_NUM_CLICKS_POS 4

#define BIG_GREEN_BUTTON_PIN 5
#define BIG_RED_BUTTON_PIN 6

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

int16_t packetnum = 0; // packet counter, we increment per xmission
Button green_button(BIG_GREEN_BUTTON_PIN);
Button red_button(BIG_RED_BUTTON_PIN);

void setup() {
  Serial.begin(115200);
  // while (!Serial)
  //   delay(100); // Wait for Serial Console (comment out line if no computer)

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

  sendBatteryPeriodically();

  handle_two_buttons();

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

void sendFlag(uint8_t flag) {
  rf69.send(&flag, 1);
  Blink(LED, 50, 1);
  // Serial.printf("%08x\n", flag);
}

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

typedef enum {
  TWO_BUTTONS_WAIT,
  TWO_BUTTONS_CHECK_FOR_OTHER_HOLD,
} two_buttons_state_t;

void handle_two_buttons() {
  static two_buttons_state_t state = TWO_BUTTONS_WAIT;
  static unsigned long first_long_click;
  static button_status_t original_status;
  static Button *other_button;
  green_button.update();
  red_button.update();

  if (green_button.status.clicked && !green_button.status.hold) {
    sendFlag(0 << PACKET_HOLD_BIT_POS |
             (green_button.status.num_clicks & 0x3) << PACKET_NUM_CLICKS_POS |
             GREEN_BUTTON_PACKET);
    Serial.printf("Green click: %d times\n", green_button.status.num_clicks);
  }
  if (red_button.status.clicked && !red_button.status.hold) {
    sendFlag(0 << PACKET_HOLD_BIT_POS |
             (red_button.status.num_clicks & 0x3) << PACKET_NUM_CLICKS_POS |
             RED_BUTTON_PACKET);
    Serial.printf("Red click: %d times\n", red_button.status.num_clicks);
  }

  switch (state) {
  case TWO_BUTTONS_WAIT:
    if (green_button.status.clicked && green_button.status.hold) {
      if (red_button.status.clicked && red_button.status.hold) {
        Serial.printf("Green click: %d times\n",
                      green_button.status.num_clicks);
        // long click
        // Both buttons long clicked
        sendFlag(1 << PACKET_HOLD_BIT_POS |
                 (green_button.status.num_clicks & 0x3)
                     << PACKET_NUM_CLICKS_POS |
                 TWO_BUTTON_PACKET);
        Serial.printf("Both hold: %d times\n", green_button.status.num_clicks);
      } else {
        first_long_click = millis();
        original_status = green_button.status;
        other_button = &red_button;
        state = TWO_BUTTONS_CHECK_FOR_OTHER_HOLD;
        Serial.printf("(Green hold): %d times\n",
                      green_button.status.num_clicks);
      }
    } else if (red_button.status.clicked && red_button.status.hold) {
      first_long_click = millis();
      original_status = red_button.status;
      other_button = &green_button;
      state = TWO_BUTTONS_CHECK_FOR_OTHER_HOLD;
      Serial.printf("(Red hold): %d times\n", red_button.status.num_clicks);
    }
    break;

  case TWO_BUTTONS_CHECK_FOR_OTHER_HOLD:
    if (millis() - first_long_click > 500) {
      sendFlag(1 << PACKET_HOLD_BIT_POS |
               (original_status.num_clicks & 0x3) << PACKET_NUM_CLICKS_POS |
               (other_button == &red_button ? GREEN_BUTTON_PACKET
                                            : RED_BUTTON_PACKET));
      state = TWO_BUTTONS_WAIT;
      Serial.printf("%s hold: %d times\n",
                    other_button == &red_button ? "Green" : "Red",
                    original_status.num_clicks);
    } else if (other_button->status.clicked && other_button->status.hold) {
      sendFlag(1 << PACKET_HOLD_BIT_POS |
               (original_status.num_clicks & 0x3) << PACKET_NUM_CLICKS_POS |
               TWO_BUTTON_PACKET);
      state = TWO_BUTTONS_WAIT;
      Serial.printf("Both hold: %d times\n", original_status.num_clicks);
    }
    break;

  default:
    break;
  }
}
