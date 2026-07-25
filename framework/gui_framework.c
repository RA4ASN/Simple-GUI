// Simple GUI от RA4ASN
#include "gui_user_include.h"

#if WITHTOUCHGUI

#include "gui_includes.h"

static gui_t gui = { 0, 0, CANCELLED, 0, 0, 0, 0, 0, };
static uint8_t opened_windows_count = 1;
static uint8_t inited = 0;
gui_sizes_t gui_sizes;

/* Возврат id parent window */
uint8_t get_parent_window(void)
{
	if (opened_windows_count == 2)
		return gui.win[1];
	else
		return NO_PARENT_WINDOW;
}

uint8_t get_current_drawing_window(void)
{
	return gui.current_drawing_window;
}

void set_parent_window(uint8_t p)
{
	gui.win[1] = p;
	opened_windows_count = p == NO_PARENT_WINDOW ? 1 : 2;
}

void gui_set_encoder2_rotate (int16_t rotate)
{
	if (rotate != 0)
	{
		// информация о вращении 2-го энкодера направляется только в активное окно
		if (get_parent_window() == NO_PARENT_WINDOW)
			put_to_wm_queue(get_win(WINDOW_MAIN), WM_MESSAGE_ENC2_ROTATE, rotate);
		else
			put_to_wm_queue(get_win(gui.win[1]), WM_MESSAGE_ENC2_ROTATE, rotate);
	}
}

void dump_queue(window_t * win)
{
	if (! win->queue.size)
		return;

	GUI_DEBUG_PRINT("dump WM queue window '%s'\n", win->title);

	if (win->queue.size == WM_MAX_QUEUE_SIZE)
		GUI_DEBUG_PRINT("WM stack full!\n");

	for (uint8_t i = 0; i < win->queue.size; i ++)
	{
		switch(win->queue.data[i].message)
		{
		case WM_MESSAGE_UPDATE:
			GUI_DEBUG_PRINT("%d: WM_MESSAGE_UPDATE\n", i);
			break;

		case WM_MESSAGE_ACTION:
			GUI_DEBUG_PRINT("%d: WM_MESSAGE_ACTION: object type - %d, action - %d\n", i, (int) win->queue.data[i].type, (int) win->queue.data[i].action);
			break;

		case WM_MESSAGE_ENC2_ROTATE:
			GUI_DEBUG_PRINT("%d: WM_MESSAGE_ENC2_ROTATE: direction - %d\n", i, (int) win->queue.data[i].action);
			break;

		case WM_MESSAGE_KEYB_CODE:
			GUI_DEBUG_PRINT("%d: WM_MESSAGE_KEYB_CODE: code - %d\n", i, (int) win->queue.data[i].action);
			break;

		default:
			GUI_DEBUG_PRINT("%d: unknown message type! - %d\n", i, (int) win->queue.data[i].message);
			break;
		}
	}
}

