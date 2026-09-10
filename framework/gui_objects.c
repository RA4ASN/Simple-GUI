// Simple GUI от RA4ASN

#include "gui_user_include.h"

#if SIMPLE_GUI

#include "gui_includes.h"

const label_t label_default = 	{ 0, CANCELLED, 0, NON_VISIBLE, "", "", GUI_COLOR_WHITE, };
const button_t button_default = { 0, 0, CANCELLED, BUTTON_NON_LOCKED, 0, 1, 0, 0, NON_VISIBLE, INT32_MAX, "", "", };
const text_field_t tf_default = { 0, 0, CANCELLED, 0, NON_VISIBLE, UP, "", };
const touch_area_t ta_default = { 0, 0, 0, 0, 0, "", 0, 0, 0, 0, 0, };
const switch_t switch_default = { 0, CANCELLED, NON_VISIBLE, 0, "", "", 0,
	GUI_COLOR_SWITCH_ON, GUI_COLOR_SWITCH_OFF, GUI_COLOR_SWITCH_KNOB,
	SWITCH_CAPTION_LEFT,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL,
	{ 0, 0, 0, 0, 0, GUI_EASE_LINEAR, 0 } };

const gui_color_t btn_bg_colors[BG_COUNT] =
		{
				GUI_COLOR_BUTTON_NON_LOCKED,
				GUI_COLOR_BUTTON_PR_NON_LOCKED,
				GUI_COLOR_BUTTON_LOCKED,
				GUI_COLOR_BUTTON_PR_LOCKED,
				GUI_COLOR_BUTTON_DISABLED,
		};

uint8_t get_commonbtn_w(void)
{
	return gui_sizes.common_btn_width;
}

uint8_t get_commonbtn_h(void)
{
	return gui_sizes.common_btn_height;
}

// *************** Labels ***************

/* Получение ширины метки в пикселях  */
uint16_t get_label_width(const label_t * const lh)
{
	return lh->bbox_w;
}

/* Получение высоты метки в пикселях  */
uint16_t get_label_height(const label_t * const lh)
{
	return lh->bbox_h;
}

uint16_t get_label_width2(const char * name)
{
	window_t * win = get_win(get_parent_window());
	label_t * lh = (label_t *) find_gui_obj(TYPE_LABEL, win, name);

	return lh->bbox_w;
}

uint16_t get_label_height2(const char * name)
{
	window_t * win = get_win(get_parent_window());
	label_t * lh = (label_t *) find_gui_obj(TYPE_LABEL, win, name);

	return lh->bbox_h;
}

void draw_label(label_t * lh)
{
	window_t * win = get_win(lh->parent);
	uint16_t x = win->x1 + lh->x;
	uint16_t y = win->y1 + lh->y;
	uint16_t xx = x;

//	__gui_draw_rect(x, y, lh->bbox_w, lh->bbox_h, GUI_COLOR_YELLOW, 0);

	if (lh->bbox_align == ALIGNMENT_CENTER)
		xx += (lh->bbox_w / 2) - (lh->width_text_pix / 2);
	else if (lh->bbox_align == ALIGNMENT_RIGHT)
		xx += lh->bbox_w - lh->width_text_pix;

    gui_sdl2_draw_text_colored(lh->text, xx, y, lh->font, lh->color);
}

// *************** Switches ****************

/* Установка состояния переключателя с опциональной анимацией.
   animate = 0: мгновенная установка (для инициализации окна)
   animate = 1: плавная анимация (для физического нажатия) */
void switch_set_payload(switch_t * sw, int new_payload, uint8_t animate)
{
	new_payload = new_payload ? 1 : 0;
	if (sw->payload == new_payload) return;

	sw->payload = new_payload;

	if (animate) {
		// Запуск анимации от текущего значения к целевому
		gui_anim_start(&sw->anim, sw->payload ? 100 : 0, switch_anim_duration_ms, GUI_EASE_IN_OUT);
	} else {
		// Мгновенная установка без анимации
		gui_anim_set(&sw->anim, sw->payload ? 100 : 0);
	}
}

/* Пересчёт метрик подписи и полного габарита переключателя.
   Вызывается при создании, смене текста, позиции подписи и размеров тела. */
static void switch_update_layout(switch_t * sw)
{
	int tw = 0, th = 0;
	if (sw->font && sw->text[0])
		gui_sdl2_get_text_size(sw->text, sw->font, &tw, &th);

	sw->cap_w = (uint16_t) tw;
	sw->cap_h = (uint16_t) th;

	const uint16_t gap = sw->text[0] ? switch_caption_indent : 0;

	switch (sw->caption_align)
	{
	case SWITCH_CAPTION_TOP:
		sw->w = (sw->sw_w > sw->cap_w) ? sw->sw_w : sw->cap_w;
		sw->h = sw->cap_h + gap + sw->sw_h;
		sw->cap_x = (sw->w - sw->cap_w) / 2;
		sw->cap_y = 0;
		sw->pill_x = (sw->w - sw->sw_w) / 2;		// тело по середине текстовой строки
		sw->pill_y = sw->cap_h + gap;
		break;

	case SWITCH_CAPTION_BOTTOM:
		sw->w = (sw->sw_w > sw->cap_w) ? sw->sw_w : sw->cap_w;
		sw->h = sw->sw_h + gap + sw->cap_h;
		sw->pill_x = (sw->w - sw->sw_w) / 2;		// тело по середине текстовой строки
		sw->pill_y = 0;
		sw->cap_x = (sw->w - sw->cap_w) / 2;
		sw->cap_y = sw->sw_h + gap;
		break;

	case SWITCH_CAPTION_RIGHT:
		sw->w = sw->sw_w + gap + sw->cap_w;
		sw->h = (sw->sw_h > sw->cap_h) ? sw->sw_h : sw->cap_h;
		sw->pill_x = 0;
		sw->pill_y = (sw->h - sw->sw_h) / 2;
		sw->cap_x = sw->sw_w + gap;
		sw->cap_y = (sw->h - sw->cap_h) / 2;
		break;

	case SWITCH_CAPTION_LEFT:
	default:
		sw->w = sw->cap_w + gap + sw->sw_w;
		sw->h = (sw->sw_h > sw->cap_h) ? sw->sw_h : sw->cap_h;
		sw->cap_x = 0;
		sw->cap_y = (sw->h - sw->cap_h) / 2;
		sw->pill_x = sw->cap_w + gap;
		sw->pill_y = (sw->h - sw->sw_h) / 2;
		break;
	}
}

/* Отрисовка переключателя (таблетка с ползунком и подписью) с анимацией */
void draw_switch(switch_t * sw)
{
	window_t * win = get_win(sw->parent);
	gui_anim_update(& sw->anim);				// прогресс по времени (каждый кадр)

	const int32_t p = sw->anim.value;			// 0..100: позиция ползунка и степень заливки
	const uint16_t x = win->x1 + sw->x + sw->pill_x;
	const uint16_t y = win->y1 + sw->y + sw->pill_y;
	const uint16_t r = sw->sw_h / 2;			// радиус скругления "таблетки"

	/* плавная смена цвета заливки и рамки */
	const gui_color_t fill = gui_color_lerp(sw->color_off, sw->color_on, (uint8_t) p);
	const gui_color_t border = gui_color_lerp(GUI_COLOR_SWITCH_OFF_BORDER, GUI_COLOR_SWITCH_ON_BORDER, (uint8_t) p);

	/* подпись */
	if (sw->cap_w && sw->font)
		gui_sdl2_draw_text(sw->text, win->x1 + sw->x + sw->cap_x, win->y1 + sw->y + sw->cap_y, sw->font, GUI_COLOR_WHITE);

	/* фон-таблетка */
	__gui_draw_rounded_rect(x, y, sw->sw_w - 1, sw->sw_h - 1, r, fill, 1);
	__gui_draw_rounded_rect(x, y, sw->sw_w - 1, sw->sw_h - 1, r, border, 0);

	/* круг-ползунок: позиция интерполируется между выкл и вкл */
	const uint16_t kd = sw->sw_h - 2 * switch_knob_indent;		// диаметр круга
	const uint16_t kx_off = x + switch_knob_indent;
	const uint16_t kx_on = x + sw->sw_w - kd - switch_knob_indent - 1;
	const uint16_t kx = kx_off + (uint16_t) ((kx_on - kx_off) * p / 100);
	const uint16_t ky = y + switch_knob_indent;

	__gui_draw_rounded_rect(kx, ky, kd - 1, kd - 1, kd / 2, sw->state == PRESSED ? GUI_COLOR_GRAY : sw->knob_color, 1);
	__gui_draw_rounded_rect(kx, ky, kd - 1, kd - 1, kd / 2, GUI_COLOR_GRAY, 0);

	/* подсветка нажатия */
	if (sw->state == PRESSED)
		__gui_draw_rounded_rect(x + 1, y + 1, sw->sw_w - 3, sw->sw_h - 3, r - 1, GUI_COLOR_BLACK, 0);
}

