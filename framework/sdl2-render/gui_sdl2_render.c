// Simple GUI от RA4ASN

#include "gui_user_include.h"

#include "common.h"
#include "ldsp.h"
//#include "gui/framework/gui_events.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <GLES3/gl32.h>
#include "framework/gui_render_queue.h"

void get_cursor_pos(uint16_t * x, uint16_t * y);
uint8_t check_is_mouse_present(void);
void gui_sdl2_walkthrough(void);

SDL_Renderer * renderer = NULL;
SDL_Window * window;
SDL_Texture * mouse_cursor;
SDL_mutex* fb_mutex;
int cursor_width, cursor_height;
extern int global_stop;
static pthread_t render_tid = 0, gui_tid = 0;
int queue_ready = 0;
uint32_t * fb_frame = NULL;

void * gui_sdl2_thread_fn(void * args)
{
	while (!queue_ready)
		usleep(1000);

	gui_initialize();

	while (!global_stop)
	{
#if LIQUIDDSP_PROCESS
		uint16_t x = lwf_get_x();
		uint16_t y = lwf_get_y();
		uint16_t w = lwf_get_w();
		uint16_t h = lwf_get_h();
		uint32_t * d = ldsp_get_ptr();

		if (d)
		{
			RenderCmd draw_cmd = { .type = RQ_CMD_DRAW_PIXELS, .data.draw.raw_pixels = d, .data.draw.x = x,
					.data.draw.y = y, .data.draw.w = w, .data.draw.h = h, };
			render_queue_push(&draw_cmd);
		}
#else
		SDL_LockMutex(fb_mutex);
		if (fb_frame)
		{
			RenderCmd draw_cmd =
			{	.type = RQ_CMD_DRAW_PIXELS, .data.draw.raw_pixels = fb_frame, .data.draw.x = 0,
					.data.draw.y = 0, .data.draw.w = DIM_X, .data.draw.h = DIM_Y,};
			render_queue_push(&draw_cmd);
		}
		SDL_UnlockMutex(fb_mutex);
#endif

		gui_sdl2_walkthrough();

		RenderCmd present_cmd = { .type = RQ_CMD_PRESENT };
		render_queue_push(&present_cmd);

		usleep(16000);
	}

}

int sdl2_print_error(void)
{
	const char* err = SDL_GetError();
	if (err[0])
	{
		printf("[SDL Error] %s\n", err);
		SDL_ClearError();
		return 1;
	}
	else
		return 0;
}

static inline void set_draw_state(SDL_Renderer* r, uint32_t color, uint8_t blend_enabled)
{
	uint8_t a = (color >> 24) & 0xFF;
	uint8_t r_ch = (color >> 16) & 0xFF;
	uint8_t g = (color >> 8) & 0xFF;
	uint8_t b = color & 0xFF;

	if (blend_enabled || a < 255)
		SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
	else
		SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

	SDL_SetRenderDrawColor(r, r_ch, g, b, a);
}