// WM_MESSAGE_ACTION:        uint32_t type, int32_t action, char * name
//                           type   - тип объекта (значения obj_type_t), передаётся как uint32_t
//                           action - код действия (PRESSED / LONG_PRESSED / MOVING / ...)
//                           name   - имя объекта (строка, копируется в очередь)
// WM_MESSAGE_ENC2_ROTATE:   int32_t rotate
//                           rotate - направление/величина поворота 2-го энкодера;
//                                    если предыдущее сообщение в очереди тоже ENC2_ROTATE,
//                                    значения суммируются вместо добавления новой записи
// WM_MESSAGE_KEYB_CODE:     int32_t keyb_code
//                           keyb_code - код нажатой аппаратной кнопки
// WM_MESSAGE_UPDATE:        (нет вариабельных аргументов)
// WM_MESSAGE_CLOSE:         (нет вариабельных аргументов)
//
// Примечание: дубли подряд идущих ACTION (с совпадающими type+action),
// UPDATE и CLOSE отбрасываются; при переполнении очереди возвращается 0.
uint8_t put_to_wm_queue(window_t * win, wm_message_t message, ...)
{
	if (win->queue.size >= WM_MAX_QUEUE_SIZE) return 0;					// очередь переполнена, ошибка
	//dump_queue(win);
	va_list arg;

	switch (message)
	{
	case WM_MESSAGE_ACTION:
		{
			va_start(arg, message);
			uint32_t type = va_arg(arg, uint32_t);
			int32_t action = va_arg(arg, int32_t);
			char * name = va_arg(arg, char *);
			va_end(arg);
			uint8_t ind = win->queue.size ? (win->queue.size - 1) : 0;
			if (win->queue.data[ind].message == WM_MESSAGE_ACTION
					&& win->queue.data[ind].type == type
					&& win->queue.data[ind].action == action)
				return 1;
			else {
				win->queue.data[win->queue.size].message = WM_MESSAGE_ACTION;
				win->queue.data[win->queue.size].type = (obj_type_t) type;
				win->queue.data[win->queue.size].action = action;
				strncpy(win->queue.data[win->queue.size].name, name,
						NAME_ARRAY_SIZE - 1);
				win->queue.data[win->queue.size].name[NAME_ARRAY_SIZE - 1] = '\0'; // явный терминатор (1.3)
				win->queue.size++;
			}
			return 1;
		}
		break;

	case WM_MESSAGE_ENC2_ROTATE:
		{
			va_start(arg, message);
			int32_t r = va_arg(arg, int32_t);
			va_end(arg);
			uint8_t ind = win->queue.size ? (win->queue.size - 1) : 0;// если первое в очереди сообщение - WM_MESSAGE_ENC2_ROTATE,
			if (win->queue.data[ind].message == WM_MESSAGE_ENC2_ROTATE)	// просуммировать текущее и новое значения поворота,
			{							// иначе добавить новое сообщение
				win->queue.data[ind].action += r;
			} else {
				win->queue.data[win->queue.size].message = WM_MESSAGE_ENC2_ROTATE;
				win->queue.data[win->queue.size].type = (obj_type_t) UINT8_MAX;
				win->queue.data[win->queue.size].action = r;
				win->queue.size++;
			}
			return 1;
		}
		break;

	case WM_MESSAGE_KEYB_CODE:
		{
			va_start(arg, message);
			win->queue.data[win->queue.size].message = WM_MESSAGE_KEYB_CODE;
			win->queue.data[win->queue.size].type = (obj_type_t) UINT8_MAX;
			win->queue.data[win->queue.size].action = va_arg(arg, int32_t);
			win->queue.size++;
			va_end(arg);
			return 1;
		}
		break;

	case WM_MESSAGE_UPDATE:
		{
			uint8_t ind = win->queue.size ? (win->queue.size - 1) : 0;
			if (win->queue.data[ind].message != WM_MESSAGE_UPDATE)// предотвращение дублей сообщения WM_MESSAGE_UPDATE
			{
				win->queue.data[win->queue.size].message = WM_MESSAGE_UPDATE;
				win->queue.data[win->queue.size].type = (obj_type_t) UINT8_MAX;
				win->queue.data[win->queue.size].action = INT32_MAX;
				win->queue.size++;
			}
		}
		return 1;
		break;

	case WM_MESSAGE_CLOSE:
		{
			uint8_t ind = win->queue.size ? (win->queue.size - 1) : 0;
			if (win->queue.data[ind].message != WM_MESSAGE_CLOSE)// предотвращение дублей сообщения WM_MESSAGE_CLOSE
			{
				win->queue.data[win->queue.size].message = WM_MESSAGE_CLOSE;
				win->queue.data[win->queue.size].type = (obj_type_t) UINT8_MAX;
				win->queue.data[win->queue.size].action = INT32_MAX;
				win->queue.size++;
			}
		}
		return 1;
		break;

	case WM_NO_MESSAGE:
	default:
		return 0;
		break;
	}

	GUI_DEBUG_PRINT("put_to_wm_queue: no valid type of messages found\n");
	GUI_ASSERT(0);
	return 0;
}

wm_message_t get_from_wm_queue(uint8_t win_id, uint8_t * type, int32_t * action, char * name)
{
	window_t * win = get_win(win_id);
	if (!win->queue.size) return WM_NO_MESSAGE;						// очередь сообщений пустая

	win->queue.size--;
	*type = win->queue.data[win->queue.size].type;
	*action = win->queue.data[win->queue.size].action;

	if (win->queue.data[win->queue.size].message == WM_MESSAGE_ACTION) {
		char obj_name[NAME_ARRAY_SIZE] = { 0 };
		strncpy(obj_name, win->queue.data[win->queue.size].name, NAME_ARRAY_SIZE - 1);
		obj_name[NAME_ARRAY_SIZE - 1] = '\0';          				// явный терминатор (1.3)
		char * r = strrchr(obj_name, '#');
		if (r) obj_name[r - obj_name] = '\0';              			// guard от NULL (1.1)
		strncpy(name, obj_name, NAME_ARRAY_SIZE - 1);
		name[NAME_ARRAY_SIZE - 1] = '\0';              				// явный терминатор (1.3)
	}

	wm_message_t m = win->queue.data[win->queue.size].message;
	win->queue.data[win->queue.size].message = WM_NO_MESSAGE;		// очистить текущую запись
	win->queue.data[win->queue.size].type = TYPE_DUMMY;
	win->queue.data[win->queue.size].action = 0;
	memset(win->queue.data[win->queue.size].name, 0, NAME_ARRAY_SIZE);

	return m;
}

