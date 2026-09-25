// Simple GUI от RA4ASN
#include "gui_user_include.h"
#if SIMPLE_GUI
#include "gui_includes.h"

uint16_t gui_get_window_draw_width(void)
{
	window_t *win = get_win(get_parent_window());
	return win->draw_x2 - win->draw_x1;
}

uint16_t gui_get_window_draw_height(void)
{
	window_t *win = get_win(get_parent_window());
	return win->draw_y2 - win->draw_y1;
}

// Нарисовать линию в границах окна
void gui_drawline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, gui_color_t color)
{
	window_t *win = get_win(get_current_drawing_window());
	const uint16_t xn = x1 + win->draw_x1;
	const uint16_t yn = y1 + win->draw_y1;
	const uint16_t xk = x2 + win->draw_x1;
	const uint16_t yk = y2 + win->draw_y1;

	GUI_ASSERT(xn < win->draw_x2);
	GUI_ASSERT(xk < win->draw_x2);
	GUI_ASSERT(yn < win->draw_y2);
	GUI_ASSERT(yk < win->draw_y2);

	__gui_draw_line(xn, yn, xk, yk, color);
}

void gui_drawrect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, gui_color_t color, uint8_t fill)
{
	if (w == 0 || h == 0) return;

	window_t *win = get_win(get_current_drawing_window());
	const uint16_t xn = x + win->draw_x1;
	const uint16_t yn = y + win->draw_y1;

	GUI_ASSERT(xn < win->draw_x2);
	GUI_ASSERT(xn + w < win->draw_x2);
	GUI_ASSERT(yn < win->draw_y2);
	GUI_ASSERT(yn + h < win->draw_y2);

	__gui_draw_rect(xn, yn, w, h, color, fill);
}

void gui_drawrect_rounded(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t radius, gui_color_t color, uint8_t fill)
{
	if (w == 0 || h == 0) return;

	window_t *win = get_win(get_current_drawing_window());
	const uint16_t xn = x + win->draw_x1;
	const uint16_t yn = y + win->draw_y1;

	GUI_ASSERT(xn < win->draw_x2);
	GUI_ASSERT(xn + w < win->draw_x2);
	GUI_ASSERT(yn < win->draw_y2);
	GUI_ASSERT(yn + h < win->draw_y2);

	__gui_draw_rounded_rect(xn, yn, w, h, radius, color, fill);
}

void gui_drawrect_transparent(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t alpha)
{
	if (w == 0 || h == 0) return;

	window_t *win = get_win(get_current_drawing_window());
	const uint16_t xn = x + win->draw_x1;
	const uint16_t yn = y + win->draw_y1;

	GUI_ASSERT(xn < win->draw_x2);
	GUI_ASSERT(xn + w < win->draw_x2);
	GUI_ASSERT(yn < win->draw_y2);
	GUI_ASSERT(yn + h < win->draw_y2);

	__gui_draw_semitransparent_rect(xn, yn, w, h, GUI_COLOR_DARKGRAY, alpha);
}

void gui_drawpoint(uint16_t x1, uint16_t y1, gui_color_t color)
{
	window_t *win = get_win(get_current_drawing_window());
	const uint16_t xp = x1 + win->draw_x1;
	const uint16_t yp = y1 + win->draw_y1;

	GUI_ASSERT(xp < win->draw_x2);
	GUI_ASSERT(yp < win->draw_y2);

	__gui_draw_point(xp, yp, color);
}

void gui_drawDashedRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t dashLength, gui_color_t color)
{
	if (w == 0 || h == 0 || dashLength == 0)
		return;

	window_t *win = get_win(get_current_drawing_window());
	const uint16_t xp = x + win->draw_x1;
	const uint16_t yp = y + win->draw_y1;

	GUI_ASSERT(xp < win->draw_x2);
	GUI_ASSERT(yp < win->draw_y2);

	__gui_graw_dashed_rectangle(xp, yp, w, h, dashLength, color);
}

// ********** Drawing into Canvases **********************

