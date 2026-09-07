// Simple GUI от RA4ASN
#include "gui_user_include.h"

#if SIMPLE_GUI && GUI_SDL2_INPUT

#include "gui_sdl2_input.h"
#include "gui_system.h"      // gui_get_max_w / gui_get_max_h
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <linux/input.h>

static struct {
    int      touching;        // 1 = палец на экране прямо сейчас
    int      have_finger;     // 1 = активен тач-палец (ставится в DOWN, сбрасывается в UP)
    SDL_FingerID finger_id;   // id последнего DOWN
    int      pressed_this_poll;
    uint16_t x, y;            // текущие координаты в системе GUI
} inp;

/* координаты мыши (window pixels) -> логические координаты GUI */
static void set_from_window(int wx, int wy)
{
    float sx = 1.0f, sy = 1.0f;
    SDL_Renderer *r = sdl2_get_renderer();
    if (r) SDL_RenderGetScale(r, &sx, &sy);
    if (sx <= 0.0f) sx = 1.0f;
    if (sy <= 0.0f) sy = 1.0f;
    inp.x = (uint16_t)((float)wx / sx);
    inp.y = (uint16_t)((float)wy / sy);
}

/* координаты тача [0,1] -> логические координаты GUI */
static void set_from_touch(float nx, float ny)
{
    inp.x = (uint16_t)(nx * (float)gui_get_max_w());
    inp.y = (uint16_t)(ny * (float)gui_get_max_h());
}

void gui_sdl2_input_init(void)
{
    memset(&inp, 0, sizeof(inp));
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
}

void gui_sdl2_input_poll(void)
{
    inp.pressed_this_poll = 0;

    SDL_Event ev;
    while (SDL_PollEvent(&ev))
    {
        switch (ev.type)
        {
        case SDL_FINGERDOWN:
        	if (inp.have_finger) break;
            inp.have_finger     = 1;
            inp.finger_id       = ev.tfinger.fingerId;
            inp.touching        = 1;
            inp.pressed_this_poll = 1;
            set_from_touch(ev.tfinger.x, ev.tfinger.y);

            break;

        case SDL_FINGERMOTION:
            set_from_touch(ev.tfinger.x, ev.tfinger.y);
            if (inp.have_finger) inp.touching = 1;

            break;

        case SDL_FINGERUP:
            if (ev.tfinger.fingerId == inp.finger_id) {
                inp.have_finger = 0;
                inp.touching    = 0;
            }
            break;

        default:
            break;
        }
    }
}

int gui_sdl2_input_get(uint16_t *x, uint16_t *y)
{
    if (x) *x = inp.x;
    if (y) *y = inp.y;
    int active = inp.touching || inp.pressed_this_poll;
    inp.pressed_this_poll = 0;
    return active;
}

#endif /* SIMPLE_GUI && GUI_SDL2_INPUT */
