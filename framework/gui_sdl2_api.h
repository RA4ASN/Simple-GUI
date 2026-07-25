// Simple GUI от RA4ASN

#ifndef GUI_SDL2_API_H_INCLUDED
#define GUI_SDL2_API_H_INCLUDED

#include "gui_user_include.h"

#if WITHTOUCHGUI && WITHSDL2VIDEO

#include <SDL2/SDL.h>
#include <math.h>

typedef uint32_t 		gui_color_t;
typedef SDL_cond		gui_cond_t;
typedef SDL_mutex 		gui_mutex_t;

SDL_Renderer * sdl2_get_renderer(void);

#define GUI_TFTRGB(red, green, blue) \
	(  (uint32_t) ( \
			((uint32_t) (255) << 24)  | /* Alpha channel value - opaque */ \
			(((uint32_t) (red) << 16) & 0xFF0000)  | \
			(((uint32_t) (green) << 8) & 0xFF00) | \
			(((uint32_t) (blue) << 0) &  0x00FF) \
		) \
	)

#define GUI_DEFAULTCOLOR            	0   // fully transparent color
#define GUI_COLOR_GREEN 				GUI_TFTRGB(0x00, 0xFF, 0x00)
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

#define GUI_DEBUG_PRINT printf

static inline int _sdl2_print_error_impl(const char* file, int line) {
    const char* err = SDL_GetError();
    if (err[0]) {
        printf("[SDL Error] %s:%d: %s\n", file, line, err);
        SDL_ClearError();
        return 1;
    }
    return 0;
}

#define SDL2_PRINT_ERROR() _sdl2_print_error_impl(__FILE__, __LINE__)
#define SDL2_CHECK(expr) \
    ({ \
        __typeof__(expr) _r = (expr); \
        _sdl2_print_error_impl(__FILE__, __LINE__); \
        _r; \
    })

// Отрисовка закрашенного прямоугольника
static inline void __gui_draw_rect(unsigned int x, unsigned int y,
		unsigned int w, unsigned int h, gui_color_t color, unsigned int fill)
{
	SDL_Renderer * renderer = sdl2_get_renderer();

	uint8_t r = (color >> 16) & 0xFF;
	uint8_t g = (color >> 8) & 0xFF;
	uint8_t b = (color >> 0) & 0xFF;
	uint8_t a = (color >> 24) & 0xFF;

	SDL_Rect rect = { .x = x, .y = y, .w = w, .h = h };

	if (a < 255)
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	else
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	if (fill)
		SDL_RenderFillRect(renderer, & rect);
	else
		SDL_RenderDrawRect(renderer, & rect);
}

// Отрисовка прямоугольника со скругленными углами
static inline void __gui_draw_rounded_rect(unsigned int x, unsigned int y,
		unsigned int w, unsigned int h, unsigned int radius, gui_color_t color, unsigned int fill)
{
	if (w == 0 || h == 0) return;

	SDL_Renderer * renderer = sdl2_get_renderer();
	uint8_t r = radius;
	if (r > w / 2) r = w / 2;
	if (r > h / 2) r = h / 2;
	if (r == 0) {
		__gui_draw_rect(x, y, w, h, color, fill);
		return;
	}

	uint8_t cr = (color >> 16) & 0xFF;
	uint8_t cg = (color >> 8) & 0xFF;
	uint8_t cb = (color >> 0) & 0xFF;
	uint8_t ca = (color >> 24) & 0xFF;
	SDL_SetRenderDrawColor(renderer, cr, cg, cb, ca);

	int x0 = x;
	int y0 = y;
	int x1 = x0 + w - 1;
	int y1 = y0 + h - 1;

	if (fill) {
		/* 1. Центральные прямоугольники (без углов) */
		if (w > 2 * r) {
			SDL_RenderFillRect(renderer, &(SDL_Rect) {x0 + r, y0, w - 2 * r, h});
		}
		if (h > 2 * r) {
			SDL_RenderFillRect(renderer, &(SDL_Rect) {x0, y0 + r, r, h - 2 * r});
			SDL_RenderFillRect(renderer, &(SDL_Rect) {x1 - r + 1, y0 + r, r, h - 2 * r});
		}

		/* Вспомогательная функция для заливки четверти круга */
#define FILL_QUARTER_CIRCLE(cx, cy, sign_x, sign_y) 				\
            do { 													\
                for (int dy = 0; dy <= r; dy++) { 					\
                    int dx = (int)sqrt((double)(r * r - dy * dy)); 	\
                    for (int dx2 = 0; dx2 <= dx; dx2++) { 			\
                        SDL_RenderDrawPoint(renderer, 				\
                            (cx) + (sign_x) * dx2, 					\
                            (cy) + (sign_y) * dy); 					\
                    }												\
                } 													\
            } while(0)

		/* Top-left corner */
		FILL_QUARTER_CIRCLE(x0 + r, y0 + r, -1, -1);
		/* Top-right corner */
		FILL_QUARTER_CIRCLE(x1 - r, y0 + r, 1, -1);
		/* Bottom-left corner */
		FILL_QUARTER_CIRCLE(x0 + r, y1 - r, -1, 1);
		/* Bottom-right corner */
		FILL_QUARTER_CIRCLE(x1 - r, y1 - r, 1, 1);

#undef FILL_QUARTER_CIRCLE
	}
	else
	{
		SDL_RenderDrawLine(renderer, x0 + r, y0, x1 - r, y0); // top
		SDL_RenderDrawLine(renderer, x0 + r, y1, x1 - r, y1); // bottom
		SDL_RenderDrawLine(renderer, x0, y0 + r, x0, y1 - r); // left
		SDL_RenderDrawLine(renderer, x1, y0 + r, x1, y1 - r); // right

		// === ДУГИ ===
		const int segments = r;
		SDL_Point pts[4 * (segments + 1)];
		int idx = 0;

		// Top-left arc: from (x0+r, y0) to (x0, y0+r)
		for (int i = 0; i <= segments; i ++)
		{
			double t = (M_PI / 2.0) * i / segments; // t: 0 → π/2
			int px = x0 + r - (int)(r * cos(t));
			int py = y0 + r - (int)(r * sin(t));
			pts[idx++] = (SDL_Point){px, py};
		}

		// Top-right arc: from (x1-r, y0) to (x1, y0+r)
		for (int i = 0; i <= segments; i ++)
		{
			double t = (M_PI / 2.0) * i / segments;
			int px = x1 - r + (int)(r * cos(t));
			int py = y0 + r - (int)(r * sin(t));
			pts[idx++] = (SDL_Point){px, py};
		}

		// Bottom-right arc: from (x1, y1-r) to (x1-r, y1)
		for (int i = 0; i <= segments; i ++)
		{
			double t = (M_PI / 2.0) * i / segments;
			int px = x1 - r + (int)(r * cos(t));
			int py = y1 - r + (int)(r * sin(t));
			pts[idx++] = (SDL_Point){px, py};
		}

		// Bottom-left arc: from (x0, y1-r) to (x0+r, y1)
		for (int i = 0; i <= segments; i ++)
		{
			double t = (M_PI / 2.0) * i / segments;
			int px = x0 + r - (int)(r * cos(t));
			int py = y1 - r + (int)(r * sin(t));
			pts[idx++] = (SDL_Point){px, py};
		}

		SDL_RenderDrawPoints(renderer, pts, idx);
	}
}

