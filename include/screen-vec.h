#ifndef __SCREEN_VEC_H
#define __SCREEN_VEC_H
	#include <includes.h>

	typedef struct ScreenVecStruct {
		int x, y;
	} ScreenVec;

	#define SCREEN_VEC(_x, _y) \
		((ScreenVec) { (_x), (_y) })

	ScreenVec RectToScreenVecSize(RECT);
	ScreenVec GetWindowSize(HWND);
	void CursorGoTo(ScreenVec);
#endif

