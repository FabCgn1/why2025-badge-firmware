#include <stdint.h>
#include <string.h>
#include "pwm.h"

// ---- HIER anpassen: dein existierender Send-Wrapper ----
static int protocomm_send_ep(const char *endpoint,
                             const uint8_t *tx, uint32_t tx_len,
                             uint8_t *rx, uint32_t *rx_len)
{
    // TODO: Diese Funktion gegen deinen echten Transport austauschen.
    // Erwartet: sendet (endpoint, payload), liest optional 1 Byte Status zurück.
    // Return 0 bei Erfolg, <0 bei Fehler.
    (void)endpoint; (void)tx; (void)tx_len; (void)rx; (void)rx_len;
    return 0; // Stub: OK
}
// --------------------------------------------------------

static int clamp01(int v) { if (v<0) v=0; if (v>100) v=100; return v; }

static int s_disp = -1;
static int s_kbd  = -1;

void pwm_host_init(void) { /* optional */ }

static int send_pwm(uint8_t channel, uint8_t percent)
{
    uint8_t payload[2] = { channel, percent };   // [0]=0 Display, 1 Keyboard ; [1]=0..100
    uint8_t resp[1] = {0};
    uint32_t rlen = sizeof(resp);

    int rc = protocomm_send_ep("pwm", payload, sizeof(payload), resp, &rlen);
    if (rc < 0) return rc;
    if (rlen == 1 && resp[0] != 0x00) return -1; // 0x00 = OK, 0x50 = clamp info, <0 = Fehler
    return 0;
}

int pwm_set_display_percent(int percent)
{
    percent = clamp01(percent);
    s_disp = percent;
    return send_pwm(0, (uint8_t)percent);
}

int pwm_set_keyboard_percent(int percent)
{
    percent = clamp01(percent);
    s_kbd = percent;
    return send_pwm(1, (uint8_t)percent);
}

int pwm_get_display_percent(void)  { return s_disp; }
int pwm_get_keyboard_percent(void) { return s_kbd;  }
