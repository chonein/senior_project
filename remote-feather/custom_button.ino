#include "custom_button.h"

const unsigned BUTTON_DEBOUNCE_TIME = 300;
const unsigned BUTTON_DOWN_DEBOUNCE_TIME = 100;

/**
 * @brief initializes button on BIG_BUTTON_PIN
 */
void init_btn() { pinMode(BIG_BUTTON_PIN, INPUT_PULLUP); }

/**
 * @brief Gets the button state. Builtin debouncing
 * Click is sent on button up
 * no new clicks are sent if button is hold
 * @return uint8_t true when button is click. otherwise false
 */
uint8_t get_btn() {
  static unsigned long button_clicked_time = 0;
  static uint8_t button_clicked = 0;
  static STATE button_state = WAIT_FOR_BUTTON_DOWN;

  switch (button_state) {
  case WAIT_FOR_BUTTON_DOWN:
    if (!digitalRead(BIG_BUTTON_PIN)) {
      // button clicked
      button_state = BUTTON_DOWN_DEBOUNCE;
      button_clicked_time = millis();
    }
    break;

  case BUTTON_DOWN_DEBOUNCE:
    if (millis() - button_clicked_time > 10) {
      // after 10 ms check if button still clicked
      if (!digitalRead(BIG_BUTTON_PIN)) {
        // button still clicked. register click
        button_state = WAIT_FOR_BUTTON_UP;
        return 1;
      } else {
        // button not clicked. ignore click
        button_state = WAIT_FOR_BUTTON_DOWN;
      }
    }
    break;

  case WAIT_FOR_BUTTON_UP:
    if (digitalRead(BIG_BUTTON_PIN) &&
        millis() - button_clicked_time > BUTTON_DOWN_DEBOUNCE_TIME) {
      // button raised
      if (millis() - button_clicked_time > BUTTON_DEBOUNCE_TIME) {
        button_state = WAIT_FOR_BUTTON_DOWN;
      } else {
        button_state = WAIT_FOR_DEBOUNCE;
      }
    }
    break;

  case WAIT_FOR_DEBOUNCE:
    if (millis() - button_clicked_time > BUTTON_DEBOUNCE_TIME) {
      button_state = WAIT_FOR_BUTTON_DOWN;
    }
    break;

  default:
    button_state = WAIT_FOR_BUTTON_DOWN;
    break;
  }

  return 0;
}
