// Simple GUI от RA4ASN
#include "gui_user_include.h"
#if SIMPLE_GUI && ! GUI_USE_PORT
#include "../gui_includes.h"
#include "gui_sdl2_api.h"

/* Кэш последнего установленного цвета, режима блендинга и рендерера.
   Позволяет избежать повторных вызовов SDL_SetRenderDrawColor/BlendMode
   при последовательной отрисовке примитивов одного цвета. */
static SDL_Renderer *gui_last_renderer = NULL;
static uint32_t gui_last_color = 0xFFFFFFFF;
static int gui_last_blend_mode = -1;

static void sdl2_set_draw_color(SDL_Renderer *r, uint32_t color)
{
    if (r != gui_last_renderer || color != gui_last_color) {
        SDL_SetRenderDrawColor(r,
            (color >> 16) & 0xFF,
            (color >> 8) & 0xFF,
            (color >> 0) & 0xFF,
            (color >> 24) & 0xFF);
        gui_last_renderer = r;
        gui_last_color = color;
    }
}

static void sdl2_set_blend_mode(SDL_Renderer *r, int mode)
{
    if (gui_last_blend_mode != mode) {
        SDL_SetRenderDrawBlendMode(r, (SDL_BlendMode)mode);
        gui_last_blend_mode = mode;
    }
}

