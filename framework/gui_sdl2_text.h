// framework/gui_sdl2_text.h
#ifndef GUI_SDL2_TEXT_H_INCLUDED
#define GUI_SDL2_TEXT_H_INCLUDED

#include "gui_user_include.h"

#if SDL2_FONTS

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "gui_sdl2_api.h" // для gui_color_t и sdl2_get_renderer()

// Инициализация модуля (загрузка шрифта, очистка кэша)
void gui_sdl2_text_init(void);

// Очистка кэша и закрытие шрифтов
void gui_sdl2_text_cleanup(void);

TTF_Font* gui_sdl2_get_button_font(void);
TTF_Font* gui_sdl2_get_label_font(void);
TTF_Font* gui_sdl2_get_window_title_font(void);

// Получение размеров текста (с учетом кэша)
void gui_sdl2_get_text_size(const char* text, TTF_Font* font, int* w, int* h);

// Отрисовка текста (с учетом кэша)
void gui_sdl2_draw_text(const char* text, int x, int y, TTF_Font* font, gui_color_t color);

// Инвалидация записей кэша, связанных с указанным шрифтом
// Вызывать ПЕРЕД TTF_CloseFont
void gui_sdl2_invalidate_font_cache(TTF_Font* font);

void gui_sdl2_invalidate_text(const char* text, TTF_Font* font);

#endif /* SDL2_FONTS */
#endif /* GUI_SDL2_TEXT_H_INCLUDED */
