// Simple GUI от RA4ASN
#include "gui_user_include.h"
#if SIMPLE_GUI
#include "gui_includes.h"

/* Коэффициент прогресса 0..100 с учётом функции плавности */
static int32_t apply_easing(gui_easing_t e, uint32_t t100)
{
	switch (e)
	{
	case GUI_EASE_IN_OUT: // косинусная плавность: 0.5 - 0.5*cos(pi * t)
		return (int32_t) (50.0f - 50.0f * cosf((float) M_PI * (float) t100 / 100.0f));

	case GUI_EASE_LINEAR:
	default:
		return (int32_t) t100;
	}
}

void gui_anim_set(gui_anim_t *a, int32_t value)
{
	a->from = value;
	a->to = value;
	a->value = value;
	a->duration = 0;
	a->active = 0;
}

void gui_anim_start(gui_anim_t *a, int32_t to, uint16_t duration, gui_easing_t easing)
{
	if (!duration || a->value == to)	// анимация не нужна
	{
		gui_anim_set(a, to);
		return;
	}

	// старт от текущего значения (корректный реверс)
	a->from = a->value;
	a->to = to;
	a->duration = duration;
	a->easing = easing;
	a->start_tick = __gui_get_ticks();
	a->active = 1;
}

uint8_t gui_anim_update(gui_anim_t *a)
{
	if (!a->active) return 0;

	const uint32_t elapsed = __gui_get_ticks() - a->start_tick;

	if (elapsed >= a->duration)
	{
		a->value = a->to;
		a->active = 0;
		return 0;
	}

	const uint32_t t100 = elapsed * 100 / a->duration;
	a->value = a->from + (a->to - a->from) * apply_easing(a->easing, t100) / 100;

	return 1;
}

gui_color_t gui_color_lerp(gui_color_t c1, gui_color_t c2, uint8_t t)
{
	const int32_t r1 = (c1 >> 16) & 0xFF, g1 = (c1 >> 8) & 0xFF, b1 = c1 & 0xFF;
	const int32_t r2 = (c2 >> 16) & 0xFF, g2 = (c2 >> 8) & 0xFF, b2 = c2 & 0xFF;
	const int32_t k = t > 100 ? 100 : t;
	const int32_t r = r1 + (r2 - r1) * k / 100;
	const int32_t g = g1 + (g2 - g1) * k / 100;
	const int32_t b = b1 + (b2 - b1) * k / 100;

	return GUI_TFTRGB(r, g, b);
}
#endif /* SIMPLE_GUI */
