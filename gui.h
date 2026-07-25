#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED

#include "gui_user_include.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if WITHTOUCHGUI

int gui_initialize (uint16_t screen_w, uint16_t screen_h);
void gui_put_keyb_code(uint8_t kbch);
void gui_set_encoder2_rotate(int16_t rotate);
void gui_update(void);
void gui_user_init(void);
int gui_scale_ui(int v);

#endif /* WITHTOUCHGUI */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GUI_H_INCLUDED */
