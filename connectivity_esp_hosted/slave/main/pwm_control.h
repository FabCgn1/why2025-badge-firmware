#include <stdint.h>
#include "driver/ledc.h"

// Pins ggf. zentral hier definieren
#ifndef BL_DISPLAY_GPIO
#define BL_DISPLAY_GPIO  15
#endif
#ifndef BL_KEYBOARD_GPIO
#define BL_KEYBOARD_GPIO 10
#endif

void pwm_init(void);

// Prozent 0..100; intern auf [0,10..80] begrenzt (wie in deinem Code)
void pwm_set_display(uint8_t percent);
void pwm_set_keyboard(uint8_t percent);

uint8_t pwm_get_display(void);
uint8_t pwm_get_keyboard(void);
