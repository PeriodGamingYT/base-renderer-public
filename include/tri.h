#ifndef __TRI_H
#define __TRI_H
	#include <line.h>

	typedef struct TriIndicesStruct {
		int a, b, c;
		Pixel albedoColor;

		Pixel outputColor;
	} TriIndices;

	void DrawTri(TriIndices *, ProgramState *);
#endif