// *************** Buttons ****************
/* Пересчёт кэша метрик двухстрочного текста: позиция разделителя и ширины строк.
   Вызывается при создании кнопки и при любом изменении её текста/шрифта. */
void button_update_text_metrics(button_t * bh)
{
	const char * sep = strchr(bh->text, '|');

	if (sep) {
		bh->sep_pos = (uint8_t) (sep - bh->text);
		char tmp[TEXT_ARRAY_SIZE];
		int n1 = bh->sep_pos < TEXT_ARRAY_SIZE - 1 ? bh->sep_pos : TEXT_ARRAY_SIZE - 1;
		memcpy(tmp, bh->text, n1);
		tmp[n1] = '\0';
		int w1 = 0, h1 = 0;
		gui_sdl2_get_text_size(tmp, bh->font, &w1, &h1);
		bh->line1_w = (uint16_t) w1;
		int w2 = 0, h2 = 0;
		gui_sdl2_get_text_size(sep + 1, bh->font, &w2, &h2);
		bh->line2_w = (uint16_t) w2;
	} else {
		bh->sep_pos = 0;
		int w = 0, h = 0;
		gui_sdl2_get_text_size(bh->text, bh->font, &w, &h);
		bh->line1_w = (uint16_t) w;
		bh->line2_w = 0;
	}
}
void draw_button(button_t * bh)
{
	window_t * win = get_win(bh->parent);
	uint16_t x = win->x1 + bh->x1;
	uint16_t y = win->y1 + bh->y1;
	gui_color_t c1 = bh->state == DISABLED ? GUI_COLOR_BUTTON_DISABLED :
			(bh->is_locked ? GUI_COLOR_BUTTON_LOCKED : GUI_COLOR_BUTTON_NON_LOCKED);
	gui_color_t c2 = bh->state == DISABLED ? GUI_COLOR_BUTTON_DISABLED :
			(bh->is_locked ? GUI_COLOR_BUTTON_PR_LOCKED : GUI_COLOR_BUTTON_PR_NON_LOCKED);

	__gui_draw_rounded_rect(x, y, bh->w - 1, bh->h - 1, button_round_radius, GUI_COLOR_GRAY, 0);
	__gui_draw_rounded_rect(x + 1, y + 1, bh->w - 3, bh->h - 3, button_round_radius, GUI_COLOR_BLACK, 0);
	__gui_draw_rounded_rect(x + 2, y + 2, bh->w - 5, bh->h - 5, button_round_radius, bh->state == PRESSED ? c2 : c1, 1);

	/* Отрисовка текста кнопки */
	if (!bh->line1_w && bh->text[0])	// самовосстановление при пустом кэше
		button_update_text_metrics(bh);

	const uint16_t shiftX = bh->state == PRESSED ? 1 : 0;
	const uint16_t shiftY = bh->state == PRESSED ? 1 : 0;
	const gui_color_t textcolor = GUI_COLOR_BLACK;
	const int line_h = TTF_FontHeight(bh->font);

	if (!bh->sep_pos) {
		gui_sdl2_draw_text(bh->text, shiftX + x + (bh->w - bh->line1_w) / 2, shiftY + y + (bh->h - line_h) / 2, bh->font, textcolor);
	} else {
		char tmp[TEXT_ARRAY_SIZE];
		int n1 = bh->sep_pos < TEXT_ARRAY_SIZE - 1 ? bh->sep_pos : TEXT_ARRAY_SIZE - 1;
		memcpy(tmp, bh->text, n1);
		tmp[n1] = '\0';
		const int y_start = shiftY + y + (bh->h - line_h * 2) / 2;
		gui_sdl2_draw_text(tmp, shiftX + x + (bh->w - bh->line1_w) / 2, y_start, bh->font, textcolor);
		gui_sdl2_draw_text(bh->text + bh->sep_pos + 1, shiftX + x + (bh->w - bh->line2_w) / 2, y_start + line_h, bh->font, textcolor);
	}
	if (bh->is_focus)
		gui_drawDashedRectangle(x + 4, y + 4, bh->w - 8, bh->h - 8, 4, GUI_COLOR_BLACK);
}

void draw_close_button(button_t * bh)
{
	window_t * win = get_win(bh->parent);
	uint16_t x = win->x1 + bh->x1;
	uint16_t y = win->y1 + bh->y1;

	uint16_t w = bh->w;
	uint16_t h = bh->h;

	__gui_draw_rect(x, y, w, h, GUI_COLOR_BLACK, 0);
	__gui_draw_line(x, y, x + w, y + h, GUI_COLOR_BLACK);
	__gui_draw_line(x, y + h, x + w, y, GUI_COLOR_BLACK);
}

// *************** Canvas ****************
void draw_canvas(canvas_t * ca)
{
	window_t * win = get_win(ca->parent);

	uint16_t x = win->draw_x1 + ca->x;
	uint16_t y = win->draw_y1 + ca->y;
	uint8_t alpha = (ca->color >> 24) & 0xFF;

	if (ca->background)
		__gui_draw_semitransparent_rect(x, y, x + ca->w - 1, y + ca->h - 1,
				ca->color, alpha);

	if (ca->border)
		__gui_draw_rect(x, y, ca->w, ca->h, GUI_COLOR_GRAY, 0);
}

void gui_canvas_set_active(const char * name)
{
	window_t * win = get_win(get_current_drawing_window());
	win->ca_current = find_gui_obj(TYPE_CANVAS, win, name);
}

void gui_canvas_print(const char * text, int x, int y, TTF_Font * font, gui_color_t color)
{
	window_t * win = get_win(get_current_drawing_window());
	canvas_t * ca = win->ca_current;
	if (ca == NULL)
		return;

	gui_sdl2_draw_text(text, win->draw_x1 + ca->x + x, win->draw_y1 + ca->y + y,
			font, color);
}

// *************** Text fields ***************

/* Рассчитать размеры текстового поля */
void textfield_update_size(text_field_t * tf)
{
    GUI_ASSERT(tf != NULL);

    int w_char = 0, h_char = 0;
    TTF_SizeText(tf->font, "M", &w_char, &h_char);
    tf->w = w_char * tf->w_sim;
    tf->h = h_char * tf->h_str;

    GUI_ASSERT(tf->w < gui_sizes.max_w);
    GUI_ASSERT(tf->h < gui_sizes.max_h - gui_sizes.window_title_height);
}

/* Добавить строку в текстовое поле */
void textfield_add_string_old(text_field_t * tf, const char * str, gui_color_t color)
{
	GUI_ASSERT(tf != NULL);

	tf_entry_t * rec = &  tf->string[tf->index];
	strncpy(rec->text, str, TEXT_ARRAY_SIZE - 1);
	rec->color_line = color;
	tf->index ++;
	tf->index = tf->index >= tf->h_str ? 0 : tf->index;
}

void textfield_add_string(const char * name, const char * str, gui_color_t color)
{
	window_t * win = get_win(get_parent_window());
	text_field_t * tf = (text_field_t *) find_gui_obj(TYPE_TEXT_FIELD, win, name);

	tf_entry_t * rec = &  tf->string[tf->index];
	GUI_ASSERT(rec);
	strncpy(rec->text, str, TEXT_ARRAY_SIZE - 1);
	rec->color_line = color;
	tf->index ++;
	tf->index = tf->index >= tf->h_str ? 0 : tf->index;
}

/* Очистить текстовое поле */
void textfield_clean(const char * name)
{
	window_t * win = get_win(get_parent_window());
	text_field_t * tf = (text_field_t *) find_gui_obj(TYPE_TEXT_FIELD, win, name);

	tf->index = 0;
	memset(tf->string, 0, tf->h_str * sizeof(tf_entry_t));
}

void draw_textfield(text_field_t * tf)
{
    window_t * win = get_win(tf->parent);
    uint16_t x = win->x1 + tf->x1;
    uint16_t y = win->y1 + tf->y1;
    int j = tf->index - 1;
    int line_h = TTF_FontHeight(tf->font);

    for (uint8_t i = 0; i < tf->h_str; i ++, j --)
    {
        uint8_t pos = tf->direction ? i : (tf->h_str - i - 1);
        j = j < 0 ? (tf->h_str - 1) : j;

        gui_sdl2_draw_text(tf->string[j].text, x, y + line_h * pos, tf->font, tf->string[j].color_line);

    }
}

// *************** Sliders ****************

static void slider_update(slider_t * sl, uint16_t x, uint16_t y)
{
	if (sl->orientation == ORIENTATION_HORIZONTAL)
	{
		sl->value_p = sl->scale_x + sl->scale_size * sl->value / 100;
		sl->x1_p = sl->value_p - gui_sizes.sliders_w;
		sl->y1_p = 0;
		sl->x2_p = sl->value_p + gui_sizes.sliders_w;
		sl->y2_p = gui_sizes.sliders_h * 2;
	}
	else if (sl->orientation == ORIENTATION_VERTICAL)
	{
		sl->value_p = sl->scale_y + sl->scale_size * sl->value / 100;
		sl->x1_p = 0;
		sl->y1_p = sl->value_p - gui_sizes.sliders_w;
		sl->x2_p = gui_sizes.sliders_h * 2;
		sl->y2_p = sl->value_p + gui_sizes.sliders_w;
	}
}

