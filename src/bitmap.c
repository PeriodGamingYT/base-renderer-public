#include <render.h>

void UpdateWindowSize(
	HDC deviceContextHandle,
	ScreenVec newSize,
	ProgramState *state
) {

	// this function is SLOW
	SetDIBitsToDevice(
		deviceContextHandle,
		0, 0, state->bitmapSize.x, state->bitmapSize.y,
		0, 0,
		0,
		state->bitmapSize.y,
		&state->bitmapBuffer[0],
		&state->bitmapInfo,
		DIB_RGB_COLORS
	);
}

void FreePage(uint8_t **);
void ResizeDIB(ScreenVec newSize, ProgramState *state) {
	FreePage((uint8_t **)(&state->bitmapBuffer));
	FreePage((uint8_t **)(&state->depthBuffer));

	state->bitmapSize = newSize;

	BITMAPINFOHEADER *header = &state->bitmapInfo.bmiHeader;
	header->biSize = sizeof(BITMAPINFOHEADER);
	header->biWidth = newSize.x;

	// trick to make (0, 0) start at the top-left corner.
	header->biHeight = -newSize.y;
	header->biPlanes = 1;
	header->biBitCount = 32;
	header->biCompression = BI_RGB;

	state->bitmapBuffer = (Pixel *)(InitPage(
		newSize.x * newSize.y * sizeof(Pixel)
	));

	state->depthBuffer = (float *)(InitPage(
		newSize.x * newSize.y * sizeof(float)
	));
}

void ClearScreen(ProgramState *state) {
	memset(
		state->bitmapBuffer,
		0,
		state->bitmapSize.x *
		state->bitmapSize.y *
		sizeof(Pixel)
	);

	memset(
		state->depthBuffer,
		0,
		state->bitmapSize.x *
		state->bitmapSize.y *
		sizeof(float)
	);
}

Pixel *PixelAt(ScreenVec pos, ProgramState *state) {
	return &state->bitmapBuffer[
		(pos.y * state->bitmapSize.x) + pos.x
	];
}

