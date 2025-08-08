#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#define SDL_WINDOW_WIDTH  600
#define SDL_WINDOW_HEIGHT 400
#define SDL_DELAY_MS 6
#define BMP_PATH "APPS:[dvd_bounce]dvd.bmp"

typedef struct {
    SDL_Window *window;
    SDL_Surface *winSurface;
    SDL_Surface *logoSurface;
    SDL_Surface *coloredLogo;
    int logoWidth;
    int logoHeight;
    Uint64 last_step;
} AppState;

SDL_Surface *LoadBMPSurface(const char *file, int *w, int *h) {
    SDL_Surface *surface = SDL_LoadBMP(file);
    if (!surface) {
        SDL_Log("SDL_LoadBMP failed: %s", SDL_GetError());
        return NULL;
    }

    if (w) *w = surface->w;
    if (h) *h = surface->h;

    printf("DVD dimensions: %i x %i\n", surface->w, surface->h);
    return surface;
}

SDL_Surface *CreateColorModulatedSurface(SDL_Surface *original, SDL_Color color) {
    if (!original) return NULL;

    // Create a copy of the original surface
    SDL_Surface *colored = SDL_CreateSurface(original->w, original->h, original->format);
    if (!colored) {
        SDL_Log("Failed to create colored surface: %s", SDL_GetError());
        return NULL;
    }

    // Copy the original surface
    SDL_BlitSurface(original, NULL, colored, NULL);

    // Apply color modulation by multiplying each pixel's RGB values
    SDL_LockSurface(colored);

    Uint32 *pixels = (Uint32*)colored->pixels;
    int pixel_count = colored->w * colored->h;

    // Get pixel format details for the new SDL3 API
    const SDL_PixelFormatDetails *format_details = SDL_GetPixelFormatDetails(colored->format);
    if (!format_details) {
        SDL_Log("Failed to get pixel format details: %s", SDL_GetError());
        SDL_UnlockSurface(colored);
        SDL_DestroySurface(colored);
        return NULL;
    }

    for (int i = 0; i < pixel_count; i++) {
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixels[i], format_details, NULL, &r, &g, &b, &a);

        // Apply color modulation (multiply and normalize)
        r = (r * color.r) / 255;
        g = (g * color.g) / 255;
        b = (b * color.b) / 255;

        pixels[i] = SDL_MapRGBA(format_details, NULL, r, g, b, a);
    }

    SDL_UnlockSurface(colored);
    return colored;
}

int xspeed = 2;
int yspeed = 2;
static int cornersCount = 0;
SDL_Rect dst;

int colorIndex = 0;
SDL_Color colors[] = {
    {255, 255, 0, 255},   // Yellow
    {255, 255, 255, 255}, // White
    {255, 0, 255, 255},   // Magenta
    {0, 255, 255, 255},   // Cyan
    {0, 0, 255, 255},     // Blue
};
const int MAX_COLORS = sizeof(colors) / sizeof(colors[0]);

void app_initialize(AppState *state) {
    printf("init\n");

    state->logoSurface = LoadBMPSurface(BMP_PATH, &state->logoWidth, &state->logoHeight);
    if (!state->logoSurface) {
        SDL_Log("Failed to load DVD logo");
        return;
    }

    // Create initial colored version
    state->coloredLogo = CreateColorModulatedSurface(state->logoSurface, colors[colorIndex]);

    dst.x = 0;
    dst.y = 0;
    dst.w = state->logoWidth;
    dst.h = state->logoHeight;
}

