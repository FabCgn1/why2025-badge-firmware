#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define WINDOW_WIDTH 700
#define WINDOW_HEIGHT 700
#define WIDTH 80
#define HEIGHT 25
#define new(T) ((T *)SDL_calloc (1, sizeof (T)))

extern const uint8_t font[256][16];

struct cell {
	uint8_t glyph;
};

struct state {
	SDL_Window		*window;
	SDL_Renderer	*renderer;
	SDL_Texture		*font;
	struct cell	 	 cells[WIDTH * HEIGHT];
	uint8_t		 	 x, y;
	bool			 debug;
};

static void draw_cell (
	struct state *state,
	int x,
	int y,
	struct cell cell,
	SDL_Rect *vp
) {
	SDL_FRect sr, dr;

	if (cell.glyph == 0)
		return;

	sr.x = (cell.glyph % 16) * 8.0f;
	sr.y = (cell.glyph / 16) * 16.0f;
	sr.w = 8.0f;
	sr.h = 16.0f;
	dr.x = (float)x / (float)WIDTH * (float)vp->w;
	dr.y = (float)y / (float)HEIGHT * (float)vp->h;
	dr.w = (float)vp->w / (float)WIDTH;
	dr.h = (float)vp->h / (float)HEIGHT;

	SDL_RenderTexture (state->renderer, state->font, &sr, &dr);
}

static bool set_cell (
	struct state *state,
	int x,
	int y,
	struct cell cell
) {
	SDL_Rect vp;

	if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT)
		return false;

	state->cells[y * WIDTH + x] = cell;

	SDL_GetRenderViewport (state->renderer, &vp);
	draw_cell (state, x, y, cell, &vp);
	SDL_RenderPresent (state->renderer);
	return true;
}

static void scroll_up (struct state *state)
{
	memmove (state->cells, &state->cells[WIDTH], sizeof (struct cell) * WIDTH * (HEIGHT - 1));
	memset (&state->cells[WIDTH * (HEIGHT - 1)], 0, sizeof (struct cell) * WIDTH);
}

static void write_char (struct state *state, char ch)
{
	struct cell cell;
	cell.glyph = ' ';

	switch (ch) {
	case '\a':
		// TODO: bell
		break;
	case '\b':
		if (state->x > 0) {
			--state->x;
		}
		set_cell (state, state->x, state->y, cell);
		break;
	case '\f':
		// TODO: feed
		break;
	case '\n':
		state->x = 0;
		++state->y;
		break;
	case '\r':
		state->x = 0;
		break;
	case '\t':
		// TODO: tab
		break;
	case '\v':
		// TODO: vertical tab
		break;
	default:
		cell.glyph = ch;
		set_cell (state, state->x, state->y, cell);
		++state->x;
		break;
	}

	if (state->x >= WIDTH) {
		state->x = 0;
		++state->y;
	}

	if (state->y >= HEIGHT)
		scroll_up (state);
}

static void redraw (struct state *);
static SDL_AppResult send_char (struct state *state, int ch)
{
	printf ("send_char(0x%02x);\n", ch);
	switch (ch) {
	case '\033':
		return SDL_APP_SUCCESS;
	case '\t':
		state->debug = !state->debug;
		printf ("debug: %d\n", state->debug);
		redraw (state);
		break;
	default:
		write_char (state, ch);
		break;
	}
	return SDL_APP_CONTINUE;
}

static void redraw (struct state *state)
{
	SDL_Rect vp;
	struct cell cell;

	SDL_RenderClear (state->renderer);

	if (state->debug) {
		SDL_RenderTexture (state->renderer, state->font, NULL, NULL);
		goto present;
	}

	SDL_GetRenderViewport (state->renderer, &vp);

	for (int y = 0; y < HEIGHT; ++y) {
		for (int x = 0; x < WIDTH; ++x) {
			cell = state->cells[y * WIDTH + x];
			draw_cell (state, x, y, cell, &vp);
		}
	}

present:
	SDL_RenderPresent (state->renderer);
}

static SDL_Surface *load_font (void)
{
	SDL_Surface *surface;
	uint8_t *pixels, row;
	uint8_t color, glyph[16];
	bool bit;

	surface = SDL_CreateSurface (16 * 8, 16 * 16, SDL_PIXELFORMAT_RGB565);
	pixels = surface->pixels;
	
	for (int y = 0; y < 16; ++y) {
		for (int x = 0; x < 16; ++x) {
			memcpy (glyph, font[y * 16 + x], sizeof (glyph));
			for (int y0 = 0; y0 < 16; ++y0) {
				row = glyph[y0];
				for (int x0 = 0; x0 < 8; ++x0) {
					bit = ((row << x0) & 0x80) == 0x80;
					color = bit ? 0xff : 0x00;
					SDL_WriteSurfacePixel (surface, x * 8 + x0, y * 16 + y0, color, color, color, 255);
				}
			}
		}
	}

	return surface;
}

