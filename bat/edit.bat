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

	REM Default parameters
	SET DefaultEditor=focus.exe
	SET DefaultIncludeAssets=0
	SET DefaultIncludeDocs=0

	REM Flag parameters
	SET IsEditorUpcoming=0
	SET Editor=!DefaultEditor!
	SET IncludeAssets=!DefaultIncludeAssets!
	SET IncludeDocs=!DefaultIncludeDocs!

	SET IsInvalidCommandLine=0

	FOR %%x IN (%*) DO (
		IF "%%x" == "use-editor" ( SET IsEditorUpcoming=1 )
		IF "!IsEditorUpcoming!" == "1" (
			SET Editor=%%x
			SET IsEditorUpcoming=0
		)

		IF "%%x" == "include-assets" ( SET IncludeAssets=1
		)
		IF "%%x" == "exclude-assets" ( SET IncludeAssets=0
		)

		IF "%%x" == "include-docs" ( SET IncludeDocs=1
		)
		IF "%%x" == "exclude-docs" ( SET IncludeDocs=0
		)

		IF "%%x" == "help" (
			ECHO edit[.bat] [use-editor editor-exe] [include-assets, exclude-assets] [include-docs, exclude-docs] [help]

			CD "!StartPath!"
			ENDLOCAL
			EXIT /B 0
		)
	)

	IF "!IsEditorUpcoming" == "1" (
		ECHO Editor wasn't provided with editor
		SET IsInvalidCommandLine=1
	)

	IF "!IsInvalidCommandLine!" == "1" (
		ECHO Invalid command line arguments were provided, exiting with error...
		ENDLOCAL
	)


	SET PathsToInclude= "!RootPath!\src" "!RootPath!\include" "!RootPath!\bat"
	IF "!IncludeAssets!" == "1" (
		SET PathsToInclude=!PathsToInclude! "!RootPath!\test-assets"
	)

	IF "!IncludeDocs!" == "1" (
		SET PathsToInclude=!PathsToInclude! "!RootPath!\docs"
	)

	START "" "!Editor!" !PathsToInclude!
CD "!StartPath!"
ENDLOCAL
