// Simple GUI от RA4ASN

#ifndef GUI_SYSTEM_H_INCLUDED
#define GUI_SYSTEM_H_INCLUDED

#include "gui_user_include.h"

#if WITHTOUCHGUI

#include "gui_structs.h"

#define GUI_ASSERT(v) do { if ((v) == 0) { \
    GUI_DEBUG_PRINT("%s(%d): Assert '%s'\n", __FILE__, __LINE__, (# v)); \
    for (;;) ; \
} } while (0)

#define GUI_VERIFY(v) do { if ((v) == 0) { \
    GUI_DEBUG_PRINT("%s(%d): Verify '%s'\n", __FILE__, __LINE__, (# v)); \
    for (;;) ; \
} } while (0)

#define GUI_MEM_ASSERT(v)   do { if (((v) == NULL)) { \
    GUI_DEBUG_PRINT("%s: %d ('%s') - memory allocate failed!\n", __FILE__, __LINE__, (# v)); \
    for (;;) ; \
} } while (0)

void * find_gui_obj(obj_type_t type, window_t * win, const char * name);
void objects_state (window_t * win);
uint8_t get_parent_window(void);
uint8_t get_current_drawing_window(void);
void set_parent_window(uint8_t p);
void get_gui_tracking(int_fast16_t * x, int_fast16_t * y);

uint8_t put_to_wm_queue(window_t * win, wm_message_t message, ...);
wm_message_t get_from_wm_queue(uint8_t win_id, uint8_t * type, int32_t * action, char * name);
void dump_queue(window_t * win);

uint16_t gui_get_max_w(void);
uint16_t gui_get_max_h(void);
uint16_t gui_get_footer_h(void);
uint16_t gui_get_window_draw_width(void);
uint16_t gui_get_window_draw_height(void);
void gui_drawline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, gui_color_t color);
void gui_drawpoint(uint16_t x1, uint16_t y1, gui_color_t color);
void gui_drawrect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, gui_color_t color, uint8_t fill);
void gui_drawrect_rounded(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t radius, gui_color_t color, uint8_t fill);
void gui_drawrect_transparent(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t alpha);
void gui_drawDashedRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t dashLength, gui_color_t color);

void process_gui(void);

#endif /* WITHTOUCHGUI */
#endif /* GUI_USER_H_INCLUDED */
