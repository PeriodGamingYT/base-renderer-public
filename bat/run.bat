@ECHO OFF

PUSHD %~dp0\..
	CALL bat\build.bat
	IF EXIST obj\result\main.exe (
		START obj\result\main.exe
	)
POPD