/* Отрисовка слайдера */
void draw_slider(slider_t * sl)
{
	window_t * win = get_win(sl->parent);
	uint16_t x = win->x1 + sl->x;
	uint16_t y = win->y1 + sl->y;

	slider_update(sl, x, y);

	if (sl->orientation == ORIENTATION_HORIZONTAL)
	{
		// scale
		__gui_draw_rect(x + sl->scale_x, y + sl->scale_y, sl->scale_size, gui_sizes.sliders_scale_thickness, GUI_COLOR_WHITE, 0);
		__gui_draw_rect(x + sl->scale_x + 1, y + sl->scale_y + 1, sl->scale_size - 2, gui_sizes.sliders_scale_thickness - 2, GUI_COLOR_BLACK, 1);

		// handle
		__gui_draw_rect(x + sl->x1_p, y + sl->y1_p, sl->x2_p - sl->x1_p, sl->y2_p - sl->y1_p,
				sl->state == PRESSED ? GUI_COLOR_BUTTON_PR_NON_LOCKED : GUI_COLOR_BUTTON_NON_LOCKED, 1);
		__gui_draw_line(x + sl->value_p, y + sl->y1_p, x + sl->value_p, y + sl->y2_p - 1, GUI_COLOR_WHITE);

	}
	else if (sl->orientation == ORIENTATION_VERTICAL)
	{
		// scale
		__gui_draw_rect(x + sl->scale_x, y + sl->scale_y, gui_sizes.sliders_scale_thickness, sl->scale_size, GUI_COLOR_WHITE, 0);
		__gui_draw_rect(x + sl->scale_x + 1, y + sl->scale_y + 1, gui_sizes.sliders_scale_thickness - 2, sl->scale_size - 2, GUI_COLOR_BLACK, 1);

		// handle
		__gui_draw_rect(x + sl->x1_p, y + sl->y1_p,  sl->x2_p - sl->x1_p, sl->y2_p - sl->y1_p,
				sl->state == PRESSED ? GUI_COLOR_BUTTON_PR_NON_LOCKED : GUI_COLOR_BUTTON_NON_LOCKED, 1);
		__gui_draw_line(x + sl->x1_p, y + sl->value_p, x + sl->x2_p - 1, y + sl->value_p, GUI_COLOR_WHITE);
	}
}

// *************** Common ***************

static obj_type_t parse_obj_name(const char * name)
{
	GUI_ASSERT(name);

	if (! strncmp(name, "btn_", 4))
		return TYPE_BUTTON;
	else if (! strncmp(name, "lbl_", 4))
		return TYPE_LABEL;
	else if (! strncmp(name, "sl_", 3))
		return TYPE_SLIDER;
	else if (! strncmp(name, "sw_", 3))
		return TYPE_SWITCH;
	else if (! strncmp(name, "btc_", 4))
		return TYPE_CLOSE_BUTTON;
	else if (! strncmp(name, "ta_", 3))
		return TYPE_TOUCH_AREA;
	else if (! strncmp(name, "tf_", 3))
		return TYPE_TEXT_FIELD;
	else if (! strncmp(name, "ca_", 3))
		return TYPE_CANVAS;

	else
	{
		GUI_DEBUG_PRINT("Unrecognized GUI object type: %s\n", name);
		GUI_ASSERT(0);
		return TYPE_DUMMY;
	}
}

static void obj_name_user(char * name)
{
	char * r = strrchr(name, '#');
	if (r) name[r - name] = '\0';
}

/* Сохранение габаритов прямоугольной области массового выравнивания:
   count объектов в сетке cols x rows с шагом interval, начиная с (x, y) */
static void save_arrange_area(window_t * win, uint16_t x, uint16_t y, uint16_t w, uint16_t h,
	uint8_t count, uint8_t cols, uint8_t interval)
{
	if (! count || ! cols) return;

	const uint8_t rows = (count + cols - 1) / cols;
	const uint8_t cols_used = count < cols ? count : cols;

	win->arrange_area.x = x;
	win->arrange_area.y = y;
	win->arrange_area.w = cols_used * w + (cols_used - 1) * interval;
	win->arrange_area.h = rows * h + (rows - 1) * interval;
	win->arrange_area_valid = 1;
}

