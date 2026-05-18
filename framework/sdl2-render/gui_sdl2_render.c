// Simple GUI от RA4ASN

#include "gui_user_include.h"

#if WITHTOUCHGUI && ! GUI_USEPORT

#include "common.h"
#include "ldsp.h"
//#include "gui/framework/gui_events.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <GLES3/gl32.h>
#include "framework/gui_render_queue.h"
#include "gui_sdl2_api.h"

#define TARGET_FPS 30

void get_cursor_pos(uint16_t * x, uint16_t * y);
uint8_t check_is_mouse_present(void);
void gui_sdl2_walkthrough(void);

SDL_Renderer * renderer = NULL;
SDL_Window * window;
SDL_Texture * screen_tex = NULL;
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

	uint64_t freq = SDL_GetPerformanceFrequency();
	uint64_t target_ticks_per_frame = freq / TARGET_FPS;

	gui_initialize();

	while (!global_stop)
	{
		Uint64 frame_start = SDL_GetPerformanceCounter();

#if LIQUIDDSP_PROCESS
		uint16_t x, y, w, h;
		lfw_get_dims(&x, &y, &w, &h);
		uint32_t * ldsp_frame = ldsp_get_frame();

		if (ldsp_frame)
		{
			RENDER_BATCH_DECL();
			RENDER_BATCH_ADD(.type = RQ_CMD_CLEAR);
			RENDER_BATCH_ADD(.type = RQ_CMD_DRAW_PIXELS, .data.draw = { ldsp_frame, x, y, w, h });
			RENDER_BATCH_FINALIZE();
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

		RenderCmd present_cmd = { .type = RQ_CMD_FINALIZE };
		render_queue_push(& present_cmd);

		uint64_t frame_end = SDL_GetPerformanceCounter();
		uint64_t elapsed_ticks = frame_end - frame_start;

		// Расчет и применение задержки
		// Если цикл выполнился быстрее, чем нужно для TARGET_FPS, ждем остаток времени
		if (elapsed_ticks < target_ticks_per_frame)
		{
			uint64_t remaining_ticks = target_ticks_per_frame - elapsed_ticks;
			// Конвертируем тики процессора в микросекунды для usleep
			// Формула: (оставшиеся_тики * 1_000_000) / частота_таймера
			uint64_t sleep_us = (remaining_ticks * 1000000) / freq;

			if (sleep_us > 0) usleep(sleep_us);
		}
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

	SDL2_PRINT_ERROR();

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer)
	{
		printf("Failed to create renderer: %s\n", SDL_GetError());
		sdl2_render_close();
		return 0;
	}

	screen_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
					SDL_TEXTUREACCESS_TARGET, DIM_X, DIM_Y);
	if (screen_tex)
	{
		SDL_SetTextureBlendMode(screen_tex, SDL_BLENDMODE_BLEND);
		SDL_SetTextureScaleMode(screen_tex, SDL_ScaleModeBest);
		SDL_SetRenderTarget(renderer, screen_tex);
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
}

static void parse_cmd(RenderCmd cmd)
{
	switch (cmd.type)
	{
	case RQ_CMD_CLEAR:
	{
		__sdl2_set_draw_state(renderer, cmd.color, cmd.blend_enabled);
		SDL_RenderClear(renderer);
		break;
	}

	case RQ_CMD_DRAW_RECT:
	{
		__sdl2_set_draw_state(renderer, cmd.color, cmd.blend_enabled);
		SDL_Rect rect =
		{ cmd.data.rect.x, cmd.data.rect.y, cmd.data.rect.w, cmd.data.rect.h };
		cmd.fill ? SDL_RenderFillRect(renderer, &rect) : SDL_RenderDrawRect(renderer, &rect);
		break;
	}

	case RQ_CMD_DRAW_ROUNDED_RECT:
	{
		__sdl2_set_draw_state(renderer, cmd.color, cmd.blend_enabled);
		__sdl2_draw_rounded_rect(renderer, cmd.data.rounded_rect.x, cmd.data.rounded_rect.y,
				cmd.data.rounded_rect.w, cmd.data.rounded_rect.h, cmd.data.rounded_rect.radius,
				cmd.fill);
		break;
	}

	case RQ_CMD_DRAW_LINE:
	{
		__sdl2_set_draw_state(renderer, cmd.color, cmd.blend_enabled);
		SDL_RenderDrawLine(renderer, cmd.data.line.x1, cmd.data.line.y1, cmd.data.line.x2,
				cmd.data.line.y2);
		break;
	}

	case RQ_CMD_DRAW_POINT:
	{
		__sdl2_set_draw_state(renderer, cmd.color, cmd.blend_enabled);
		SDL_RenderDrawPoint(renderer, cmd.data.point.x, cmd.data.point.y);
		break;
	}

	case RQ_CMD_DRAW_SEMITRANSPARENT_RECT:
	{
		uint32_t c = ((cmd.color) & 0xFFFFFF) | (cmd.data.semitransparent_rect.alpha << 24);
		__sdl2_set_draw_state(renderer, c, 1);
		int w = cmd.data.semitransparent_rect.x2 - cmd.data.semitransparent_rect.x1;
		int h = cmd.data.semitransparent_rect.y2 - cmd.data.semitransparent_rect.y1;
		SDL_Rect rect_st =
		{ cmd.data.semitransparent_rect.x1, cmd.data.semitransparent_rect.y1, w, h };
		SDL_RenderFillRect(renderer, &rect_st);
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

	case RQ_CMD_FINALIZE:
	{
		SDL_SetRenderTarget(renderer, NULL);
		SDL_RenderCopy(renderer, screen_tex, NULL, NULL);
		SDL_RenderPresent(renderer);
		SDL_SetRenderTarget(renderer, screen_tex);

		break;
	}
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
		SDL2_PRINT_ERROR();
	}

	printf("[RenderThread] Exiting\n");
	sdl2_render_close();
}

void gui_sdl2_init(void)
{
	fb_mutex = SDL_CreateMutex();
	fb_frame = calloc(DIM_X * DIM_Y, sizeof(uint32_t));
	ASSERT(fb_frame);

	linux_create_thread(&render_tid, sdl2_render_thread_fn, 10, 2);
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
#endif /* WITHTOUCHGUI && ! GUI_USEPORT */
