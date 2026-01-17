#include <base-renderer.h>

// https://raw.githubusercontent.com/OneLoneCoder/Javidx9/refs/heads/master/ConsoleGameEngine/BiggerProjects/Engine3D/OneLoneCoder_olcEngine3D_Part4.cpp
// ^-- started from there (some of the concepts in this code are still in here) but there are many things
// that differ from that starting reference of code
typedef struct TriPointStruct {
	int x, y;

	// i don't need z if i have w (which is inversed)
	float w;

	// used only for culling
	Vec cameraPoint;
} TriPoint;

typedef struct TriLinesStateStruct {
	float xStep, wStep;
	Pixel color; __m256i simdColor;

	float *startDepthLine;
	Pixel *startColorLine;
	ScreenVec size;
} TriLinesState;

static inline __m128i BlendEpi8(__m128i a, __m128i b, __m128i mask) {
	return _mm_or_si128(
		_mm_and_si128(a, _mm_xor_si128(mask, _mm_set1_epi8(0xff))),
		_mm_and_si128(b, mask)
	);
}

static inline __m128i Si256HighLane(__m256i x) {
	__m256i *ptr = &x;
	return _mm_loadu_si128(
		(__m128i *)((uint8_t *)(ptr) + 16)
	);
}

static inline void DrawTriLines(
	TriPoint startTriPoint, TriPoint endTriPoint,
	TriPoint innerStartTriPoint, TriPoint innerEndTriPoint,
	TriLinesState *state
) {
	float diffXShort = endTriPoint.x - startTriPoint.x;
	float diffYShort = endTriPoint.y - startTriPoint.y;
	float diffWShort = endTriPoint.w - startTriPoint.w;
	if(diffYShort < 1) { return; }

	float shortXStep = diffXShort / diffYShort;
	float shortWStep = diffWShort / diffYShort;

	float startYDiff = startTriPoint.y - innerStartTriPoint.y;
	float endYDiff = startTriPoint.y - innerEndTriPoint.y;

	float startX = innerStartTriPoint.x + (startYDiff * shortXStep);
	float endX = innerEndTriPoint.x + (endYDiff * state->xStep);

	float startW = innerStartTriPoint.w + (startYDiff * shortWStep);
	float endW = innerEndTriPoint.w + (endYDiff * state->wStep);

	if(startTriPoint.y < 0) {
		float increaseY = 0 - startTriPoint.y;
		diffYShort += increaseY;
		startX += increaseY * shortXStep;
		endX += increaseY * state->xStep;
		startW += increaseY * shortWStep;
		endW += increaseY * state->wStep;

		startTriPoint.y = 0;
	}

	if(endTriPoint.y >= state->size.y - 2) {
		endTriPoint.y = state->size.y - 2;
	}

	__m256 simdGenericWStep = _mm256_set1_ps(8);
	__m256 simdGenericW = _mm256_setr_ps(0, 1, 2, 3, 4, 5, 6, 7);
	for(int y = startTriPoint.y; y <= endTriPoint.y; y++) {
		int currentStartX = startX;
		int currentEndX = endX;

		float currentStartW = startW;
		float currentEndW = endW;
		if(startX > endX) {
			currentStartX = endX;
			currentEndX = startX;

			currentStartW = endW;
			currentEndW = startW;
		}

		float diffX = currentEndX - currentStartX;
		float startWStep = currentStartW / diffX;
		float endWStep = currentEndW / diffX;
		currentEndW = 0;

		if(currentStartX < 0) {
			int increaseX = 0 - currentStartX;
			currentStartW -= increaseX * startWStep;
			currentEndW += increaseX * endWStep;
			currentStartX = 0;
		}

		__m256 simdStartWStep = _mm256_mul_ps(
			_mm256_set1_ps(startWStep),
			simdGenericWStep
		);

		__m256 simdStartW = _mm256_sub_ps(
			_mm256_set1_ps(currentStartW),
			_mm256_mul_ps(
				_mm256_set1_ps(startWStep),
				simdGenericW
			)
		);

		__m256 simdEndWStep = _mm256_mul_ps(
			_mm256_set1_ps(endWStep),
			simdGenericWStep
		);

		__m256 simdEndW = _mm256_add_ps(
			_mm256_set1_ps(currentEndW),
			_mm256_mul_ps(
				_mm256_set1_ps(endWStep),
				simdGenericW
			)
		);

		if(currentEndX >= state->size.x - 2)  {
			currentEndX = state->size.x - 2;
		}

		_mm_prefetch((char *)(&state->startDepthLine[currentStartX]), _MM_HINT_T0);
		_mm_prefetch((char *)(&state->startColorLine[currentStartX]), _MM_HINT_T0);

		int x = currentStartX;
		float *currentSimdDepthPointer = (float *)(&state->startDepthLine[x]);
		__m256i *currentSimdColorPointer = (__m256i *)(&state->startColorLine[x]);
		for(; x <= currentEndX - 8; x += 8) {
			__m256 currentSimdDepth = _mm256_load_ps(currentSimdDepthPointer);
			__m256i currentSimdColor = _mm256_load_si256(currentSimdColorPointer);

			__m256 simdDepth = _mm256_add_ps(simdStartW, simdEndW);
			__m256i mask = _mm256_castps_si256(
				_mm256_cmp_ps(simdDepth, currentSimdDepth, _CMP_GT_OS)
			);

			_mm256_maskstore_ps(
				currentSimdDepthPointer,
				mask,
				simdDepth
			);

			// no good support for AVX integer math, UGH!
			// also _mm_maskstore_epi32,
			// _mm_blendv_epi8, _mm256_extractsi128_si256,
			// _mm256_xor_si256, _mm256_and_si256, and _mm256_or_si256,
			// _mm256_inserti128_si256 are illegal on my machine :(
			union {
				__m256i fullPixel;
				struct { __m128i lowPixel, highPixel; };
			} simdPackedPixel;

			simdPackedPixel.lowPixel = BlendEpi8(
				_mm256_castsi256_si128(currentSimdColor),
				_mm256_castsi256_si128(state->simdColor),
				_mm256_castsi256_si128(mask)
			);

			simdPackedPixel.highPixel = BlendEpi8(
				Si256HighLane(currentSimdColor),

				// save on a function call to Si256HighLane
				// (state->simdColor is all the same 32-bit Pixel)
				_mm256_castsi256_si128(state->simdColor),
				Si256HighLane(mask)
			);

			_mm256_store_si256(
				currentSimdColorPointer,
				simdPackedPixel.fullPixel
			);

			simdStartW = _mm256_sub_ps(simdStartW, simdStartWStep);
			simdEndW = _mm256_add_ps(simdEndW, simdEndWStep);

			currentSimdDepthPointer += 8;
			currentSimdColorPointer++;
		}

		int stepsToTake = x - currentStartX;
		currentStartW -= startWStep * stepsToTake;
		currentEndW += endWStep * stepsToTake;

		for(; x <= currentEndX + 1; x++) {
			float currentW = currentStartW + currentEndW;
			if(currentW > state->startDepthLine[x]) {
				state->startDepthLine[x] = currentW;
				state->startColorLine[x] = state->color;
			}

			currentStartW -= startWStep;
			currentEndW += endWStep;
		}

		startX += shortXStep;
		endX += state->xStep;

		startW += shortWStep;
		endW += state->wStep;

		state->startDepthLine += state->size.x;
		state->startColorLine += state->size.x;
	}
}

