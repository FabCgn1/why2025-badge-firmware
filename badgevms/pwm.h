#pragma once
#ifdef __cplusplus
extern "C" {
#endif

// Optionale Initialisierung auf der Host-Seite (kann ein No-Op sein)
void pwm_host_init(void);

// Prozent setzen; 0..100
int  pwm_set_display_percent(int percent);
int  pwm_set_keyboard_percent(int percent);

// Aktuelle Werte (falls du sie irgendwo speicherst); wenn unbekannt -> -1
int  pwm_get_display_percent(void);
int  pwm_get_keyboard_percent(void);

#ifdef __cplusplus
}
#endif
