#include "pwm_control.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define PWM_TIMER_BASE_CLK LEDC_USE_XTAL_CLK
#define PWM_TIMER_RESOLUTION LEDC_TIMER_8_BIT      // wie im Original
#define PWM_TIMER_FREQ_HZ 25000
#define PWM_MODE  LEDC_LOW_SPEED_MODE
#define PWM_TIMER LEDC_TIMER_0
#define CH_DISPLAY  LEDC_CHANNEL_0
#define CH_KEYBOARD LEDC_CHANNEL_1

static uint8_t g_disp = 3;  // Prozent-Default wie vorher gesetzt
static uint8_t g_kb   = 3;

static inline uint32_t pct_to_duty(uint8_t pct)
{
    // Clamp 0..100 → 0,10..80% (dein Limit)
    if (pct > 100) pct = 100;
    if (pct != 0 && pct < 10) pct = 10;
    if (pct > 80) pct = 80;

    const uint32_t max = (1u << PWM_TIMER_RESOLUTION) - 1; // 255
    return (max * pct) / 100;
}

void pwm_init(void)
{
    // Timer
    ledc_timer_config_t timer_conf = {
        .speed_mode = PWM_MODE,
        .duty_resolution = PWM_TIMER_RESOLUTION,
        .timer_num = PWM_TIMER,
        .freq_hz = PWM_TIMER_FREQ_HZ,
        .clk_cfg = PWM_TIMER_BASE_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));

    // Display Channel
    ledc_channel_config_t c0 = {
        .gpio_num = BL_DISPLAY_GPIO,
        .speed_mode = PWM_MODE,
        .channel = CH_DISPLAY,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = PWM_TIMER,
        .duty = pct_to_duty(g_disp),
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&c0));

    // Keyboard Channel
    ledc_channel_config_t c1 = {
        .gpio_num = BL_KEYBOARD_GPIO,
        .speed_mode = PWM_MODE,
        .channel = CH_KEYBOARD,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = PWM_TIMER,
        .duty = pct_to_duty(g_kb),
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&c1));
}

void pwm_set_display(uint8_t percent)
{
    g_disp = percent;
    ESP_ERROR_CHECK(ledc_set_duty(PWM_MODE, CH_DISPLAY, pct_to_duty(g_disp)));
    ESP_ERROR_CHECK(ledc_update_duty(PWM_MODE, CH_DISPLAY));
}

void pwm_set_keyboard(uint8_t percent)
{
    g_kb = percent;
    ESP_ERROR_CHECK(ledc_set_duty(PWM_MODE, CH_KEYBOARD, pct_to_duty(g_kb)));
    ESP_ERROR_CHECK(ledc_update_duty(PWM_MODE, CH_KEYBOARD));
}

uint8_t pwm_get_display(void)  { return g_disp; }
uint8_t pwm_get_keyboard(void) { return g_kb;   }
