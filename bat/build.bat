@ECHO OFF

CLS
PUSHD %~dp0\..
	DEL /F /S /Q obj

	SET DebugCompilerFlags=/Zi /MTd
	SET ReleaseCompilerFlags=/O2
	MKDIR obj
	PUSHD obj

		REM You will need to install Visual Studio w/ MSVC and Windows SDK.
		REM If you need someone to decrypt sanskirt symbols dating back
		REM to ancient Egypt, just call me!
		CL ^
			/I..\include /arch:AVX ^
			^
				%DebugCompilerFlags% ^
			^
				..\src\includes.c ^
				..\src\screen-vec.c ^
				..\src\vec.c ^
				..\src\program-state.c ^
				..\src\bitmap.c ^
				..\src\line.c ^
				..\src\tri.c ^
				..\src\mesh.c ^
				..\src\light.c ^
				..\src\render.c ^
				..\src\main.c ^
			^
			/link /OUT:main.exe

		MKDIR result
		PUSHD result
			COPY ..\main.exe
			MKDIR assets
			XCOPY ..\..\assets assets
		POPD
	POPD
POPD