void clean_wm_queue (window_t * win)
{
	win->queue.size = 0;
	memset(win->queue.data, 0, sizeof win->queue.data);
}

/* Запрос на обновление состояния элементов GUI */
void gui_update(void)
{
	put_to_wm_queue(get_win(WINDOW_MAIN), WM_MESSAGE_UPDATE);	// главное окно всегда нужно обновлять
	uint8_t win2 = get_parent_window();

	if (win2 != NO_PARENT_WINDOW)								// если открыто второе окно,
		put_to_wm_queue(get_win(win2), WM_MESSAGE_UPDATE);		// добавить сообщение на обновление в его очередь
}

/* Получить относительные координаты перемещения точки касания экрана */
void get_gui_tracking(int_fast16_t * x, int_fast16_t * y)
{
	* x = gui.vector_move_x;
	* y = gui.vector_move_y;
	gui.vector_move_x = 0;
	gui.vector_move_y = 0;
}

/* Возврат ссылки на запись в структуре по названию и типу окна */
void * find_gui_obj(obj_type_t type, window_t * win, const char * name)
{
	GUI_ASSERT(win);
	GUI_ASSERT(name);

	char obj_name[NAME_ARRAY_SIZE] = { 0 };
	snprintf(obj_name, NAME_ARRAY_SIZE, "%s#%02d", name, win->window_id);

	switch (type)
	{
	case TYPE_BUTTON:
		for (uint8_t i = 0; i < win->bh_count; i ++)
		{
			button_t * bh = & win->bh_ptr[i];
			if (! strcmp(bh->name, obj_name))
				return (button_t *) bh;
		}
		goto not_found;
		break;

	case TYPE_LABEL:
		for (uint8_t i = 0; i < win->lh_count; i ++)
		{
			label_t * lh = & win->lh_ptr[i];
			if (! strcmp(lh->name, obj_name))
				return (label_t *) lh;
		}
		goto not_found;
		break;

	case TYPE_SLIDER:
		for (uint8_t i = 0; i < win->sh_count; i ++)
		{
			slider_t * sh = & win->sh_ptr[i];
			if (! strcmp(sh->name, obj_name))
				return (slider_t *) sh;
		}
		goto not_found;
		break;

	case TYPE_TOUCH_AREA:
		for (uint8_t i = 0; i < win->ta_count; i ++)
		{
			touch_area_t * ta = & win->ta_ptr[i];
			if (! strcmp(ta->name, obj_name))
				return (touch_area_t *) ta;
		}
		goto not_found;
		break;

	case TYPE_TEXT_FIELD:
		for (uint8_t i = 0; i < win->tf_count; i ++)
		{
			text_field_t * tf = & win->tf_ptr[i];
			if (! strcmp(tf->name, obj_name))
				return (text_field_t *) tf;
		}
		goto not_found;
		break;

	default:
		GUI_DEBUG_PRINT("%s: undefined type %d\n", __func__, type);
		GUI_ASSERT(0);
		return NULL;
	}

not_found:
	if (win->window_id == WINDOW_MAIN)
	{
		GUI_DEBUG_PRINT("%s: object '%s' not found\n", __func__, name);
		GUI_ASSERT(0);
		return NULL;
	}
	else
		return find_gui_obj(type, get_win(WINDOW_MAIN), name);
}

