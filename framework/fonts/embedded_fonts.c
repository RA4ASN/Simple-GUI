// Simple GUI от RA4ASN
//--------------------------------------------------------------
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
//--------------------------------------------------------------

#include "gui_user_include.h"

#if WITHTOUCHGUI && ! GUI_EXTERNAL_FONTS

#include "../gui_system.h"
#include "../gui_structs.h"
#include "../gui_settings.h"
#include "../gui_windows.h"
#include "embedded_fonts.h"
#include "../sdl2-render/gui_sdl2_api.h"

// *********** Пакетный рендеринг текста ***********

RenderCmd * add_task(RenderCmd ** ptr, uint16_t * idx);

// *********** пропорциональные шрифты *************

static uint16_t UB_Font_DrawPChar32_batch(uint16_t x, uint16_t y, uint8_t ascii, const gui_prop_font_t * font,
		gui_color_t vg, RenderCmd ** batch, uint16_t * batch_idx)
{
	uint16_t xn, yn, width;
	uint_fast32_t start_maske, maske;
	const uint32_t * wert;

	// Проверка границы символа
	if (ascii < font->first_char)
		return 0;

	if (ascii > font->last_char)
		return 0;

	ascii -= font->first_char;
	wert = &font->table[ascii * (font->height + 1)];
	width = wert[0];
	start_maske = 0x01;
	start_maske = start_maske << (width - 1);

	for (yn = 0; yn < font->height; yn ++)
	{
		maske = start_maske;
		// Установка курсора

		for (xn = 0; xn < width; xn++)
		{
			if ((wert[yn + 1] & maske))
				*add_task(batch, batch_idx) = (RenderCmd){ .type = RQ_CMD_DRAW_POINT, .color = vg,
						.blend_enabled = 0, .data.point = { x + xn, yn + y } };

			maske = (maske >> 1);
		}
	}

	return (width);
}

static void gui_UB_Font_DrawPString32_batch(uint16_t x, uint16_t y, const char * ptr,
		const gui_prop_font_t * font, uint32_t vg, RenderCmd ** batch, uint16_t * batch_idx)
{
	uint16_t pos = x, width;

	while (* ptr != 0)
	{
		width = UB_Font_DrawPChar32_batch(pos, y, * ptr, font, vg, batch, batch_idx);
		pos += width;
		ptr ++;
	}
}

void __gui_print_batch_prop(uint16_t x, uint16_t y, const char * text, const gui_prop_font_t * font,
		gui_color_t color, RenderCmd ** batch, uint16_t * batch_idx)
{
	gui_UB_Font_DrawPString32_batch(x, y, text, font, color, batch, batch_idx);
}

// *********** моноширинные шрифты *************

static void UB_Font_DrawChar32_batch(uint16_t x, uint16_t y, uint8_t ascii, const gui_mono_font_t * font,
		uint32_t vg, RenderCmd ** batch, uint16_t * batch_idx)
{
	uint16_t xn, yn;
	uint32_t start_maske, maske;
	const uint32_t * wert;

	ascii -= 32;
	wert = & font->table[ascii * font->height];

	start_maske = 0x80;
	if (font->width > 8)  start_maske = 0x8000;
	if (font->width > 16) start_maske = 0x80000000;

	for (yn = 0; yn < font->height; yn ++)
	{
		maske = start_maske;
		// Установка курсора

		for (xn = 0; xn < font->width; xn ++)
		{
			if ((wert[yn] & maske))
				*add_task(batch, batch_idx) = (RenderCmd){ .type = RQ_CMD_DRAW_POINT, .color = vg,
						.blend_enabled = 0, .data.point = { x + xn, y + yn } };

			maske = (maske >> 1);
		}
	}
}

static void gui_UB_Font_DrawString32_batch(uint16_t x, uint16_t y, const char * ptr,
		const gui_mono_font_t * font, uint32_t vg, RenderCmd ** batch, uint16_t * batch_idx)
{
	uint16_t pos;

	pos = x;
	while (* ptr != '\0') {
		UB_Font_DrawChar32_batch(pos, y, * ptr, font, vg, batch, batch_idx);
		pos += font->width;
		ptr ++;
	}
}

void __gui_print_batch_mono(uint16_t x, uint16_t y, const char * text, const gui_mono_font_t * font,
		gui_color_t color, RenderCmd ** batch, uint16_t * batch_idx)
{
	gui_UB_Font_DrawString32_batch(x, y, text, font, color, batch, batch_idx);
}

void gui_print_mono(uint16_t x, uint16_t y, const char * text, const gui_mono_font_t * font, gui_color_t color)
{
	window_t * win = get_win(get_current_drawing_window());
	const uint16_t xn = x + win->draw_x1;
	const uint16_t yn = y + win->draw_y1;
	RenderCmd * draw_batch = NULL;
	uint16_t batch_idx = 0;

	gui_UB_Font_DrawString32_batch(xn, yn, text, font, color, & draw_batch, & batch_idx);
	render_queue_push_batch(draw_batch, batch_idx);
	free(draw_batch);
}