// Вариабельные аргументы зависят от типа создаваемого объекта, который
// определяется ПРЕФИКСОМ имени 'name' (btn_ / lbl_ / sl_ / ta_ / tf_).
//
// TYPE_LABEL  (lbl_):       gui_color_t color, uint32_t width_by_symbols
//                           color            - цвет текста метки
//                           width_by_symbols - ширина bounding box в символах
//                                              (по ней заранее считается bbox_w/bbox_h)
//
// TYPE_BUTTON (btn_):       int w, int h, uint32_t is_repeating, uint32_t is_long_press, char * text
//                           w, h          - размеры кнопки в пикселях
//                           is_repeating  - флаг автоповтора при удержании
//                           is_long_press - флаг обработки долгого нажатия
//                           text          - текст кнопки ('|' - разделитель двух строк)
//                           Примечание: is_repeating и is_long_press не должны
//                           быть включены одновременно (проверка в objects_state).
//
// TYPE_TEXT_FIELD (tf_):    uint32_t w_sim, uint32_t h_str, uint32_t direction, void * font
//                           w_sim     - ширина поля в символах
//                           h_str     - число строк поля
//                           direction - направление прокрутки (значения tf_direction_t: UP/DOWN)
//                           font      - СЧИТЫВАЕТСЯ из varargs, но в SDL2-сборке ИГНОРИРУЕТСЯ
//                                       (всегда используется общий моноширинный шрифт меток).
//                                       Передавать ОБЯЗАТЕЛЬНО (например NULL), иначе va_arg
//                                       прочитает непереданный аргумент = неопределённое поведение.
//
// TYPE_TOUCH_AREA (ta_):    int x, int y, int w, int h, int is_trackable
//                           x, y, w, h   - геометрия области в координатах окна
//                           is_trackable - флаг возврата относительных координат перемещения
//
// TYPE_SLIDER (sl_):        int orientation, int size, int step
//                           orientation - ориентация шкалы (значения orientation_t:
//                                         ORIENTATION_VERTICAL / ORIENTATION_HORIZONTAL)
//                           size        - длина шкалы в пикселях
//                           step        - шаг изменения значения (если 0, принудительно ставится 1)
//
// TYPE_SWITCH (sw_):        int w, int h, char * caption
//                           w, h - размеры переключателя в пикселях
//                           caption - подпись переключателя
uint8_t gui_obj_create(const char * name, ...)
{
	uint8_t idx, window_id = get_parent_window();
	window_t * win = get_win(window_id);
	va_list arg;
	va_start(arg, name);

	char obj_name[NAME_ARRAY_SIZE] = { 0 };
	snprintf(obj_name, NAME_ARRAY_SIZE, "%s#%02d", name, win->window_id);
	obj_type_t type = parse_obj_name(obj_name);

	switch (type)
	{
	case TYPE_SWITCH:
	{
		switch_t * sw_tmp = (switch_t *) realloc(win->sw_ptr, sizeof(switch_t) * (win->sw_count + 1));
		GUI_MEM_ASSERT(sw_tmp);
		win->sw_ptr = sw_tmp;

		switch_t * sw = & win->sw_ptr[win->sw_count];
		memcpy(sw, & switch_default, sizeof(switch_t));

		sw->parent = window_id;
		sw->sw_w = va_arg(arg, int);
		sw->sw_h = va_arg(arg, int);
		strncpy(sw->text, va_arg(arg, char *), TEXT_ARRAY_SIZE - 1);
		sw->text[TEXT_ARRAY_SIZE - 1] = '\0';
		strncpy(sw->name, obj_name, NAME_ARRAY_SIZE);
		sw->visible = 1;
		sw->index = win->sw_count;
		sw->caption_align = SWITCH_CAPTION_LEFT;
		sw->x = 0;
		sw->y = 0;
		sw->font = gui_sdl2_get_label_font();
		sw->payload = 0;

		gui_anim_set(& sw->anim, sw->payload ? 100 : 0);
		switch_update_layout(sw);

		idx = win->sw_count;
		win->sw_count ++;
		break;
	}
	case TYPE_CANVAS:
	{
		canvas_t * ca_tmp = (canvas_t *) realloc(win->ca_ptr, sizeof(canvas_t) * (win->ca_count + 1));
		GUI_MEM_ASSERT(ca_tmp);
		win->ca_ptr = ca_tmp;

		canvas_t * ca = & win->ca_ptr[win->ca_count];
		memset(ca, 0, sizeof(canvas_t));

		ca->parent = window_id;
		ca->w = va_arg(arg, int);
		ca->h = va_arg(arg, int);
		ca->color = (GUI_COLOR_DARKGRAY & 0x00FFFFFF) | ((uint32_t) DEFAULT_ALPHA << 24);
		ca->visible = 1;
		ca->border = 0;
		ca->background = 0;
		ca->index = win->ca_count;
		ca->x = 0;
		ca->y = 0;
		strncpy(ca->name, obj_name, NAME_ARRAY_SIZE);

		idx = win->ca_count;
		win->ca_count ++;
		break;
	}

	case TYPE_LABEL:
	{
		label_t * lh_tmp = (label_t *) realloc(win->lh_ptr, sizeof(label_t) * (win->lh_count + 1));   // 1.2
		GUI_MEM_ASSERT(lh_tmp);
		win->lh_ptr = lh_tmp;

		label_t * lh = &win->lh_ptr[win->lh_count];
		memcpy(lh, &label_default, sizeof(label_t));

		lh->parent = window_id;
		lh->color = va_arg(arg, gui_color_t);
		lh->visible = 1;
		lh->index = win->lh_count;
		lh->x = 0;
		lh->y = 0;
		strncpy(lh->name, obj_name, NAME_ARRAY_SIZE);
		lh->width = va_arg(arg, uint32_t);
		lh->font_size = gui_sizes.labels_font_size;
		lh->bbox_align = ALIGNMENT_LEFT;
		lh->font_owned = 0;

		lh->font = gui_sdl2_get_label_font();
		memset(lh->text, '0', lh->width);		// для расчёта bbox
		lh->text[lh->width] = '\0';
		TTF_SizeText(lh->font, lh->text, &lh->bbox_w, &lh->bbox_h);
		lh->width_text_pix = lh->bbox_w;
		lh->baseline = TTF_FontAscent(lh->font);

		idx = win->lh_count;
		win->lh_count++;
		break;
	}

	case TYPE_BUTTON:
	{
		button_t * bh_tmp = (button_t *) realloc(win->bh_ptr, sizeof(button_t) * (win->bh_count + 1));   // 1.2
		GUI_MEM_ASSERT(bh_tmp);
		win->bh_ptr = bh_tmp;

		button_t * bh = & win->bh_ptr[win->bh_count];
		memcpy(bh, & button_default, sizeof(button_t));
		bh->parent = window_id;
		bh->w = va_arg(arg, int);
		bh->h = va_arg(arg, int);
		bh->is_repeating = va_arg(arg, uint32_t);
		bh->is_long_press = va_arg(arg, uint32_t);
		strncpy(bh->name, obj_name, NAME_ARRAY_SIZE);
		strncpy(bh->text, va_arg(arg, char *), TEXT_ARRAY_SIZE - 1);
		bh->visible = 1;
		bh->index = win->bh_count;
		bh->x1 = 0;
		bh->y1 = 0;
		bh->font = gui_sdl2_get_button_font();
		button_update_text_metrics(bh);			// кэш метрик текста с момента создания

		idx = win->bh_count;
		win->bh_count ++;
		break;
	}

	case TYPE_TEXT_FIELD:
	{
		text_field_t * tf_tmp = (text_field_t *) realloc(win->tf_ptr, sizeof(text_field_t) * (win->tf_count + 1));   // 1.2
		GUI_MEM_ASSERT(tf_tmp);
		win->tf_ptr = tf_tmp;

		text_field_t * tf = & win->tf_ptr[win->tf_count];
		memcpy(tf, & tf_default, sizeof(text_field_t));
		tf->parent = window_id;
		tf->w_sim = va_arg(arg, uint32_t);
		tf->h_str = va_arg(arg, uint32_t);
		tf->direction = (tf_direction_t) va_arg(arg, uint32_t);

		void * passed_font = va_arg(arg, void *);	// Убрать
		tf->font = gui_sdl2_get_label_font(); 		// Переиспользуем моноширинный шрифт меток

		strncpy(tf->name, obj_name, NAME_ARRAY_SIZE);
		tf->visible = 1;
		tf->index = win->tf_count;
		tf->x1 = 0;
		tf->y1 = 0;
		tf->string = (tf_entry_t *) calloc(tf->h_str, sizeof(tf_entry_t));
		GUI_MEM_ASSERT(tf->string);
		tf->index = 0;
		textfield_update_size(tf);
		idx = win->tf_count;
		win->tf_count ++;
		break;
	}

	case TYPE_TOUCH_AREA:
	{
		touch_area_t * ta_tmp = (touch_area_t *) realloc(win->ta_ptr, sizeof(touch_area_t) * (win->ta_count + 1));   // 1.2
		GUI_MEM_ASSERT(ta_tmp);
		win->ta_ptr = ta_tmp;

		touch_area_t * ta = & win->ta_ptr[win->ta_count];
		memcpy(ta, & ta_default, sizeof(touch_area_t));

		ta->parent = window_id;
		ta->x1 = va_arg(arg, int);
		ta->y1 = va_arg(arg, int);
		ta->w = va_arg(arg, int);
		ta->h = va_arg(arg, int);
		ta->is_trackable = va_arg(arg, int);
		strncpy(ta->name, obj_name, NAME_ARRAY_SIZE);
		ta->visible = 1;
		ta->index = win->ta_count;

		idx = win->ta_count;
		win->ta_count ++;
		break;
	}

	case TYPE_SLIDER:
	{
		slider_t * sh_tmp = (slider_t *) realloc(win->sh_ptr, sizeof(slider_t) * (win->sh_count + 1));   // 1.2
		GUI_MEM_ASSERT(sh_tmp);
		win->sh_ptr = sh_tmp;

		slider_t * sh = & win->sh_ptr[win->sh_count];
		memset(sh, 0, sizeof(slider_t));

		sh->parent = window_id;
		sh->orientation = va_arg(arg, int);
		strncpy(sh->name, obj_name, NAME_ARRAY_SIZE);
		sh->state = CANCELLED;
		sh->visible = 1;
		sh->size = va_arg(arg, int);
		sh->step = va_arg(arg, int);
		if (sh->step == 0) sh->step = 1;
		sh->value = 0;
		sh->value_old = 255;
		sh->index = win->sh_count;

		if (sh->orientation)	// ORIENTATION_HORIZONTAL
		{
			sh->width = sh->size;
			sh->height = gui_sizes.sliders_h * 2;
			sh->scale_x = gui_sizes.sliders_w;
			sh->scale_y = gui_sizes.sliders_h - gui_sizes.sliders_scale_thickness / 2;
			sh->scale_size = sh->size - gui_sizes.sliders_w * 2;
		}
		else					// ORIENTATION_VERTICAL
		{
			sh->width = gui_sizes.sliders_h * 2;
			sh->height = sh->size;
			sh->scale_x = gui_sizes.sliders_h - gui_sizes.sliders_scale_thickness / 2;
			sh->scale_y = gui_sizes.sliders_w;
			sh->scale_size = sh->size - gui_sizes.sliders_h * 2;
		}

		idx = win->sh_count;
		win->sh_count ++;
		break;
	}

	default:
		idx = 0;
		break;
	}

	va_end(arg);
	return idx;
}

char * gui_obj_get_string_prop(const char * name, object_prop_t prop)
{
	window_t * win = get_win(get_parent_window());
	obj_type_t type = parse_obj_name(name);
	void * obj = find_gui_obj(type, win, name);

	switch(type)
	{
	case TYPE_LABEL:
		label_t * lh = (label_t *) obj;
		if (prop == GUI_OBJ_TEXT) return lh->text;
		break;

	case TYPE_BUTTON:
		button_t * bh = (button_t *) obj;
		if (prop == GUI_OBJ_TEXT) return bh->text;
		break;

	case TYPE_SWITCH:
		switch_t * sw = (switch_t *) obj;
		if (prop == GUI_OBJ_TEXT) return sw->text;
		break;

	default:
		break;
	}

	return NULL;
}

