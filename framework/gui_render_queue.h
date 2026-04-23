#ifndef GUI_RENDER_QUEUE_H
#define GUI_RENDER_QUEUE_H

#include <SDL2/SDL.h>
#include <stdint.h>

#define RENDER_QUEUE_CAPACITY 2048

typedef enum {
	RQ_CMD_DRAW_RECT,
	RQ_CMD_DRAW_ROUNDED_RECT,
	RQ_CMD_DRAW_LINE,
	RQ_CMD_DRAW_POINT,
	RQ_CMD_DRAW_SEMITRANSPARENT_RECT,
	RQ_CMD_DRAW_PIXELS,
	RQ_CMD_SET_TARGET,
	RQ_CMD_CLEAR_TARGET,
	RQ_CMD_COPY_TEXTURE,
	RQ_CMD_CREATE_TEXTURE,
	RQ_CMD_UPDATE_TEXTURE,
	RQ_CMD_DESTROY_TEXTURE,
	RQ_CMD_PRESENT,
	RQ_CMD_EXIT,
} RenderCmdType;

typedef struct {
	RenderCmdType type;
	uint32_t color;			// 0xAARRGGBB
	uint8_t fill;           // 1 - заливка, 0 - контур
	uint8_t blend_enabled;  // 1 - SDL_BLENDMODE_BLEND, 0 - NONE

	union {
		// RQ_CMD_DRAW_RECT
		struct {
			uint16_t x, y, w, h;
		} rect;

		// RQ_CMD_DRAW_ROUNDED_RECT
		struct {
			uint16_t x, y, w, h, radius;
		} rounded_rect;

		// RQ_CMD_DRAW_LINE
		struct {
			uint16_t x1, y1, x2, y2;
		} line;

		// RQ_CMD_DRAW_POINT
		struct {
			uint16_t x, y;
		} point;

		// RQ_CMD_DRAW_SEMITRANSPARENT_RECT
		struct {
			uint16_t x1, y1, x2, y2;
			uint8_t alpha;
		} semitransparent_rect;

		// RQ_CMD_SET_TARGET
		SDL_Texture** target;

		// RQ_CMD_CREATE_TEXTURE
		struct {
			SDL_Texture** out_tex;
			uint16_t w, h;
		} create;

		// RQ_CMD_DESTROY_TEXTURE
		struct {
			SDL_Texture* tex;
			void* ptr;
		} destroy;

		// RQ_CMD_DRAW_PIXELS
		struct {
			uint32_t* raw_pixels;
			uint16_t x, y, w, h;
		} draw;

		//RQ_CMD_COPY_TEXTURE
		struct {
			SDL_Texture** src;
			uint16_t dst_x, dst_y, tex_w, tex_h;
		} copy;
	} data;
} RenderCmd;

void render_queue_init(void);
int render_queue_push(const RenderCmd* cmd);
int render_queue_push_batch(const RenderCmd* cmds, uint32_t count);
int render_queue_pop(RenderCmd* out_cmd);
void render_queue_signal_exit(void);
void render_queue_destroy(void);

#endif /* GUI_RENDER_QUEUE_H */
