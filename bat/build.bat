@ECHO OFF

PUSHD %~dp0\..
	CALL bat\vs-setup-caller-snippet.bat
POPD

REM Default parameters
SET DebugCompilerFlags=/Zi /MTd /D DEBUG_MODE /DEBUG
SET ReleaseCompilerFlags=/O2

SET DefaultUseDebug=1

REM Flag Parameters
SET UseDebug=%DefaultUseDebug%

REM This variable is here for consistency with other
REM Batch files and for extra build options in the future
SET IsInvalidCommandLine=0

CLS
SETLOCAL ENABLEDELAYEDEXPANSION
PUSHD %~dp0\..
	FOR %%x IN (%*) DO (
		IF "%%x" == "debug" (
			SET UseDebug=1
		)

		IF "%%x" == "release" (
			SET UseDebug=0
		)

		IF "%%x" == "help" (
			ECHO build[.bat] [debug] [release] [help]
			ENDLOCAL
			POPD
			EXIT /B 0
		)
	)

	IF "!IsInvalidCommandLine!" == "1" (
		ECHO Invalid command line arguments were provided, shutting down...
		ENDLOCAL
		POPD
		EXIT /B 1
	)

	DEL /F /S /Q obj >NUL 2>&1

	SET FilesToCompile=
	FOR /R src %%x IN (*.c) DO (
		SET FilesToCompile=!FilesToCompile! %%x
	)

	MKDIR obj
	PUSHD obj
		SET CompilerFlags=%ReleaseCompilerFlags%
		IF "!UseDebug!" == "1" (
			SET CompilerFlags=%DebugCompilerFlags%
		)

		CL ^
			/I..\include ^
				!CompilerFlags! ^
				%FilesToCompile% ^
			/link /OUT:main.exe

		MKDIR result
		PUSHD result
			COPY ..\main.exe
			MKDIR assets
			XCOPY ..\..\assets assets
		POPD
	POPD
POPD
ENDLOCAL