SDL_AppResult SDL_AppInit (
	void **appstate,
	int argc,
	char *argv[]
) {
	struct state *state;
	SDL_Surface *font;

	if (!SDL_SetAppMetadata ("Console", "1.0", "xyz.stuerz.why2025.console"))
		return SDL_APP_FAILURE;

	if (!SDL_Init (SDL_INIT_VIDEO)) {
		printf ("SDL_Init(): %s\n", SDL_GetError ());
		return SDL_APP_FAILURE;
	}
	*appstate = state = new (struct state);

	state->window = SDL_CreateWindow ("Console", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
	if (state->window == NULL) {
		printf ("SDL_CreateWindow(): %s\n", SDL_GetError ());
		return SDL_APP_FAILURE;
	}

	// Check display capabilities
	SDL_DisplayID          display      = SDL_GetDisplayForWindow(state->window);
	SDL_DisplayMode const *current_mode = SDL_GetCurrentDisplayMode(display);
	if (current_mode) {
		printf(
				"Current display mode: %dx%d @%.2fHz, format: %s",
				current_mode->w,
				current_mode->h,
				current_mode->refresh_rate,
				SDL_GetPixelFormatName(current_mode->format)
			  );
	}


	int ndrivers = SDL_GetNumRenderDrivers ();
	for (int i = 0; i < ndrivers; ++i) {
		printf ("driver %d: %s\n", i, SDL_GetRenderDriver (i));
	}

	puts ("Created window");

	state->renderer = SDL_CreateRenderer (state->window, NULL);
	if (state->renderer == NULL) {
		printf ("SDL_CreateRenderer(): %s\n", SDL_GetError ());
		return SDL_APP_FAILURE;
	}

	puts ("Created renderer");

	font = load_font ();

	puts ("Loaded font");

	state->font = SDL_CreateTextureFromSurface (state->renderer, font);
	if (state->font == NULL) {
		printf ("SDL_CreateTextureFromSurface(): %s\n", SDL_GetError ());
		return SDL_APP_FAILURE;
	}

	puts ("Created font texture from surface");

	SDL_DestroySurface (font);

	puts ("Destroyed font surface");

	redraw (state);

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit (
	void *appstate,
	SDL_AppResult result
) {
	struct state *state = appstate;

	if (state == NULL)
		return;

	SDL_DestroyTexture (state->font);
	SDL_DestroyRenderer (state->renderer);
	SDL_DestroyWindow (state->window);
	SDL_free (state);
}

#define SCANMAP												\
	E(SDL_SCANCODE_ESCAPE,		'\033',	-1,		-1)			\
	E(SDL_SCANCODE_BACKSPACE,	'\b',	-1,		-1)			\
															\
	E(SDL_SCANCODE_GRAVE,		'`',	'~',	-1)			\
	E(SDL_SCANCODE_1,			'1',	'!',	-1)			\
	E(SDL_SCANCODE_2,			'2',	'@',	-1)			\
	E(SDL_SCANCODE_3,			'3',	'#',	-1)			\
	E(SDL_SCANCODE_4,			'4',	'$',	-1)			\
	E(SDL_SCANCODE_5,			'5',	'%',	-1)			\
	E(SDL_SCANCODE_6,			'6',	'^',	-1)			\
	E(SDL_SCANCODE_7,			'7',	'&',	-1)			\
	E(SDL_SCANCODE_8,			'8',	'*',	-1)			\
	E(SDL_SCANCODE_9,			'9',	'(',	-1)			\
	E(SDL_SCANCODE_0,			'0',	')',	-1)			\
	E(SDL_SCANCODE_MINUS,		'-',	'_',	-1)			\
	E(SDL_SCANCODE_EQUALS,		'=',	'+',	-1)			\
															\
	E(SDL_SCANCODE_TAB,			'\t',	-1,		-1)			\
	E(SDL_SCANCODE_Q,			'q',	'Q',	'Q' - 0x40)	\
	E(SDL_SCANCODE_W,			'w',	'W',	'W' - 0x40)	\
	E(SDL_SCANCODE_E,			'e',	'E',	'E' - 0x40)	\
	E(SDL_SCANCODE_R,			'r',	'R',	'R' - 0x40)	\
	E(SDL_SCANCODE_T,			't',	'T',	'T' - 0x40)	\
	E(SDL_SCANCODE_Y,			'y',	'Y',	'Y' - 0x40)	\
	E(SDL_SCANCODE_U,			'u',	'U',	'U' - 0x40)	\
	E(SDL_SCANCODE_I,			'i',	'I',	'I' - 0x40)	\
	E(SDL_SCANCODE_O,			'o',	'O',	'O' - 0x40)	\
	E(SDL_SCANCODE_P,			'p',	'P',	'P' - 0x40)	\
	E(SDL_SCANCODE_LEFTBRACKET,	'[',	'{',	']' - 0x40)	\
	E(SDL_SCANCODE_RIGHTBRACKET,']',	'}',	']' - 0x40)	\
	E(SDL_SCANCODE_BACKSLASH,	'\\',	'|',	'\\' - 0x40)\
															\
	E(SDL_SCANCODE_CAPSLOCK,	-1,		-1,		-1)			\
	E(SDL_SCANCODE_A,			'a',	'A',	'A' - 0x40)	\
	E(SDL_SCANCODE_S,			's',	'S',	'S' - 0x40)	\
	E(SDL_SCANCODE_D,			'd',	'D',	'D' - 0x40)	\
	E(SDL_SCANCODE_F,			'f',	'F',	'F' - 0x40)	\
	E(SDL_SCANCODE_G,			'g',	'G',	'G' - 0x40)	\
	E(SDL_SCANCODE_H,			'h',	'H',	'H' - 0x40)	\
	E(SDL_SCANCODE_J,			'j',	'J',	'J' - 0x40)	\
	E(SDL_SCANCODE_K,			'k',	'K',	'K' - 0x40)	\
	E(SDL_SCANCODE_L,			'l',	'L',	'L' - 0x40)	\
	E(SDL_SCANCODE_SEMICOLON,	';',	':',	-1)			\
	E(SDL_SCANCODE_APOSTROPHE,	'\'',	'"',	-1)			\
	E(SDL_SCANCODE_RETURN,		'\n',	-1,		-1)			\
															\
	E(SDL_SCANCODE_Z,			'z',	'Z',	'Z' - 0x40)	\
	E(SDL_SCANCODE_X,			'x',	'X',	'X' - 0x40)	\
	E(SDL_SCANCODE_C,			'c',	'C',	'C' - 0x40)	\
	E(SDL_SCANCODE_V,			'v',	'V',	'V' - 0x40)	\
	E(SDL_SCANCODE_B,			'b',	'B',	'B' - 0x40)	\
	E(SDL_SCANCODE_N,			'n',	'N',	'N' - 0x40)	\
	E(SDL_SCANCODE_M,			'm',	'M',	'M' - 0x40)	\
	E(SDL_SCANCODE_COMMA,		',',	'<',	-1)			\
	E(SDL_SCANCODE_PERIOD,		'.',	'>',	-1)			\
	E(SDL_SCANCODE_SLASH,		'/',	'?',	-1)			\
															\
	E(SDL_SCANCODE_NONUSHASH,	'\\',	'|',	-1)			\
	E(SDL_SCANCODE_NONUSBACKSLASH,'\\',	'|',	-1)			\
	E(SDL_SCANCODE_SPACE,		' ',	' ',	-1)				\

static int select_key (SDL_Keymod mod, int normal, int shift, int ctrl)
{
	if ((mod & SDL_KMOD_CTRL) != 0)
		return ctrl;

	if ((mod & SDL_KMOD_SHIFT) != 0)
		return shift;

	return normal;
}

static int translate (SDL_Scancode code, SDL_Keymod mod)
{
	switch (code) {
#define E(code, normal, shift, ctrl) case code: return select_key (mod, normal, shift, ctrl);
SCANMAP
#undef E
	default:
		printf ("unhandled scancode: 0x%02x\n", code);
		return -1;
	}
}


static SDL_AppResult handle_key_event (
	struct state *state,
	SDL_KeyboardEvent *event
) {
	int key;

	key = translate (event->scancode, event->mod);

	return key != -1 ? send_char (state, key) : SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent (
	void *appstate,
	SDL_Event *event
) {
	struct state *state = appstate;

	switch (event->type) {
	case SDL_EVENT_QUIT:
		return SDL_APP_SUCCESS;
	case SDL_EVENT_KEY_DOWN:
		return handle_key_event (state, &event->key);
	case SDL_EVENT_WINDOW_RESIZED:
	case SDL_EVENT_WINDOW_MOVED:
	case SDL_EVENT_WINDOW_MAXIMIZED:
		redraw (state);
		break;
	default:
		break;
	}
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate (void *appstate)
{
	return SDL_APP_CONTINUE;
}

/* vim: set ts=4 sw=4 noet: */
