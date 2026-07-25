// Simple GUI от RA4ASN

#include "gui_user_include.h"

#if WITHTOUCHGUI && SDL2_FONTS

#include "gui_sdl2_text.h"
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include "gui_settings.h"

//#define GUI_SDL2_TEXT_CACHE_STATS 1

#define SIZE_CACHE_SIZE 128
#define TEX_CACHE_SIZE 128

typedef struct {
    char text[TEXT_ARRAY_SIZE];
    TTF_Font* font;
    int w, h;
    int valid;
    int lru_counter;
} gui_size_cache_entry_t;

static gui_size_cache_entry_t size_cache[SIZE_CACHE_SIZE];
static int size_lru_counter = 0;

typedef struct {
    char text[TEXT_ARRAY_SIZE];
    TTF_Font* font;
    SDL_Color color;
    SDL_Texture* texture;
    int w, h;
    int valid;
    int lru_counter;
} gui_tex_cache_entry_t;

static gui_tex_cache_entry_t tex_cache[TEX_CACHE_SIZE];
static int tex_lru_counter = 0;

static TTF_Font* btn_font = NULL;
static TTF_Font* label_font = NULL;
static TTF_Font* title_font = NULL;

#if GUI_SDL2_TEXT_CACHE_STATS
static uint32_t size_cache_hits_sec = 0;
static uint32_t size_cache_misses_sec = 0;
static uint32_t tex_cache_hits_sec = 0;
static uint32_t tex_cache_misses_sec = 0;
static uint32_t last_stats_tick = 0;
#endif

void gui_sdl2_text_init(void)
{
    TTF_Init();

    if (!btn_font) {
        btn_font = TTF_OpenFont(BTN_FONT_PATH, gui_sizes.buttons_font_size);
        if (!btn_font) printf("[GUI SDL2] Failed to load button font %s: %s\n", BTN_FONT_PATH, TTF_GetError());
    }
    if (!label_font) {
        label_font = TTF_OpenFont(LABEL_FONT_PATH, gui_sizes.labels_font_size);
        if (!label_font) printf("[GUI SDL2] Failed to load label font %s: %s\n", LABEL_FONT_PATH, TTF_GetError());
    }
    if (!title_font) {
        title_font = TTF_OpenFont(WINDOW_TITLE_FONT_PATH, gui_sizes.win_title_font_size);
        if (!title_font) printf("[GUI SDL2] Failed to load title font %s: %s\n", WINDOW_TITLE_FONT_PATH, TTF_GetError());
    }
    memset(size_cache, 0, sizeof(size_cache));
    memset(tex_cache, 0, sizeof(tex_cache));
    size_lru_counter = 0;
    tex_lru_counter = 0;

#if GUI_SDL2_TEXT_CACHE_STATS
    last_stats_tick = SDL_GetTicks();
#endif
}

void gui_sdl2_text_cleanup(void)
{
    for (int i = 0; i < TEX_CACHE_SIZE; i++) {
        if (tex_cache[i].valid && tex_cache[i].texture) {
            SDL_DestroyTexture(tex_cache[i].texture);
            tex_cache[i].texture = NULL;
            tex_cache[i].valid = 0;
        }
    }
    if (btn_font) { TTF_CloseFont(btn_font); btn_font = NULL; }
    if (label_font) { TTF_CloseFont(label_font); label_font = NULL; }
    if (title_font) { TTF_CloseFont(title_font); title_font = NULL; }
}

TTF_Font* gui_sdl2_get_button_font(void) { return btn_font; }
TTF_Font* gui_sdl2_get_label_font(void) { return label_font; }
TTF_Font* gui_sdl2_get_window_title_font(void) { return title_font; }