void gui_print_prop(uint16_t x, uint16_t y, const char * text, const gui_prop_font_t * font, gui_color_t color)
{
	window_t * win = get_win(get_current_drawing_window());
	RenderCmd * draw_batch = NULL;
	uint16_t batch_idx = 0;

	const uint16_t xn = x + win->draw_x1;
	const uint16_t yn = y + win->draw_y1;

	gui_UB_Font_DrawPString32_batch(xn, yn, text, font, color, & draw_batch, & batch_idx);
	render_queue_push_batch(draw_batch, batch_idx);
	free(draw_batch);
}

//--------------------------------------------------------------
// Рисует ASCII символ шрифтом одного размера на позиции х, у.
// Цвет шрифта и фон (шрифт = макс 32 пикселя в ширину)
// Шрифт должен быть передан с оператором &
//--------------------------------------------------------------
static void UB_Font_DrawChar32(uint16_t x, uint16_t y, uint8_t ascii, const gui_mono_font_t * font, uint32_t vg)
{
	uint16_t xn, yn;
	uint32_t start_maske, maske;
	const uint32_t * wert;

	ascii -= 32;
	wert = & font->table[ascii * font->height];

	start_maske = 0x80;
	if (font->width > 8)  start_maske = 0x8000;
	if (font->width > 16) start_maske = 0x80000000;

	for (yn = 0; yn < font->height; yn ++)
	{
		maske = start_maske;
		// Установка курсора

		for (xn = 0; xn < font->width; xn ++)
		{
			if ((wert[yn] & maske)) __gui_draw_point(x + xn, yn + y, vg);
			maske = (maske >> 1);
		}
	}
}

//--------------------------------------------------------------
// Рисует строку шрифтом одного размера на позиции х, у.
// Цвет шрифта и фон (шрифт = макс 32 пикселя в ширину)
// Шрифт должен быть передан с оператором &
//--------------------------------------------------------------
static void gui_UB_Font_DrawString32(uint16_t x, uint16_t y, const char * ptr, const gui_mono_font_t * font, uint32_t vg)
{
	uint16_t pos;

	pos = x;
	while (* ptr != '\0') {
		UB_Font_DrawChar32(pos, y, * ptr, font, vg);
		pos += font->width;
		ptr ++;
	}
}

//--------------------------------------------------------------
// Рисование ASCII символ пропорционального шрифта с позицией X, Y
// Цвет шрифта плана и фона (шрифт = макс 32 пикселя в ширину)
// Шрифт должен быть передан с оператором &
// Возвращает: ширину нарисованного символа
//--------------------------------------------------------------
static uint16_t UB_Font_DrawPChar32(uint16_t x, uint16_t y, uint8_t ascii, const gui_prop_font_t * font, gui_color_t vg)
{
	uint16_t xn, yn, width;
	uint_fast32_t start_maske, maske;
	const uint32_t * wert;

	// Проверка границы символа
	if (ascii < font->first_char)
		return 0;

	if (ascii > font->last_char)
		return 0;

	ascii -= font->first_char;
	wert = &font->table[ascii * (font->height + 1)];
	width = wert[0];
	start_maske = 0x01;
	start_maske = start_maske << (width - 1);

	for (yn = 0; yn < font->height; yn ++)
	{
		maske = start_maske;
		// Установка курсора

		for (xn = 0; xn < width; xn++)
		{
			if ((wert[yn + 1] & maske)) __gui_draw_point(x + xn, yn + y, vg);

			maske = (maske >> 1);
		}
	}

	return (width);
}

//--------------------------------------------------------------
// Рисование строку пропорционального шрифта с позицией X, Y
// Цвет шрифта плана и фона (шрифт = макс 32 пикселя в ширину)
// Шрифт должен быть передан с оператором &
//--------------------------------------------------------------
static void gui_UB_Font_DrawPString32(uint16_t x, uint16_t y, const char * ptr, const gui_prop_font_t * font, uint32_t vg)
{
	uint16_t pos = x, width;

	while (* ptr != 0)
	{
		width = UB_Font_DrawPChar32(pos, y, * ptr, font, vg);
		pos += width;
		ptr ++;
	}
}

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

void __gui_print_mono(uint16_t x, uint16_t y, const char * text, const gui_mono_font_t * font, gui_color_t color)
{
	gui_UB_Font_DrawString32(x, y,	text, font, color);
}

void __gui_print_prop(uint16_t x, uint16_t y, const char * text, const gui_prop_font_t * font, gui_color_t color)
{
	gui_UB_Font_DrawPString32(x, y, text, font, color);
}

#endif /* WITHTOUCHGUI && ! GUI_EXTERNAL_FONTS */