static void element_touch_info(obj_type_t type, void * link,
		uint16_t * x1, uint16_t * y1, uint16_t * x2, uint16_t * y2,
		uint8_t * visible, uint8_t * state,
		uint8_t * trackable, uint8_t * longpress, uint8_t * repeat)
{
	uint16_t e = gui_sizes.touch_area_enlarge;

	switch (type)
	{
	case TYPE_BUTTON:
	case TYPE_CLOSE_BUTTON:
		{
			button_t * bh = (button_t *) link;
			*visible = bh->visible; *state = bh->state;
			*trackable = 0; *longpress = bh->is_long_press; *repeat = bh->is_repeating;
			*x1 = (bh->x1 - e) < 0 ? 0 : (bh->x1 - e);
			*x2 = (bh->x1 + bh->w + e) > gui_sizes.max_w ? gui_sizes.max_w : (bh->x1 + bh->w + e);
			*y1 = (bh->y1 - e) < 0 ? 0 : (bh->y1 - e);
			*y2 = (bh->y1 + bh->h + e) > gui_sizes.max_h ? gui_sizes.max_h : (bh->y1 + bh->h + e);
		}
		break;

	case TYPE_LABEL:
		{
			label_t * lh = (label_t *) link;
			*visible = lh->visible; *state = lh->state;
			*trackable = lh->is_trackable; *longpress = 0; *repeat = 0;
			*x1 = (lh->x - e) < 0 ? 0 : (lh->x - e);
			*x2 = (lh->x + get_label_width(lh) + e) > gui_sizes.max_w ? gui_sizes.max_w : (lh->x + get_label_width(lh) + e);
			*y1 = (lh->y - get_label_height(lh) - e) < 0 ? 0 : (lh->y - get_label_height(lh) - e);
			*y2 = (lh->y + get_label_height(lh) * 2 + e) > gui_sizes.max_h ? gui_sizes.max_h : (lh->y + get_label_height(lh) * 2 + e);
		}
		break;

	case TYPE_SLIDER:
		{
			slider_t * sh = (slider_t *) link;
			*visible = sh->visible; *state = sh->state;
			*trackable = 1; *longpress = 0; *repeat = 0;
			*x1 = (sh->x - e) < 0 ? 0 : (sh->x - e);
			*x2 = (sh->x + sh->width + e) > gui_sizes.max_w ? gui_sizes.max_w : (sh->x + sh->width + e);
			*y1 = (sh->y - e) < 0 ? 0 : (sh->y - e);
			*y2 = (sh->y + sh->height + e) > gui_sizes.max_h ? gui_sizes.max_h : (sh->y + sh->height + e);
		}
		break;

	case TYPE_TOUCH_AREA:
		{
			touch_area_t * ta = (touch_area_t *) link;
			*visible = ta->visible; *state = ta->state;
			*trackable = ta->is_trackable; *longpress = 0; *repeat = 0;
			*x1 = (ta->x1) < 0 ? 0 : (ta->x1);
			*x2 = (ta->x1 + ta->w) > gui_sizes.max_w ? gui_sizes.max_w : (ta->x1 + ta->w);
			*y1 = (ta->y1) < 0 ? 0 : (ta->y1);
			*y2 = (ta->y1 + ta->h) > gui_sizes.max_h ? gui_sizes.max_h : (ta->y1 + ta->h);
		}
		break;

	case TYPE_TEXT_FIELD:
		{
			text_field_t * tf = (text_field_t *) link;
			*visible = tf->visible; *state = tf->state;
			*trackable = 0; *longpress = 0; *repeat = 0;
			*x1 = (tf->x1) < 0 ? 0 : (tf->x1);
			*x2 = (tf->x1 + tf->w) > gui_sizes.max_w ? gui_sizes.max_w : (tf->x1 + tf->w);
			*y1 = (tf->y1) < 0 ? 0 : (tf->y1);
			*y2 = (tf->y1 + tf->h) > gui_sizes.max_h ? gui_sizes.max_h : (tf->y1 + tf->h);
		}
		break;

	default:
		*visible = 0; *state = 0; *trackable = 0; *longpress = 0; *repeat = 0;
		*x1 = 0; *y1 = 0; *x2 = 0; *y2 = 0;
		break;
	}
}

/* Проверка попадания точки (экранные координаты) в один элемент. */
static int check_one(obj_type_t t, void * lk, window_t * w, uint16_t px, uint16_t py,
		void ** o_link, obj_type_t * o_type, window_t ** o_win,
		uint8_t * o_tr, uint8_t * o_lp, uint8_t * o_rp)
{
	uint16_t x1, y1, x2, y2;
	uint8_t vis, st, tr, lp, rp;

	element_touch_info(t, lk, &x1, &y1, &x2, &y2, &vis, &st, &tr, &lp, &rp);

	if (vis == VISIBLE && st != DISABLED
			&& w->x1 + x1 < px && w->x1 + x2 > px
			&& w->y1 + y1 < py && w->y1 + y2 > py)
	{
		*o_link = lk; *o_type = t; *o_win = w;
		*o_tr = tr; *o_lp = lp; *o_rp = rp;
		return 1;
	}
	return 0;
}

