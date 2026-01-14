@ECHO OFF

PUSHD %~dp0\..
	CALL bat\vs-setup-caller-snippet.bat
POPD

REM Default parameters
SET DefaultDebugger=REMEDYBG

REM Flag parameters
SET NoCompile=0

SET IsDebuggerUpcoming=0
SET Debugger=%DefaultDebugger%

SET IsInvalidCommandLine=0

CLS
SETLOCAL ENABLEDELAYEDEXPANSION
PUSHD %~dp0\..
	FOR %%x IN (%*) DO (
		IF "%%x" == "no-compile" (
			SET NoCompile=1
		)


		IF "!IsDebuggerUpcoming!" == "1" (
			SET Debugger=%%x
			SET IsDebuggerUpcoming=0
		)

		IF "%%x" == "use-debugger" (
			SET IsDebuggerUpcoming=1
		)


		IF "%%x" == "help" (
			ECHO debug[.bat] [no-compile] [use-debugger debugger-exe] [help]
			ENDLOCAL
			POPD
			EXIT /B 0
		)
	)

	IF "!IsDebuggerUpcoming!" == "1" (
		ECHO Debugger wasn't provided with use-debugger
		SET IsInvalidCommandLine=1
	)

	IF "!IsInvalidCommandLine!" == "1" (
		ECHO Invalid command line arguments were provided, shutting down...
		ENDLOCAL
		POPD
		EXIT /B 1
	)

	IF "!NoCompile!" == "0" (
		CALL bat\build.bat debug
	)

	IF EXIST obj\result\main.exe (
		START !Debugger! obj\result\main.exe
	)
POPD
ENDLOCAL
