#ifndef _SIMPLE_SCREEN_H_
#define _SIMPLE_SCREEN_H_

#include <nds.h>
#include "amigascreen.h"

using namespace std;

class Alert;
class SimpleWindow;
class SuperBitmap;

class SimpleScreen : public AmigaScreen {

public:
	SimpleScreen(char* title, FontBase* font = NULL);
	virtual ~SimpleScreen();
	
	SimpleWindow* newWindow(s16 x, s16 y, u16 width, u16 height, char* title, u32 flags);
	SuperBitmap* newSuperBitmap(s16 x, s16 y, u16 width, u16 height, u32 bitmapWidth, u32 bitmapHeight, bool isDecoration);
	Alert* addAlert(s16 x, s16 y, u16 width, u16 height, char* title, char* text);
};

#endif