// Simple GUI от RA4ASN
#include "gui_user_include.h"
#if SIMPLE_GUI && ! GUI_USE_PORT
#include "../gui_includes.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>

//#define GUI_SDL2_TEXT_CACHE_STATS	1
#define SIZE_CACHE_SIZE 			128
#define TEX_CACHE_SIZE 				128

/* FNV-1a хэш для ускорения поиска строк в кэше */
static inline uint32_t text_hash(const char *s)
{
    uint32_t h = 2166136261u;
    while (*s) { h ^= (uint8_t) *s++; h *= 16777619u; }
    return h;
}

typedef struct {
    char text[TEXT_ARRAY_SIZE];
    gui_font_t *font;
    uint32_t hash;
    int w, h;
    int valid;
    int lru_counter;
} gui_size_cache_entry_t;

static gui_size_cache_entry_t size_cache[SIZE_CACHE_SIZE];
static int size_lru_counter = 0;

typedef struct {
    char text[TEXT_ARRAY_SIZE];
    gui_font_t *font;
    SDL_Color color;
    SDL_Texture *texture;
    uint32_t hash;
    int w, h;
    int valid;
    int lru_counter;
} gui_tex_cache_entry_t;

static gui_tex_cache_entry_t tex_cache[TEX_CACHE_SIZE];
static int tex_lru_counter = 0;

static gui_font_t *btn_font = NULL;
static gui_font_t *label_font = NULL;
static gui_font_t *title_font = NULL;

#if GUI_SDL2_TEXT_CACHE_STATS
static uint32_t size_cache_hits_sec = 0;
static uint32_t size_cache_misses_sec = 0;
static uint32_t tex_cache_hits_sec = 0;
static uint32_t tex_cache_misses_sec = 0;
static uint32_t last_stats_tick = 0;
#endif

void gui_text_init(void)
{
	TTF_Init();

	if (!btn_font) {
		btn_font = gui_open_font(BTN_FONT_PATH, gui_sizes.buttons_font_size);
		if (!btn_font) printf("[GUI SDL2] Failed to load button font %s: %s\n", BTN_FONT_PATH, TTF_GetError());
	}

	if (!label_font) {
		label_font = gui_open_font(LABEL_FONT_PATH, gui_sizes.labels_font_size);
		if (!label_font) printf("[GUI SDL2] Failed to load label font %s: %s\n", LABEL_FONT_PATH, TTF_GetError());
	}

	if (!title_font) {
		title_font = gui_open_font(WINDOW_TITLE_FONT_PATH, gui_sizes.win_title_font_size);
		if (!title_font) printf("[GUI SDL2] Failed to load title font %s: %s\n", WINDOW_TITLE_FONT_PATH, TTF_GetError());
	}

#if GUI_SDL2_TEXT_CACHE
	memset(size_cache, 0, sizeof(size_cache));
	memset(tex_cache, 0, sizeof(tex_cache));
	size_lru_counter = 0;
	tex_lru_counter = 0;

#if GUI_SDL2_TEXT_CACHE_STATS
	last_stats_tick = __gui_get_ticks();
#endif
#endif
}

void gui_text_cleanup(void)
{
#if GUI_SDL2_TEXT_CACHE
	for (int i = 0; i < TEX_CACHE_SIZE; i++) {
		if (tex_cache[i].valid && tex_cache[i].texture) {
			SDL_DestroyTexture(tex_cache[i].texture);
			tex_cache[i].texture = NULL;
			tex_cache[i].valid = 0;
		}
	}
#endif

	if (btn_font) {
		TTF_CloseFont(btn_font);
		btn_font = NULL;
	}

	if (label_font) {
		TTF_CloseFont(label_font);
		label_font = NULL;
	}

	if (title_font) {
		TTF_CloseFont(title_font);
		title_font = NULL;
	}
}

gui_font_t *gui_get_button_font(void) { return btn_font; }
gui_font_t *gui_get_label_font(void) { return label_font; }
gui_font_t *gui_get_window_title_font(void) { return title_font; }

gui_font_t *gui_open_font(const char *name, int size)
{
    assert(name);
    return TTF_OpenFont(name, size);
}

int gui_get_font_height(const gui_font_t *font)
{
    return TTF_FontHeight(font);
}

