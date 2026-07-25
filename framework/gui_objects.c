// Simple GUI от RA4ASN

#include "gui_user_include.h"

#if WITHTOUCHGUI

#include "gui_includes.h"

const label_t label_default = 	{ 0, CANCELLED, 0, NON_VISIBLE, "", "", GUI_COLOR_WHITE, };
const button_t button_default = { 0, 0, CANCELLED, BUTTON_NON_LOCKED, 0, 1, 0, 0, NON_VISIBLE, INT32_MAX, "", "", };
const text_field_t tf_default = { 0, 0, CANCELLED, 0, NON_VISIBLE, UP, "", };
const touch_area_t ta_default = { 0, 0, 0, 0, 0, "", 0, 0, 0, 0, 0, };

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

	if	(lh->bbox_align == ALIGNMENT_CENTER)
		xx += (lh->bbox_w / 2) - (lh->width_text_pix / 2);
	else if	(lh->bbox_align == ALIGNMENT_RIGHT)
		xx += lh->bbox_w - lh->width_text_pix;

	gui_sdl2_draw_text(lh->text, xx, y, lh->font, lh->color);
}

// *************** Buttons ****************

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
	const uint16_t shiftX = bh->state == PRESSED ? 1 : 0;
	const uint16_t shiftY = bh->state == PRESSED ? 1 : 0;
	const gui_color_t textcolor = GUI_COLOR_BLACK;

    static const char delimeters[] = "|";
    if (strchr(bh->text, delimeters[0]) == NULL)
    {
        int tw, th;
        gui_sdl2_get_text_size(bh->text, bh->font, &tw, &th);
        gui_sdl2_draw_text(bh->text, shiftX + x + (bh->w - tw) / 2, shiftY + y + (bh->h - th) / 2, bh->font, textcolor);
    }
    else
    {
        char buf[TEXT_ARRAY_SIZE];
        strcpy(buf, bh->text);
        char *next;
        int line_h = TTF_FontHeight(bh->font);
        int total_h = line_h * 2;
        int y_start = shiftY + y + (bh->h - total_h) / 2;

        char *text2 = strtok_r(buf, delimeters, &next);
        if (text2) {
            int tw1, th1;
            gui_sdl2_get_text_size(text2, bh->font, &tw1, &th1);
            gui_sdl2_draw_text(text2, shiftX + x + (bh->w - tw1) / 2, y_start, bh->font, textcolor);
        }

        text2 = strtok_r(NULL, delimeters, &next);
        if (text2) {
            int tw2, th2;
            gui_sdl2_get_text_size(text2, bh->font, &tw2, &th2);
            gui_sdl2_draw_text(text2, shiftX + x + (bh->w - tw2) / 2, y_start + line_h, bh->font, textcolor);
        }
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
	else if (! strncmp(name, "btc_", 4))
		return TYPE_CLOSE_BUTTON;
	else if (! strncmp(name, "ta_", 3))
		return TYPE_TOUCH_AREA;
	else if (! strncmp(name, "tf_", 3))
		return TYPE_TEXT_FIELD;
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

void gui_obj_align_to(const char * name1, const char * name2, object_alignment_t align, uint16_t offset)
{
	window_t * win = get_win(get_parent_window());
	obj_type_t type1 = parse_obj_name(name1);
	obj_type_t type2 = parse_obj_name(name2);
	void * oh1 = find_gui_obj(type1, win, name1);
	void * oh2 = find_gui_obj(type2, win, name2);

	if (oh1 == oh2)
		return;

	uint16_t x2 = 0, y2 = 0, w2 = 0, h2 = 0, baseline2 = 0;

	switch(type2)
	{
	case TYPE_LABEL:
		label_t * lh2 = (label_t *) oh2;
		x2 = lh2->x;
		y2 = lh2->y;
		w2 = get_label_width(lh2);
		h2 = get_label_height(lh2);
		baseline2 = lh2->baseline;

		break;

	case TYPE_BUTTON:
		button_t * bh2 = (button_t *) oh2;
		x2 = bh2->x1;
		y2 = bh2->y1;
		w2 = bh2->w;
		h2 = bh2->h;
		break;

	case TYPE_TEXT_FIELD:
		text_field_t * tf2 = (text_field_t *) oh2;
		x2 = tf2->x1;
		y2 = tf2->y1;
		w2 = tf2->w;
		h2 = tf2->h;
		break;

	case TYPE_SLIDER:
		slider_t * sh2 = (slider_t *) oh2;
		x2 = sh2->x;
		y2 = sh2->y;
		w2 = sh2->width;
		h2 = sh2->height;
		break;

	default:
		break;
	}

	switch(type1)
	{
	case TYPE_LABEL:
		label_t * lh1 = (label_t *) oh1;

		if (align == ALIGN_RIGHT_UP) { lh1->x = x2 + w2 + offset; lh1->y = y2; }
		else if (align == ALIGN_RIGHT_UP_MID) { lh1->x = x2 + w2 + offset; lh1->y = y2 + (h2 / 2 - get_label_height(lh1) / 2); }
		else if (align == ALIGN_LEFT_UP)  { lh1->x = x2 - get_label_width(lh1) - offset; lh1->y = y2; }
		else if (align == ALIGN_DOWN_LEFT) { lh1->x = x2; lh1->y = y2 + h2 + offset; }
		else if (align == ALIGN_DOWN_MID) { lh1->x = x2 + w2 / 2 - get_label_width(lh1) / 2; lh1->y = y2 + h2 + offset; }
		else if (align == ALIGN_DOWN_RIGHT) { lh1->x = x2 + w2 - get_label_width(lh1); lh1->y = y2 + h2 + offset; }
		else if (align == ALIGN_LEFT_TOP) { lh1->x = x2; lh1->y = y2 - get_label_height(lh1) - offset; }
		else if (align == ALIGN_RIGHT_DOWN) { lh1->x = x2 + w2 + offset; lh1->y = y2 + baseline2 - lh1->baseline; }

		break;

	case TYPE_BUTTON:
		button_t * bh1 = (button_t *) oh1;

		if (align == ALIGN_RIGHT_UP) { bh1->x1 = x2 + w2 + offset; bh1->y1 = y2; }
		else if (align == ALIGN_RIGHT_UP_MID) { bh1->x1 = x2 + w2 + offset; bh1->y1 = y2 + (h2 / 2 - bh1->h / 2); }
		else if (align == ALIGN_LEFT_UP)  { bh1->x1 = x2 - bh1->w - offset; bh1->y1 = y2; }
		else if (align == ALIGN_DOWN_LEFT) { bh1->x1 = x2; bh1->y1 = y2 + h2 + offset; }
		else if (align == ALIGN_DOWN_MID) { bh1->x1 = x2 + w2 / 2 - bh1->w / 2; bh1->y1 = y2 + h2 + offset; }
		else if (align == ALIGN_DOWN_RIGHT) { bh1->x1 = x2 + w2 - bh1->w; bh1->y1 = y2 + h2 + offset; }
		break;

	case TYPE_SLIDER:
		slider_t * sh1 = (slider_t *) oh1;

		if (align == ALIGN_RIGHT_UP) { sh1->x = x2 + w2 + offset; sh1->y = y2; }
		else if (align == ALIGN_RIGHT_UP_MID) { sh1->x = x2 + w2 + offset; sh1->y = y2 + (h2 / 2 - sh1->height / 2); }
		else if (align == ALIGN_LEFT_UP)  { sh1->x = x2 - sh1->width - offset; sh1->y = y2; }
		else if (align == ALIGN_DOWN_LEFT) { sh1->x = x2; sh1->y = y2 + h2 + offset; }
		else if (align == ALIGN_DOWN_MID) { sh1->x = x2 + w2 / 2 - sh1->width / 2; sh1->y = y2 + h2 + offset; }
		else if (align == ALIGN_DOWN_RIGHT) { sh1->x = x2 + w2 - sh1->width; sh1->y = y2 + h2 + offset; }
		break;

	default:
		break;
	}
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
	case TYPE_LABEL:
		label_t * lh = (label_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) return lh->visible;
		else if (prop == GUI_OBJ_POS_X) return lh->x;
		else if (prop == GUI_OBJ_POS_Y) return lh->y;
		else if (prop == GUI_OBJ_PAYLOAD) return lh->payload;
		else if (prop == GUI_OBJ_STATE) return lh->state;
		else if (prop == GUI_OBJ_ALIGN) return lh->bbox_align;        /* + было только в set */
		else if (prop == GUI_OBJ_COLOR) return (int) lh->color;       /* + было только в set */
		else if (prop == GUI_OBJ_WIDTH) return lh->bbox_w;
		else if (prop == GUI_OBJ_HEIGHT) return lh->bbox_h;
		else if (prop == GUI_OBJ_INDEX) return lh->index;
		break;

	case TYPE_BUTTON:
		button_t * bh = (button_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) return bh->visible;
		else if (prop == GUI_OBJ_POS_X) return bh->x1;
		else if (prop == GUI_OBJ_POS_Y) return bh->y1;
		else if (prop == GUI_OBJ_PAYLOAD) return bh->payload;
		else if (prop == GUI_OBJ_STATE) return bh->state;
		else if (prop == GUI_OBJ_LOCK) return bh->is_locked;
		else if (prop == GUI_OBJ_REPEAT) return bh->is_repeating;     /* + было только в set */
		else if (prop == GUI_OBJ_LONG_PRESS) return bh->is_long_press;/* + было только в set */
		else if (prop == GUI_OBJ_WIDTH) return bh->w;
		else if (prop == GUI_OBJ_HEIGHT) return bh->h;
		else if (prop == GUI_OBJ_INDEX) return bh->index;
		/* GUI_OBJ_FONT намеренно не читается: это указатель, в int не помещается. */
		break;

	case TYPE_SLIDER:
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

	case TYPE_TOUCH_AREA:
		touch_area_t * ta = (touch_area_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) return ta->visible;
		else if (prop == GUI_OBJ_POS_X) return ta->x1;
		else if (prop == GUI_OBJ_POS_Y) return ta->y1;
		else if (prop == GUI_OBJ_PAYLOAD) return ta->payload;
		else if (prop == GUI_OBJ_STATE) return ta->state;             /* + отсутствовало */
		else if (prop == GUI_OBJ_WIDTH) return ta->w;                 /* + отсутствовало */
		else if (prop == GUI_OBJ_HEIGHT) return ta->h;                /* + отсутствовало */
		else if (prop == GUI_OBJ_INDEX) return ta->index;
		break;

	case TYPE_TEXT_FIELD:
		text_field_t * tf = (text_field_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) return tf->visible;
		else if (prop == GUI_OBJ_POS_X) return tf->x1;
		else if (prop == GUI_OBJ_POS_Y) return tf->y1;
		else if (prop == GUI_OBJ_STATE) return tf->state;             /* + отсутствовало */
		else if (prop == GUI_OBJ_WIDTH) return tf->w;
		else if (prop == GUI_OBJ_HEIGHT) return tf->h;
		else if (prop == GUI_OBJ_INDEX) return tf->index;
		break;

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
	case TYPE_LABEL:
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
		    if (strcmp(tmp, lh->text) != 0) {            // текст реально изменился
		        gui_sdl2_invalidate_text(lh->text, lh->font);   // инвалидируем СТАРЫЙ текст
		        strcpy(lh->text, tmp);                          // пишем новый
		        flag = 1;
		    }
		    /* если тексты равны — ничего не делаем: кэш остаётся валидным,
		       TTF_SizeText ниже не вызывается, перерисовка текста не нужна */
		}
		else if (prop == GUI_OBJ_STATE) lh->state = va_arg(arg, int);
		else if (prop == GUI_OBJ_ALIGN) lh->bbox_align = va_arg(arg, int);
		else if (prop == GUI_OBJ_COLOR) lh->color = va_arg(arg, gui_color_t);
		else if (prop == GUI_OBJ_FONT) { flag = 1;
			// Инвалидируем кэш для старого шрифта
			gui_sdl2_invalidate_text(lh->text, lh->font);
			// Закрываем старый динамический шрифт
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
			// Пересчитываем ТОЛЬКО width_text_pix (реальная ширина текущего текста)
			// bbox_w и bbox_h пересчитываем ТОЛЬКО при смене шрифта
			int w, h;
			TTF_SizeText(lh->font, lh->text, &w, &h);
			lh->width_text_pix = w;
			lh->baseline = TTF_FontAscent(lh->font);
			lh->bbox_h = h;  // высота шрифта не зависит от текста
			// bbox_w пересчитываем только при смене шрифта (prop == GUI_OBJ_FONT)
			// При смене текста bbox_w остаётся неизменным!
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

	case TYPE_BUTTON:
		button_t * bh = (button_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) bh->visible = !! va_arg(arg, int);
		else if (prop == GUI_OBJ_POS_X) bh->x1 = va_arg(arg, int);
		else if (prop == GUI_OBJ_POS_Y) bh->y1 = va_arg(arg, int);
		else if (prop == GUI_OBJ_POS) { bh->x1 = va_arg(arg, int); bh->y1 = va_arg(arg, int); }
		else if (prop == GUI_OBJ_PAYLOAD) bh->payload = va_arg(arg, int);
		else if (prop == GUI_OBJ_TEXT) strncpy(bh->text, va_arg(arg, char *), TEXT_ARRAY_SIZE - 1);
		else if (prop == GUI_OBJ_TEXT_FMT) vsnprintf(bh->text, TEXT_ARRAY_SIZE - 1, va_arg(arg, char *), arg);
		else if (prop == GUI_OBJ_STATE) bh->state = va_arg(arg, int);
		else if (prop == GUI_OBJ_LOCK) bh->is_locked = !! va_arg(arg, int);
		else if (prop == GUI_OBJ_WIDTH) bh->w = va_arg(arg, int);
		else if (prop == GUI_OBJ_HEIGHT) bh->h = va_arg(arg, int);
		else if (prop == GUI_OBJ_SIZE) { bh->w = va_arg(arg, int); bh->h = va_arg(arg, int); }
		else if (prop == GUI_OBJ_REPEAT) bh->is_repeating = !! va_arg(arg, int);
		else if (prop == GUI_OBJ_LONG_PRESS) bh->is_long_press = !! va_arg(arg, int);
		else if (prop == GUI_OBJ_FONT) { bh->font = va_arg(arg, TTF_Font *); }
		break;

	case TYPE_SLIDER:
		slider_t * sh = (slider_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) sh->visible = !! va_arg(arg, int);
		else if (prop == GUI_OBJ_POS_X) sh->x = va_arg(arg, int);
		else if (prop == GUI_OBJ_POS_Y) sh->y = va_arg(arg, int);
		else if (prop == GUI_OBJ_POS) { sh->x = va_arg(arg, int); sh->y = va_arg(arg, int); }
		else if (prop == GUI_OBJ_PAYLOAD) sh->value = va_arg(arg, int);
		else if (prop == GUI_OBJ_STATE) sh->state = va_arg(arg, int);   /* + читалось, но не писалось */
		else if (prop == GUI_OBJ_SIZE) sh->size = va_arg(arg, int);     /* + читалось, но не писалось */
		break;

	case TYPE_TOUCH_AREA:                                                /* + case отсутствовал целиком */
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

	case TYPE_TEXT_FIELD:
		text_field_t * tf = (text_field_t *) obj;
		if (prop == GUI_OBJ_VISIBLE) tf->visible = !! va_arg(arg, int);
		else if (prop == GUI_OBJ_POS_X) tf->x1 = va_arg(arg, int);
		else if (prop == GUI_OBJ_POS_Y) tf->y1 = va_arg(arg, int);
		else if (prop == GUI_OBJ_POS) { tf->x1 = va_arg(arg, int); tf->y1 = va_arg(arg, int); }
		else if (prop == GUI_OBJ_STATE) tf->state = va_arg(arg, int);   /* + отсутствовало */
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

	default:
		break;
	}

	va_end(arg);
}