static void draw_rounded_rect(SDL_Renderer* r, uint16_t x, uint16_t y, uint16_t w, uint16_t h,
		uint16_t radius, uint8_t fill)
{
	if (w == 0 || h == 0)
		return;
	uint16_t rad = radius;
	if (rad > w / 2)
		rad = w / 2;
	if (rad > h / 2)
		rad = h / 2;

	if (rad == 0)
	{
		SDL_Rect rect = { x, y, w, h };
		fill ? SDL_RenderFillRect(r, &rect) : SDL_RenderDrawRect(r, &rect);
		return;
	}

	int x1 = x + w - 1;
	int y1 = y + h - 1;
	int cx_tl = x + rad, cy_tl = y + rad;
	int cx_tr = x1 - rad, cy_tr = y + rad;
	int cx_bl = x + rad, cy_bl = y1 - rad;
	int cx_br = x1 - rad, cy_br = y1 - rad;

	if (fill)
	{
		if (w > 2 * rad)
		{
			SDL_Rect mid =
			{ x + rad, y, w - 2 * rad, h };
			SDL_RenderFillRect(r, &mid);
		}
		if (h > 2 * rad)
		{
			SDL_Rect left =
			{ x, y + rad, rad, h - 2 * rad };
			SDL_Rect right =
			{ x1 - rad + 1, y + rad, rad, h - 2 * rad };
			SDL_RenderFillRect(r, &left);
			SDL_RenderFillRect(r, &right);
		}

#define FILL_QUARTER(cx, cy, sx, sy) \
        do { \
            for (int dy = 0; dy <= rad; dy++) { \
                int dx = (int)sqrt((double)(rad * rad - dy * dy)); \
                for (int dx2 = 0; dx2 <= dx; dx2++) { \
                    SDL_RenderDrawPoint(r, cx + sx * dx2, cy + sy * dy); \
                } \
            } \
        } while(0)

		FILL_QUARTER(cx_tl, cy_tl, -1, -1);
		FILL_QUARTER(cx_tr, cy_tr, 1, -1);
		FILL_QUARTER(cx_bl, cy_bl, -1, 1);
		FILL_QUARTER(cx_br, cy_br, 1, 1);
#undef FILL_QUARTER
	}
	else
	{
		SDL_RenderDrawLine(r, x + rad, y, x1 - rad, y);
		SDL_RenderDrawLine(r, x + rad, y1, x1 - rad, y1);
		SDL_RenderDrawLine(r, x, y + rad, x, y1 - rad);
		SDL_RenderDrawLine(r, x1, y + rad, x1, y1 - rad);

		// Ограничиваем сегменты для производительности
		const int seg = (rad > 32) ? 32 : rad;
		SDL_Point pts[4 * (seg + 1)];
		int idx = 0;
		double step = (M_PI / 2.0) / seg;

		for (int i = 0; i <= seg; i++)
		{
			double t = i * step;
			pts[idx++] = (SDL_Point) { cx_tl - (int) (rad * cos(t)), cy_tl - (int) (rad * sin(t)) };
			pts[idx++] = (SDL_Point) { cx_tr + (int) (rad * cos(t)), cy_tr - (int) (rad * sin(t)) };
			pts[idx++] = (SDL_Point) { cx_br + (int) (rad * cos(t)), cy_br + (int) (rad * sin(t)) };
			pts[idx++] = (SDL_Point) { cx_bl - (int) (rad * cos(t)), cy_bl + (int) (rad * sin(t)) };
		}
		SDL_RenderDrawPoints(r, pts, idx);
	}
}

static int sdl2_render_init(void)
{
	//	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Failed to initialize SDL: %s\n", SDL_GetError());
		return 0;
	}

	SDL_DisplayMode display_mode;
	SDL_GetCurrentDisplayMode(0, &display_mode);

#if ! X11
	// Установить атрибуты для настройки аппаратного ускорения графики
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
#endif /* ! X11 */

	SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

	window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED,
	SDL_WINDOWPOS_CENTERED, DIM_X, DIM_Y, SDL_WINDOW_FULLSCREEN_DESKTOP
#if ! X11
			| SDL_WINDOW_OPENGL
#endif /* ! X11 */
			);

	sdl2_print_error();

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer)
	{
		printf("Failed to create renderer: %s\n", SDL_GetError());
		sdl2_render_close();
		return 0;
	}

	// Get OpenGL version information
	const char * glVersion = (const char *) glGetString(GL_VERSION);
	const char * glRenderer = (const char *) glGetString(GL_RENDERER);
	printf("OpenGL Version: %s\n", glVersion);
	printf("OpenGL Renderer: %s\n", glRenderer);

	SDL_RendererInfo info;
	if (SDL_GetRendererInfo(renderer, &info) == 0)
	{
		printf("Renderer name: %s\n", info.name);

		if (info.flags & SDL_RENDERER_ACCELERATED)
			printf("Hardware accelerated\n");
		else
			printf("Software rendering\n");
	}

	if ((display_mode.w > DIM_X) && (display_mode.h > DIM_Y))
	{
		float d_x = (float) display_mode.w / DIM_X;
		float d_y = (float) display_mode.h / DIM_Y;
		SDL_RenderSetScale(renderer, d_x, d_y); // масштабирование до размеров экрана
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); // Antialiasing для масштабированных объектов
	}

