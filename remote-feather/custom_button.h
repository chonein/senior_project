/**
 * @brief Interface for big button. builtin debouncing
 * author Christian Honein
 */
#ifndef BUTTON_H_
#define BUTTON_H_

enum State {
  WAIT_FOR_BUTTON_DOWN,
  BUTTON_DOWN_DEBOUNCE,
  WAIT_FOR_BUTTON_UP,
  WAIT_FOR_DEBOUNCE
};
typedef enum State STATE;

#define BIG_BUTTON_PIN 5

void init_btn();
uint8_t get_btn();

#endif
