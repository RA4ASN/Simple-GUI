#ifndef _gui_h
#define _gui_h

#include "gui_user_include.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int gui_initialize (uint16_t screen_w, uint16_t screen_h);
void gui_put_keyb_code(uint8_t kbch);
void gui_set_encoder2_rotate(int16_t rotate);
void gui_update(void);
void gui_user_init(void);
int gui_scale_ui(int v);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _gui_h */