void app_iterate(AppState *state) {
    // Get window surface
    state->winSurface = SDL_GetWindowSurface(state->window);
    if (!state->winSurface) {
        SDL_Log("Failed to get window surface: %s", SDL_GetError());
        return;
    }

    // Clear the screen to black
    const SDL_PixelFormatDetails *format_details = SDL_GetPixelFormatDetails(state->winSurface->format);
    if (format_details) {
        Uint32 black = SDL_MapRGB(format_details, NULL, 0, 0, 0);
        SDL_FillSurfaceRect(state->winSurface, NULL, black);
    }

    // Update position
    dst.x += xspeed;
    dst.y += yspeed;

    bool colorChanged = false;

    // Check for corners
    if (dst.x >= SDL_WINDOW_WIDTH - state->logoWidth && dst.y >= SDL_WINDOW_HEIGHT - state->logoHeight)
        cornersCount++;
    if (dst.x <= 0 && dst.y <= 0)
        cornersCount++;
    if (dst.x <= 0 && dst.y >= SDL_WINDOW_HEIGHT - state->logoHeight)
        cornersCount++;
    if (dst.y <= 0 && dst.x >= SDL_WINDOW_WIDTH - state->logoWidth)
        cornersCount++;

    // Bounce off walls and change color
    if (dst.x >= SDL_WINDOW_WIDTH - state->logoWidth || dst.x <= 0) {
        xspeed *= -1;
        colorIndex = (colorIndex + 1) % MAX_COLORS;
        colorChanged = true;
    }

    if (dst.y >= SDL_WINDOW_HEIGHT - state->logoHeight || dst.y <= 0) {
        yspeed *= -1;
        colorIndex = (colorIndex + 1) % MAX_COLORS;
        colorChanged = true;
    }

    // Update colored surface if color changed
    if (colorChanged) {
        if (state->coloredLogo) {
            SDL_DestroySurface(state->coloredLogo);
        }
        state->coloredLogo = CreateColorModulatedSurface(state->logoSurface, colors[colorIndex]);
    }

    // Blit the colored logo to the window surface
    if (state->coloredLogo) {
        SDL_BlitSurface(state->coloredLogo, NULL, state->winSurface, &dst);
    }

    // Update the window
    SDL_UpdateWindowSurface(state->window);

    SDL_Delay(SDL_DELAY_MS);
}

static SDL_AppResult app_key_event(AppState *state, SDL_Scancode key_code) {
    switch (key_code) {
        case SDL_SCANCODE_ESCAPE:
        case SDL_SCANCODE_Q:
            return SDL_APP_SUCCESS;
        default:
            break;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState *as = (AppState *)appstate;

    app_iterate(appstate);

    return SDL_APP_CONTINUE;
}

static const struct {
    char const *key;
    char const *value;
} extended_metadata[] = {
    {SDL_PROP_APP_METADATA_CREATOR_STRING, "@franga2000"},
    {SDL_PROP_APP_METADATA_COPYRIGHT_STRING, "Placed in the public domain"},
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    size_t i;

    if (!SDL_SetAppMetadata("DVD bounce demo", "1.0", "org.why2025.badge.dvd")) {
        return SDL_APP_FAILURE;
    }

    for (i = 0; i < SDL_arraysize(extended_metadata); i++) {
        if (!SDL_SetAppMetadataProperty(extended_metadata[i].key, extended_metadata[i].value)) {
            return SDL_APP_FAILURE;
        }
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    AppState *as = (AppState *)SDL_calloc(1, sizeof(AppState));
    if (!as) {
        return SDL_APP_FAILURE;
    }

    *appstate = as;

    // Create window
    as->window = SDL_CreateWindow("DVD Bounce Demo", SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT, 0);
    if (!as->window) {
        printf("Failed to create window: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Check display capabilities
    SDL_DisplayID display = SDL_GetDisplayForWindow(as->window);
    SDL_DisplayMode const *current_mode = SDL_GetCurrentDisplayMode(display);
    if (current_mode) {
        printf(
            "Current display mode: %dx%d @%.2fHz, format: %s\n",
            current_mode->w,
            current_mode->h,
            current_mode->refresh_rate,
            SDL_GetPixelFormatName(current_mode->format)
        );
    }

    app_initialize(as);
    as->last_step = SDL_GetTicks();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
        case SDL_EVENT_KEY_DOWN:
            return app_key_event(appstate, event->key.scancode);
        default:
            break;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    if (appstate != NULL) {
        AppState *as = (AppState *)appstate;

        if (as->coloredLogo) {
            SDL_DestroySurface(as->coloredLogo);
        }
        if (as->logoSurface) {
            SDL_DestroySurface(as->logoSurface);
        }
        if (as->window) {
            SDL_DestroyWindow(as->window);
        }

        SDL_free(as);
    }
}