void __gui_graw_dashed_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t dashLength, gui_color_t color)
{
	uint16_t pos = 0;
	uint16_t x1 = x;
	uint16_t y1 = y;
	uint16_t x2 = x + width - 1;
	uint16_t y2 = y + height - 1;

	/* Максимальное количество точек в пунктирной рамке: полный периметр 2*(width+height).
	   В худшем случае (при длине штриха 1) каждая точка периметра может быть отрисована. */
	SDL_Point points[(width + height) * 2];

	int n = 0;
	// Верхняя линия (слева направо)
	for (uint16_t i = 0; i < width; i++)
		if ((pos++ / dashLength) % 2 == 0)
			points[n++] = (SDL_Point ) { x1 + i, y1 };

	// Правая линия (сверху вниз)
	for (uint16_t i = 1; i < height; i++)
		if ((pos++ / dashLength) % 2 == 0)
			points[n++] = (SDL_Point ) { x2, y1 + i };

	// Нижняя линия (справа налево)
	for (uint16_t i = 1; i < width; i++)
		if ((pos++ / dashLength) % 2 == 0)
			points[n++] = (SDL_Point ) { x2 - i, y2 };

	// Левая линия (снизу вверх)
	for (uint16_t i = 1; i < height - 1; i++)
		if ((pos++ / dashLength) % 2 == 0)
			points[n++] = (SDL_Point ) { x1, y2 - i };

	if (n > 0)
	{
	    SDL_Renderer *renderer = sdl2_get_renderer();
	    sdl2_set_blend_mode(renderer, ((color >> 24) & 0xFF) < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
	    sdl2_set_draw_color(renderer, color);
	    SDL_RenderDrawPoints(renderer, points, n);
	}
}

// Отрисовка прямоугольника: при fill != 0 - закрашенный (заполненный),
// при fill == 0 - только контур (рамка 1 пиксель)
void __gui_draw_rect(uint16_t x, uint16_t y,
    uint16_t w, uint16_t h, gui_color_t color, uint16_t fill)
{
    SDL_Renderer *renderer = sdl2_get_renderer();
    SDL_Rect rect = { .x = x, .y = y, .w = w, .h = h };
    sdl2_set_blend_mode(renderer, ((color >> 24) & 0xFF) < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
    sdl2_set_draw_color(renderer, color);
    if (fill)
        SDL_RenderFillRect(renderer, &rect);
    else
        SDL_RenderDrawRect(renderer, &rect);
}

#define GUI_RRECT_BATCH		64			// макс. элементов контура/заливки в одном пакете

// Отрисовка прямоугольника со скругленными углами
void __gui_draw_rounded_rect(uint16_t x, uint16_t y,
    uint16_t w, uint16_t h, uint16_t radius, gui_color_t color, uint16_t fill)
{
    if (w == 0 || h == 0) return;
    SDL_Renderer *renderer = sdl2_get_renderer();
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

    if (ca < 255)
        sdl2_set_blend_mode(renderer, SDL_BLENDMODE_BLEND);
    else
        sdl2_set_blend_mode(renderer, SDL_BLENDMODE_NONE);

    sdl2_set_draw_color(renderer, color);

    int x0 = x;
    int y0 = y;
    int x1 = x0 + w - 1;
    int y1 = y0 + h - 1;

    if (fill) {
        /* 1. Центральные прямоугольники (без углов) - одним пакетом */
        SDL_Rect mid[3];
        int m = 0;
        if (w > 2 * r) mid[m++] = (SDL_Rect) {x0 + r, y0, w - 2 * r, h};
        if (h > 2 * r) {
            mid[m++] = (SDL_Rect) {x0, y0 + r, r, h - 2 * r};
            mid[m++] = (SDL_Rect) {x1 - r + 1, y0 + r, r, h - 2 * r};
        }
        if (m) SDL_RenderFillRects(renderer, mid, m);

        /* 2. Углы: построчные полосы четвертей круга, пакеты точных SDL_Rect */
        SDL_Rect batch[4 * GUI_RRECT_BATCH];
        for (int i0 = 0; i0 < r; i0 += GUI_RRECT_BATCH) {
            int i1 = i0 + GUI_RRECT_BATCH;
            if (i1 > r) i1 = r;
            int n = 0;
            for (int i = i0; i < i1; i ++) {
                int dyv = r - i;
                int hw = (int) lround(sqrt((double) (r * r - dyv * dyv)));
                if (hw <= 0) continue;
                batch[n++] = (SDL_Rect) {x0 + r - hw, y0 + i, hw, 1};
                batch[n++] = (SDL_Rect) {x1 - r + 1, y0 + i, hw, 1};
                batch[n++] = (SDL_Rect) {x0 + r - hw, y1 - i, hw, 1};
                batch[n++] = (SDL_Rect) {x1 - r + 1, y1 - i, hw, 1};
            }
            if (n) SDL_RenderFillRects(renderer, batch, n);
        }
    }
    else
    {
        /* Прямые участки контура */
        SDL_RenderDrawLine(renderer, x0 + r, y0, x1 - r, y0); // top
        SDL_RenderDrawLine(renderer, x0 + r, y1, x1 - r, y1); // bottom
        SDL_RenderDrawLine(renderer, x0, y0 + r, x0, y1 - r); // left
        SDL_RenderDrawLine(renderer, x1, y0 + r, x1, y1 - r); // right

        const int segments = 2 * r;
        SDL_Point arc[GUI_RRECT_BATCH + 1];

#define DRAW_ARC(cx, cy, sx, sy) 									\
    do { 														\
        int s0 = 0; 											\
        while (s0 < segments) { 								\
            int s1 = s0 + GUI_RRECT_BATCH; 						\
            if (s1 > segments) s1 = segments; 					\
            int n = 0; 											\
            for (int s = s0; s <= s1; s ++) { 					\
                double t = (M_PI / 2.0) * s / segments; 		\
                arc[n].x = (cx) + (sx) * (int) lround(r * cos(t)); 	\
                arc[n].y = (cy) + (sy) * (int) lround(r * sin(t)); 	\
                n ++; 											\
            } 													\
            if (n >= 2) SDL_RenderDrawLines(renderer, arc, n); 	\
            s0 = s1; 											\
        } 														\
    } while(0)

        DRAW_ARC(x0 + r, y0 + r, -1, -1);
        DRAW_ARC(x1 - r, y0 + r, 1, -1);
        DRAW_ARC(x1 - r, y1 - r, 1, 1);
        DRAW_ARC(x0 + r, y1 - r, -1, 1);

#undef DRAW_ARC
    }
}

void __gui_draw_line(uint16_t x1, uint16_t y1,
    uint16_t x2, uint16_t y2, gui_color_t color)
{
    SDL_Renderer *renderer = sdl2_get_renderer();
    sdl2_set_blend_mode(renderer, ((color >> 24) & 0xFF) < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
    sdl2_set_draw_color(renderer, color);
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

void __gui_draw_point(uint16_t x, uint16_t y, gui_color_t color)
{
    SDL_Renderer *renderer = sdl2_get_renderer();
    sdl2_set_blend_mode(renderer, ((color >> 24) & 0xFF) < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
    sdl2_set_draw_color(renderer, color);
    SDL_RenderDrawPoint(renderer, x, y);
}

void __gui_draw_semitransparent_rect(uint16_t x, uint16_t y,
    uint16_t w, uint16_t h, gui_color_t color, uint16_t alpha)
{
    SDL_Renderer *renderer = sdl2_get_renderer();
    SDL_Rect rect = { .x = x, .y = y, .w = w, .h = h };
    sdl2_set_blend_mode(renderer, SDL_BLENDMODE_BLEND);
    sdl2_set_draw_color(renderer, (color & 0x00FFFFFF) | ((uint32_t) alpha << 24));
    SDL_RenderFillRect(renderer, & rect);
}

uint8_t __gui_get_touch_event(uint16_t * x, uint16_t * y) {
    uint16_t xx, yy, p;
    p = evdev_get_event(&xx, &yy);
    *x = xx;
    *y = yy;
    return p;
}

uint32_t __gui_get_ticks(void)
{
    return SDL_GetTicks();
}

#endif /* SIMPLE_GUI && ! GUI_USE_PORT */
