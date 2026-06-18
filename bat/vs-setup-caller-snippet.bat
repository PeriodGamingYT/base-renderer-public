REM source-treks, grok and jump through C source code easily
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

REM Setup Visual Studio here, since doing it inside
REM of the SETLOCAL/ENDLOCAL will guarantee that
REM the environment variables that were set up
REM will be guaranteed to be lost after ENDLOCAL

CLS
SET StartPath=%CD%
SET RootPath=%~dp0.\..
	WHERE /Q CL
	IF %ERRORLEVEL% NEQ 0 (
		ECHO Visual Studio environment variables weren't detected, loading them now...
		CALL "%RootPath%\bat\vs-setup.bat"

		WHERE /Q CL
		IF %ERRORLEVEL% NEQ 0 (
			ECHO Visual Studio failed to setup, exiting with error...

			CD "%StartPath%"
			EXIT /B 1
		)
	)
CD %StartPath%