#if MOUSE_EVDEV
	int imgFlags = IMG_INIT_PNG;
	if (! (IMG_Init(imgFlags) & imgFlags))
	{
		printf("Failed to initialize SDL_image: %s\n", IMG_GetError());
		SDL_Quit();
		return 0;
	}

	SDL_Surface * surface = IMG_Load(MOUSE_CURSOR_PATH);
	if (! surface)
	{
		printf("Failed to load image: %s\n", IMG_GetError());
		return 0;
	}

	mouse_cursor = SDL_CreateTextureFromSurface(renderer, surface);
	if (! mouse_cursor)
	{
		printf("Failed to create texture: %s\n", SDL_GetError());
	}

	SDL_QueryTexture(mouse_cursor, NULL, NULL, & cursor_width, & cursor_height);

	SDL_FreeSurface(surface);
#endif /* MOUSE_EVDEV */

#if GUI_EXTERNAL_FONTS
	if (TTF_Init() == -1)
	{
		printf("TTF init error\n");
		return 0;
	}
#endif /* GUI_EXTERNAL_FONTS */
}

static void parse_cmd(RenderCmd cmd)
{
	switch (cmd.type)
	{
	case RQ_CMD_SET_TARGET:
	{
		if (cmd.data.target != NULL)
		{
			if (*cmd.data.target == NULL)
				break; // Защита от NULL-текстуры
			SDL_SetRenderTarget(renderer, *cmd.data.target);
		}
		else
			SDL_SetRenderTarget(renderer, NULL);
		break;
	}

	case RQ_CMD_COPY_TEXTURE:
	{
		if (cmd.data.copy.src != NULL)
		{
			if (*cmd.data.copy.src == NULL)
				break; // Защита от NULL-текстуры

			SDL_Rect dst = { cmd.data.copy.dst_x, cmd.data.copy.dst_y,
					cmd.data.copy.tex_w, cmd.data.copy.tex_h };
			SDL_RenderCopy(renderer, *cmd.data.copy.src, NULL, & dst);
		}

		break;
	}

	case RQ_CMD_DRAW_RECT:
	{
		set_draw_state(renderer, cmd.color, cmd.blend_enabled);
		SDL_Rect rect =
		{ cmd.data.rect.x, cmd.data.rect.y, cmd.data.rect.w, cmd.data.rect.h };
		cmd.fill ? SDL_RenderFillRect(renderer, &rect) : SDL_RenderDrawRect(renderer, &rect);
		break;
	}

	case RQ_CMD_DRAW_ROUNDED_RECT:
	{
		set_draw_state(renderer, cmd.color, cmd.blend_enabled);
		draw_rounded_rect(renderer, cmd.data.rounded_rect.x, cmd.data.rounded_rect.y,
				cmd.data.rounded_rect.w, cmd.data.rounded_rect.h, cmd.data.rounded_rect.radius,
				cmd.fill);
		break;
	}

	case RQ_CMD_DRAW_LINE:
	{
		set_draw_state(renderer, cmd.color, cmd.blend_enabled);
		SDL_RenderDrawLine(renderer, cmd.data.line.x1, cmd.data.line.y1, cmd.data.line.x2,
				cmd.data.line.y2);
		break;
	}

	case RQ_CMD_DRAW_POINT:
	{
		set_draw_state(renderer, cmd.color, cmd.blend_enabled);
		SDL_RenderDrawPoint(renderer, cmd.data.point.x, cmd.data.point.y);
		break;
	}

	case RQ_CMD_DRAW_SEMITRANSPARENT_RECT:
	{
		set_draw_state(renderer, cmd.color, 1);
		int w = cmd.data.semitransparent_rect.x2 - cmd.data.semitransparent_rect.x1;
		int h = cmd.data.semitransparent_rect.y2 - cmd.data.semitransparent_rect.y1;
		SDL_Rect rect_st =
		{ cmd.data.semitransparent_rect.x1, cmd.data.semitransparent_rect.y1, w, h };
		SDL_RenderFillRect(renderer, &rect_st);
		break;
	}

	case RQ_CMD_CREATE_TEXTURE:
	{
		SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
				SDL_TEXTUREACCESS_TARGET, cmd.data.create.w, cmd.data.create.h);
		if (tex)
		{
			SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
			SDL_SetTextureScaleMode(tex, SDL_ScaleModeBest);
			*cmd.data.create.out_tex = tex;
		}
		else
		{
			*cmd.data.create.out_tex = NULL;
		}
		break;
	}

	case RQ_CMD_DRAW_PIXELS:
	{
		SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
				SDL_TEXTUREACCESS_STATIC, cmd.data.draw.w, cmd.data.draw.h);
		SDL_UpdateTexture(tex, NULL, cmd.data.draw.raw_pixels, cmd.data.draw.w * 4);
		SDL_Rect dst =
		{ cmd.data.draw.x, cmd.data.draw.y, cmd.data.draw.w, cmd.data.draw.h };
		SDL_RenderCopy(renderer, tex, NULL, &dst);
		SDL_DestroyTexture(tex);
		break;
	}

	case RQ_CMD_DESTROY_TEXTURE:
	{
		if (cmd.data.destroy.tex)
			SDL_DestroyTexture(cmd.data.destroy.tex);
		if (cmd.data.destroy.ptr)
			free(cmd.data.destroy.ptr);
		break;
	}

	case RQ_CMD_PRESENT:
		SDL_RenderPresent(renderer);
		break;

	case RQ_CMD_EXIT:
		return;
	}
}