/* Hit-test: обход открытых окон сверху вниз, внутри окна — типы в обратном
   порядке (CLOSE, TEXT_FIELD, TOUCH_AREA, SLIDER, LABEL, BUTTON). */
static int hit_test(uint16_t px, uint16_t py,
		void ** o_link, obj_type_t * o_type, window_t ** o_win,
		uint8_t * o_tr, uint8_t * o_lp, uint8_t * o_rp)
{
	for (int wi = (int) opened_windows_count - 1; wi >= 0; wi--)
	{
		window_t * w = get_win(gui.win[wi]);
		if (w->state != VISIBLE)
			continue;
		if (w->is_close && check_one(TYPE_CLOSE_BUTTON, &w->close_button, w, px, py, o_link, o_type, o_win, o_tr, o_lp, o_rp))
			return 1;
		for (int i = (int) w->tf_count - 1; i >= 0; i--)
			if (check_one(TYPE_TEXT_FIELD, &w->tf_ptr[i], w, px, py, o_link, o_type, o_win, o_tr, o_lp, o_rp)) return 1;
		for (int i = (int) w->ta_count - 1; i >= 0; i--)
			if (check_one(TYPE_TOUCH_AREA, &w->ta_ptr[i], w, px, py, o_link, o_type, o_win, o_tr, o_lp, o_rp)) return 1;
		for (int i = (int) w->sh_count - 1; i >= 0; i--)
			if (check_one(TYPE_SLIDER, &w->sh_ptr[i], w, px, py, o_link, o_type, o_win, o_tr, o_lp, o_rp)) return 1;
		for (int i = (int) w->lh_count - 1; i >= 0; i--)
			if (check_one(TYPE_LABEL, &w->lh_ptr[i], w, px, py, o_link, o_type, o_win, o_tr, o_lp, o_rp)) return 1;
		for (int i = (int) w->bh_count - 1; i >= 0; i--)
			if (check_one(TYPE_BUTTON, &w->bh_ptr[i], w, px, py, o_link, o_type, o_win, o_tr, o_lp, o_rp)) return 1;
	}
	return 0;
}

/* Отрисовка всех видимых элементов окна в прямом порядке типов. */
static void draw_window_objects(window_t * win)
{
	for (uint8_t i = 0; i < win->bh_count; i++)
		if (win->bh_ptr[i].visible) draw_button(&win->bh_ptr[i]);
	for (uint8_t i = 0; i < win->lh_count; i++)
		if (win->lh_ptr[i].visible) draw_label(&win->lh_ptr[i]);
	for (uint8_t i = 0; i < win->sh_count; i++)
		if (win->sh_ptr[i].visible) draw_slider(&win->sh_ptr[i]);
	for (uint8_t i = 0; i < win->tf_count; i++)
		if (win->tf_ptr[i].visible) draw_textfield(&win->tf_ptr[i]);
	if (win->is_close && win->close_button.visible)
		draw_close_button(&win->close_button);
}

/* Системный обработчик слайдера в момент его перемещения */
static void slider_process(slider_t * sl)
{
	window_t * win = get_win(sl->parent);
	if (!win)
	{
		gui.vector_move_x = 0;
		gui.vector_move_y = 0;
		return;
	}

	// Позиция касания относительно начала шкалы
	int pos = 0;
	if (sl->orientation == ORIENTATION_HORIZONTAL)
		pos = (int) gui.last_pressed_x - win->x1 - (int) sl->x - sl->scale_x;
	else
		pos = (int) gui.last_pressed_y - win->y1 - (int) sl->y - sl->scale_y;

	// Расчёт значения (0..100) на основе абсолютной позиции
	if (sl->scale_size > 0)
	{
		// Округление до ближайшего целого
		int v = (pos * 100 + sl->scale_size / 2) / sl->scale_size;
		if (v < 0) v = 0;
		if (v > 100) v = 100;
		sl->value = (uint8_t) v;
	}

	gui.vector_move_x = 0;
	gui.vector_move_y = 0;
}