#if GUI_SDL2_TEXT_CACHE_STATS
static void print_cache_stats(void)
{
    uint32_t current_tick = SDL_GetTicks();
    if (current_tick - last_stats_tick >= 1000) {
        uint32_t size_total = size_cache_hits_sec + size_cache_misses_sec;
        uint32_t tex_total = tex_cache_hits_sec + tex_cache_misses_sec;

        float size_rate = size_total > 0 ? (100.0f * size_cache_hits_sec) / size_total : 0.0f;
        float tex_rate = tex_total > 0 ? (100.0f * tex_cache_hits_sec) / tex_total : 0.0f;

        printf("[GUI SDL2 CACHE] 1s Stats -> Size: %u hits, %u misses (%.1f%%) | Tex: %u hits, %u misses (%.1f%%)\n",
               size_cache_hits_sec, size_cache_misses_sec, size_rate,
               tex_cache_hits_sec, tex_cache_misses_sec, tex_rate);

        // Сброс счетчиков для следующей секунды
        size_cache_hits_sec = 0;
        size_cache_misses_sec = 0;
        tex_cache_hits_sec = 0;
        tex_cache_misses_sec = 0;
        last_stats_tick = current_tick;
    }
}
#endif

// === Кэш размеров (LRU) ===
static gui_size_cache_entry_t* find_size_cache(const char* text, TTF_Font* font)
{
    int first_empty_idx = -1;
    int min_lru = INT_MAX;
    int min_lru_idx = 0;
    int is_hit = 0;

    for (int i = 0; i < SIZE_CACHE_SIZE; i++) {
        // 1. Ищем точное совпадение
        if (size_cache[i].valid &&
            size_cache[i].font == font &&
            strcmp(size_cache[i].text, text) == 0) {
            size_cache[i].lru_counter = ++size_lru_counter;
            is_hit = 1;
            min_lru_idx = i;
            break; // Нашли, дальше искать не нужно
        }
        // 2. Запоминаем первую свободную ячейку (на случай промаха)
        if (!size_cache[i].valid && first_empty_idx == -1) {
            first_empty_idx = i;
        }
        // 3. Ищем самую старую (LRU)
        if (size_cache[i].lru_counter < min_lru) {
            min_lru = size_cache[i].lru_counter;
            min_lru_idx = i;
        }
    }

    // Определяем целевой индекс: при попадании - это найденный, при промахе - свободный или самый старый
    int target_idx = is_hit ? min_lru_idx : (first_empty_idx != -1 ? first_empty_idx : min_lru_idx);

#if GUI_SDL2_TEXT_CACHE_STATS
    if (is_hit) {
        size_cache_hits_sec++;
    } else {
        size_cache_misses_sec++;
    }
    print_cache_stats();
#endif

    gui_size_cache_entry_t* entry = &size_cache[target_idx];

    if (font && text) {
        TTF_SizeText(font, text, &entry->w, &entry->h);
        strncpy(entry->text, text, TEXT_ARRAY_SIZE - 1);
        entry->text[TEXT_ARRAY_SIZE - 1] = '\0';
        entry->font = font;
        entry->valid = 1;
        entry->lru_counter = ++size_lru_counter;
    }
    return entry;
}

