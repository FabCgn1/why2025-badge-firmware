
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