int gui_obj_get_int_prop(const char * name, object_prop_t prop)
{
	window_t * win = get_win(get_parent_window());
	obj_type_t type = parse_obj_name(name);
	void * obj = find_gui_obj(type, win, name);
	switch(type)
	{
	case TYPE_SWITCH:
	{
		switch_t * sw = (switch_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) return sw->visible;
		else if (prop == GUI_OBJ_POS_X) return sw->x;
		else if (prop == GUI_OBJ_POS_Y) return sw->y;
		else if (prop == GUI_OBJ_PAYLOAD) return sw->payload;
		else if (prop == GUI_OBJ_STATE) return sw->state;
		else if (prop == GUI_OBJ_COLOR) return (int) sw->color_on;
		else if (prop == GUI_OBJ_WIDTH) return sw->w;
		else if (prop == GUI_OBJ_HEIGHT) return sw->h;
		else if (prop == GUI_OBJ_INDEX) return sw->index;
		else if (prop == GUI_OBJ_ALIGN) return sw->caption_align;
		break;
	}

	case TYPE_LABEL:
	{
		label_t * lh = (label_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) return lh->visible;
		else if (prop == GUI_OBJ_POS_X) return lh->x;
		else if (prop == GUI_OBJ_POS_Y) return lh->y;
		else if (prop == GUI_OBJ_PAYLOAD) return lh->payload;
		else if (prop == GUI_OBJ_STATE) return lh->state;
		else if (prop == GUI_OBJ_ALIGN) return lh->bbox_align;
		else if (prop == GUI_OBJ_COLOR) return (int) lh->color;
		else if (prop == GUI_OBJ_WIDTH) return lh->bbox_w;
		else if (prop == GUI_OBJ_HEIGHT) return lh->bbox_h;
		else if (prop == GUI_OBJ_INDEX) return lh->index;
		break;
	}

	case TYPE_BUTTON:
	{
		button_t * bh = (button_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) return bh->visible;
		else if (prop == GUI_OBJ_POS_X) return bh->x1;
		else if (prop == GUI_OBJ_POS_Y) return bh->y1;
		else if (prop == GUI_OBJ_PAYLOAD) return bh->payload;
		else if (prop == GUI_OBJ_STATE) return bh->state;
		else if (prop == GUI_OBJ_LOCK) return bh->is_locked;
		else if (prop == GUI_OBJ_REPEAT) return bh->is_repeating;
		else if (prop == GUI_OBJ_LONG_PRESS) return bh->is_long_press;
		else if (prop == GUI_OBJ_WIDTH) return bh->w;
		else if (prop == GUI_OBJ_HEIGHT) return bh->h;
		else if (prop == GUI_OBJ_INDEX) return bh->index;
		break;
	}

	case TYPE_SLIDER:
	{
		slider_t * sh = (slider_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) return sh->visible;
		else if (prop == GUI_OBJ_POS_X) return sh->x;
		else if (prop == GUI_OBJ_POS_Y) return sh->y;
		else if (prop == GUI_OBJ_STATE) return sh->state;
		else if (prop == GUI_OBJ_WIDTH) return sh->width;
		else if (prop == GUI_OBJ_HEIGHT) return sh->height;
		else if (prop == GUI_OBJ_PAYLOAD) return sh->value;
		else if (prop == GUI_OBJ_SIZE) return sh->size;
		else if (prop == GUI_OBJ_INDEX) return sh->index;
		break;
	}

	case TYPE_TOUCH_AREA:
	{
		touch_area_t * ta = (touch_area_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) return ta->visible;
		else if (prop == GUI_OBJ_POS_X) return ta->x1;
		else if (prop == GUI_OBJ_POS_Y) return ta->y1;
		else if (prop == GUI_OBJ_PAYLOAD) return ta->payload;
		else if (prop == GUI_OBJ_STATE) return ta->state;
		else if (prop == GUI_OBJ_WIDTH) return ta->w;
		else if (prop == GUI_OBJ_HEIGHT) return ta->h;
		else if (prop == GUI_OBJ_INDEX) return ta->index;
		break;
	}

	case TYPE_TEXT_FIELD:
	{
		text_field_t * tf = (text_field_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) return tf->visible;
		else if (prop == GUI_OBJ_POS_X) return tf->x1;
		else if (prop == GUI_OBJ_POS_Y) return tf->y1;
		else if (prop == GUI_OBJ_STATE) return tf->state;
		else if (prop == GUI_OBJ_WIDTH) return tf->w;
		else if (prop == GUI_OBJ_HEIGHT) return tf->h;
		else if (prop == GUI_OBJ_INDEX) return tf->index;
		break;
	}

	case TYPE_CANVAS:
	{
		canvas_t * ca = (canvas_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) return ca->visible;
		else if (prop == GUI_OBJ_POS_X) return ca->x;
		else if (prop == GUI_OBJ_POS_Y) return ca->y;
		else if (prop == GUI_OBJ_COLOR) return (int) ca->color;
		else if (prop == GUI_OBJ_WIDTH) return ca->w;
		else if (prop == GUI_OBJ_HEIGHT) return ca->h;
		else if (prop == GUI_OBJ_STATE) return ca->state;
		else if (prop == GUI_OBJ_BORDER) return ca->border;
		else if (prop == GUI_OBJ_INDEX) return ca->index;
		break;
	}

	default:
		break;
	}
	return 0;
}