// === Кэш текстур (LRU) ===
static gui_tex_cache_entry_t* find_tex_cache(const char* text, TTF_Font* font, SDL_Color color)
{
    int first_empty_idx = -1;
    int min_lru = INT_MAX;
    int min_lru_idx = 0;
    int is_hit = 0;

    for (int i = 0; i < TEX_CACHE_SIZE; i++) {
        // 1. Ищем точное совпадение
        if (tex_cache[i].valid &&
            tex_cache[i].font == font &&
            tex_cache[i].color.r == color.r &&
            tex_cache[i].color.g == color.g &&
            tex_cache[i].color.b == color.b &&
            tex_cache[i].color.a == color.a &&
            strcmp(tex_cache[i].text, text) == 0) {
            tex_cache[i].lru_counter = ++tex_lru_counter;
            is_hit = 1;
            min_lru_idx = i;
            break; // Нашли, дальше искать не нужно
        }
        // 2. Запоминаем первую свободную ячейку (на случай промаха)
        if (!tex_cache[i].valid && first_empty_idx == -1) {
            first_empty_idx = i;
        }
        // 3. Ищем самую старую (LRU)
        if (tex_cache[i].lru_counter < min_lru) {
            min_lru = tex_cache[i].lru_counter;
            min_lru_idx = i;
        }
    }

    // Определяем целевой индекс
    int target_idx = is_hit ? min_lru_idx : (first_empty_idx != -1 ? first_empty_idx : min_lru_idx);

#if GUI_SDL2_TEXT_CACHE_STATS
    if (is_hit) {
        tex_cache_hits_sec++;
    } else {
        tex_cache_misses_sec++;
    }
    print_cache_stats();
#endif

    gui_tex_cache_entry_t* entry = &tex_cache[target_idx];

    // Если ячейка была занята другой текстурой (промах кэша + вытеснение LRU), уничтожаем старую
    if (entry->valid && entry->texture) {
        SDL_DestroyTexture(entry->texture);
    }

    strncpy(entry->text, text, TEXT_ARRAY_SIZE - 1);
    entry->text[TEXT_ARRAY_SIZE - 1] = '\0';
    entry->font = font;
    entry->color = color;
    entry->valid = 1;
    entry->lru_counter = ++tex_lru_counter;

    SDL_Surface* surf = TTF_RenderText_Blended(font, text, color);
    if (surf) {
        entry->texture = SDL_CreateTextureFromSurface(sdl2_get_renderer(), surf);
        if (entry->texture) {
            SDL_SetTextureBlendMode(entry->texture, SDL_BLENDMODE_BLEND);
        }
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

void gui_sdl2_get_text_size(const char* text, TTF_Font* font, int* w, int* h)
{
    if (!text || !font) { if (w) *w = 0; if (h) *h = 0; return; }
    gui_size_cache_entry_t* entry = find_size_cache(text, font);
    if (w) *w = entry->w;
    if (h) *h = entry->h;
}

void gui_sdl2_draw_text(const char* text, int x, int y, TTF_Font* font, gui_color_t color)
{
    if (!text || !font) return;

    SDL_Color c;
    c.r = (color >> 16) & 0xFF;
    c.g = (color >> 8) & 0xFF;
    c.b = (color >> 0) & 0xFF;
    c.a = (color >> 24) & 0xFF;

    gui_tex_cache_entry_t* entry = find_tex_cache(text, font, c);
    if (entry->texture) {
        SDL_Rect dst = {x, y, entry->w, entry->h};
        SDL_RenderCopy(sdl2_get_renderer(), entry->texture, NULL, &dst);
    }
}

void gui_sdl2_invalidate_font_cache(TTF_Font* font)
{
    if (!font) return;

    // Инвалидируем записи кэша размеров
    for (int i = 0; i < SIZE_CACHE_SIZE; i++) {
        if (size_cache[i].valid && size_cache[i].font == font) {
            size_cache[i].valid = 0;
            size_cache[i].text[0] = '\0';
            size_cache[i].w = 0;
            size_cache[i].h = 0;
        }
    }

    // Инвалидируем записи кэша текстур
    for (int i = 0; i < TEX_CACHE_SIZE; i++) {
        if (tex_cache[i].valid && tex_cache[i].font == font) {
            if (tex_cache[i].texture) {
                SDL_DestroyTexture(tex_cache[i].texture);
                tex_cache[i].texture = NULL;
            }
            tex_cache[i].valid = 0;
            tex_cache[i].text[0] = '\0';
            tex_cache[i].w = 0;
            tex_cache[i].h = 0;
        }
    }
}

void gui_sdl2_invalidate_text(const char* text, TTF_Font* font)
{
    if (!text || !font) return;

    // Удалить из кэша размеров
    for (int i = 0; i < SIZE_CACHE_SIZE; i++) {
        if (size_cache[i].valid &&
            size_cache[i].font == font &&
            strcmp(size_cache[i].text, text) == 0) {
            size_cache[i].valid = 0;
            size_cache[i].text[0] = '\0';
            size_cache[i].w = 0;
            size_cache[i].h = 0;
            size_cache[i].lru_counter = 0;
        }
    }

    // Удалить из кэша текстур
    for (int i = 0; i < TEX_CACHE_SIZE; i++) {
        if (tex_cache[i].valid &&
            tex_cache[i].font == font &&
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
}

#endif /* WITHTOUCHGUI && SDL2_FONTS */
