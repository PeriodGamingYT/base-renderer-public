#include <includes.h>

void ErrorMessageBox(char *message) {
	MessageBox(
		NULL,
		message,
		TEXT("Error"),
		MB_OK | MB_ICONERROR
	);
}

// windows is dumb, JUST USE NULL!
#define FILE_HANDLE_ERROR ((HANDLE)(-1))
FileMap OpenFileMap(char *fileName) {
	FileMap result = { 0 };

	// why open() and mmap() when you can CreateFile() (Also opens because OpenFile() is for
	// 16-bit Windows), CreateFileMapping(), and MapViewOfFile()?
	result.fileHandle = CreateFile(
		fileName,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if(result.fileHandle == NULL || result.fileHandle == FILE_HANDLE_ERROR) {
		ErrorMessageBox("Can't open file!");
		return (FileMap) { 0 };
	}

	LARGE_INTEGER fileSize = { 0 };

	// i know that this probably has "Ex" at the end of it
	// because GetFileSize() is old dinosaur-age bullshit.
	// but why not "2" or "New"? only god knows...
	GetFileSizeEx(result.fileHandle, &fileSize);
	result.size = fileSize.LowPart;

	result.fileMapHandle = CreateFileMapping(
		result.fileHandle,
		NULL,
		PAGE_READONLY,
		fileSize.HighPart, fileSize.LowPart,
		NULL
	);

	if(result.fileMapHandle == NULL) {
		ErrorMessageBox("Can't create file mapping!");
		CloseHandle(result.fileMapHandle);
		return (FileMap) { 0 };
	}

	// can fail but i've never had it crash, if reading the map
	// crashes (since failure is NULL) then go ahead and add a error
	// check for this
	result.map = MapViewOfFile(
		result.fileMapHandle,
		FILE_MAP_READ,
		0, 0, 0
	);

	result.current = result.map;
	result.end = &result.map[result.size];
	return result;
}

FileMap OpenWriteFileMap(char *fileName, int fileSize) {
	FileMap result = { 0 };
	result.fileHandle = CreateFile(
		fileName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if(result.fileHandle == NULL || result.fileHandle == FILE_HANDLE_ERROR) {
		ErrorMessageBox("Can't open file!");
		return (FileMap) { 0 };
	}

	result.size = fileSize;
	result.fileMapHandle = CreateFileMapping(
		result.fileHandle,
		NULL,
		PAGE_READWRITE,
		0, fileSize,
		NULL
	);

	if(result.fileMapHandle == NULL) {
		ErrorMessageBox("Can't create file mapping!");
		CloseHandle(result.fileMapHandle);
		return (FileMap) { 0 };
	}

	// can fail but i've never had it crash, if reading the map
	// crashes (since failure is NULL) then go ahead and add a error
	// check for this
	result.map = MapViewOfFile(
		result.fileMapHandle,
		FILE_MAP_READ | FILE_MAP_WRITE,
		0, 0, 0
	);

	result.current = result.map;
	result.end = &result.map[result.size];
	return result;
}

void CloseFileMap(FileMap *fileMap) {
	UnmapViewOfFile(fileMap->map);
	CloseHandle(fileMap->fileMapHandle);
	CloseHandle(fileMap->fileHandle);

	memset(fileMap, 0, sizeof(FileMap));
}

