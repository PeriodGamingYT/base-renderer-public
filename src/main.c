#include <render.h>

void Cleanup(ProgramState *state) {
	state->isRunning = FALSE;

	FreePage((uint8_t **)(&state->bitmapBuffer));

	FreeArena(&state->meshes);
	FreeArena(&state->triIndices);

	FreeArena(&state->meshPoints);
	FreePage((uint8_t **)(&state->worldPoints));
	FreePage((uint8_t **)(&state->cameraPoints));

	FreeArena(&state->lights);
}

LRESULT WindowProc(
	HWND windowHandle,
	UINT messageType,
	WPARAM extraMessage,
	LPARAM userDataParam
) {
	ProgramState *state = (ProgramState *)(GetWindowLongPtr(
		windowHandle,
		GWLP_USERDATA
	));

	switch(messageType) {
		case WM_CREATE: { return 0; }
		case WM_DESTROY: {
			state->isRunning = FALSE;
			PostQuitMessage(0);
			return 0;
		}

		case WM_SIZE: {
			ResizeDIB(GetWindowSize(windowHandle), state);
			return 0;
		}

		case WM_KEYDOWN: {
			state->keys[extraMessage] = TRUE;
			return 0;
		}

		case WM_KEYUP: {
			state->keys[extraMessage] = FALSE;
			return 0;
		}

		default: { break; }
	}

	return DefWindowProc(
		windowHandle,
		messageType,
		extraMessage,
		userDataParam
	);
}

static ScreenVec GetWindowPos(HWND windowHandle) {
	RECT windowRect = { 0 };
	GetWindowRect(windowHandle, &windowRect);
	return SCREEN_VEC(
		windowRect.left,
		windowRect.top
	);
}

