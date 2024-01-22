#include "custom_button.h"

const unsigned BUTTON_DEBOUNCE_TIME = 300;
const unsigned BUTTON_DOWN_DEBOUNCE_TIME = 100;

struct button buttons[NUM_BUTTONS];
/**
 * @brief initializes button on BIG_BUTTON_PIN
 */
void init_btn() {
  buttons[BIG_GREEN_IDX].pin = BIG_GREEN_BUTTON_PIN;
  buttons[BIG_RED_IDX].pin = BIG_RED_BUTTON_PIN;

  for (struct button button : buttons) {
    pinMode(button.pin, INPUT_PULLUP);
  }
}

/**
 * @brief Gets the button state. Builtin debouncing
 * Click is sent on button up
 * no new clicks are sent if button is hold
 * @return uint8_t true when button is click. otherwise false
 */
uint8_t get_btn(uint8_t button_idx) {
  struct button *b = buttons + button_idx;

  switch (b->button_state) {
  case WAIT_FOR_BUTTON_DOWN:
    // Serial.println("WAIT_FOR_BUTTON_DOWN");
    if (!digitalRead(b->pin)) {
      // button clicked
      b->button_state = BUTTON_DOWN_DEBOUNCE;
      b->button_clicked_time = millis();
    }
    break;

  case BUTTON_DOWN_DEBOUNCE:
    if (millis() - b->button_clicked_time > 10) {
      // after 10 ms check if button still clicked
      if (!digitalRead(b->pin)) {
        // button still clicked. register click
        b->button_state = WAIT_FOR_BUTTON_UP;
        // return 1;
      } else {
        // button not clicked. ignore click
        b->button_state = WAIT_FOR_BUTTON_DOWN;
      }
    }
    break;

  case WAIT_FOR_BUTTON_UP:
    if (digitalRead(b->pin) &&
        millis() - b->button_clicked_time > BUTTON_DOWN_DEBOUNCE_TIME) {
      // button raised
      b->hold_duration = millis() - b->button_clicked_time;
      if (b->hold_duration > BUTTON_DEBOUNCE_TIME) {
        b->button_state = WAIT_FOR_BUTTON_DOWN;
      } else {
        b->button_state = WAIT_FOR_DEBOUNCE;
      }
      return 1;
    }
    break;

  case WAIT_FOR_DEBOUNCE:
    if (millis() - b->button_clicked_time > BUTTON_DEBOUNCE_TIME) {
      b->button_state = WAIT_FOR_BUTTON_DOWN;
    }
    break;

  default:
    b->button_state = WAIT_FOR_BUTTON_DOWN;
    break;
  }

  return 0;
}

unsigned long get_btn_hold_dur(uint8_t button_idx) {
  return buttons[button_idx].hold_duration;
}
