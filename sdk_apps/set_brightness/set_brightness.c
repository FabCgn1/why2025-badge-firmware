// Minimaler PWM-Controller als SDK-App
// Tasten: ↑/↓ Auswahl, ←/→ +-5%, 0..9 direkte Eingabe (Zehnerschritte), S speichern, ESC Ende

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <badgevms/pwm.h>   // unsere dünne API

#define SCREEN_W 720
#define SCREEN_H 720

static void draw_text(SDL_Renderer *r, int x, int y, const char *txt);
static void draw_ui(SDL_Renderer *r, int sel, int disp, int kbd);

int main(void)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window   *win = SDL_CreateWindow("PWM Control", SCREEN_W, SCREEN_H, SDL_WINDOW_FULLSCREEN);
    SDL_Renderer *ren = SDL_CreateRenderer(win, NULL);
    if (!win || !ren) { SDL_Log("SDL create failed: %s", SDL_GetError()); return 1; }

    // aktuelle Werte vom System holen (wenn nicht implementiert -> kommt 0 zurück)
    int disp = pwm_get_display_percent();
    int kbd  = pwm_get_keyboard_percent();
    if (disp < 0) disp = 0;
    if (kbd  < 0) kbd  = 0;

    int sel = 0; // 0 = Display, 1 = Keyboard
    bool quit = false;

    // einmalige Initialisierung auf dem Coprozessor (optional; macht auf dem Slave nichts kaputt)
    pwm_host_init(); // no-op, falls leer implementiert

    while (!quit) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) quit = true;
            if (ev.type == SDL_EVENT_KEY_DOWN) {
                SDL_Keycode k = ev.key.key;
                if (k == SDLK_ESCAPE) quit = true;
                else if (k == SDLK_UP   ) sel = (sel + 1) % 2;
                else if (k == SDLK_DOWN ) sel = (sel + 1) % 2;
                else if (k == SDLK_LEFT ) { if (sel==0) disp = (disp>=5?disp-5:0); else kbd = (kbd>=5?kbd-5:0); }
                else if (k == SDLK_RIGHT) { if (sel==0) disp = (disp<=95?disp+5:100); else kbd = (kbd<=95?kbd+5:100); }
                else if (k >= SDLK_0 && k <= SDLK_9) {
                    int step = (k - SDLK_0) * 10;
                    if (sel==0) disp = step; else kbd = step;
                }
                else if (k == SDLK_S || k == SDLK_S) {
                    // S: explizit beide Werte nochmal senden
                    pwm_set_display_percent(disp);
                    pwm_set_keyboard_percent(kbd);
                }

                // Live-Übernahme: bei jeder Änderung direkt senden
                if (sel==0) pwm_set_display_percent(disp);
                else        pwm_set_keyboard_percent(kbd);
            }
        }

        SDL_SetRenderDrawColor(ren, 25, 25, 25, 255);
        SDL_RenderClear(ren);
        draw_ui(ren, sel, disp, kbd);
        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

// —————————————————————— super einfache UI (nur Rechtecke + Text) ——————————————————————

static void bar(SDL_Renderer *r, int x, int y, int w, int h, int percent, bool sel) {
    SDL_Rect bg = {x, y, w, h};
    SDL_Rect fg = {x+2, y+2, (w-4) * percent / 100, h-4};
    SDL_SetRenderDrawColor(r, sel?40:60, sel?80:60, sel?160:60, 255); SDL_RenderFillRect(r, &bg);
    SDL_SetRenderDrawColor(r, 200, 200, 200, 255); SDL_RenderFillRect(r, &fg);
}

static void draw_text(SDL_Renderer *r, int x, int y, const char *txt) {
    // Platzhalter: Wenn du font.h / Bitmapfont hast, benutze das hier.
    // Für jetzt: nichts (die App funktioniert auch ohne Text hübsch).
    (void)r; (void)x; (void)y; (void)txt;
}

static void draw_ui(SDL_Renderer *r, int sel, int disp, int kbd) {
    int x=80, w=560, h=80;
    bar(r, x, 200, w, h, disp, sel==0);
    bar(r, x, 360, w, h, kbd, sel==1);
    // Optional Text:
    // draw_text(r, x, 170, "Display");
    // draw_text(r, x, 330, "Keyboard");
}