// i should probably make this DrawTris (for simd support) at some point
static void InnerDrawTri(
	TriPoint triPointA,
	TriPoint triPointB,
	TriPoint triPointC,
	Pixel color, ProgramState *state
) {
	if(
		(triPointA.x < 0 && triPointB.x < 0 && triPointC.x < 0) ||
		(triPointA.y < 0 && triPointB.y < 0 && triPointC.y < 0) || (
			triPointA.x >= state->bitmapSize.x &&
			triPointB.x >= state->bitmapSize.x &&
			triPointC.x >= state->bitmapSize.x
		) || (
			triPointA.y >= state->bitmapSize.y &&
			triPointB.y >= state->bitmapSize.y &&
			triPointC.y >= state->bitmapSize.y
		)
	) { return; }

	TriPoint temp = { 0 };
	if(triPointB.y < triPointA.y) {
		temp = triPointA;
		triPointA = triPointB;
		triPointB = temp;
	}

	if(triPointC.y < triPointA.y) {
		temp = triPointA;
		triPointA = triPointC;
		triPointC = temp;
	}

	if(triPointC.y < triPointB.y) {
		temp = triPointB;
		triPointB = triPointC;
		triPointC = temp;
	}

	float diffXLong = triPointC.x - triPointA.x;
	float diffYLong = triPointC.y - triPointA.y;
	float diffWLong = triPointC.w - triPointA.w;
	if(diffYLong == 0) { return; }

	int startYIndex = triPointA.y * state->bitmapSize.x;
	if(triPointA.y < 0) {
		startYIndex = 0;
	}

	TriLinesState triState = {
		.xStep = diffXLong / diffYLong,
		.wStep = diffWLong / diffYLong,

		.color = color,
		.simdColor = _mm256_set1_epi32(
			color.b |
			(color.g << 8) |
			(color.r << 16)
		),

		.startDepthLine = &state->depthBuffer[startYIndex],
		.startColorLine = &state->bitmapBuffer[startYIndex],
		.size = state->bitmapSize
	};

	DrawTriLines(
		triPointA, triPointB,
		triPointA, triPointA,
		&triState
	);

	DrawTriLines(
		triPointB, triPointC,
		triPointB, triPointA,
		&triState
	);
}

