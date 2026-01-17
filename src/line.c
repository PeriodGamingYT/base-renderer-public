#include <base-renderer.h>

#define BRANCHLESS_ABS(_x) \
		(((_x) + ((sizeof(int) * 8) - 1)) ^ ((sizeof(int) * 8) - 1))

typedef struct {
	int endValue;
	int iAdd;
	int jAdd;
	BOOL yLonger;
} LineCompute;

static inline LineCompute ComputeDrawLine(
	ScreenVec start,
	ScreenVec end
) {
	LineCompute result = { 0 };
	int shortDiff = end.y - start.y;
	int longDiff = end.x - start.x;
	if(
		BRANCHLESS_ABS(shortDiff) >
		BRANCHLESS_ABS(longDiff)
	) {
		int tempDiff = shortDiff;
		shortDiff = longDiff;
		longDiff = tempDiff;

		result.yLonger = TRUE;
	}

	result.endValue = longDiff;

	// (branchless) bool --> -1/1
	result.iAdd = ((longDiff >= 0) << 1) - 1;
	longDiff = longDiff * result.iAdd;
	if(longDiff != 0) {
		result.jAdd = (shortDiff << 16) / longDiff;
	}

	return result;
}

void DrawLine(
	ScreenVec start,
	ScreenVec end,
	Pixel color,
	ProgramState *state
) {
	int j = 0;
	LineCompute lineCompute = ComputeDrawLine(start, end);

	// could improve performance by just having an offset to add by every time
	// the line goes vertical/horizontal (depending on lineCompute.yLonger) by
	// 1
	if(lineCompute.yLonger) {
		for(int i = 0; i != lineCompute.endValue; i += lineCompute.iAdd) {
			*PixelAt(
				SCREEN_VEC(start.x + (j >> 16), start.y + i),
				state
			) = color;

			j += lineCompute.jAdd;
		}
	} else {
		for(int i = 0; i != lineCompute.endValue; i += lineCompute.iAdd) {
			*PixelAt(
				SCREEN_VEC(start.x + i, start.y + (j >> 16)),
				state
			) = color;

			j += lineCompute.jAdd;
		}
	}
}

