// Simple GUI от RA4ASN
#ifndef _GUI_INCLUDES_H
#define _GUI_INCLUDES_H

#include "gui_user_include.h"
#if SIMPLE_GUI

#ifdef GUI_USE_PORT
    #include "gui_user_port.h"
#else
    #include "SDL2_API/gui_sdl2_api.h"
#endif /* GUI_USE_PORT */

#include "gui_settings.h"
#include "gui_animation.h"
#include "gui_structs.h"
#include "gui_system.h"
#include "gui_windows.h"
#include "gui_objects.h"
#include "gui_utils.h"
#include "gui_user.h"

#endif /* SIMPLE_GUI */
#endif /* _GUI_INCLUDES_H */