static TriPoint CameraPointToTriPoint(Vec cameraPoint, ProgramState *state) {
	Vec projPoint = VecXMat(
		cameraPoint,
		&state->camera.projMat
	);

	return (TriPoint) {
		.x = (projPoint.x + 0.5) * (float)(state->bitmapSize.x),
		.y = (projPoint.y + 0.5) * (float)(state->bitmapSize.y),
		.w = 1.0 / projPoint.w,

		.cameraPoint = cameraPoint,
	};
}

static Vec PointAtZPoint(
	float dividingZPoint,
	Vec insidePoint,
	Vec outsidePoint
) {
	float zDiff = insidePoint.z - outsidePoint.z;
	float linearInterpTime = (insidePoint.z - dividingZPoint) / zDiff;

	// testing showed that this was the wrong way around
	linearInterpTime = 1.0 - linearInterpTime;
	#define LERP(_start, _end, _time) \
		((_start) + (((_end) - (_start)) * (_time)))

		return VEC(
			LERP(outsidePoint.x, insidePoint.x, linearInterpTime),
			LERP(outsidePoint.y, insidePoint.y, linearInterpTime),
			LERP(outsidePoint.z, insidePoint.z, linearInterpTime)
		);
	#undef LERP
}

typedef struct PointIndicesStruct {
	int tipIndex;
	int otherIndex0;
	int otherIndex1;
} PointIndices;