// Селектор запуска функций обработки событий.
static void set_state_record(window_t * win, obj_type_t type, void * link, uint8_t state)
{
	GUI_ASSERT(link != NULL);
	switch (type)
	{
	case TYPE_CLOSE_BUTTON:
		{
			button_t * bh = (button_t *) link;
			bh->state = state;
			if (bh->state == RELEASED) close_all_windows();
		}
		break;

	case TYPE_BUTTON:
		{
			button_t * bh = (button_t *) link;
			bh->state = state;
			if (bh->state == RELEASED || bh->state == LONG_PRESSED || bh->state == PRESS_REPEATING)
			{
				if (! put_to_wm_queue(win, WM_MESSAGE_ACTION, TYPE_BUTTON, bh->state == LONG_PRESSED ? LONG_PRESSED : PRESSED, bh->name))
					dump_queue(win);
			}
		}
		break;

	case TYPE_LABEL:
		{
			label_t * lh = (label_t *) link;
			lh->state = state;
			if (lh->state == RELEASED)
			{
				if (! put_to_wm_queue(win, WM_MESSAGE_ACTION, TYPE_LABEL, PRESSED, lh->name))
					dump_queue(win);
			}
			else if (lh->state == PRESSED && lh->is_trackable)
			{
				if (! put_to_wm_queue(win, WM_MESSAGE_ACTION, TYPE_LABEL, MOVING, lh->name))
					dump_queue(win);
			}
		}
		break;

	case TYPE_SLIDER:
		{
			slider_t * sh = (slider_t *) link;
			sh->state = state;
			if (sh->state == PRESSED)
			{
				slider_process(sh);
				if (! put_to_wm_queue(win, WM_MESSAGE_ACTION, TYPE_SLIDER, PRESSED, sh->name))
					dump_queue(win);
			}
		}
		break;

	case TYPE_TOUCH_AREA:
		{
			touch_area_t * ta = (touch_area_t *) link;
			ta->state = state;
			if (ta->state == RELEASED)
			{
				if (! put_to_wm_queue(win, WM_MESSAGE_ACTION, TYPE_TOUCH_AREA, PRESSED, ta->name))
					dump_queue(win);
			}
			else if (ta->state == PRESSED && ta->is_trackable)
			{
				if (strstr(ta->name, "ta_winmove"))
				{
					move_window(win, gui.vector_move_x, gui.vector_move_y);
					gui.vector_move_x = 0;
					gui.vector_move_y = 0;
				}
				else if (! put_to_wm_queue(win, WM_MESSAGE_ACTION, TYPE_TOUCH_AREA, MOVING, ta->name))
					dump_queue(win);
			}
		}
		break;

	case TYPE_TEXT_FIELD:
		break;

	default:
		{
			GUI_DEBUG_PRINT("set_state_record: undefined type %d\n", type);
			GUI_ASSERT(0);
		}
		break;
	}
}

/* Передать менеджеру GUI код нажатой кнопки на клавиатуре */
void gui_put_keyb_code (uint8_t kbch)
{
	// перенаправить код нажатой аппаратной кнопки в активное окно
	if (get_parent_window() == NO_PARENT_WINDOW)
		put_to_wm_queue(get_win(WINDOW_MAIN), WM_MESSAGE_KEYB_CODE, kbch);
	else
		put_to_wm_queue(get_win(gui.win[1]), WM_MESSAGE_KEYB_CODE, kbch);
}

void gui_put_event(gui_event_type type, uint16_t code)
{
	window_t * win = get_win(get_parent_window());

	if (type == EVENT_TYPE_CONTROL)
	{
		int p = 0;
		if (code == CODE_CURSOR_LEFT) p = -1;
		else if (code == CODE_CURSOR_RIGHT) p = 1;
		else if (code == CODE_KEY_ENTER && win->idx_bh_focus_old != UINT8_MAX)
		{
			button_t * bh = & win->bh_ptr[win->idx_bh_focus];
			if (bh->state != DISABLED)
			{
				if (! put_to_wm_queue(win, WM_MESSAGE_ACTION, TYPE_BUTTON, PRESSED, bh->name))
					dump_queue(win);
			}
			return;
		}
		else if (code == CODE_KEY_ESCAPE)
		{
			close_window(1);
			if (! get_parent_window())
				footer_buttons_state(CANCELLED);
			return;
		}
		if (win->idx_bh_focus_old != UINT8_MAX)
		{
			win->idx_bh_focus += p;
			if (win->idx_bh_focus < 0) win->idx_bh_focus = win->bh_count - 1;
			if (win->idx_bh_focus >= win->bh_count) win->idx_bh_focus = 0;
		}
		win->bh_ptr[win->idx_bh_focus_old].is_focus = 0;
		win->bh_ptr[win->idx_bh_focus].is_focus = 1;
		win->idx_bh_focus_old = win->idx_bh_focus;
	}
}

