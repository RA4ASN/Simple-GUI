#ifndef GUI_SDL2_API_H_INCLUDED
#define GUI_SDL2_API_H_INCLUDED

#include "gui_user_include.h"

#if WITHTOUCHGUI && ! GUI_USEPORT

#include <SDL2/SDL.h>
#include <math.h>
#include "../gui_render_queue.h"

typedef uint32_t 		gui_color_t;
typedef SDL_cond		gui_cond_t;
typedef SDL_mutex 		gui_mutex_t;

#define GUI_TFTRGB(red, green, blue) \
	(  (uint32_t) ( \
			((uint32_t) (255) << 24)  | /* Alpha channel value - opaque */ \
			(((uint32_t) (red) << 16) & 0xFF0000)  | \
			(((uint32_t) (green) << 8) & 0xFF00) | \
			(((uint32_t) (blue) << 0) &  0x00FF) \
		) \
	)

#define GUI_DEFAULTCOLOR            	0   // fully transparent color
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

#define GUI_ASSERT(v) do { if ((v) == 0) { \
    GUI_DEBUG_PRINT("%s(%d): Assert '%s'\n", __FILE__, __LINE__, (# v)); \
    for (;;) ; \
} } while (0)

#define GUI_VERIFY(v) do { if ((v) == 0) { \
    GUI_DEBUG_PRINT("%s(%d): Verify '%s'\n", __FILE__, __LINE__, (# v)); \
    for (;;) ; \
} } while (0)

#define GUI_MEM_ASSERT(v)   do { if (((v) == NULL)) { \
    GUI_DEBUG_PRINT("%s: %d ('%s') - memory allocate failed!\n", __FILE__, __LINE__, (# v)); \
    for (;;) ; \
} } while (0)

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

static inline gui_mutex_t * _gui_mutex_init(void)
{
	return SDL_CreateMutex();
}

static inline void _gui_mutex_destroy(gui_mutex_t * m)
{
	SDL_DestroyMutex(m);
}

static inline void _gui_mutex_lock(gui_mutex_t * m)
{
	SDL_LockMutex(m);
}

static inline void _gui_mutex_unlock(gui_mutex_t * m)
{
	SDL_UnlockMutex(m);
}

static inline gui_cond_t * _gui_cond_init(void)
{
	return SDL_CreateCond();
}

static inline void _gui_cond_wait(gui_cond_t * c, gui_mutex_t * m)
{
	SDL_CondWait(c, m);
}

static inline void _gui_cond_signal(gui_cond_t * c)
{
	SDL_CondSignal(c);
}

static inline void _gui_cond_broadcast(gui_cond_t * c)
{
	SDL_CondBroadcast(c);
}

static inline void _gui_cond_destroy(gui_cond_t * c)
{
	SDL_DestroyCond(c);
}

// Вспомогательная: проверка альфа-канала
static inline uint8_t _gui_is_semi_transparent(gui_color_t color) {
	return ((color >> 24) & 0xFF) < 255 ? 1 : 0;
}

// Отрисовка закрашенного прямоугольника
static inline void __gui_draw_rect(unsigned int x, unsigned int y,
		unsigned int w, unsigned int h, gui_color_t color,
		unsigned int fill) {
	RenderCmd cmd = { 0 };
	cmd.type = RQ_CMD_DRAW_RECT;
	cmd.color = color;
	cmd.fill = fill;
	cmd.blend_enabled = 0; //_gui_is_semi_transparent(color);
	cmd.data.rect.x = x;
	cmd.data.rect.y = y;
	cmd.data.rect.w = w;
	cmd.data.rect.h = h;
	render_queue_push(&cmd);
}

// Отрисовка прямоугольника со скругленными углами
static inline void __gui_draw_rounded_rect(
		unsigned int x, unsigned int y, unsigned int w, unsigned int h,
		unsigned int radius, gui_color_t color, unsigned int fill) {
	RenderCmd cmd = { 0 };
	cmd.type = RQ_CMD_DRAW_ROUNDED_RECT;
	cmd.color = color;
	cmd.fill = fill;
	cmd.blend_enabled = 0; //_gui_is_semi_transparent(color);
	cmd.data.rounded_rect.x = x;
	cmd.data.rounded_rect.y = y;
	cmd.data.rounded_rect.w = w;
	cmd.data.rounded_rect.h = h;
	cmd.data.rounded_rect.radius = radius;
	render_queue_push(&cmd);
}

