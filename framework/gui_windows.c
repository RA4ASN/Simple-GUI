// Simple GUI от RA4ASN

#include "gui_user_include.h"

#if WITHTOUCHGUI

#include "gui_includes.h"

void gui_user_actions_after_close_window(void);

static window_t windows[] = {
	#define X(window_id, parent_id, align_mode, title, is_close, onVisibleProcess) \
		{ WINDOW_##window_id, WINDOW_##parent_id, align_mode, title, is_close, onVisibleProcess, },
	#include "window_list.h"
	WINDOW_LIST(X)
	#undef X
	#undef WINDOW_LIST
};

uint8_t is_win_init(void)
{
	window_t * win = get_win(get_parent_window());
	uint8_t s = win->first_call;

	if (s)
		win->first_call = 0;

	return s;
}

// only for gui_main_process()
uint8_t is_winmain_init(void)
{
	window_t * win = get_win(WINDOW_MAIN);
	uint8_t s = win->first_call;

	if (s)
		win->first_call = 0;

	return s;
}

/* Возврат ссылки на окно */
window_t * get_win(uint8_t window_id)
{
	if (window_id == NO_PARENT_WINDOW)	//	костыль
		return & windows[0];

	GUI_ASSERT(window_id < WINDOWS_COUNT);
	return & windows[window_id];
}

/* Открыть окно */
void open_window(window_t * win)
{
	uint8_t pwin = get_parent_window();

	if (win->parent_id != NO_PARENT_WINDOW && pwin == win->parent_id)	// Если открыто parent window, закрыть его и оставить child window
		close_window(0);

	win->idx_bh_focus_old = UINT8_MAX;
	win->state = VISIBLE;
	win->first_call = 1;
	win->is_moving = 0;
	win->title_align = ALIGNMENT_LEFT;
	set_parent_window(win->window_id);

}

/* Освободить выделенную память в куче и обнулить счетчики элементов окна */
static void free_win_ptr (window_t * win)
{
	free(win->bh_ptr);
	free(win->lh_ptr);
	free(win->sh_ptr);
	free(win->ta_ptr);
	free(win->tf_ptr);

	win->bh_count = 0;
	win->lh_count = 0;
	win->sh_count = 0;
	win->ta_count = 0;
	win->tf_count = 0;

	win->bh_ptr = NULL;
	win->lh_ptr = NULL;
	win->sh_ptr = NULL;
	win->ta_ptr = NULL;
	win->tf_ptr = NULL;
//	GUI_DEBUG_PRINT("free: %d %s\n", win->window_id, win->title);
}

/* Установка признака видимости окна */
void close_window(uint8_t parent_action) // 0 - не открывать parent window, 1 - открыть
{
	uint8_t pwin = get_parent_window();

	if(pwin != NO_PARENT_WINDOW)
	{
		window_t * win = get_win(pwin);
		win->state = NON_VISIBLE;
		objects_state(win);

		if (put_to_wm_queue(win, WM_MESSAGE_CLOSE))
			win->onVisibleProcess();
		else
			dump_queue(win);

		free_win_ptr(win);
		set_parent_window(NO_PARENT_WINDOW);

		if (win->parent_id != NO_PARENT_WINDOW && parent_action)	// При закрытии child window открыть parent window, если есть и если разрешено
			open_window(get_win(win->parent_id));

		gui_user_actions_after_close_window();
	}
}

/* Закрыть все окна (обработчик системной кнопки закрытия окна) */
void close_all_windows(void)
{
	close_window(DONT_OPEN_PARENT_WINDOW);
	footer_buttons_state(CANCELLED);
	gui_user_actions_after_close_window();
}

/* Разрешить перетаскивание окна */
void enable_window_move(void)
{
	window_t * win = get_win(get_parent_window());
	win->is_moving = 1;
}

void move_window(window_t * win, int_fast16_t ax, int_fast16_t ay)
{
	GUI_ASSERT(win != NULL);

	// защита от переполнения экранных координат
	if (win->x1 + ax < 0 || win->x1 + win->w + ax >= gui_sizes.max_w || win->y1 + ay < 0 || win->y1 + win->h + ay >= gui_sizes.max_h - gui_sizes.footer_height)
		return;

	win->x1 += ax;
	win->y1 += ay;
	win->draw_x1 += ax;
	win->draw_x2 += ax;
	win->draw_y1 += ay;
	win->draw_y2 += ay;
}

/* Расчет экранных координат окна */
/* при mode = WINDOW_POSITION_MANUAL_SIZE в качестве необязательных параметров передать xmax и ymax */
void calculate_window_position(uint8_t mode, ...)
{
	window_t * win = get_win(get_parent_window());
	uint16_t xmax = 0, ymax = 0, shift_x, shift_y, x_start, y_start;

	uint16_t align_left_x = gui_sizes.max_w / 4;
	uint16_t align_center_x = gui_sizes.max_w / 2;
	uint16_t align_right_x = align_left_x + align_center_x;
	uint16_t align_y = gui_sizes.max_h / 2 - gui_sizes.footer_height / 2;

    uint16_t title_length = 0;
    if (strcmp(win->title, "")) {
#if SDL2_FONTS
        int tw = 0, th = 0;
        gui_sdl2_get_text_size(win->title, gui_sdl2_get_window_title_font(), &tw, &th);
        title_length = (uint16_t)tw;
#else
        title_length = get_strwidth_prop(win->title, & WINDOW_TITLE_FONTP);
#endif
    }

	GUI_ASSERT(win != NULL);
	win->size_mode = mode;

	switch (mode)
	{
	case WINDOW_POSITION_MANUAL_SIZE:
		{
			va_list arg;
			va_start(arg, mode);
			xmax = va_arg(arg, int);
			ymax = va_arg(arg, int);
			va_end(arg);
		}
		break;

	case WINDOW_POSITION_MANUAL_POSITION:
		{
			va_list arg;
			va_start(arg, mode);
			x_start = va_arg(arg, int);
			y_start = va_arg(arg, int);
			va_end(arg);
		}
		// no break

	case WINDOW_POSITION_FULLSCREEN:
	case WINDOW_POSITION_AUTO:
		{
			if (win->bh_ptr != NULL)
			{
				for (uint8_t i = 0; i < win->bh_count; i++)
				{
					const button_t * bh = & win->bh_ptr[i];
					xmax = (xmax > bh->x1 + bh->w) ? xmax : (bh->x1 + bh->w);
					ymax = (ymax > bh->y1 + bh->h) ? ymax : (bh->y1 + bh->h);
					GUI_ASSERT(xmax < gui_sizes.max_w);
					GUI_ASSERT(ymax < gui_sizes.max_h);
				}
			}

			if (win->lh_ptr != NULL)
			{
				for (uint8_t i = 0; i < win->lh_count; i++)
				{
					const label_t * lh = & win->lh_ptr[i];
					xmax = (xmax > lh->x + get_label_width(lh)) ? xmax : (lh->x + get_label_width(lh));
					ymax = (ymax > lh->y + get_label_height(lh)) ? ymax : (lh->y + get_label_height(lh));
					GUI_ASSERT(xmax < gui_sizes.max_w);
					GUI_ASSERT(ymax < gui_sizes.max_h);
				}
			}

			if (win->tf_ptr != NULL)
			{
				for (uint8_t i = 0; i < win->tf_count; i++)
				{
					const text_field_t * tf = & win->tf_ptr[i];
					xmax = (xmax > tf->x1 + tf->w) ? xmax : (tf->x1 + tf->w);
					ymax = (ymax > tf->y1 + tf->h) ? ymax : (tf->y1 + tf->h);
					GUI_ASSERT(xmax < gui_sizes.max_w);
					GUI_ASSERT(ymax < gui_sizes.max_h);
				}
			}

			if (win->sh_ptr != NULL)
			{
				for (uint8_t i = 0; i < win->sh_count; i++)
				{
					const slider_t * sh = & win->sh_ptr[i];
					if (sh->orientation)	// ORIENTATION_HORIZONTAL
					{
						xmax = (xmax > sh->x + sh->size + gui_sizes.sliders_w) ? xmax : (sh->x + sh->size + gui_sizes.sliders_w);
						ymax = (ymax > sh->y + gui_sizes.sliders_h * 2) ? ymax : (sh->y + gui_sizes.sliders_h * 2);
					}
					else					// ORIENTATION_VERTICAL
					{
						xmax = (xmax > sh->x + gui_sizes.sliders_w * 2) ? xmax : (sh->x + gui_sizes.sliders_w * 2);
						ymax = (ymax > sh->y + sh->size + gui_sizes.sliders_h) ? ymax : (sh->y + sh->size + gui_sizes.sliders_h);
					}
					GUI_ASSERT(xmax < gui_sizes.max_w);
					GUI_ASSERT(ymax < gui_sizes.max_h);
				}
			}
		}
		break;

	default:

		break;
	}

	shift_x = gui_sizes.edge_step;
	shift_y = (title_length ? gui_sizes.window_title_height : 0) + gui_sizes.edge_step;

	// Выравнивание массива оконных элементов по центру окна
	if (win->window_id != WINDOW_MAIN)
	{
		if (win->bh_ptr != NULL)
		{
			for (uint8_t i = 0; i < win->bh_count; i++)
			{
				button_t * bh = & win->bh_ptr[i];
				bh->x1 += shift_x;
				bh->y1 += shift_y;
				GUI_ASSERT(bh->x1 + bh->w < gui_sizes.max_w);
				GUI_ASSERT(bh->y1 + bh->h < gui_sizes.max_h);
			}
		}

		if (win->lh_ptr != NULL)
		{
			for (uint8_t i = 0; i < win->lh_count; i++)
			{
				label_t * lh = & win->lh_ptr[i];
				lh->x += shift_x;
				lh->y += shift_y;
				GUI_ASSERT(lh->x + get_label_width(lh) < gui_sizes.max_w);
				GUI_ASSERT(lh->y + get_label_height(lh) < gui_sizes.max_h);
			}
		}

		if (win->tf_ptr != NULL)
		{
			for (uint8_t i = 0; i < win->tf_count; i++)
			{
				text_field_t * tf = & win->tf_ptr[i];
				tf->x1 += shift_x;
				tf->y1 += shift_y;
				GUI_ASSERT(tf->x1 + tf->w < gui_sizes.max_w);
				GUI_ASSERT(tf->y1 + tf->h < gui_sizes.max_h);
			}
		}

		if (win->ta_ptr != NULL)
		{
			for (uint8_t i = 0; i < win->ta_count; i++)
			{
				touch_area_t * ta = & win->ta_ptr[i];
				ta->x1 += shift_x;
				ta->y1 += shift_y;
				GUI_ASSERT(ta->x1 + ta->w < gui_sizes.max_w);
				GUI_ASSERT(ta->y1 + ta->h < gui_sizes.max_h);
			}
		}

		if (win->sh_ptr != NULL)
		{
			for (uint8_t i = 0; i < win->sh_count; i++)
			{
				slider_t * sh = & win->sh_ptr[i];
				sh->x += shift_x;
				sh->y += shift_y;
				GUI_ASSERT(sh->x < gui_sizes.max_w);
				GUI_ASSERT(sh->y < gui_sizes.max_h);
			}
		}
	}

	if (mode == WINDOW_POSITION_FULLSCREEN)
	{
		const window_t * win_main = get_win(WINDOW_MAIN);
		const uint8_t h = win_main->bh_ptr[0].h;

		win->x1 = 0;
		win->y1 = 0;
		win->w = gui_sizes.max_w - 1;
		win->h = gui_sizes.max_h - gui_sizes.footer_height - 1;
	}
	else if (mode == WINDOW_POSITION_MANUAL_POSITION)
	{
		win->x1 = x_start;
		win->y1 = y_start;
		win->w = xmax > title_length ? (xmax + gui_sizes.edge_step * 2) : (title_length + gui_sizes.edge_step * 2);
		win->h = ymax + shift_y + gui_sizes.edge_step;
		if (win->x1 + win->w >= gui_sizes.max_w)
			win->x1 = gui_sizes.max_w - win->w - 1;

		GUI_ASSERT(win->x1 + win->w < gui_sizes.max_w);
		GUI_ASSERT(win->y1 + win->h < gui_sizes.max_h);
	}
	else
	{
		win->w = xmax > title_length ? (xmax + gui_sizes.edge_step * 2) : (title_length + gui_sizes.edge_step * 2);
		win->w = (win->is_close && win->w < title_length + gui_sizes.window_close_button_size * 2) ? (win->w + gui_sizes.window_close_button_size) : win->w;
		win->h = ymax + shift_y + gui_sizes.edge_step;
		win->y1 = align_y - win->h / 2;

		switch (win->align_mode)
		{
		case ALIGN_LEFT_X:
			if (align_left_x - win->w / 2 < 0)
				win->x1 = 0;
			else
				win->x1 = align_left_x - win->w / 2;
			break;

		case ALIGN_RIGHT_X:
			if (align_right_x + win->w / 2 > gui_sizes.max_w)
				win->x1 = gui_sizes.max_w - win->w;
			else
				win->x1 = align_right_x - win->w / 2;
			break;

		case ALIGN_CENTER_X:
		default:
			win->x1 = align_center_x - win->w / 2;
			break;
		}

		GUI_ASSERT(win->x1 + win->w < gui_sizes.max_w);
		GUI_ASSERT(win->y1 + win->h < gui_sizes.max_h);
	}

	if (win->window_id == WINDOW_MAIN)	// для главного окна рисование без отступов
	{
		win->draw_x1 = win->x1;
		win->draw_y1 = win->y1;
		win->draw_x2 = win->x1 + win->w;
		win->draw_y2 = win->y1 + win->h;
	}
	else
	{
		win->draw_x1 = win->x1 + gui_sizes.edge_step;
		win->draw_y1 = win->y1 + gui_sizes.edge_step + (title_length ? gui_sizes.window_title_height : 0);
		win->draw_x2 = win->x1 + win->w - gui_sizes.edge_step;
		win->draw_y2 = win->y1 + win->h - gui_sizes.edge_step;
	}

	if (win->is_moving)
	{
		gui_obj_create("ta_winmove", 0, 0, win->w - gui_sizes.window_close_button_size, gui_sizes.window_title_height, 1);
		touch_area_t * tm = (touch_area_t *) find_gui_obj(TYPE_TOUCH_AREA, win, "ta_winmove");
		tm->visible = VISIBLE;
		tm->state = CANCELLED;
	}

	//GUI_DEBUG_PRINT("%d %d %d %d\n", win->x1, win->y1, win->h, win->w);
	objects_state(win);
}

void window_set_title(const char * text)
{
	window_t * win = get_win(get_parent_window());
	strncpy(win->title, text, NAME_ARRAY_SIZE - 1);
}

void window_set_title_align(align_t align)
{
	window_t * win = get_win(get_parent_window());
	win->title_align = align;
}

void draw_window(window_t * win)
{
    uint16_t x = win->x1;
    uint16_t y = win->y1;
    if (win->window_id == WINDOW_MAIN) return;
    GUI_ASSERT(win->w > 0 || win->h > 0);

    __gui_draw_semitransparent_rect(x, strcmp(win->title, "") ? (y + gui_sizes.window_title_height) : y,
    x + win->w - 1, y + win->h - 1, DEFAULT_ALPHA);

    // вывод заголовка окна
    if (strcmp(win->title, ""))
    {
        uint16_t title_lenght = 0;
#if SDL2_FONTS
        int tw = 0, th = 0;
        gui_sdl2_get_text_size(win->title, gui_sdl2_get_window_title_font(), &tw, &th);
        title_lenght = (uint16_t)tw;
#else
        title_lenght = get_strwidth_prop(win->title, & WINDOW_TITLE_FONTP);
#endif

        uint16_t xt = 0;
        switch(win->title_align)
        {
        case ALIGNMENT_LEFT:
            xt = x + gui_sizes.window_title_indent;
            break;
        case ALIGNMENT_RIGHT:
            xt = x + win->w - title_lenght - gui_sizes.window_title_indent - (win->is_close ? gui_sizes.window_close_button_size : 0);
            break;
        case ALIGNMENT_CENTER:
            xt = x + win->w / 2 - title_lenght / 2;
            break;
        default:
            GUI_DEBUG_PRINT("Title alignment value %d incorrect for window %s\n", win->title_align, win->title);
            GUI_ASSERT(0);
            break;
        }

        __gui_draw_rect(x, y, win->w, gui_sizes.window_title_height, GUI_WINDOWTITLECOLOR, 1);

#if SDL2_FONTS
        gui_sdl2_draw_text(win->title, xt, y + 5, gui_sdl2_get_window_title_font(), GUI_COLOR_BLACK);
#else
        __gui_print_prop(xt, y + 5, win->title, & WINDOW_TITLE_FONTP, GUI_COLOR_BLACK);
#endif
        // ============================================
    }
}

#endif /* WITHTOUCHGUI */
