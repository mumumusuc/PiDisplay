//
// Created by mumumusuc on 19-1-22.
//

#ifndef PI_DISPLAY_DISPLAY_PRIV_H
#define PI_DISPLAY_DISPLAY_PRIV_H

#include "display.h"

// constructor & destructor
Display *new_Display();

void init_Base(Display *display, void *, fpDestroy);

void init_Display(Display *, DisplayOps *, DisplayInfo *);

void del_Display(Display *);
// end constructor

#endif //PI_DISPLAY_DISPLAY_PRIV_H
