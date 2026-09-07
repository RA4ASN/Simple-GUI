// Simple GUI от RA4ASN
#ifndef GUI_SDL2_INPUT_H_INCLUDED
#define GUI_SDL2_INPUT_H_INCLUDED

#include "gui_user_include.h"

#if SIMPLE_GUI && GUI_SDL2_INPUT

#include <stdint.h>

/* Инициализация подсистема ввода SDL2 (хинты тач/мышь, сброс состояния).
   Вызывать один раз после gui_initialize(). Сигнатура без аргументов:
   размеры берутся из gui_get_max_w/h() в момент обработки события. */
void gui_sdl2_input_init(void);

/* Опросить очередь SDL-событий и обновить внутреннее состояние касания/курсора.
   Вызывать в начале каждого кадра из process_gui(). Внутри while(SDL_PollEvent). */
void gui_sdl2_input_poll(void);

/* Уровень касания за текущий кадр: 1, если палец/ЛКМ на экране сейчас ИЛИ
   в этом опросе очереди был DOWN (чтобы не терять быстрые тапы).
   Записывает текущие координаты в системе GUI (0..max_w / 0..max_h). */
int  gui_sdl2_input_get(uint16_t *x, uint16_t *y);

/* Освободить ресурсы (опционально, при завершении). */
void gui_sdl2_input_quit(void);

#endif /* SIMPLE_GUI && GUI_SDL2_INPUT */

#endif /* GUI_SDL2_INPUT_H_INCLUDED */
