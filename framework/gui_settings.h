// Simple GUI от RA4ASN

#ifndef GUI_SETTINGS_H_INCLUDED
#define GUI_SETTINGS_H_INCLUDED

#include "gui_user_include.h"

#if WITHTOUCHGUI

typedef struct {
	uint16_t max_w;
	uint16_t max_h;
	uint8_t sliders_scale_thickness;	// ширина шкалы слайдера
	uint8_t sliders_w;					// размеры ползунка слайдера
	uint8_t sliders_h;					// от центра (*2)
	uint8_t window_title_height;		// высота области заголовка окна
	uint8_t edge_step;					// отступ от границ окна
	uint8_t window_close_button_size;
	uint8_t window_title_indent;		// горизонтальный отступ заголовка
	uint8_t touch_area_enlarge;			// увеличение области вокруг элементов для упрощения попадания по мелким элементам
	uint8_t common_btn_width;
	uint8_t common_btn_height;
	uint8_t footer_height;
	uint8_t buttons_font_size;
	uint8_t labels_font_size;
	uint8_t win_title_font_size;
} gui_sizes_t ;

extern gui_sizes_t gui_sizes;

enum {
	button_round_radius = 3,	// радиус закругления кнопки
	edge_step = 15,				// отступ от границ окна
	autorepeat_delay = 4,		// задержка автоповтора действий
	footer_buttons_count = 9,
	common_btn_interval = 3,
	NAME_ARRAY_SIZE = 40,
	MENU_ARRAY_SIZE = 50,
	TEXT_ARRAY_SIZE = 70,
	GUI_OBJECTS_ARRAY_SIZE = 60
};

#define COMMON_BUTTON_STYLE		get_commonbtn_w(), get_commonbtn_h()
#define SMALL_BUTTON_STYLE		get_commonbtn_h(), get_commonbtn_h()
#define LONG_BUTTON_STYLE		(get_commonbtn_w() + get_commonbtn_w() / 3), get_commonbtn_h()

#if EMBEDDED_FONTS
#define BUTTONS_FONTP_DEFAULT	msgothic_15x17_prop
#define LABELS_FONT_DEFAULT		msgothic_15x17_mono
#define INFOBAR_FONTP			msgothic_13x16_prop
#define WINDOW_TITLE_FONTP		msgothic_15x17_prop
#endif

#endif /* WITHTOUCHGUI */

#endif /* GUI_STRUCTS_H_INCLUDED */
