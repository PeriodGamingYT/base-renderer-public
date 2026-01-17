#ifndef __BASE_RENDERER_H
#define __BASE_RENDERER_H

    //// struct definitions
    typedef struct FileMapStruct FileMap;

    typedef struct ScreenVecStruct ScreenVec;

    typedef struct VecStruct Vec;
    typedef struct QuaternionStruct Quaternion;
    typedef struct CameraRotStruct CameraRot;
    typedef struct CameraStruct Camera;

    typedef struct ArenaStruct Arena;
    typedef struct ProgramStateStruct ProgramState;

    typedef struct PixelStruct Pixel;

    typedef struct TriIndicesStruct TriIndices;

    typedef struct MeshStruct Mesh;

	typedef struct LightsSimdStruct LightsSimd;


    //// base stuff (includes.c)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <winuser.h>

	#pragma comment(lib, "user32.lib")
	#pragma comment(lib, "gdi32.lib")

	#define _USE_MATH_DEFINES
	#include <math.h>
	#include <stdint.h>
	#include <stdarg.h>

	#include <xmmintrin.h>
	#include <emmintrin.h>
	#include <immintrin.h>

	#define ARRAY_SIZE(_array) \
		(sizeof(_array) / sizeof((_array)[0]))

	// don't use this often! also this is an int
	// because old posix habits die hard (-1 is error,
	// 0 is success)
	typedef int Error;

	void ErrorMessageBox(char *);

	typedef struct FileMapStruct {
		HANDLE fileHandle;
		HANDLE fileMapHandle;
		uint8_t *map;

		int size;

		// can't think of a good name for this that is
		// also short :/
		uint8_t *current;

		// one over because the end is checked by doing
		// FileMap.current < FileMap.end
		uint8_t *end;
	} FileMap;

	FileMap OpenFileMap(char *);
	FileMap OpenWriteFileMap(char *, int);
	void CloseFileMap(FileMap *);


    //// screen vectors (screen-vec.c)
    typedef struct ScreenVecStruct {
		int x, y;
	} ScreenVec;

	#define SCREEN_VEC(_x, _y) \
		((ScreenVec) { (_x), (_y) })

	ScreenVec RectToScreenVecSize(RECT);
	ScreenVec GetWindowSize(HWND);
	void CursorGoTo(ScreenVec);


    //// vectors (vec.c)
	typedef struct VecStruct {
		float x, y, z, w;
	} Vec;

	#define VEC(_x, _y, _z) \
		((Vec) { (_x), (_y), (_z), 1 })

	#define VEC_ADD(_a, _b) \
		VEC((_a).x + (_b).x, (_a).y + (_b).y, (_a).z + (_b).z)

	#define VEC_SUB(_a, _b) \
		VEC((_a).x - (_b).x, (_a).y - (_b).y, (_a).z - (_b).z)

	#define VEC_MUL(_a, _b) \
		VEC((_a).x * (_b).x, (_a).y * (_b).y, (_a).z * (_b).z)

	#define VEC_DIV(_a, _b) \
		VEC((_a).x / (_b).x, (_a).y / (_b).y, (_a).z / (_b).z)

	#define VEC_NEG(_vec) \
		VEC(-(_vec).x, -(_vec).y, -(_vec).z)

	Vec VecCross(Vec, Vec);
	float VecDot(Vec, Vec);
	float VecLength(Vec);
	Vec VecNormal(Vec);

	typedef struct QuaternionStruct {
		float x, y, z, w; // w is real
	} Quaternion;

	#define QUATERNION(_x, _y, _z, _w) \
		((Quaternion) { (_x), (_y), (_z), (_w) })

	// from wikipedia
	// ZYX order: yaw, pitch, roll
	float QuaternionLength(Quaternion);
	Quaternion QuaternionNormal(Quaternion);
	Quaternion EulerToQuaternion(Vec);
	Quaternion QuaternionConjugate(Quaternion);

	// NOTE: Mat[row][column|rowInnerIndex], so row-major
	typedef float Mat[4][4];

	// a pointer is needed because msvc (and c, for
	// that matter) is stupid and can't comprehend returning
	// an array (structs are okay, though!) >:(
	void MoveMat(Mat *, Vec);
	void RotMat(Mat *, Quaternion);

	// use these matrices if you want gimbal lock
	void RotXMat(Mat *, float);
	void RotYMat(Mat *, float);
	void RotZMat(Mat *, float);
	void IdentMat(Mat *);
	void TransposeMat(Mat *);
	void MatXMat(Mat *, Mat *, Mat *);

	// w is assumed to be 1 (and as such is not multiplied by its
	// corresponding matrix)
	Vec VecXMat(Vec, Mat *);
	void VecsXMat(Vec *, Vec *, int, Mat *);

	typedef struct CameraRotStruct {
		float yaw;
		float targetY;
	} CameraRot;

	typedef struct CameraStruct {
		float zNear;
		float zFar;
		float fov; // degrees
		Mat projMat;

		Vec pos;

		CameraRot rot;
		Mat cameraMat;

		Mat finalMat;
	} Camera;

	Camera InitCamera(ScreenVec);

	// since fov can change, i will need to implement
	// this (zFar and zNear don't need change)
	void UpdateCameraFov(Camera *, float, ScreenVec);

	#define CAMERA_ROT(_yaw, _targetY) \
		((CameraRot) { _yaw, _targetY })

	void CameraRotOnlyMat(Mat *, CameraRot);
	void LookAtViewMat(Mat *, Vec, Vec, Vec);
	void CameraRotMat(Mat *, CameraRot, Vec);
	void UpdateCamera(Camera *, Vec, CameraRot);
	ScreenVec ProjPointToScreenVec(Vec, ScreenVec);


    //// bitmaps (bitmap.c)
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


    //// line drawing (line.c)
	void DrawLine(ScreenVec, ScreenVec, Pixel, ProgramState *);


    //// triangle drawing (tri.c)
	typedef struct TriIndicesStruct {
		int a, b, c;
		Pixel albedoColor;

		Pixel outputColor;
	} TriIndices;

	void DrawTri(TriIndices *, ProgramState *);


    //// meshes (mesh.c)
	typedef struct MeshStruct {
		Vec pos;
		Quaternion rot;

		TriIndices *triIndices;
		int triIndicesSize;

		Vec *meshPoints;
		Vec *worldPoints;
		int pointsSize;

		Mat mat;
	} Mesh;

	Mesh InitMesh(
		TriIndices *, int,
		Vec *, int,
		ProgramState *
	);

	// the int is to make converting from .obj more tolerable
	// because point indices in .obj files start at 1
	Mesh LoadMeshFromText(char *, ProgramState *, int);
	Mesh LoadMeshFromBinary(char *, ProgramState *);
	void WriteBinaryFromMesh(Mesh *, char *);
	void UpdateMesh(Mesh *, Vec, Quaternion);


    //// lighting (light.c)
	typedef enum {
		LIGHT_TYPE_END = 255, // (uint8_t)(-1)

		LIGHT_TYPE_POINT = 0,
		LIGHT_TYPE_DIRECTIONAL
	} LightType;

	typedef struct LightsSimdStruct {
		int pointLightsSize;
		float *pointLightsPosX;
		float *pointLightsPosY;
		float *pointLightsPosZ;
		float *pointLightsIntensity;

		int dirLightsSize;
		float *dirLightsDirX;
		float *dirLightsDirY;
		float *dirLightsDirZ;
		float *dirLightsIntensity;

		float ambientLightAmount;
	} LightsSimd;

	// Variadic arguments are:
	// 	- LIGHT_TYPE_DIRETIONAL, float intensity, Vec pos
	//  - LIGHT_TYPE_POINT, float intensity, Vec dir
	// 	- LIGHT_TYPE_END (Terminator of the variadic arguments)
	LightsSimd InitLightsSimd(float, ...);
	void FreeLightsSimd(LightsSimd *);

	float ComputeLights(
		LightsSimd *,
		Vec,
		Vec
	);

	void ComputeLightsForTri(TriIndices *, ProgramState *);


	    //// program state (program-state.c)
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

		LightsSimd lights;

		Camera camera;
	} ProgramState;

	// BOOL IsKeyReleased(ProgramState *, int);
	// BOOL IsKeyPressed(ProgramState *, int);
	BOOL IsKeyDown(ProgramState *, int);


    //// rendering (render.c)
    void RenderFrame(ProgramState *);
#endif
