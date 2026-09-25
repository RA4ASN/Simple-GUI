// Simple GUI от RA4ASN
#ifndef _gui_animation_h
#define _gui_animation_h
#include "gui_user_include.h"
#if SIMPLE_GUI

typedef enum {
	GUI_EASE_LINEAR,				// линейная интерполяция
	GUI_EASE_IN_OUT,				// плавный разгон/торможение (косинусная)
} gui_easing_t;

typedef struct {
	uint32_t start_tick;			// момент старта
	uint16_t duration;				// длительность, мс
	int32_t from;					// начальное значение
	int32_t to;						// целевое значение
	int32_t value;					// текущее значение (обновляется по времени)
	gui_easing_t easing;
	uint8_t active;					// 1 - анимация в процессе
} gui_anim_t;

/* Мгновенно установить значение (без анимации) */
void gui_anim_set(gui_anim_t *a, int32_t value);

/* Запустить анимацию от ТЕКУЩЕГО value к to (повторный старт корректно разворачивает её) */
void gui_anim_start(gui_anim_t *a, int32_t to, uint16_t duration, gui_easing_t easing);

/* Пересчитать value по прошедшему времени; возвращает 1, пока анимация активна */
uint8_t gui_anim_update(gui_anim_t *a);

/* Линейная интерполяция цветов: t = 0..100 (0 - c1, 100 - c2) */
gui_color_t gui_color_lerp(gui_color_t c1, gui_color_t c2, uint8_t t);

#endif /* SIMPLE_GUI */
#endif /* _gui_animation_h */