// Вариабельные аргументы зависят одновременно от ТИПА объекта (префикс имени 'name')
// и от значения 'prop'. Ниже для каждого типа объекта перечислены prop, требующие
// аргументов, и сами аргументы в порядке извлечения va_arg().
// prop, не обрабатываемые для данного типа, просто игнорируются (va_arg не вызывается).
//
// ===================== TYPE_LABEL (lbl_) =====================
// GUI_OBJ_VISIBLE:          int visible                 (0/1)
// GUI_OBJ_POS_X:            int x
// GUI_OBJ_POS_Y:            int y
// GUI_OBJ_POS:              int x, int y                (две координаты за один вызов)
// GUI_OBJ_PAYLOAD:          int payload
// GUI_OBJ_TEXT:             char * text                 (копируется; кэш текста инвалидируется)
// GUI_OBJ_TEXT_FMT:         char * format, ...          (format + аргументы форматирования,
//                                                        потребляемые vsnprintf из того же va_list)
// GUI_OBJ_STATE:            int state
// GUI_OBJ_ALIGN:            int align                   (значения align_t)
// GUI_OBJ_COLOR:            gui_color_t color
// GUI_OBJ_FONT:             char * path, int font_size  (ДВА аргумента: путь к TTF-шрифту и размер;
//                                                        старый динамический шрифт закрывается)
//   После TEXT/TEXT_FMT/FONT пересчитываются метрики текста (width_text_pix/bbox_h/baseline);
//   bbox_w пересчитывается только при GUI_OBJ_FONT.
//
// ===================== TYPE_BUTTON (btn_) =====================
// GUI_OBJ_VISIBLE:          int visible                 (0/1)
// GUI_OBJ_POS_X:            int x1
// GUI_OBJ_POS_Y:            int y1
// GUI_OBJ_POS:              int x1, int y1
// GUI_OBJ_PAYLOAD:          int payload
// GUI_OBJ_TEXT:             char * text
// GUI_OBJ_TEXT_FMT:         char * format, ...          (format + аргументы форматирования для vsnprintf)
// GUI_OBJ_STATE:            int state
// GUI_OBJ_LOCK:             int is_locked               (0/1)
// GUI_OBJ_WIDTH:            int w
// GUI_OBJ_HEIGHT:           int h
// GUI_OBJ_SIZE:             int w, int h                (два размера за один вызов)
// GUI_OBJ_REPEAT:           int is_repeating            (0/1)
// GUI_OBJ_LONG_PRESS:       int is_long_press           (0/1)
// GUI_OBJ_FONT:             TTF_Font * font
//
// ===================== TYPE_SLIDER (sl_) =====================
// GUI_OBJ_VISIBLE:          int visible                 (0/1)
// GUI_OBJ_POS_X:            int x
// GUI_OBJ_POS_Y:            int y
// GUI_OBJ_POS:              int x, int y
// GUI_OBJ_PAYLOAD:          int value                   (0..100 %)
// GUI_OBJ_STATE:            int state
// GUI_OBJ_SIZE:             int size                    (длина шкалы в пикселях)
//
// ===================== TYPE_TOUCH_AREA (ta_) =====================
// GUI_OBJ_VISIBLE:          int visible                 (0/1)
// GUI_OBJ_POS_X:            int x1
// GUI_OBJ_POS_Y:            int y1
// GUI_OBJ_POS:              int x1, int y1
// GUI_OBJ_PAYLOAD:          int payload
// GUI_OBJ_STATE:            int state
// GUI_OBJ_WIDTH:            int w
// GUI_OBJ_HEIGHT:           int h
// GUI_OBJ_SIZE:             int w, int h
//
// ===================== TYPE_TEXT_FIELD (tf_) =====================
// GUI_OBJ_VISIBLE:          int visible                 (0/1)
// GUI_OBJ_POS_X:            int x1
// GUI_OBJ_POS_Y:            int y1
// GUI_OBJ_POS:              int x1, int y1
// GUI_OBJ_STATE:            int state
// GUI_OBJ_TEXT:             char * text, int color_line
// GUI_OBJ_TEXT_FMT:         char * format, ..., int color_line
// GUI_OBJ_CLEAN:            (нет вариабельных аргументов; очищает поле и сбрасывает индекс)
//
// ===================== TYPE_SWITCH (sw_) =====================
// GUI_OBJ_VISIBLE:          int visible                 (0/1)
// GUI_OBJ_POS_X:            int x                       (левый верх полного габарита)
// GUI_OBJ_POS_Y:            int y
// GUI_OBJ_POS:              int x, int y
// GUI_OBJ_PAYLOAD:          int on                      (0/1 - состояние переключателя)
// GUI_OBJ_STATE:            int state
// GUI_OBJ_COLOR:            gui_color_t color_on        (заливка во включенном состоянии)
// GUI_OBJ_TEXT:             char * text                 (подпись; полный габарит пересчитывается)
// GUI_OBJ_TEXT_FMT:         char * format, ...          (подпись с форматированием)
// GUI_OBJ_ALIGN:            int caption_align           (значения switch_caption_t; пересчёт)
// GUI_OBJ_WIDTH:            int w                       (ширина ТЕЛА; полный габарит пересчитывается)
// GUI_OBJ_HEIGHT:           int h                       (высота ТЕЛА; полный габарит пересчитывается)
// GUI_OBJ_SIZE:             int w, int h                (размеры ТЕЛА; полный габарит пересчитывается)
void gui_obj_set_prop(const char * name, object_prop_t prop, ...)
{
	window_t * win = get_win(get_parent_window());
	obj_type_t type = parse_obj_name(name);
	void * obj = find_gui_obj(type, win, name);
	uint8_t flag = 0;
	va_list arg;
	va_start(arg, prop);
	switch(type)
	{
	case TYPE_SWITCH:
	{
		switch_t * sw = (switch_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) sw->visible = !! va_arg(arg, int);
		else if (prop == GUI_OBJ_POS_X) sw->x = va_arg(arg, int);
		else if (prop == GUI_OBJ_POS_Y) sw->y = va_arg(arg, int);
		else if (prop == GUI_OBJ_POS) { sw->x = va_arg(arg, int); sw->y = va_arg(arg, int); }
		else if (prop == GUI_OBJ_PAYLOAD) { switch_set_payload(sw, va_arg(arg, int), 0); }
		else if (prop == GUI_OBJ_STATE) sw->state = va_arg(arg, int);
		else if (prop == GUI_OBJ_COLOR) sw->color_on = va_arg(arg, gui_color_t);
		else if (prop == GUI_OBJ_TEXT) {
			strncpy(sw->text, va_arg(arg, char *), TEXT_ARRAY_SIZE - 1);
			sw->text[TEXT_ARRAY_SIZE - 1] = '\0';
			switch_update_layout(sw);
		}
		else if (prop == GUI_OBJ_TEXT_FMT) {
			vsnprintf(sw->text, TEXT_ARRAY_SIZE - 1, va_arg(arg, char *), arg);
			sw->text[TEXT_ARRAY_SIZE - 1] = '\0';
			switch_update_layout(sw);
		}
		else if (prop == GUI_OBJ_ALIGN) {
			sw->caption_align = (switch_caption_t) va_arg(arg, int);
			switch_update_layout(sw);
		}
		else if (prop == GUI_OBJ_WIDTH) { sw->sw_w = va_arg(arg, int); switch_update_layout(sw); }
		else if (prop == GUI_OBJ_HEIGHT) { sw->sw_h = va_arg(arg, int); switch_update_layout(sw); }
		else if (prop == GUI_OBJ_SIZE) { sw->sw_w = va_arg(arg, int); sw->sw_h = va_arg(arg, int); switch_update_layout(sw); }
		break;
	}
	case TYPE_LABEL:
	{
		label_t * lh = (label_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) lh->visible = !! va_arg(arg, int);
		else if (prop == GUI_OBJ_POS_X) lh->x = va_arg(arg, int);
		else if (prop == GUI_OBJ_POS_Y) lh->y = va_arg(arg, int);
		else if (prop == GUI_OBJ_POS) { lh->x = va_arg(arg, int); lh->y = va_arg(arg, int); }
		else if (prop == GUI_OBJ_PAYLOAD) lh->payload = va_arg(arg, int);
        else if (prop == GUI_OBJ_TEXT || prop == GUI_OBJ_TEXT_FMT) {
            char tmp[TEXT_ARRAY_SIZE];
            if (prop == GUI_OBJ_TEXT) {
                const char * s = va_arg(arg, char *);
                strncpy(tmp, s, TEXT_ARRAY_SIZE - 1);
                tmp[TEXT_ARRAY_SIZE - 1] = '\0';
            } else {
                vsnprintf(tmp, TEXT_ARRAY_SIZE - 1, va_arg(arg, char *), arg);
                tmp[TEXT_ARRAY_SIZE - 1] = '\0';
            }

            // Сравниваем с сырым текстом (который может содержать маркеры)
            if (strcmp(tmp, lh->text) != 0) {
                // Инвалидируем кэш для СТАРОГО текста
                gui_sdl2_invalidate_text(lh->text, lh->font);

                // Сохраняем новый текст (с маркерами)
                strcpy(lh->text, tmp);
                flag = 1;
            }
        }
		else if (prop == GUI_OBJ_STATE) lh->state = va_arg(arg, int);
		else if (prop == GUI_OBJ_ALIGN) lh->bbox_align = va_arg(arg, int);
		else if (prop == GUI_OBJ_COLOR) lh->color = va_arg(arg, gui_color_t);
		else if (prop == GUI_OBJ_FONT) { flag = 1;
			gui_sdl2_invalidate_text(lh->text, lh->font);
			if (lh->font_owned && lh->font) {
				TTF_CloseFont(lh->font);
				lh->font = NULL;
			}
			char * path = va_arg(arg, char *);
			lh->font_size = va_arg(arg, int);
			lh->font = TTF_OpenFont(path, lh->font_size);
			lh->font_owned = 1;
			if (!lh->font) {
				printf("[GUI] Failed to open font %s size %d: %s\n", path, lh->font_size, TTF_GetError());
				lh->font = gui_sdl2_get_label_font();
				lh->font_owned = 0;
			}
		}
        if (flag)
        {
            int w, h;
            gui_sdl2_get_text_size_colored(lh->text, lh->font, &w, &h);

            lh->width_text_pix = w;
            lh->baseline = TTF_FontAscent(lh->font);
            lh->bbox_h = h;

            // bbox_w (максимальная ширина поля) пересчитываем только при смене шрифта
            if (prop == GUI_OBJ_FONT) {
                char widest[TEXT_ARRAY_SIZE];
                memset(widest, '0', lh->width);
                widest[lh->width] = '\0';
                int ww, wh;
                TTF_SizeText(lh->font, widest, &ww, &wh);
                lh->bbox_w = ww;
            }
        }
		break;
	}
	case TYPE_BUTTON:
	{
		button_t * bh = (button_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) bh->visible = !! va_arg(arg, int);
		else if (prop == GUI_OBJ_POS_X) bh->x1 = va_arg(arg, int);
		else if (prop == GUI_OBJ_POS_Y) bh->y1 = va_arg(arg, int);
		else if (prop == GUI_OBJ_POS) { bh->x1 = va_arg(arg, int); bh->y1 = va_arg(arg, int); }
		else if (prop == GUI_OBJ_PAYLOAD) bh->payload = va_arg(arg, int);
		else if (prop == GUI_OBJ_TEXT) {
			strncpy(bh->text, va_arg(arg, char *), TEXT_ARRAY_SIZE - 1);
			bh->text[TEXT_ARRAY_SIZE - 1] = '\0';
			button_update_text_metrics(bh);
		}
		else if (prop == GUI_OBJ_TEXT_FMT) {
			vsnprintf(bh->text, TEXT_ARRAY_SIZE - 1, va_arg(arg, char *), arg);
			bh->text[TEXT_ARRAY_SIZE - 1] = '\0';
			button_update_text_metrics(bh);
		}
		else if (prop == GUI_OBJ_STATE) bh->state = va_arg(arg, int);
		else if (prop == GUI_OBJ_LOCK) bh->is_locked = !! va_arg(arg, int);
		else if (prop == GUI_OBJ_WIDTH) bh->w = va_arg(arg, int);
		else if (prop == GUI_OBJ_HEIGHT) bh->h = va_arg(arg, int);
		else if (prop == GUI_OBJ_SIZE) { bh->w = va_arg(arg, int); bh->h = va_arg(arg, int); }
		else if (prop == GUI_OBJ_REPEAT) bh->is_repeating = !! va_arg(arg, int);
		else if (prop == GUI_OBJ_LONG_PRESS) bh->is_long_press = !! va_arg(arg, int);
		else if (prop == GUI_OBJ_FONT) {
			bh->font = va_arg(arg, TTF_Font *);
			button_update_text_metrics(bh);
		}
		break;
	}
	case TYPE_SLIDER:
	{
		slider_t * sh = (slider_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) sh->visible = !! va_arg(arg, int);
		else if (prop == GUI_OBJ_POS_X) sh->x = va_arg(arg, int);
		else if (prop == GUI_OBJ_POS_Y) sh->y = va_arg(arg, int);
		else if (prop == GUI_OBJ_POS) { sh->x = va_arg(arg, int); sh->y = va_arg(arg, int); }
		else if (prop == GUI_OBJ_PAYLOAD) sh->value = va_arg(arg, int);
		else if (prop == GUI_OBJ_STATE) sh->state = va_arg(arg, int);
		else if (prop == GUI_OBJ_SIZE) sh->size = va_arg(arg, int);
		break;
	}
	case TYPE_TOUCH_AREA:
	{
		touch_area_t * ta = (touch_area_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) ta->visible = !! va_arg(arg, int);
		else if (prop == GUI_OBJ_POS_X) ta->x1 = va_arg(arg, int);
		else if (prop == GUI_OBJ_POS_Y) ta->y1 = va_arg(arg, int);
		else if (prop == GUI_OBJ_POS) { ta->x1 = va_arg(arg, int); ta->y1 = va_arg(arg, int); }
		else if (prop == GUI_OBJ_PAYLOAD) ta->payload = va_arg(arg, int);
		else if (prop == GUI_OBJ_STATE) ta->state = va_arg(arg, int);
		else if (prop == GUI_OBJ_WIDTH) ta->w = va_arg(arg, int);
		else if (prop == GUI_OBJ_HEIGHT) ta->h = va_arg(arg, int);
		else if (prop == GUI_OBJ_SIZE) { ta->w = va_arg(arg, int); ta->h = va_arg(arg, int); }
		break;
	}
	case TYPE_TEXT_FIELD:
	{
		text_field_t * tf = (text_field_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) tf->visible = !! va_arg(arg, int);
		else if (prop == GUI_OBJ_POS_X) tf->x1 = va_arg(arg, int);
		else if (prop == GUI_OBJ_POS_Y) tf->y1 = va_arg(arg, int);
		else if (prop == GUI_OBJ_POS) { tf->x1 = va_arg(arg, int); tf->y1 = va_arg(arg, int); }
		else if (prop == GUI_OBJ_STATE) tf->state = va_arg(arg, int);
		else if (prop == GUI_OBJ_TEXT) {
			tf_entry_t * rec = &  tf->string[tf->index];
			strncpy(rec->text, va_arg(arg, char *), TEXT_ARRAY_SIZE - 1);
			rec->color_line = va_arg(arg, int);
			tf->index ++;
			tf->index = tf->index >= tf->h_str ? 0 : tf->index;
		}
		else if (prop == GUI_OBJ_TEXT_FMT) {
			tf_entry_t * rec = &  tf->string[tf->index];
			vsnprintf(rec->text, TEXT_ARRAY_SIZE - 1, va_arg(arg, char *), arg);
			rec->color_line = va_arg(arg, int);
			tf->index ++;
			tf->index = tf->index >= tf->h_str ? 0 : tf->index;
		}
		else if (prop == GUI_OBJ_CLEAN) {
			tf->index = 0;
			memset(tf->string, 0, tf->h_str * sizeof(tf_entry_t));
		}
		break;
	}
	case TYPE_CANVAS:
	{
		canvas_t * ca = (canvas_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) ca->visible = !! va_arg(arg, int);
		else if (prop == GUI_OBJ_POS_X) ca->x = va_arg(arg, int);
		else if (prop == GUI_OBJ_POS_Y) ca->y = va_arg(arg, int);
		else if (prop == GUI_OBJ_POS) { ca->x = va_arg(arg, int); ca->y = va_arg(arg, int); }
		else if (prop == GUI_OBJ_COLOR) ca->color = va_arg(arg, gui_color_t);
		else if (prop == GUI_OBJ_WIDTH) ca->w = va_arg(arg, int);
		else if (prop == GUI_OBJ_HEIGHT) ca->h = va_arg(arg, int);
		else if (prop == GUI_OBJ_SIZE) { ca->w = va_arg(arg, int); ca->h = va_arg(arg, int); }
		else if (prop == GUI_OBJ_STATE) ca->state = va_arg(arg, int);
		else if (prop == GUI_OBJ_BORDER) ca->border = !! va_arg(arg, int);
		else if (prop == GUI_OBJ_BACK) ca->background = !! va_arg(arg, int);
		break;
	}
	default:
		break;
	}
	va_end(arg);
}

