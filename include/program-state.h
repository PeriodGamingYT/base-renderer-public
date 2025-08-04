#ifndef __USER_DATA_H
#define __USER_DATA_H
	#include <vec.h>

	uint8_t *InitPage(int);
	void FreePage(uint8_t **);

	typedef struct ArenaStruct {
		uint8_t *memory;
		int bytes;
		int size;
		int cap;
	} Arena;

	Arena InitArena(int, int);
	uint8_t *AppendToArena(Arena *, int);
	void FreeArena(Arena *);

	typedef BOOL Keys[256];
	typedef struct ProgramStateStruct {
		BOOL isRunning;
		int frameCount;

		// doesn't take the average frame time, time is
		// in seconds
		float frameTime;
		float totalFrameTime;

		// mouse buttons are stored in here because
		// Win32 is kinda stupid
		Keys pastKeys;
		Keys keys;

		ScreenVec mousePos;
		ScreenVec pastMousePos;
		ScreenVec beforeLockedMousePos;
		ScreenVec mouseMove;
		BOOL mouseLocked;

		BITMAPINFO bitmapInfo;
		struct PixelStruct *bitmapBuffer;
		float *depthBuffer;
		ScreenVec bitmapSize;

		Arena meshes; // type: Mesh
		Arena triIndices; // type: TriIndices

		// meshPoints -> worldPoints -> projPoints
		// meshPoints -> worldPoints only when needed
		// worldPoints -> projPoints every frame
		Arena meshPoints; // type: Vec
		Vec *worldPoints; // uses meshPoints size and cap
		Vec *cameraPoints; // uses meshPoints size and cap

		// to be split up into multiple light types once ComputeLight
		// is made into ComputeLights with simd (if it's bad enough)
		Arena lights; // type: Light

		Camera camera;
	} ProgramState;

	// BOOL IsKeyReleased(ProgramState *, int);
	// BOOL IsKeyPressed(ProgramState *, int);
	BOOL IsKeyDown(ProgramState *, int);
#endif

