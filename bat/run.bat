@ECHO OFF

PUSHD %~dp0\..
	CALL bat\vs-setup-caller-snippet.bat
POPD

REM Default parameters
REM ...

REM Flag parameters
SET NoCompile=0

CLS
SETLOCAL ENABLEDELAYEDEXPANSION
PUSHD %~dp0\..
	FOR %%x IN (%*) DO (
		IF "%%x" == "no-compile" (
			SET NoCompile=1
		)

		IF "%%x" == "help" (
			ECHO run[.bat] [no-compile]
			ENDLOCAL
			POPD
			EXIT /B 0
		)
	)

	IF "!NoCompile!" == "0" (
		CALL bat\build.bat
	)

	IF EXIST obj\result\main.exe (
		START obj\result\main.exe
	)
POPD
ENDLOCAL