int gui_get_font_ascent(gui_font_t *font)
{
    assert(font);
    return TTF_FontAscent(font);
}

void gui_close_font(gui_font_t *font)
{
    assert(font);
    TTF_CloseFont(font);
}

#if GUI_SDL2_TEXT_CACHE_STATS
static void print_cache_stats(void)
{
    uint32_t current_tick = __gui_get_ticks();
    if (current_tick - last_stats_tick >= 1000) {
        uint32_t size_total = size_cache_hits_sec + size_cache_misses_sec;
        uint32_t tex_total = tex_cache_hits_sec + tex_cache_misses_sec;
        float size_rate = size_total > 0 ? (100.0f * size_cache_hits_sec) / size_total : 0.0f;
        float tex_rate = tex_total > 0 ? (100.0f * tex_cache_hits_sec) / tex_total : 0.0f;
        printf("[GUI SDL2 CACHE] 1s Stats -> Size: %u hits, %u misses (%.1f%%) | Tex: %u hits, %u misses (%.1f%%)\n",
               size_cache_hits_sec, size_cache_misses_sec, size_rate,
               tex_cache_hits_sec, tex_cache_misses_sec, tex_rate);
        size_cache_hits_sec = 0;
        size_cache_misses_sec = 0;
        tex_cache_hits_sec = 0;
        tex_cache_misses_sec = 0;
        last_stats_tick = current_tick;
    }
}
#endif

// === Кэш размеров (LRU) ===
static gui_size_cache_entry_t *find_size_cache(const char *text, gui_font_t *font)
{
    int first_empty_idx = -1;
    int min_lru = INT_MAX;
    int min_lru_idx = 0;
    int is_hit = 0;
    uint32_t h = text_hash(text);

    for (int i = 0; i < SIZE_CACHE_SIZE; i++) {
        if (size_cache[i].valid && size_cache[i].font == font
            && size_cache[i].hash == h
            && strcmp(size_cache[i].text, text) == 0) {
            size_cache[i].lru_counter = ++size_lru_counter;
            is_hit = 1;
            min_lru_idx = i;
            break;
        }
        if (!size_cache[i].valid && first_empty_idx == -1)
            first_empty_idx = i;
        if (size_cache[i].lru_counter < min_lru) {
            min_lru = size_cache[i].lru_counter;
            min_lru_idx = i;
        }
    }

    int target_idx = is_hit ? min_lru_idx : (first_empty_idx != -1 ? first_empty_idx : min_lru_idx);

#if GUI_SDL2_TEXT_CACHE_STATS
    if (is_hit) size_cache_hits_sec++; else size_cache_misses_sec++;
    print_cache_stats();
#endif

    if (is_hit)
        return &size_cache[target_idx];

    gui_size_cache_entry_t *entry = &size_cache[target_idx];
    if (font && text) {
        TTF_SizeText(font, text, &entry->w, &entry->h);
        strncpy(entry->text, text, TEXT_ARRAY_SIZE - 1);
        entry->text[TEXT_ARRAY_SIZE - 1] = '\0';
        entry->font = font;
        entry->hash = h;
        entry->valid = 1;
        entry->lru_counter = ++size_lru_counter;
    }
    return entry;
}

