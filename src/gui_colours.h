#ifndef _GUI_COLOURS_H_
#define _GUI_COLOURS_H_

#include "common_irrlicht.h"

#define GUI_BG_TOP		video::SColor(128,0,0,0)
#define GUI_BG_BTM		video::SColor(128,0,0,0)
#define GUI_BORDER		video::SColor(0,0,0,0)

#define GUI_BG_DEATH		video::SColor(180,50,0,0)

#define GUI_BG_TT		video::SColor(200,70,130,30)
#define GUI_TT_TEXT		video::SColor(255,255,255,255)

#define GUI_INV_BORDER		video::SColor(128,150,150,150)
#define GUI_INV_HIGHLIGHT	video::SColor(240,170,30,30)
#define GUI_INV_BG		video::SColor(128,60,60,60)

#define GUI_3D_HIGH_LIGHT	video::SColor(128,70,130,30)
#define GUI_HIGH_LIGHT		video::SColor(255,70,100,50)
#define GUI_HIGH_LIGHT_TEXT	video::SColor(255,255,255,255)
#define GUI_BUTTON_TEXT		video::SColor(255,255,255,255)

#if (IRRLICHT_VERSION_MAJOR >= 1 && IRRLICHT_VERSION_MINOR >= 8) || IRRLICHT_VERSION_MAJOR >= 2
// Irrlicht 1.8 input colours
#define GUI_3D_FACE		video::SColor(255,70,130,30)
#define GUI_3D_SHADOW		video::SColor(255,70,130,30)
#define GUI_3D_DARK_SHADOW	video::SColor(255,70,130,30)
#define GUI_EDITABLE		video::SColor(128,70,130,30)
#define GUI_FOCUSED_EDITABLE	video::SColor(192,70,130,30)

#else

#define GUI_3D_SHADOW		video::SColor(255,70,130,30)

#endif

#endif