uint8_t gui_check_obj(const char * name1, const char * name2)
{
	return strcmp(name1, name2) == 0;
}

static uint8_t get_obj_idx_by_name(window_t * win, obj_type_t type, const char * name)
{
	void * p = find_gui_obj(type, win, name);
	if (type == TYPE_BUTTON)
		return ((button_t *) p)->index;
	else if (type == TYPE_LABEL)
		return ((label_t *) p)->index;
	else if (type == TYPE_SLIDER)
		return ((slider_t *) p)->index;
	else if (type == TYPE_CANVAS)
		return ((canvas_t *) p)->index;
	else if (type == TYPE_SWITCH)
		return ((switch_t *) p)->index;
	GUI_ASSERT(0);
	return 0;
}

/* Применение выравнивания к объекту относительно прямоугольника (x2, y2, w2, h2) */
static void obj_apply_align(obj_type_t type, void * oh, object_alignment_t align, uint16_t offset,
	uint16_t x2, uint16_t y2, uint16_t w2, uint16_t h2, uint16_t baseline2)
{
	switch (type)
	{
	case TYPE_LABEL:
	{
		label_t * lh = (label_t *) oh;
		if (align == ALIGN_RIGHT_UP) { lh->x = x2 + w2 + offset; lh->y = y2; }
		else if (align == ALIGN_RIGHT_UP_MID) { lh->x = x2 + w2 + offset; lh->y = y2 + (h2 / 2 - get_label_height(lh) / 2); }
		else if (align == ALIGN_LEFT_UP)  { lh->x = x2 - get_label_width(lh) - offset; lh->y = y2; }
		else if (align == ALIGN_DOWN_LEFT) { lh->x = x2; lh->y = y2 + h2 + offset; }
		else if (align == ALIGN_DOWN_MID) { lh->x = x2 + w2 / 2 - get_label_width(lh) / 2; lh->y = y2 + h2 + offset; }
		else if (align == ALIGN_DOWN_RIGHT) { lh->x = x2 + w2 - get_label_width(lh); lh->y = y2 + h2 + offset; }
		else if (align == ALIGN_LEFT_TOP) { lh->x = x2; lh->y = y2 - get_label_height(lh) - offset; }
		else if (align == ALIGN_RIGHT_DOWN) { lh->x = x2 + w2 + offset; lh->y = y2 + baseline2 - lh->baseline; }
		break;
	}
	case TYPE_BUTTON:
	{
		button_t * bh = (button_t *) oh;
		if (align == ALIGN_RIGHT_UP) { bh->x1 = x2 + w2 + offset; bh->y1 = y2; }
		else if (align == ALIGN_RIGHT_UP_MID) { bh->x1 = x2 + w2 + offset; bh->y1 = y2 + (h2 / 2 - bh->h / 2); }
		else if (align == ALIGN_LEFT_UP)  { bh->x1 = x2 - bh->w - offset; bh->y1 = y2; }
		else if (align == ALIGN_DOWN_LEFT) { bh->x1 = x2; bh->y1 = y2 + h2 + offset; }
		else if (align == ALIGN_DOWN_MID) { bh->x1 = x2 + w2 / 2 - bh->w / 2; bh->y1 = y2 + h2 + offset; }
		else if (align == ALIGN_DOWN_RIGHT) { bh->x1 = x2 + w2 - bh->w; bh->y1 = y2 + h2 + offset; }
		break;
	}
	case TYPE_SLIDER:
	{
		slider_t * sh = (slider_t *) oh;
		if (align == ALIGN_RIGHT_UP) { sh->x = x2 + w2 + offset; sh->y = y2; }
		else if (align == ALIGN_RIGHT_UP_MID) { sh->x = x2 + w2 + offset; sh->y = y2 + (h2 / 2 - sh->height / 2); }
		else if (align == ALIGN_LEFT_UP)  { sh->x = x2 - sh->width - offset; sh->y = y2; }
		else if (align == ALIGN_DOWN_LEFT) { sh->x = x2; sh->y = y2 + h2 + offset; }
		else if (align == ALIGN_DOWN_MID) { sh->x = x2 + w2 / 2 - sh->width / 2; sh->y = y2 + h2 + offset; }
		else if (align == ALIGN_DOWN_RIGHT) { sh->x = x2 + w2 - sh->width; sh->y = y2 + h2 + offset; }
		break;
	}
	case TYPE_CANVAS:
	{
		canvas_t * ca = (canvas_t *) oh;
		if (align == ALIGN_RIGHT_UP) { ca->x = x2 + w2 + offset; ca->y = y2; }
		else if (align == ALIGN_RIGHT_UP_MID) { ca->x = x2 + w2 + offset; ca->y = y2 + (h2 / 2 - ca->h / 2); }
		else if (align == ALIGN_LEFT_UP) { ca->x = x2 - ca->w - offset; ca->y = y2; }
		else if (align == ALIGN_DOWN_LEFT) { ca->x = x2; ca->y = y2 + h2 + offset; }
		else if (align == ALIGN_DOWN_MID) { ca->x = x2 + w2 / 2 - ca->w / 2; ca->y = y2 + h2 + offset; }
		else if (align == ALIGN_DOWN_RIGHT) { ca->x = x2 + w2 - ca->w; ca->y = y2 + h2 + offset; }
		break;
	}
	case TYPE_SWITCH:
	{
		switch_t * sw = (switch_t *) oh;
		if (align == ALIGN_RIGHT_UP) { sw->x = x2 + w2 + offset; sw->y = y2; }
		else if (align == ALIGN_RIGHT_UP_MID) { sw->x = x2 + w2 + offset; sw->y = y2 + (h2 / 2 - sw->h / 2); }
		else if (align == ALIGN_LEFT_UP)  { sw->x = x2 - sw->w - offset; sw->y = y2; }
		else if (align == ALIGN_DOWN_LEFT) { sw->x = x2; sw->y = y2 + h2 + offset; }
		else if (align == ALIGN_DOWN_MID) { sw->x = x2 + w2 / 2 - sw->w / 2; sw->y = y2 + h2 + offset; }
		else if (align == ALIGN_DOWN_RIGHT) { sw->x = x2 + w2 - sw->w; sw->y = y2 + h2 + offset; }
		break;
	}
	default:
		break;
	}
}

