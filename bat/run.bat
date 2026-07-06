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

@ECHO OFF
CLS
SET StartPath=%CD%
SET RootPath=%~dp0.\..
CALL "%RootPath%\bat\vs-setup-caller-snippet.bat"
SETLOCAL ENABLEDELAYEDEXPANSION

	REM Default parameters
	SET DefaultDoCompile=1

	REM Flag parameters
	SET DoCompile=!DefaultDoCompile!

	SET OtherArgs=
	SET IsOption=0
	FOR %%x IN (%*) DO (
		IF "%%x" == "compile" (
			SET DoCompile=1
			SET IsOption=1
		)

		IF "%%x" == "no-compile" (
			SET DoCompile=0
			SET IsOption=1
		)

		IF "%%x" == "help" (
			ECHO run[.bat] [compile, no-compile]

			CD "!StartPath!"
			ENDLOCAL
			EXIT /B 0
		)

		IF "!IsOption!" == "0" (
			SET OtherArgs=!OtherArgs! %%x
		)

		SET IsOption=0
	)

	IF "!DoCompile!" == "1" (
		CALL "!RootPath!\bat\build.bat" !OtherArgs!
	)

	IF %ERRORLEVEL% NEQ 0 (
		ECHO build.bat failed, exiting with error...

		CD "!StartPath!"
		ENDLOCAL
		EXIT /B 1
	)

	IF EXIST "!RootPath!\obj\result\main.exe" (
		START "" "!RootPath!\obj\result\main.exe"
	)
CD "!StartPath!"
ENDLOCAL
