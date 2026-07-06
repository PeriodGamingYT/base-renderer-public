REM base-renderer-public, a zero-dependency (besides C, Win32, and instrinsics)
REM software renderer.
REM Copyright (C) 2025  Nathan Phillips

REM This program is free software; you can redistribute it and/or modify
REM it under the terms of the GNU General Public License as published by
REM the Free Software Foundation; either version 2 of the License, or
REM (at your option) any later version.

REM This program is distributed in the hope that it will be useful,
REM but WITHOUT ANY WARRANTY; without even the implied warranty of
REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM GNU General Public License for more details.

REM You should have received a copy of the GNU General Public License along
REM with this program; if not, write to the Free Software Foundation, Inc.,
REM 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

REM TODO(ElkElan): Merge this file into run.bat by making debug a
REM recognizable option. That will to run the debugger instead of
REM just running the file normally.
@ECHO OFF
CLS
SET StartPath=%CD%
SET RootPath=%~dp0.\..
CALL "%RootPath%\bat\vs-setup-caller-snippet.bat"
SETLOCAL ENABLEDELAYEDEXPANSION

	REM Default parameters
	SET DefaultDebugger=remedybg.exe

	REM Flag parameters
	SET NoCompile=0

	SET IsDebuggerUpcoming=0
	SET Debugger=!DefaultDebugger!

	SET IsInvalidCommandLine=0

	SET OtherArgs=
	SET IsOption=0
	FOR %%x IN (%*) DO (
		IF "%%x" == "no-compile" (
			SET NoCompile=1
			SET IsOption=1
		)

		IF "!IsDebuggerUpcoming!" == "1" (
			SET Debugger=%%x
			SET IsDebuggerUpcoming=0
			SET IsOption=1
		)

		IF "%%x" == "use-debugger" (
			SET IsDebuggerUpcoming=1
			SET IsOption=1
		)

		IF "%%x" == "help" (
			ECHO debug[.bat] [no-compile] [use-debugger debugger-exe] [help]

			CD "!StartPath!"
			ENDLOCAL
			EXIT /B 0
		)

		IF "!IsOption!" == "0" (
			SET OtherArgs=!OtherArgs! %%x
		)

		SET IsOption=0
	)

	IF "!IsDebuggerUpcoming!" == "1" (
		ECHO Debugger wasn't provided with use-debugger
		SET IsInvalidCommandLine=1
	)

	IF "!IsInvalidCommandLine!" == "1" (
		ECHO Invalid command line arguments were provided, exiting with error...

		ENDLOCAL
		CD !StartPath!
		EXIT /B 1
	)

	IF "!NoCompile!" == "0" (
		CALL "!RootPath!\bat\build.bat" debug !OtherArgs!
		IF NOT "!ERRORLEVEL!" == "0" (
			ECHO build.bat failed, exiting with error...

			ENDLOCAL
			CD !StartPath!
			EXIT /B 1
		)
	)

	IF EXIST "!RootPath!\obj\result\main.exe" (
		TASKLIST /FI "IMAGENAME eq !Debugger!" 2>NUL | FIND /I "!Debugger!" >NUL
		IF "!ERRORLEVEL!" == "0" ( TASKKILL /F /IM !Debugger! )
		START !Debugger! "!RootPath!\obj\result\main.exe"
	)
CD "!StartPath!"
ENDLOCAL