// === Кэш текстур (LRU) ===
static gui_tex_cache_entry_t *find_tex_cache(const char *text, gui_font_t *font, SDL_Color color)
{
    int first_empty_idx = -1;
    int min_lru = INT_MAX;
    int min_lru_idx = 0;
    int is_hit = 0;
    uint32_t h = text_hash(text);

    for (int i = 0; i < TEX_CACHE_SIZE; i++) {
        if (tex_cache[i].valid && tex_cache[i].font == font
            && tex_cache[i].color.r == color.r
            && tex_cache[i].color.g == color.g
            && tex_cache[i].color.b == color.b
            && tex_cache[i].color.a == color.a
            && tex_cache[i].hash == h
            && strcmp(tex_cache[i].text, text) == 0) {
            tex_cache[i].lru_counter = ++tex_lru_counter;
            is_hit = 1;
            min_lru_idx = i;
            break;
        }
        if (!tex_cache[i].valid && first_empty_idx == -1)
            first_empty_idx = i;
        if (tex_cache[i].lru_counter < min_lru) {
            min_lru = tex_cache[i].lru_counter;
            min_lru_idx = i;
        }
    }

    int target_idx = is_hit ? min_lru_idx : (first_empty_idx != -1 ? first_empty_idx : min_lru_idx);

#if GUI_SDL2_TEXT_CACHE_STATS
    if (is_hit) tex_cache_hits_sec++; else tex_cache_misses_sec++;
    print_cache_stats();
#endif

    if (is_hit)
        return &tex_cache[target_idx];

    gui_tex_cache_entry_t *entry = &tex_cache[target_idx];
    if (entry->valid && entry->texture)
        SDL_DestroyTexture(entry->texture);

    strncpy(entry->text, text, TEXT_ARRAY_SIZE - 1);
    entry->text[TEXT_ARRAY_SIZE - 1] = '\0';
    entry->font = font;
    entry->color = color;
    entry->hash = h;
    entry->valid = 1;
    entry->lru_counter = ++tex_lru_counter;

    SDL_Surface *surf = TTF_RenderText_Blended(font, text, color);
    if (surf) {
        entry->texture = SDL_CreateTextureFromSurface(sdl2_get_renderer(), surf);
        if (entry->texture)
            SDL_SetTextureBlendMode(entry->texture, SDL_BLENDMODE_BLEND);
        entry->w = surf->w;
        entry->h = surf->h;
        SDL_FreeSurface(surf);
    } else {
        entry->texture = NULL;
        entry->w = 0;
        entry->h = 0;
    }
    return entry;
}

void gui_invalidate_text(const char *text, gui_font_t *font)
{
#if GUI_SDL2_TEXT_CACHE
	if (!text || !font) return;

	uint32_t h = text_hash(text);

	for (int i = 0; i < SIZE_CACHE_SIZE; i++) {
		if (size_cache[i].valid &&
			size_cache[i].font == font &&
			size_cache[i].hash == h &&
			strcmp(size_cache[i].text, text) == 0) {
			size_cache[i].valid = 0;
			size_cache[i].text[0] = '\0';
			size_cache[i].w = 0;
			size_cache[i].h = 0;
			size_cache[i].lru_counter = 0;
		}
	}

	for (int i = 0; i < TEX_CACHE_SIZE; i++) {
		if (tex_cache[i].valid &&
			tex_cache[i].font == font &&
			tex_cache[i].hash == h &&
			strcmp(tex_cache[i].text, text) == 0) {
			if (tex_cache[i].texture) {
				SDL_DestroyTexture(tex_cache[i].texture);
				tex_cache[i].texture = NULL;
			}

			tex_cache[i].valid = 0;
			tex_cache[i].text[0] = '\0';
			tex_cache[i].w = 0;
			tex_cache[i].h = 0;
			tex_cache[i].lru_counter = 0;
		}
	}
#else
	(void)text;
	(void)font;
#endif
}

void gui_print(const char * text, int x, int y, gui_font_t *font, gui_color_t color)
{
    window_t *win = get_win(get_current_drawing_window());
    gui_draw_text(text, win->x1 + x, win->y1 + y, font, color);
}

/* =====================================================================
   ЦВЕТОВАЯ РАЗМЕТКА ТЕКСТА
   ===================================================================== */

typedef void (*colored_seg_cb)(const char *seg, int len,
                               gui_color_t color, void *ctx);

static int is_hex6(const char *p)
{
    for (int i = 0; i < 6; i++)
        if (!isxdigit((unsigned char)p[i])) return 0;
    return 1;
}

static int is_marker(const char *p)
{
    return (p[0] == '/' && p[1] == 'c' &&
            (p[2] == 's' || p[2] == 'd' || p[2] == 'e'));
}

static void parse_colored_text(const char *text, gui_color_t default_color,
                               colored_seg_cb cb, void *ctx)
{
    gui_color_t color = default_color;
    const char *p = text;

    while (*p)
    {
        if (p[0] == '/' && p[1] == 'c' && p[2] == 's' && is_hex6(p + 3))
        {
            char hex[7] = { p[3], p[4], p[5], p[6], p[7], p[8], '\0' };
            uint32_t rgb = (uint32_t)strtoul(hex, NULL, 16);
            color = 0xFF000000u | rgb;
            p += 9;
        }
        else if (p[0] == '/' && p[1] == 'c' && p[2] == 'd')
        {
            color = default_color;
            p += 3;
        }
        else if (p[0] == '/' && p[1] == 'c' && p[2] == 'e')
        {
            p += 3;
        }
        else
        {
            const char *start = p;
            while (*p && !is_marker(p))
                p++;
            if (p > start)
                cb(start, (int)(p - start), color, ctx);
        }
    }
}

