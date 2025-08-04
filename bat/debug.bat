@ECHO OFF

PUSHD %~dp0\..
	CALL bat/build.bat
	IF EXIST obj\result\main.exe (

		REM Use this if you have RemedyBG installed and in your PATH
		START REMEDYBG obj\result\main.exe

		REM If you want to use Visual Stupido
		REM START DEVEND obj\result\main.exe
	)
POPD