uint8_t gui_check_obj(const char * name1, const char * name2)
{
	return strcmp(name1, name2) == 0;
}

// выравнивание однотипных объектов (кнопка, метка, слайдер) с передачей массива имен объектов
void gui_arrange_objects(const char names[][NAME_ARRAY_SIZE], uint8_t count, uint8_t cols, uint8_t interval)
{
	if (count <= 1) return;

	window_t * win = get_win(get_parent_window());

	obj_type_t type = parse_obj_name(names[0]);
	if (type != TYPE_BUTTON && type != TYPE_LABEL && type != TYPE_SLIDER)
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

	GUI_ASSERT(0);
	return NULL;
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

	GUI_ASSERT(0);
	return 0;
}

// выравнивание однотипных объектов (кнопка, метка, слайдер) с передачей имени первого объекта,
// обработка по возрастанию индекса (в порядке создания)
void gui_arrange_objects_from(const char * name, uint8_t count, uint8_t cols, uint8_t interval)
{
	if (count <= 1) return;

	window_t * win = get_win(get_parent_window());

	obj_type_t type = parse_obj_name(name);
	if (type != TYPE_BUTTON && type != TYPE_LABEL && type != TYPE_SLIDER)
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
}

void gui_objects_init(void)
{

}

#endif /* WITHTOUCHGUI */