// if i turn this into SIMD...how? can't just use the multi-array trick
// like i can with the lights (since where the triangles are/aren't visible
// can change every single frame), so what do i do? idk lol.
void DrawTri(TriIndices *triIndices, ProgramState *state) {
	Vec pointA = state->cameraPoints[triIndices->a];
	Vec pointB = state->cameraPoints[triIndices->b];
	Vec pointC = state->cameraPoints[triIndices->c];

	// i won't do zFar because that realistically won't ever
	// happen (maybe in a game like Fractal Block World or a
	// VERY large open world game but still there are better things
	// there than just clipping the zFar)
	float zDividingLine = state->camera.zNear;
	if(
		pointA.z <= zDividingLine &&
		pointB.z <= zDividingLine &&
		pointC.z <= zDividingLine
	) {
		return;
	}

	Vec triNormal = VecNormal(VecCross(
		VEC_SUB(pointB, pointA),
		VEC_SUB(pointC, pointA)
	));

	if(VecDot(triNormal, pointA) >= 0) { return; }

	// if you are to try to understand this, whiteboard and marker
	// ARE MUSTS!!
	if(
		pointA.z <= zDividingLine ||
		pointB.z <= zDividingLine ||
		pointC.z <= zDividingLine
	) {
		Vec points[3] = {
			pointA, pointB, pointC
		};

		// this will preserve triangle winding
		PointIndices allPointIndices[3] = {
			{ .tipIndex = 0, 1, 2 },
			{ .tipIndex = 1, 0, 2 },
			{ .tipIndex = 2, 0, 1 }
		};

		int insidePointIndex = 0;
		int insidePointsSize = 0;
		int outsidePointIndex = 0;
		int outsidePointsSize = 0;
		for(int i = 0; i < ARRAY_SIZE(points); i++) {
			if(points[i].z <= zDividingLine) {
				outsidePointsSize++;
				outsidePointIndex = i;
			} else {
				insidePointsSize++;
				insidePointIndex = i;
			}
		}

		if(outsidePointsSize == 2) {
			TriPoint triPoints[3] = { 0 };
			PointIndices *pointIndices = &allPointIndices[insidePointIndex];
			triPoints[pointIndices->tipIndex] = CameraPointToTriPoint(points[pointIndices->tipIndex], state);
			triPoints[pointIndices->otherIndex0] = CameraPointToTriPoint(PointAtZPoint(
				zDividingLine,
				points[pointIndices->tipIndex],
				points[pointIndices->otherIndex0]
			), state);

			triPoints[pointIndices->otherIndex1] = CameraPointToTriPoint(PointAtZPoint(
				zDividingLine,
				points[pointIndices->tipIndex],
				points[pointIndices->otherIndex1]
			), state);

			InnerDrawTri(
				triPoints[0],
				triPoints[1],
				triPoints[2],

				triIndices->outputColor, state
			);

			return;
		} else {
			PointIndices *pointIndices = &allPointIndices[outsidePointIndex];
			TriPoint otherTriPoint1 = CameraPointToTriPoint(points[pointIndices->otherIndex1], state);
			TriPoint otherTriPoint0Clipped = CameraPointToTriPoint(PointAtZPoint(
				zDividingLine,
				points[pointIndices->otherIndex0],
				points[pointIndices->tipIndex]
			), state);

			TriPoint triPoints0[3] = { 0 };
			triPoints0[pointIndices->tipIndex] = otherTriPoint0Clipped;
			triPoints0[pointIndices->otherIndex0] = CameraPointToTriPoint(points[pointIndices->otherIndex0], state);
			triPoints0[pointIndices->otherIndex1] = otherTriPoint1;
			InnerDrawTri(
				triPoints0[0],
				triPoints0[1],
				triPoints0[2],

				triIndices->outputColor, state
			);

			TriPoint triPoints1[3] = { 0 };
			triPoints1[pointIndices->tipIndex] = CameraPointToTriPoint(PointAtZPoint(
				zDividingLine,
				points[pointIndices->otherIndex1],
				points[pointIndices->tipIndex]
			), state);

			triPoints1[pointIndices->otherIndex0] = otherTriPoint0Clipped;
			triPoints1[pointIndices->otherIndex1] = otherTriPoint1;
			InnerDrawTri(
				triPoints1[0],
				triPoints1[1],
				triPoints1[2],

				triIndices->outputColor, state
			);

			return;
		}

		// shouldn't get here
		return;
	}

	InnerDrawTri(
		CameraPointToTriPoint(pointA, state),
		CameraPointToTriPoint(pointB, state),
		CameraPointToTriPoint(pointC, state),

		triIndices->outputColor, state
	);
}