static inline void __gui_draw_line(unsigned int x1, unsigned int y1,
		unsigned int x2, unsigned int y2, gui_color_t color)
{
	SDL_Renderer * renderer = sdl2_get_renderer();

	uint8_t r = (color >> 16) & 0xFF;
	uint8_t g = (color >> 8) & 0xFF;
	uint8_t b = (color >> 0) & 0xFF;
	uint8_t a = (color >> 24) & 0xFF;

	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

static inline void __gui_draw_point(unsigned int x, unsigned int y, gui_color_t color)
{
	SDL_Renderer * renderer = sdl2_get_renderer();

	uint8_t r = (color >> 16) & 0xFF;
	uint8_t g = (color >> 8) & 0xFF;
	uint8_t b = (color >> 0) & 0xFF;
	uint8_t a = (color >> 24) & 0xFF;

	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_RenderDrawPoint(renderer, x, y);
}

static inline void __gui_draw_semitransparent_rect(unsigned int x1, unsigned int y1,
		unsigned int x2, unsigned int y2, unsigned int alpha)
{
	SDL_Renderer * renderer = sdl2_get_renderer();

	uint8_t r = (GUI_COLOR_DARKGRAY >> 16) & 0xFF;
	uint8_t g = (GUI_COLOR_DARKGRAY >> 8) & 0xFF;
	uint8_t b = (GUI_COLOR_DARKGRAY >> 0) & 0xFF;
	uint8_t a = (uint8_t)alpha;

	SDL_Rect rect = { .x = x1, .y = y1, .w = (x2 - x1), .h = (y2 - y1)};

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_RenderFillRect(renderer, & rect);
}

static inline uint8_t __gui_get_touch_event(uint16_t * x, uint16_t * y) {
	uint16_t xx, yy, p;
	p = evdev_get_event(&xx, &yy);
	*x = xx;
	*y = yy;
	return p;
}

#ifdef GUI_TIME_PROFILER
#define TIME_PROFILE_START(label) \
uint32_t _tp_start_##label = SDL_GetTicks(); \
const char *_tp_name_##label = #label
#define TIME_PROFILE_STOP(label, description) \
do { \
    uint32_t _tp_end_##label = SDL_GetTicks(); \
    uint32_t _tp_elapsed_##label = _tp_end_##label - _tp_start_##label; \
    printf("[PROFILE] %-24s | %s:%d | %s | elapsed: %u ms\n", \
           _tp_name_##label, __FILE__, __LINE__, \
           (description), _tp_elapsed_##label); \
} while(0)
#else
/* Заглушки при отключённом профилировании */
#define TIME_PROFILE_START(label)      ((void)0)
#define TIME_PROFILE_STOP(label, desc) ((void)0)
#endif /* GUI_TIME_PROFILER */

#endif /* WITHTOUCHGUI && WITHSDL2VIDEO */
#endif /* GUI_SDL2_API_H_INCLUDED */
