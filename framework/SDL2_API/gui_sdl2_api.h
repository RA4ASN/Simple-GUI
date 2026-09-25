// Simple GUI от RA4ASN
#ifndef _gui_sdl2_api_h
#define _gui_sdl2_api_h

#include "gui_user_include.h"
#if SIMPLE_GUI && ! GUI_USE_PORT

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>

typedef uint32_t 		gui_color_t;
typedef TTF_Font		gui_font_t;

SDL_Renderer *sdl2_get_renderer(void);

#define GUI_TFTRGB(red, green, blue) \
(  (uint32_t) ( \
((uint32_t) (255) << 24)  | /* Alpha channel value - opaque */ \
(((uint32_t) (red) << 16) & 0xFF0000)  | \
(((uint32_t) (green) << 8) & 0xFF00) | \
(((uint32_t) (blue) << 0) &  0x00FF) \
) \
)

#define GUI_DEFAULTCOLOR            	0   // fully transparent color
#define GUI_COLOR_RED 					GUI_TFTRGB(0xFF, 0x00, 0x00)
#define GUI_COLOR_GREEN 				GUI_TFTRGB(0x00, 0xFF, 0x00)
#define GUI_COLOR_BLUE	 				GUI_TFTRGB(0x00, 0x00, 0xFF)
#define GUI_COLOR_DARKGRAY              GUI_TFTRGB(0x80, 0x80, 0x80)
#define GUI_COLOR_YELLOW                GUI_TFTRGB(0xFF, 0xFF, 0x00)
#define GUI_COLOR_WHITE                 GUI_TFTRGB(0xFF, 0xFF, 0xFF)
#define GUI_COLOR_GRAY                  GUI_TFTRGB(0xA9, 0xA9, 0xA9)
#define GUI_COLOR_BLACK                 GUI_TFTRGB(0x00, 0x00, 0x00)
#define GUI_WINDOWTITLECOLOR            GUI_TFTRGB(0x87, 0xCE, 0xEB)
#define GUI_WINDOWBGCOLOR               GUI_COLOR_DARKGRAY
#define GUI_SLIDERLAYOUTCOLOR           GUI_TFTRGB(0x00, 0xFF, 0x00)
#define GUI_MENUSELECTCOLOR             GUI_TFTRGB(0x00, 0xFF, 0x00)
#define GUI_COLOR_BUTTON_NON_LOCKED     GUI_TFTRGB(0x00, 0xFF, 0x00)
#define GUI_COLOR_BUTTON_PR_NON_LOCKED  GUI_TFTRGB(0x00, 0x64, 0x00)
#define GUI_COLOR_BUTTON_LOCKED         GUI_COLOR_YELLOW
#define GUI_COLOR_BUTTON_PR_LOCKED      GUI_TFTRGB(0x3C, 0x3C, 0x00)
#define GUI_COLOR_BUTTON_DISABLED       GUI_COLOR_GRAY
#define GUI_COLOR_SWITCH_ON				GUI_TFTRGB(0x69, 0xC7, 0x79)
#define GUI_COLOR_SWITCH_ON_BORDER		GUI_TFTRGB(0x3E, 0x8E, 0x4E)
#define GUI_COLOR_SWITCH_OFF			GUI_TFTRGB(0x9E, 0x9E, 0x9E)
#define GUI_COLOR_SWITCH_OFF_BORDER		GUI_TFTRGB(0x60, 0x60, 0x60)
#define GUI_COLOR_SWITCH_KNOB			GUI_COLOR_WHITE

#define GUI_DEBUG_PRINT printf

void __gui_graw_dashed_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t dashLength, gui_color_t color);
void __gui_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, gui_color_t color, uint16_t fill);
void __gui_draw_rounded_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t radius, gui_color_t color, uint16_t fill);
void __gui_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, gui_color_t color);
void __gui_draw_point(uint16_t x, uint16_t y, gui_color_t color);
void __gui_draw_semitransparent_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, gui_color_t color, uint16_t alpha);

uint8_t __gui_get_touch_event(uint16_t *x, uint16_t *y);
uint32_t __gui_get_ticks(void);

#ifdef GUI_TIME_PROFILER
#define TIME_PROFILE_START(label) \
    uint32_t _tp_start_##label = __gui_get_ticks(); \
    const char *_tp_name_##label = #label

#define TIME_PROFILE_STOP(label, description) \
    do { \
        uint32_t _tp_end_##label = __gui_get_ticks(); \
        uint32_t _tp_elapsed_##label = _tp_end_##label - _tp_start_##label; \
        printf("[PROFILE] %-24s | %s:%d | %s | elapsed: %u ms\n", \
               _tp_name_##label, __FILE__, __LINE__, \
               (description), _tp_elapsed_##label); \
    } while(0)
#else
#define TIME_PROFILE_START(label)      ((void)0)
#define TIME_PROFILE_STOP(label, desc) ((void)0)
#endif /* GUI_TIME_PROFILER */

#if GUI_SDL2_INPUT
void gui_sdl2_input_init(void);
int  gui_sdl2_input_get(uint16_t *x, uint16_t *y);
#endif /* GUI_SDL2_INPUT */

void gui_text_init(void);
void gui_text_cleanup(void);

gui_font_t *gui_get_button_font(void);
gui_font_t *gui_get_label_font(void);
gui_font_t *gui_get_window_title_font(void);
void gui_draw_text(const char *text, int x, int y, gui_font_t *font, gui_color_t color);
void gui_invalidate_text(const char *text, gui_font_t *font);
void gui_print(const char *text, int x, int y, gui_font_t *font, gui_color_t color);
gui_font_t *gui_open_font(const char *name, int size);
int gui_get_font_height(const gui_font_t *font);
void gui_get_text_sizes(const char *text, gui_font_t *font, int *w, int *h);
int gui_get_font_ascent(gui_font_t *font);
void gui_close_font(gui_font_t *font);

#endif /* SIMPLE_GUI && ! GUI_USE_PORT */
#endif /* _gui_sdl2_api_h */
