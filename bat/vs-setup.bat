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
SETLOCAL ENABLEDELAYEDEXPANSION
CLS
SET StartPath=%CD%
SET RootPath=%~dp0.\..

	REM This Batch script is meant to aid setting up the environment
	REM variables for the CL compiler in Visual Studio. If you
	REM are using a different compiler that isn't from Visual Studio
	REM then this script won't help you
	REM
	REM As a side note, when I tried Visual Studio 2026 on my machine,
	REM I found it was referred to as "18", so use "vs-year 18" to load
	REM Visual Studio 2026.

	REM Default parameters
	SET VisualStudioStartPath=C:\Program Files\Microsoft Visual Studio
	SET VisualStudioBatchPathFragment=Common7\Tools\VsDevCmd.bat

	SET DefaultVisualStudioVersion=18
	SET DefaultVisualStudioKind=Community
	SET DefaultVisualStudioArch=x64

	REM Flag parameters
	SET IsVersionUpcoming=0
	SET VisualStudioVersion=!DefaultVisualStudioVersion!

	SET IsKindUpcoming=0
	SET VisualStudioKind=!DefaultVisualStudioKind!

	SET IsVisualStudioArchUpcoming=0
	SET VisualStudioArch=!DefaultVisualStudioArch!

	SET IsInvalidCommandLine=0

	REM SET var=... will take up all whitespace until terminating
	REM symbol or newline appears. As such, the closing parens
	REM are on a separate line.
	FOR %%x IN (%*) DO (
		IF "!IsVersionUpcoming!" == "1" (
			SET VisualStudioVersion=%%x
			SET IsVersionUpcoming=0
		)

		IF "%%x" == "vs-ver" ( SET IsVersionUpcoming=1
		)


		IF "!IsKindUpcoming!" == "1" (
			SET VisualStudioKind=%%x
			SET IsKindUpcoming=0
		)

		IF "%%x" == "vs-kind" ( SET IsKindUpcoming=1
		)


		IF "!IsVisualStudioArchUpcoming!" == "1" (
			SET VisualStudioArch=%%x
			SET IsVisualStudioArchUpcoming=0
		)

		IF "%%x" == "vs-arch" ( SET IsVisualStudioArchUpcoming=1
		)

		IF "%%x" == "help" (
			ECHO vs-setup[.bat] [vs-ver version] [vs-kind kind] [vs-arch arch] [help]

			CD "!StartPath!"
			ENDLOCAL
			EXIT /B 0
		)
	)

	IF "!IsVersionUpcoming!" == "1" (
		ECHO Version of Visual Studio wasn't provided with vs-ver
		SET IsInvalidCommandLine=1
	)

	IF "!IsKindUpcoming!" == "1" (
		ECHO Kind of Visual Studio wasn't provided with vs-kind
		ECHO The kind of Visual Studio should be Community, Professional, or Enterprise
		SET IsInvalidCommandLine=1
	)

	IF "!IsVisualStudioArchUpcoming!" == "1" (
		ECHO Architecture of Visual Studio wasn't provided with vs-arch
		SET IsInvalidCommandLine=1
	)

	IF "!IsInvalidCommandLine!" == "1" (
		ECHO Invalid command line arguments were provided, exiting with error...

		CD "!StartPath!"
		ENDLOCAL
		EXIT /B 1
	)


	ECHO Attempting to load Visual Studio !VisualStudioVersion! !VisualStudioKind! for !VisualStudioArch!...

	REM I had to put these on seperate lines in order to
	REM make the path more understandable, but don't
	REM try to indent the path parts/fragments forward
	REM lest the VisualStudioBatchPath will have tabs
	REM and not work!
	SET VisualStudioBatchPath=^
%VisualStudioStartPath%\^
!VisualStudioVersion!\^
!VisualStudioKind!\^
%VisualStudioBatchPathFragment%

	REM Again with stupid indentation edge cases in Batch.
	REM UGH! Don't program in Batch if you can, it's a
	REM horrible language!
ENDLOCAL & ^
SET VisualStudioArch=%VisualStudioArch% & ^
SET VisualStudioBatchPath=%VisualStudioBatchPath% & ^
SET StartPath=%StartPath%

IF NOT EXIST "%VisualStudioBatchPath%" (
	ECHO The path determined for the Visual Studio environment variable setup script, being:
	ECHO %VisualStudioBatchPath%
	ECHO doesn't exist.
	ECHO:
	ECHO You can try to consult vs-setup[.bat] to learn what your options for setup are.
	ECHO:
	ECHO Exiting with error...

	CD %StartPath%
	EXIT /B 1
)

CALL "%VisualStudioBatchPath%" ^
	-startdir=none ^
	-arch=%VisualStudioArch% ^
	-host_arch=%VisualStudioArch%
CD %StartPath%
