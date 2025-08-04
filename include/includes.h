#ifndef __INCLUDES_H
#define __INCLUDES_H
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
#endif
