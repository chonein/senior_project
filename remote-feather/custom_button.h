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

// typedef enum ButtonRes { NOTHING, SHORT_CLICK, LONG_CLICK } ButtonRes;

struct button {
  uint32_t pin;
  unsigned long button_clicked_time = 0;
  unsigned long hold_duration = 0;
  uint8_t button_clicked = 0;
  STATE button_state = WAIT_FOR_BUTTON_DOWN;
};

#define NUM_BUTTONS 2
#define BIG_GREEN_BUTTON_PIN 5
#define BIG_RED_BUTTON_PIN 6
#define BIG_GREEN_IDX 0
#define BIG_RED_IDX 1

void init_btn();
uint8_t get_btn(uint8_t button_idx);
unsigned long get_btn_hold_dur(uint8_t button_idx);

#endif