void * sdl2_render_thread_fn(void * args)
{
	sdl2_render_init();
	render_queue_init();

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

	queue_ready = 1;

	RenderCmd cmd;
	printf("[RenderThread] Started\n");

	// Бесконечный цикл обработки очереди
	while (render_queue_pop(&cmd) == 0 || global_stop)
	{
		parse_cmd(cmd);
		sdl2_print_error();
	}

	printf("[RenderThread] Exiting\n");
	sdl2_render_close();
}

void gui_sdl2_init(void)
{
	fb_mutex = SDL_CreateMutex();
	fb_frame = calloc(DIM_X * DIM_Y, sizeof(uint32_t));
	ASSERT(fb_frame);

	linux_create_thread(&render_tid, sdl2_render_thread_fn, 50, 1);
	linux_create_thread(&gui_tid, gui_sdl2_thread_fn, 50, 2);
}

void sdl2_render_close(void)
{
	render_queue_signal_exit();
	linux_cancel_thread(render_tid);
	render_queue_destroy();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void sdl2_render_update(uintptr_t frame)
{
	if (!queue_ready)
		return;

	SDL_LockMutex(fb_mutex);
	memcpy(fb_frame, (uint32_t*) frame, DIM_X * DIM_Y * sizeof(uint32_t));
	SDL_UnlockMutex(fb_mutex);
}

#if SDL2_EVENTS

pthread_t sdl2events_t;
int mouse_x = 0, mouse_y = 0, press = 0;

int get_mouse_move(uint_fast16_t * x, uint_fast16_t * y)
{
	* x = mouse_x;
#if defined (TSC_EVDEV_RAWX)
	* x = normalize(* x, 0, TSC_EVDEV_RAWX, DIM_X - 1);
#endif /* defined (TSC_EVDEV_RAWX)*/

	* y = mouse_y;
#if defined (TSC_EVDEV_RAWY)
	* y = normalize(* y, 0, TSC_EVDEV_RAWY, DIM_Y - 1);
#endif /* defined (TSC_EVDEV_RAWY) */

	return press;
}

void * sdl2_events_thread(void * args)
{
	SDL_Event e;

	while(! global_stop)
	{
		while (SDL_PollEvent(& e) != 0)
		{
			if (e.type == SDL_MOUSEMOTION)
			{
				mouse_x = e.motion.x;
				mouse_y = e.motion.y;
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
			press = 1;
			else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT)
			press = 0;
			// Обработка кнопок курсора
			else if (e.type == SDL_KEYDOWN)
			{
				if (e.key.keysym.sym == SDLK_LEFT)
					gui_put_event(EVENT_TYPE_CONTROL, CODE_CURSOR_LEFT);
				else if (e.key.keysym.sym == SDLK_RIGHT)
					gui_put_event(EVENT_TYPE_CONTROL, CODE_CURSOR_RIGHT);
				else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER)
					gui_put_event(EVENT_TYPE_CONTROL, CODE_KEY_ENTER);
				else if (e.key.keysym.sym == SDLK_ESCAPE)
					gui_put_event(EVENT_TYPE_CONTROL, CODE_KEY_ESCAPE);
			}
		}

		usleep(5000);
	}
}

void sdl2_events_start(void)
{
	linux_create_thread(& sdl2events_t, sdl2_events_thread, 50, 1);
}

#endif /* SDL2_EVENTS */