uint16_t gui_get_max_w(void)
{
	return gui_sizes.max_w;
}

uint16_t gui_get_max_h(void)
{
	return gui_sizes.max_h;
}

uint16_t gui_get_footer_h(void)
{
	return gui_sizes.footer_height;
}

int gui_scale_ui(int v)
{
	return (int)lroundf((float) v * gui_sizes.scale_ui);
}

/* Инициализация GUI */
int gui_initialize (uint16_t screen_w, uint16_t screen_h)
{
	if (screen_w < GUI_ETALON_W || screen_h < GUI_ETALON_H)
	{
		GUI_DEBUG_PRINT("GUI: screen resolution %dx%d lesser than supported\n", screen_w, screen_h);
		return 1;
	}

	gui_sizes.max_w = screen_w;
	gui_sizes.max_h = screen_h;
	float scale_x = (float) screen_w / (float) GUI_ETALON_W;
	float scale_y = (float) screen_h / (float) GUI_ETALON_H;
	gui_sizes.scale_ui = fmin(scale_x, scale_y);
	gui_sizes.sliders_scale_thickness = gui_scale_ui(sliders_scale_thickness_default);
	gui_sizes.sliders_w = gui_scale_ui(sliders_w_default);
	gui_sizes.sliders_h = gui_scale_ui(sliders_h_default);
	gui_sizes.window_title_height = gui_scale_ui(window_title_height_default);
	gui_sizes.edge_step = gui_scale_ui(edge_step_default);
	gui_sizes.window_close_button_size = gui_scale_ui(window_close_button_size_default);
	gui_sizes.window_title_indent = gui_scale_ui(window_title_indent_default);
	gui_sizes.touch_area_enlarge = gui_scale_ui(touch_area_enlarge_default);
	gui_sizes.footer_height = gui_scale_ui(footer_height_default);
	gui_sizes.common_btn_width = screen_w / footer_buttons_count + 1 - common_btn_interval;
	gui_sizes.common_btn_height = gui_sizes.footer_height - 6;
	gui_sizes.buttons_font_size = gui_scale_ui(buttons_font_size_default);
	gui_sizes.labels_font_size = gui_scale_ui(labels_font_size_default);
	gui_sizes.win_title_font_size = gui_scale_ui(win_title_font_size_default);

	gui_objects_init();
	gui_sdl2_text_init();

	open_window(get_win(WINDOW_MAIN));
	gui_user_init();

	inited = 1;
	return 0;
}

