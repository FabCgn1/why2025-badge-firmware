#include <badgevms/compositor.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define COLS 80
#define ROWS 25
#define GLYPH_WIDTH 8
#define GLYPH_HEIGHT 16
#define WINDOW_TITLE "Console"
#define WINDOW_WIDTH (COLS * GLYPH_WIDTH)
#define WINDOW_HEIGHT (ROWS * GLYPH_HEIGHT)

struct cell {
	uint8_t glyph;
};

extern uint8_t			 font[256][16];
extern int translate_scancode (keyboard_event_t *);

static window_handle_t	 window;
static framebuffer_t	*framebuffer;
static struct cell		 cells[COLS * ROWS];
static int				 posx, posy;

static void draw_pixel (int x, int y, uint16_t color)
{
	if (x < 0 || y < 0 || x >= WINDOW_WIDTH || y >= WINDOW_HEIGHT)
		return;
	framebuffer->pixels[y * WINDOW_WIDTH + x] = color;
}

static void fill_rect (int x, int y, int w, int h, uint16_t color)
{
	for (int y0 = 0; y0 < h; ++y0) {
		for (int x0 = 0; x0 < w; ++x0) {
			draw_pixel (x0 + x, y0 + y, color);
		}
	}
}

static void draw_cell (int x, int y, struct cell cell)
{
	uint16_t color;
	uint8_t row;
	bool bit;

	//fill_rect (x * GLYPH_WIDTH, y * GLYPH_HEIGHT, GLYPH_WIDTH, GLYPH_HEIGHT, cell.glyph != 0 ? 0xffff : 0);
	for (int y0 = 0; y0 < GLYPH_HEIGHT; ++y0) {
		row = font[cell.glyph][y0];
		for (int x0 = 0; x0 < GLYPH_WIDTH; ++x0, row <<= 1) {
			bit = (row & 0x80) == 0x80;
			color = bit ? 0xffff : 0x0000;
			draw_pixel (x * GLYPH_WIDTH + x0, y * GLYPH_HEIGHT + y0, color);
		}
	}
}

static void redraw (void)
{
	for (int y = 0; y < ROWS; ++y) {
		for (int x = 0; x < COLS; ++x) {
			draw_cell (x, y, cells[y * COLS + x]);
		}
	}
	window_present (window, true, NULL, 0);
}

static void set_cell (int x, int y, struct cell cell)
{
	if (x < 0 || y < 0 || x >= COLS || y >= ROWS)
		return;

	cells[y * COLS + x] = cell;
	draw_cell (x, y, cell);
}

static void write_char (int ch)
{
	struct cell cell;
	cell.glyph = ' ';

	switch (ch) {
	case '\a':
		// TODO: bell
		break;
	case '\b':
		if (posx == 0)
			break;
		--posx;
		set_cell (posx, posy, cell);
		break;
	case '\f':
		// TODO: feed
		break;
	case '\n':
		++posy;
		/* fallthrough */
	case '\r':
		posx = 0;
		break;
	case '\t':
		// TODO: tab
		break;
	case '\v':
		// TODO: vertical tab
		break;
	default:
		cell.glyph = ch;
		set_cell (posx++, posy, cell);
		break;
	}

	if (posx >= COLS) {
		posx = 0;
		++posy;
	}

	if (posy >= ROWS) {
		posy = ROWS - 1;
		memmove (cells, &cells[COLS], sizeof (struct cell) * COLS * (ROWS - 1));
		memset (&cells[COLS * (ROWS - 1)], 0, sizeof (struct cell) * COLS);
		redraw ();
	}
}

static void send_key (int key)
{
	write_char (key);
	redraw ();
}

static bool on_key (keyboard_event_t *e)
{
	int key;

	key = translate_scancode (e);
	switch (key) {
	case -1:
		return true;
	case '\014':
		redraw ();
		return true;
	case '\033':
		return false;
	default:
		send_key (key);
		return true;
	}
}

int main (void)
{
	window_size_t size;
	event_t event;

	size.w = WINDOW_WIDTH;
	size.h = WINDOW_HEIGHT;

	window = window_create (WINDOW_TITLE, size, 0);
	framebuffer = window_framebuffer_create (window, size, BADGEVMS_PIXELFORMAT_RGB565);

	redraw ();

	while (1) {
		event = window_event_poll (window, true, 1000);
		switch (event.type) {
		case EVENT_NONE:
			break;
		case EVENT_QUIT:
			return 0;
		case EVENT_KEY_DOWN:
			if (!on_key (&event.keyboard))
				break;
			break;
		default:
			break;
		}
	}

	return 0;
}