void gui_canvas_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, gui_color_t color)
{
	window_t *win = get_win(get_current_drawing_window());
	canvas_t *ca = win->ca_current;
	if (ca == NULL) return;

	const uint16_t xn = x1 + win->draw_x1 + ca->x;
	const uint16_t yn = y1 + win->draw_y1 + ca->y;
	const uint16_t xk = x2 + win->draw_x1 + ca->x;
	const uint16_t yk = y2 + win->draw_y1 + ca->y;

	GUI_ASSERT(xn < win->draw_x2);
	GUI_ASSERT(xk < win->draw_x2);
	GUI_ASSERT(yn < win->draw_y2);
	GUI_ASSERT(yk < win->draw_y2);

	__gui_draw_line(xn, yn, xk, yk, color);
}

void gui_canvas_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, gui_color_t color, uint8_t fill)
{
	window_t *win = get_win(get_current_drawing_window());
	canvas_t *ca = win->ca_current;
	if (ca == NULL || w == 0 || h == 0) return;

	const uint16_t xn = x + win->draw_x1 + ca->x;
	const uint16_t yn = y + win->draw_y1 + ca->y;

	GUI_ASSERT(xn < win->draw_x2);
	GUI_ASSERT(xn + w < win->draw_x2);
	GUI_ASSERT(yn < win->draw_y2);
	GUI_ASSERT(yn + h < win->draw_y2);

	__gui_draw_rect(xn, yn, w, h, color, fill);
}

void gui_canvas_draw_rect_rounded(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t radius, gui_color_t color, uint8_t fill)
{
	window_t *win = get_win(get_current_drawing_window());
	canvas_t *ca = win->ca_current;
	if (ca == NULL || w == 0 || h == 0) return;

	const uint16_t xn = x + win->draw_x1 + ca->x;
	const uint16_t yn = y + win->draw_y1 + ca->y;

	GUI_ASSERT(xn < win->draw_x2);
	GUI_ASSERT(xn + w < win->draw_x2);
	GUI_ASSERT(yn < win->draw_y2);
	GUI_ASSERT(yn + h < win->draw_y2);

	__gui_draw_rounded_rect(xn, yn, w, h, radius, color, fill);
}

void gui_canvas_draw_rect_transparent(uint16_t x, uint16_t y, uint16_t w, uint16_t h, gui_color_t color, uint8_t alpha)
{
	window_t *win = get_win(get_current_drawing_window());
	canvas_t *ca = win->ca_current;
	if (ca == NULL || w == 0 || h == 0) return;

	const uint16_t xn = x + win->draw_x1 + ca->x;
	const uint16_t yn = y + win->draw_y1 + ca->y;

	GUI_ASSERT(xn < win->draw_x2);
	GUI_ASSERT(xn + w < win->draw_x2);
	GUI_ASSERT(yn < win->draw_y2);
	GUI_ASSERT(yn + h < win->draw_y2);

	__gui_draw_semitransparent_rect(xn, yn, w, h, color, alpha);
}

void gui_canvas_draw_point(uint16_t x, uint16_t y, gui_color_t color)
{
	window_t *win = get_win(get_current_drawing_window());
	canvas_t *ca = win->ca_current;
	if (ca == NULL) return;

	const uint16_t xp = x + win->draw_x1 + ca->x;
	const uint16_t yp = y + win->draw_y1 + ca->y;

	GUI_ASSERT(xp < win->draw_x2);
	GUI_ASSERT(yp < win->draw_y2);

	__gui_draw_point(xp, yp, color);
}

void gui_canvas_draw_dashed_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t dashLength, gui_color_t color)
{
	window_t *win = get_win(get_current_drawing_window());
	canvas_t *ca = win->ca_current;

	if (ca == NULL) return;
	if (w == 0 || h == 0 || dashLength == 0) return;

	const uint16_t ox = win->draw_x1 + ca->x;
	const uint16_t oy = win->draw_y1 + ca->y;

	__gui_graw_dashed_rectangle(ox, oy, w, h, dashLength, color);
}

#endif /* SIMPLE_GUI */
