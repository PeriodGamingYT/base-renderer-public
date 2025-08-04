#ifndef __BITMAP_H
#define __BITMAP_H
	#include <program-state.h>

	typedef struct PixelStruct {
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t x; // unused
	} Pixel;

	#define PIXEL(_r, _g, _b) \
		((Pixel) { (_b), (_g), (_r), 0 })

	void UpdateWindowSize(HDC, ScreenVec, ProgramState *);
	void ResizeDIB(ScreenVec, ProgramState *);
	void ClearScreen(ProgramState *);
	Pixel *PixelAt(ScreenVec, ProgramState *);
#endif