/* GUI state mashine */
void process_gui(void)
{
	uint16_t tx, ty;
	static uint16_t x_old = 0, y_old = 0, long_press_counter = 0;
	static void * cap_link = NULL;
	static obj_type_t cap_type = TYPE_DUMMY;
	static window_t * cap_win = NULL;
	static uint8_t cap_trackable = 0, cap_longpress = 0, cap_repeat = 0;
	const uint8_t long_press_limit = 20;
	static uint8_t is_long_press = 0;		// 1 - долгое нажатие уже обработано
	static uint8_t is_repeating = 0, repeating_cnt = 0;
	if (!inited) return;

	if (__gui_get_touch_event(&tx, &ty))
	{
		gui.last_pressed_x = tx;
		gui.last_pressed_y = ty;
		gui.is_touching_screen = 1;
	}
	else
	{
		gui.is_touching_screen = 0;
		gui.is_after_touch = 0;
	}
	if (gui.state == CANCELLED && gui.is_touching_screen && !gui.is_after_touch)
	{
		if (hit_test(gui.last_pressed_x, gui.last_pressed_y,
				&cap_link, &cap_type, &cap_win,
				&cap_trackable, &cap_longpress, &cap_repeat))
		{
			gui.state = PRESSED;
			is_long_press = 0;
			is_repeating = 0;
			long_press_counter = 0;
			x_old = tx;
			y_old = ty;
		}
	}
	if (gui.is_tracking && !gui.is_touching_screen)
	{
		gui.is_tracking = 0;
		gui.vector_move_x = 0;
		gui.vector_move_y = 0;
		x_old = 0;
		y_old = 0;
	}
	if (gui.state == PRESSED)
	{
		GUI_ASSERT(cap_link != NULL);
		if (cap_trackable && gui.is_touching_screen)
		{
			gui.vector_move_x = (int16_t) gui.last_pressed_x - (int16_t) x_old;
			gui.vector_move_y = (int16_t) gui.last_pressed_y - (int16_t) y_old;
			if (gui.vector_move_x != 0 || gui.vector_move_y != 0)
			{
				gui.is_tracking = 1;
			}
			set_state_record(cap_win, cap_type, cap_link, PRESSED);
			x_old = gui.last_pressed_x;
			y_old = gui.last_pressed_y;
		}
		else
		{
			uint16_t rx1, ry1, rx2, ry2;
			uint8_t _v, _s, _t, _l, _r;
			element_touch_info(cap_type, cap_link, &rx1, &ry1, &rx2, &ry2, &_v, &_s, &_t, &_l, &_r);
			if (cap_win->x1 + rx1 < gui.last_pressed_x && cap_win->x1 + rx2 > gui.last_pressed_x
					&& cap_win->y1 + ry1 < gui.last_pressed_y && cap_win->y1 + ry2 > gui.last_pressed_y
					&& !gui.is_after_touch)
			{
				if (gui.is_touching_screen)
				{
					GUI_ASSERT(cap_link != NULL);
					if (is_repeating)
					{
						repeating_cnt++;
						if (repeating_cnt > autorepeat_delay)
						{
							repeating_cnt = 0;
							set_state_record(cap_win, cap_type, cap_link, PRESS_REPEATING);		// для запуска обработчика нажатия
						}
					}
					else set_state_record(cap_win, cap_type, cap_link, PRESSED);
					if (cap_longpress)
					{
						if (gui.state != LONG_PRESSED && !is_long_press && lp_delay_10ms(0)) long_press_counter++;
						if (long_press_counter > long_press_limit)
						{
							long_press_counter = 0;
							gui.state = LONG_PRESSED;
						}
					}
					else if (cap_repeat)
					{
						if (!is_repeating) long_press_counter++;
						if (long_press_counter > long_press_limit)
						{
							long_press_counter = 0;
							repeating_cnt = 0;
							is_repeating = 1;
						}
					}
				}
				else gui.state = RELEASED;
			}
			else
			{
				GUI_ASSERT(cap_link != NULL);
				gui.state = CANCELLED;
				set_state_record(cap_win, cap_type, cap_link, CANCELLED);
				gui.is_after_touch = 1; // точка непрерывного касания вышла за пределы выбранного элемента, не поддерживающего tracking
			}
		}
	}
	if (gui.state == RELEASED)
	{
		GUI_ASSERT(cap_link != NULL);
		if (!is_long_press)	// если было долгое нажатие, обработчик по короткому не запускать
			set_state_record(cap_win, cap_type, cap_link, RELEASED);
		set_state_record(cap_win, cap_type, cap_link, CANCELLED);
		gui.is_after_touch = 0;
		gui.state = CANCELLED;
		gui.is_tracking = 0;
	}
	else if (gui.state == LONG_PRESSED)
	{
		set_state_record(cap_win, cap_type, cap_link, LONG_PRESSED);		// для запуска обработчика нажатия
		lp_delay_10ms(1);				// инициализация задержки
		gui.state = PRESSED;
		is_long_press = 1;				// долгое нажатие обработано
	}
	TIME_PROFILE_START(gui);
	for (uint8_t i = 0; i < opened_windows_count; i++)
	{
		window_t * win = get_win(gui.win[i]);
		gui.current_drawing_window = gui.win[i];
		if (win->state == VISIBLE)
		{
			if (win->first_call) win->onVisibleProcess();	// запуск только процедуры инициализации окна
			else
			{
				draw_window(win);
				win->onVisibleProcess();					// запуск процедуры фоновой обработки для окна
				draw_window_objects(win);					// отрисовка принадлежащих окну элементов напрямую из его массивов
			}
		}
	}
	TIME_PROFILE_STOP(gui, "");
}

#endif /* WITHTOUCHGUI */
