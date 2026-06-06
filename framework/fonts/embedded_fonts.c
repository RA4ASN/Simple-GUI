// Simple GUI от RA4ASN
//--------------------------------------------------------------
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
//--------------------------------------------------------------

#include "gui_user_include.h"

#if WITHTOUCHGUI

#include "../gui_system.h"
#include "../gui_structs.h"
#include "../gui_settings.h"
#include "../gui_windows.h"
#include "embedded_fonts.h"

typedef struct {
    uint8_t  width;        // Ширина глифа в пикселях
    uint8_t  height;       // Высота глифа в пикселях
    const uint32_t *data;  // Указатель на начало битовых данных
    uint32_t start_mask;   // Предварительно рассчитанная начальная маска
} gui_glyph_t;

static uint16_t UB_Font_getPcharw32(uint8_t ascii, const gui_prop_font_t * font)
{
	uint16_t width;
	uint32_t start_maske, maske;
	const uint32_t * wert;

	// Проверка границы символа
	if (ascii < font->first_char)
		return 0;

	if (ascii > font->last_char)
		return 0;

	ascii -= font->first_char;
	wert = & font->table[ascii * (font->height + 1)];
	width = wert[0];

	return (width);
}

// Возврат ширины строки в пикселях, пропорциональный шрифт 32 бит
uint16_t get_strwidth_prop(const char * str, const gui_prop_font_t * font)
{
	uint16_t width = 0;

	while (* str != 0)
	{
		width += UB_Font_getPcharw32(* str, font);
		str ++;
	}

	return width;
}

// Возвращает ширину строки в пикселях, моноширинный шрифт
uint16_t get_strwidth_mono(const char * str, const gui_mono_font_t * font) {
	GUI_ASSERT(str != NULL);
	return strlen(str) * font->width;
}

uint16_t get_strheight_mono(const gui_mono_font_t * font)
{
	return font->height;
}

uint16_t get_strheight_prop(const gui_prop_font_t * font)
{
	return font->height;
}

int gui_get_glyph(uint8_t ascii, const gui_mono_font_t *mono, const gui_prop_font_t *prop,
		gui_glyph_t *out)
{
	if (!out) return -1;

	if (prop != NULL)
	{
		// === Пропорциональный шрифт ===
		if (ascii < prop->first_char || ascii > prop->last_char) return -1;
		uint16_t idx = ascii - prop->first_char;
		uint16_t stride = prop->height + 1;
		const uint32_t *p = &prop->table[idx * stride];

		out->width = (uint8_t) p[0];
		out->height = prop->height;
		out->data = &p[1];
		out->start_mask = (out->width > 0) ? (0x01u << (out->width - 1)) : 0;
	}
	else if (mono != NULL)
	{
		// === Моноширинный шрифт ===
		if (ascii < 32) return -1;
		uint16_t idx = ascii - 32;

		out->width = mono->width;
		out->height = mono->height;
		out->data = &mono->table[idx * mono->height];

		if (out->width > 16) out->start_mask = 0x80000000u;
		else if (out->width > 8) out->start_mask = 0x8000u;
		else out->start_mask = 0x80u;
	}
	else
	{
		return -1;
	}
	return 0;
}

void gui_draw_glyph(uint16_t x, uint16_t y, const gui_glyph_t *g, gui_color_t color)
{
	if (!g || g->width == 0 || g->height == 0) return;

	for (uint16_t row = 0; row < g->height; row++)
	{
		uint32_t bits = g->data[row];
		uint32_t mask = g->start_mask;

		uint16_t run_start = 0;
		uint8_t in_run = 0;

		for (uint16_t col = 0; col < g->width; col++)
		{
			uint8_t is_set = (bits & mask) ? 1 : 0;

			if (is_set)
			{
				if (!in_run)
				{
					run_start = col;
					in_run = 1;
				}
			}
			else
			{
				if (in_run)
				{
					// Конец пробега → одна команда RECT
					__gui_draw_rect(x + run_start, y + row, col - run_start, 1, color, 1);
					in_run = 0;
				}
			}
			mask >>= 1;
		}

		// Закрыть пробег, если он дошёл до края
		if (in_run)
			__gui_draw_rect(x + run_start, y + row, g->width - run_start, 1, color, 1);
	}
}

void gui_draw_string_glyph(uint16_t x, uint16_t y, const char *text, const gui_mono_font_t *mono,
		const gui_prop_font_t *prop, gui_color_t color)
{
	if (!text) return;
	uint16_t cur_x = x;
	gui_glyph_t glyph;

	while (*text != '\0')
	{
		if (gui_get_glyph((uint8_t) *text, mono, prop, &glyph) == 0)
		{
			gui_draw_glyph(cur_x, y, &glyph, color);
			cur_x += glyph.width;
		}
		text++;
	}
}

void __gui_print_mono(uint16_t x, uint16_t y, const char * text, const gui_mono_font_t * font, gui_color_t color)
{
	gui_draw_string_glyph(x, y, text, font, NULL, color);
}

void __gui_print_prop(uint16_t x, uint16_t y, const char * text, const gui_prop_font_t * font, gui_color_t color)
{
	gui_draw_string_glyph(x, y, text, NULL, font, color);
}

void gui_print_mono(uint16_t x, uint16_t y, const char * text, const gui_mono_font_t * font, gui_color_t color)
{
	window_t * win = get_win(get_current_drawing_window());
	const uint16_t xn = x + win->draw_x1;
	const uint16_t yn = y + win->draw_y1;

	gui_draw_string_glyph(xn, yn, text, font, NULL, color);
}

void gui_print_prop(uint16_t x, uint16_t y, const char * text, const gui_prop_font_t * font, gui_color_t color)
{
	window_t * win = get_win(get_current_drawing_window());
	const uint16_t xn = x + win->draw_x1;
	const uint16_t yn = y + win->draw_y1;

	gui_draw_string_glyph(xn, yn, text, NULL, font, color);
}

#endif /* WITHTOUCHGUI */