void gui_obj_align_to(const char * name1, const char * name2, object_alignment_t align, uint16_t offset)
{
	window_t * win = get_win(get_parent_window());
	obj_type_t type1 = parse_obj_name(name1);
	obj_type_t type2 = parse_obj_name(name2);
	void * oh1 = find_gui_obj(type1, win, name1);
	void * oh2 = find_gui_obj(type2, win, name2);
	uint16_t x2 = 0, y2 = 0, w2 = 0, h2 = 0, baseline2 = 0;

	if (oh1 == oh2) return;

	switch(type2)
	{
	case TYPE_SWITCH:
	{
		switch_t * sw2 = (switch_t *) oh2;
		x2 = sw2->x; y2 = sw2->y; w2 = sw2->w; h2 = sw2->h;
		break;
	}
	case TYPE_LABEL:
	{
		label_t * lh2 = (label_t *) oh2;
		x2 = lh2->x;
		y2 = lh2->y;
		w2 = get_label_width(lh2);
		h2 = get_label_height(lh2);
		baseline2 = lh2->baseline;
		break;
	}
	case TYPE_BUTTON:
	{
		button_t * bh2 = (button_t *) oh2;
		x2 = bh2->x1;
		y2 = bh2->y1;
		w2 = bh2->w;
		h2 = bh2->h;
		break;
	}
	case TYPE_TEXT_FIELD:
	{
		text_field_t * tf2 = (text_field_t *) oh2;
		x2 = tf2->x1;
		y2 = tf2->y1;
		w2 = tf2->w;
		h2 = tf2->h;
		break;
	}
	case TYPE_SLIDER:
	{
		slider_t * sh2 = (slider_t *) oh2;
		x2 = sh2->x;
		y2 = sh2->y;
		w2 = sh2->width;
		h2 = sh2->height;
		break;
	}
	case TYPE_CANVAS:
	{
		canvas_t * ca2 = (canvas_t *) oh2;
		x2 = ca2->x;
		y2 = ca2->y;
		w2 = ca2->w;
		h2 = ca2->h;
		break;
	}
	default:
		break;
	}

	obj_apply_align(type1, oh1, align, offset, x2, y2, w2, h2, baseline2);
}

void gui_arrange_objects_from(const char * name, uint8_t count, uint8_t cols, uint8_t interval)
{
	if (count <= 1) return;

	window_t * win = get_win(get_parent_window());
	obj_type_t type = parse_obj_name(name);

	if (type != TYPE_BUTTON && type != TYPE_LABEL && type != TYPE_SLIDER &&
	    type != TYPE_CANVAS && type != TYPE_SWITCH)
	{
		GUI_DEBUG_PRINT("%s: idx %d unsupported object type to arrange\n", __func__, 0);
		GUI_ASSERT(0);
	}

	uint16_t x = gui_obj_get_int_prop(name, GUI_OBJ_POS_X);
	uint16_t y = gui_obj_get_int_prop(name, GUI_OBJ_POS_Y);
	uint16_t w = gui_obj_get_int_prop(name, GUI_OBJ_WIDTH);
	uint16_t h = gui_obj_get_int_prop(name, GUI_OBJ_HEIGHT);
	uint8_t idx = get_obj_idx_by_name(win, type, name) + 1;

	for (int i = 1; i < count; i ++)
	{
		uint8_t row = i / cols;
		uint8_t col = i % cols;
		const char * obj = get_obj_name_by_idx(type, idx ++);
		gui_obj_set_prop(obj, GUI_OBJ_POS_X, x + (w + interval) * col);
		gui_obj_set_prop(obj, GUI_OBJ_POS_Y, y + (h + interval) * row);
	}

	save_arrange_area(win, x, y, w, h, count, cols, interval);
}

/* Выравнивание объекта относительно прямоугольной области последнего массового
   выравнивания (gui_arrange_objects / gui_arrange_objects_from) текущего окна.
   baseline2 для области принимается равным 0 (верхняя граница). */
void gui_obj_align_to_arrange_area(const char * name, object_alignment_t align, uint16_t offset)
{
	window_t * win = get_win(get_parent_window());
	if (! win->arrange_area_valid)
	{
		GUI_DEBUG_PRINT("%s: no arrange area saved in window '%s'\n", __func__, win->title);
		GUI_ASSERT(0);
	}
	obj_type_t type = parse_obj_name(name);
	void * oh = find_gui_obj(type, win, name);
	obj_apply_align(type, oh, align, offset,
		win->arrange_area.x, win->arrange_area.y, win->arrange_area.w, win->arrange_area.h, 0);
}

char * get_obj_name_by_idx(obj_type_t type, uint8_t idx)
{
	window_t * win = get_win(get_parent_window());
	static char obj_name[NAME_ARRAY_SIZE] = { 0 };
	if (type == TYPE_BUTTON)
	{
		GUI_ASSERT(idx < win->bh_count);
		strncpy(obj_name, win->bh_ptr[idx].name, NAME_ARRAY_SIZE);
		obj_name_user(obj_name);
		return obj_name;
	}
	else if (type == TYPE_LABEL)
	{
		GUI_ASSERT(idx < win->lh_count);
		strncpy(obj_name, win->lh_ptr[idx].name, NAME_ARRAY_SIZE);
		obj_name_user(obj_name);
		return obj_name;
	}
	else if (type == TYPE_SLIDER)
	{
		GUI_ASSERT(idx < win->sh_count);
		strncpy(obj_name, win->sh_ptr[idx].name, NAME_ARRAY_SIZE);
		obj_name_user(obj_name);
		return obj_name;
	}
	else if (type == TYPE_CANVAS)
	{
		GUI_ASSERT(idx < win->ca_count);
		strncpy(obj_name, win->ca_ptr[idx].name, NAME_ARRAY_SIZE);
		obj_name_user(obj_name);
		return obj_name;
	}
	else if (type == TYPE_SWITCH)
	{
		GUI_ASSERT(idx < win->sw_count);
		strncpy(obj_name, win->sw_ptr[idx].name, NAME_ARRAY_SIZE);
		obj_name_user(obj_name);
		return obj_name;
	}
	GUI_ASSERT(0);
	return NULL;
}

// выравнивание однотипных объектов (кнопка, метка, слайдер) с передачей имени первого объекта,
// обработка по возрастанию индекса (в порядке создания)
void gui_arrange_objects(const char names[][NAME_ARRAY_SIZE], uint8_t count, uint8_t cols, uint8_t interval)
{
	if (count <= 1) return;

	window_t * win = get_win(get_parent_window());
	obj_type_t type = parse_obj_name(names[0]);

	if (type != TYPE_BUTTON && type != TYPE_LABEL && type != TYPE_SLIDER &&
	    type != TYPE_CANVAS && type != TYPE_SWITCH)
	{
		GUI_DEBUG_PRINT("%s: idx %d unsupported object type to arrange\n", __func__, 0);
		GUI_ASSERT(0);
	}

	uint16_t x = gui_obj_get_int_prop(names[0], GUI_OBJ_POS_X);
	uint16_t y = gui_obj_get_int_prop(names[0], GUI_OBJ_POS_Y);
	uint16_t w = gui_obj_get_int_prop(names[0], GUI_OBJ_WIDTH);
	uint16_t h = gui_obj_get_int_prop(names[0], GUI_OBJ_HEIGHT);

	for (int i = 1; i < count; i ++)
	{
		uint8_t row = i / cols;
		uint8_t col = i % cols;
		const char * obj = names[i];
		obj_type_t typex = parse_obj_name(obj);
		if (typex != type)
		{
			GUI_DEBUG_PRINT("%s: idx %d - arrange various objects not supported\n", __func__, i);
			GUI_ASSERT(0);
		}
		gui_obj_set_prop(obj, GUI_OBJ_POS_X, x + (w + interval) * col);
		gui_obj_set_prop(obj, GUI_OBJ_POS_Y, y + (h + interval) * row);
	}

	save_arrange_area(win, x, y, w, h, count, cols, interval);
}

void gui_objects_init(void)
{

}

#endif /* SIMPLE_GUI */