typedef struct { gui_font_t *font; int total_w; int max_h; } size_ctx_t;

static void size_cb(const char *seg, int len, gui_color_t color, void *ctx)
{
    (void)color;
    size_ctx_t *sc = (size_ctx_t *)ctx;
    char tmp[TEXT_ARRAY_SIZE];
    int n = len < TEXT_ARRAY_SIZE - 1 ? len : TEXT_ARRAY_SIZE - 1;
    memcpy(tmp, seg, n);
    tmp[n] = '\0';
    int w = 0, h = 0;
    gui_get_text_sizes(tmp, sc->font, &w, &h);
    sc->total_w += w;
    if (h > sc->max_h) sc->max_h = h;
}

static int gui_has_color_markers(const char *text)
{
    if (!text) return 0;
    return (strstr(text, "/c") != NULL) ? 1 : 0;
}

void gui_get_text_sizes(const char *text, gui_font_t *font,
                               int *w, int *h)
{
    if (!text || !font)
    {
    	if (w) *w = 0;
    	if (h) *h = 0; return;
    }

    if (!gui_has_color_markers(text))
    {
    #if GUI_SDL2_TEXT_CACHE
    	gui_size_cache_entry_t *entry = find_size_cache(text, font);
    	if (w) *w = entry->w;
    	if (h) *h = entry->h;
    #else
    	int ww = 0;
    	int hh = 0;

    	TTF_SizeText(font, text, &ww, &hh);

    	if (w) *w = ww;
    	if (h) *h = hh;
    #endif
        return;
    }

    size_ctx_t sc = { font, 0, 0 };
    parse_colored_text(text, 0, size_cb, &sc);

    if (w) *w = sc.total_w;
    if (h) *h = sc.max_h;
}

typedef struct { gui_font_t *font; int x, y; } draw_ctx_t;

static void draw_cb(const char *seg, int len, gui_color_t color, void *ctx)
{
    draw_ctx_t *dc = (draw_ctx_t *)ctx;
    char tmp[TEXT_ARRAY_SIZE];
    int n = len < TEXT_ARRAY_SIZE - 1 ? len : TEXT_ARRAY_SIZE - 1;
    memcpy(tmp, seg, n);
    tmp[n] = '\0';
    int w = 0, h = 0;
    gui_get_text_sizes(tmp, dc->font, &w, &h);
    gui_draw_text(tmp, dc->x, dc->y, dc->font, color);
    dc->x += w;
}

void gui_draw_text(const char *text, int x, int y, gui_font_t *font, gui_color_t default_color)
{
    if (!text || !font) return;
    if (!gui_has_color_markers(text))
    {
    	SDL_Color c;
    	c.r = (default_color >> 16) & 0xFF;
    	c.g = (default_color >> 8) & 0xFF;
    	c.b = (default_color >> 0) & 0xFF;
    	c.a = (default_color >> 24) & 0xFF;

    	SDL_Renderer *renderer = sdl2_get_renderer();
    	if (!renderer) return;

    #if GUI_SDL2_TEXT_CACHE
    	gui_tex_cache_entry_t *entry = find_tex_cache(text, font, c);

    	if (entry->texture) {
    		SDL_Rect dst = {x, y, entry->w, entry->h};
    		SDL_RenderCopy(renderer, entry->texture, NULL, &dst);
    	}
    #else
    	SDL_Surface *surf = TTF_RenderText_Blended(font, text, c);
    	if (!surf) return;

    	SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);

    	if (tex) {
    		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    		SDL_Rect dst = {x, y, surf->w, surf->h};
    		SDL_RenderCopy(renderer, tex, NULL, &dst);

    		SDL_DestroyTexture(tex);
    	}

    	SDL_FreeSurface(surf);
    #endif
        return;
    }
    draw_ctx_t dc = { font, x, y };
    parse_colored_text(text, default_color, draw_cb, &dc);
}

#endif /* SIMPLE_GUI && ! GUI_USE_PORT */