static void __sdl2_draw_rounded_rect(SDL_Renderer* r, uint16_t x, uint16_t y, uint16_t w, uint16_t h,
		uint16_t radius, uint8_t fill)
{
	if (w == 0 || h == 0)
		return;
	uint16_t rad = radius;
	if (rad > w / 2)
		rad = w / 2;
	if (rad > h / 2)
		rad = h / 2;

	if (rad == 0)
	{
		SDL_Rect rect = { x, y, w, h };
		fill ? SDL_RenderFillRect(r, &rect) : SDL_RenderDrawRect(r, &rect);
		return;
	}

	int x1 = x + w - 1;
	int y1 = y + h - 1;
	int cx_tl = x + rad, cy_tl = y + rad;
	int cx_tr = x1 - rad, cy_tr = y + rad;
	int cx_bl = x + rad, cy_bl = y1 - rad;
	int cx_br = x1 - rad, cy_br = y1 - rad;

	if (fill)
	{
		if (w > 2 * rad)
		{
			SDL_Rect mid =
			{ x + rad, y, w - 2 * rad, h };
			SDL_RenderFillRect(r, &mid);
		}
		if (h > 2 * rad)
		{
			SDL_Rect left =
			{ x, y + rad, rad, h - 2 * rad };
			SDL_Rect right =
			{ x1 - rad + 1, y + rad, rad, h - 2 * rad };
			SDL_RenderFillRect(r, &left);
			SDL_RenderFillRect(r, &right);
		}

#define FILL_QUARTER(cx, cy, sx, sy) \
        do { \
            for (int dy = 0; dy <= rad; dy++) { \
                int dx = (int)sqrt((double)(rad * rad - dy * dy)); \
                for (int dx2 = 0; dx2 <= dx; dx2++) { \
                    SDL_RenderDrawPoint(r, cx + sx * dx2, cy + sy * dy); \
                } \
            } \
        } while(0)

		FILL_QUARTER(cx_tl, cy_tl, -1, -1);
		FILL_QUARTER(cx_tr, cy_tr, 1, -1);
		FILL_QUARTER(cx_bl, cy_bl, -1, 1);
		FILL_QUARTER(cx_br, cy_br, 1, 1);
#undef FILL_QUARTER
	}
	else
	{
		SDL_RenderDrawLine(r, x + rad, y, x1 - rad, y);
		SDL_RenderDrawLine(r, x + rad, y1, x1 - rad, y1);
		SDL_RenderDrawLine(r, x, y + rad, x, y1 - rad);
		SDL_RenderDrawLine(r, x1, y + rad, x1, y1 - rad);

		// Ограничиваем сегменты для производительности
		const int seg = (rad > 32) ? 32 : rad;
		SDL_Point pts[4 * (seg + 1)];
		int idx = 0;
		double step = (M_PI / 2.0) / seg;

		for (int i = 0; i <= seg; i++)
		{
			double t = i * step;
			pts[idx++] = (SDL_Point) { cx_tl - (int) (rad * cos(t)), cy_tl - (int) (rad * sin(t)) };
			pts[idx++] = (SDL_Point) { cx_tr + (int) (rad * cos(t)), cy_tr - (int) (rad * sin(t)) };
			pts[idx++] = (SDL_Point) { cx_br + (int) (rad * cos(t)), cy_br + (int) (rad * sin(t)) };
			pts[idx++] = (SDL_Point) { cx_bl - (int) (rad * cos(t)), cy_bl + (int) (rad * sin(t)) };
		}
		SDL_RenderDrawPoints(r, pts, idx);
	}
}

// Отрисовка линии
static inline void __gui_draw_line(unsigned int x1,
		unsigned int y1, unsigned int x2, unsigned int y2, gui_color_t color) {
	RenderCmd cmd = { 0 };
	cmd.type = RQ_CMD_DRAW_LINE;
	cmd.color = color;
	cmd.blend_enabled = _gui_is_semi_transparent(color);
	cmd.data.line.x1 = x1;
	cmd.data.line.y1 = y1;
	cmd.data.line.x2 = x2;
	cmd.data.line.y2 = y2;
	render_queue_push(&cmd);
}

// Отрисовка точки
static inline void __gui_draw_point(unsigned int x,
		unsigned int y, gui_color_t color) {
	RenderCmd cmd = { 0 };
	cmd.type = RQ_CMD_DRAW_POINT;
	cmd.color = color;
	cmd.blend_enabled = _gui_is_semi_transparent(color);
	cmd.data.point.x = x;
	cmd.data.point.y = y;
	render_queue_push(&cmd);
}

// Полупрозрачный прямоугольник
static inline void __gui_draw_semitransparent_rect(
		unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2,
		unsigned int alpha) {
	RenderCmd cmd = { 0 };
	cmd.type = RQ_CMD_DRAW_SEMITRANSPARENT_RECT;
	cmd.fill = 1;
	cmd.blend_enabled = 1;
	cmd.data.semitransparent_rect.x1 = x1;
	cmd.data.semitransparent_rect.y1 = y1;
	cmd.data.semitransparent_rect.x2 = x2;
	cmd.data.semitransparent_rect.y2 = y2;
	cmd.data.semitransparent_rect.alpha = (uint8_t) alpha;

	// Формируем цвет из базового серого и переданной альфы
	uint8_t base_r = (COLORPIP_DARKGRAY >> 16) & 0xFF;
	uint8_t base_g = (COLORPIP_DARKGRAY >> 8) & 0xFF;
	uint8_t base_b = COLORPIP_DARKGRAY & 0xFF;
	cmd.color = ((uint32_t) alpha << 24) | ((uint32_t) base_r << 16)
			| ((uint32_t) base_g << 8) | base_b;

	render_queue_push(&cmd);
}

static inline void __sdl2_set_draw_state(SDL_Renderer* r, uint32_t color, uint8_t blend_enabled)
{
	uint8_t a = (color >> 24) & 0xFF;
	uint8_t r_ch = (color >> 16) & 0xFF;
	uint8_t g = (color >> 8) & 0xFF;
	uint8_t b = color & 0xFF;

	if (blend_enabled || a < 255)
		SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
	else
		SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

	SDL_SetRenderDrawColor(r, r_ch, g, b, a);
}

static inline uint8_t __gui_get_touch_event(uint16_t * x, uint16_t * y) {
	uint_fast16_t xx, yy, p;
	p = board_tsc_getxy(&xx, &yy);
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

#endif /* WITHTOUCHGUI  && ! GUI_USEPORT */
#endif /* GUI_SDL2_API_H_INCLUDED */