int WINAPI WinMain(
	HINSTANCE appId,
	HINSTANCE ignoredPrevAppId,
	LPSTR commandLine,
	int windowShowFlags
) {
	TCHAR exePathBuffer[MAX_PATH] = { 0 };
	int fileNameIndex = GetModuleFileName(NULL, exePathBuffer, MAX_PATH);
	for(
		fileNameIndex = fileNameIndex - 1;
		fileNameIndex >= 0 && exePathBuffer[fileNameIndex] != '\\';
		fileNameIndex--
	);

	// "main.exe"
	// "assets" <-- shorter than main.exe so it's okay (still a hack though)
	TCHAR assetName[] = "assets";
	fileNameIndex++;
	memcpy(
		&exePathBuffer[fileNameIndex],
		assetName,
		ARRAY_SIZE(assetName) * sizeof(TCHAR)
	);

	exePathBuffer[fileNameIndex + ARRAY_SIZE(assetName)] = '\0';

	// "If the function succeeds, then the return value is non-zero." - MSDN.
	// I HATE MICROSOFT, GAHHHH!
	if(SetCurrentDirectory(exePathBuffer) == 0) {
		ErrorMessageBox(
			"Can't find assets folder! "
			"There needs to be an assets folder in the same root folder as this executable."
		);

		return 1;
	}

	ProgramState state = {
		.isRunning = TRUE,
		.frameTime = 1
	};

	#define MESHES_CAP 256
	state.meshes = InitArena(
		sizeof(Mesh), MESHES_CAP
	);

	#define TRI_POINT_INDICES_CAP 8192
	state.triIndices = InitArena(
		sizeof(TriIndices), TRI_POINT_INDICES_CAP
	);

	#define MESH_POINTS_CAP 4096
	state.meshPoints = InitArena(
		sizeof(Vec), MESH_POINTS_CAP
	);

	state.worldPoints = (Vec *)(InitPage(
		MESH_POINTS_CAP * sizeof(Vec)
	));

	state.cameraPoints = (Vec *)(InitPage(
		MESH_POINTS_CAP * sizeof(Vec)
	));

	#define LIGHTS_CAP 128
	state.lights = InitArena(
		sizeof(Light), LIGHTS_CAP
	);

	// START DEMO INIT CODE SECTION
		Mesh *mesh = (Mesh *)(AppendToArena(
			&state.meshes, 1
		));

		*mesh = LoadMeshFromText("mountains.text-mesh", &state, 1);
		UpdateMesh(
			mesh,
			VEC(0, 10, 0),
			EulerToQuaternion(VEC(M_PI, 1.75 * M_PI, 0))
		);

		Light *ambientLight = (Light *)(AppendToArena(
			&state.lights, 1
		));

		ambientLight->type = LIGHT_TYPE_AMBIENT;
		ambientLight->intensity = 0.1;

		Light *directionalLight = (Light *)(AppendToArena(
			&state.lights, 1
		));

		directionalLight->type = LIGHT_TYPE_DIRECTIONAL;
		directionalLight->intensity = 0.5;
		directionalLight->directional.direction = VecNormal(VEC(5, 0.5, 1));

		Light *pointLight = (Light *)(AppendToArena(
			&state.lights, 1
		));

		pointLight->type = LIGHT_TYPE_POINT;
		pointLight->intensity = 25;
		pointLight->point.pos = VEC(-2, 0, -3);
	// END DEMO INIT CODE SECTION

	LPCTSTR className = TEXT("BaseRenderer");
	WNDCLASS windowClass = {
		.lpfnWndProc = WindowProc,
		.hInstance = appId,
		.lpszClassName = className,
		.hCursor = LoadCursor(NULL, IDC_ARROW)
	};

	RegisterClass(&windowClass);

	LPCTSTR windowName = TEXT("BaseRenderer");
	HWND windowHandle = CreateWindowEx(
		WS_EX_TRANSPARENT, // just for fun
		className, windowName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,
		NULL,
		appId,
		NULL
	);

	SetWindowLongPtr(
		windowHandle,
		GWLP_USERDATA,
		(LONG_PTR)(&state)
	);

	state.camera = InitCamera(GetWindowSize(windowHandle));

	if(windowHandle == NULL) { return 1; }
	ShowWindow(windowHandle, windowShowFlags);

	LARGE_INTEGER performanceFreq = { 0 };
	QueryPerformanceFrequency(&performanceFreq);

	#define PERFORMANCE_COUNTERS_SIZE 30
	float performanceCounters[PERFORMANCE_COUNTERS_SIZE] = { 0 };
	int performanceCounterIndex = 0;
	while(state.isRunning) {
		MSG message = { 0 };
		while(PeekMessage(
			&message,
			NULL,
			0,
			0,
			PM_REMOVE
		) > 0) {

			// virtual key --> character key
			// TranslateMessage(&message);
			DispatchMessage(&message);
		}

		LARGE_INTEGER pastPerformanceCounter = { 0 };
		QueryPerformanceCounter(&pastPerformanceCounter);

		POINT mousePos = { 0 };
		GetCursorPos(&mousePos);
		ScreenVec windowPos = GetWindowPos(windowHandle);
		state.mousePos = SCREEN_VEC(
			mousePos.x - windowPos.x,
			mousePos.y - windowPos.y
		);

		if(
			state.keys[VK_ESCAPE] &&
			!state.pastKeys[VK_ESCAPE]
		) {
			state.mouseLocked = !state.mouseLocked;
			ShowCursor(!state.mouseLocked);
			if(state.mouseLocked) {
				state.beforeLockedMousePos = SCREEN_VEC(
					mousePos.x,
					mousePos.y
				);

				// a trick to not have crazy mouseMove values when we
				// lock the mouse cursor
				ScreenVec lockedMousePos = SCREEN_VEC(
					windowPos.x + 100,
					windowPos.y + 100
				);

				mousePos.x = lockedMousePos.x;
				mousePos.y = lockedMousePos.y;
			} else {
				ScreenVec restoredMousePos = state.beforeLockedMousePos;
				CursorGoTo(restoredMousePos);
			}
		}

		if(state.mouseLocked) {
			ScreenVec lockedMousePos = SCREEN_VEC(
				windowPos.x + 100,
				windowPos.y + 100
			);

			state.mouseMove = SCREEN_VEC(
				mousePos.x - lockedMousePos.x,
				mousePos.y - lockedMousePos.y
			);

			CursorGoTo(lockedMousePos);
		} else {
			state.mouseMove = SCREEN_VEC(0, 0);
		}

		RenderFrame(&state);
		HDC deviceContextHandle = GetDC(windowHandle);
			UpdateWindowSize(
				deviceContextHandle,
				GetWindowSize(windowHandle),
				&state
			);
		ReleaseDC(windowHandle, deviceContextHandle);

		LARGE_INTEGER performanceCounter = { 0 };
		QueryPerformanceCounter(&performanceCounter);
		performanceCounters[performanceCounterIndex] = (float)(
			performanceCounter.QuadPart -
			pastPerformanceCounter.QuadPart
		) / (float)(performanceFreq.QuadPart);

		performanceCounterIndex = (performanceCounterIndex + 1) % PERFORMANCE_COUNTERS_SIZE;
		if(performanceCounterIndex >= PERFORMANCE_COUNTERS_SIZE - 1) {
			float averageFrameTime = 0;
			for(int i = 1; i < PERFORMANCE_COUNTERS_SIZE; i++) {
				averageFrameTime += performanceCounters[i];
			}

			averageFrameTime /= (float)(PERFORMANCE_COUNTERS_SIZE);

			#define TITLE_BUFFER_SIZE 64
			char titleBuffer[TITLE_BUFFER_SIZE] = { 0 };
			snprintf(
				titleBuffer,
				TITLE_BUFFER_SIZE,
				"%s, FPS: %d",
				windowName,
				(int)(1.0f / averageFrameTime)
			);

			SetWindowText(windowHandle, titleBuffer);
		}

		state.frameTime = (float)(
			performanceCounter.QuadPart -
			pastPerformanceCounter.QuadPart
		) / (float)(performanceFreq.QuadPart);

		state.totalFrameTime += state.frameTime;
		state.pastMousePos = state.mousePos;
		memcpy(state.pastKeys, state.keys, sizeof(Keys));
		state.frameCount++;
	}

	return 0;
}

