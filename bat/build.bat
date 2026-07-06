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

	REM Address sanitizer isn't used here since it gets hung up on not being able
	REM to scan kernel32.lib and ntdll.lib
	SET DebugCompilerFlags=/Z7 /MTd /DEBUG:FULL
	SET ReleaseCompilerFlags=/O2
	SET TestingCompilerFlags=/DTESTING_MODE

	SET DefaultUseDebug=1
	SET DefaultShouldMakeAssetsFolder=0
	SET DefaultDoTesting=0
	SET DefaultShouldMakeTestAssetsFolder=1

	REM Flag Parameters
	SET UseDebug=!DefaultUseDebug!
	SET ShouldMakeAssetsFolder=!DefaultShouldMakeAssetsFolder!
	SET DoTesting=!DefaultDoTesting!

	REM NOTE: This will only be done when the testing flag is also enabled.
	SET ShouldMakeTestAssetsFolder=!DefaultShouldMakeTestAssetsFolder!

	SET OtherArgs=
	SET IsOption=0

	REM This variable is here for consistency with other
	REM Batch files and for extra build options in the future
	SET IsInvalidCommandLine=0

	REM SET var=... will take up all whitespace until terminating
	REM symbol or newline appears. As such, the closing parens
	REM are on a separate line.
	FOR %%x IN (%*) DO (
		SET IsOption=0
		IF "%%x" == "debug" (
			SET UseDebug=1
			SET IsOption=1
		)

		IF "%%x" == "release" (
			SET UseDebug=0
			SET IsOption=1
		)


		IF "%%x" == "test" (
			SET DoTesting=1
			SET IsOption=1
		)

		IF "%%x" == "no-test" (
			SET DoTesting=0
			SET IsOption=1
		)


		IF "%%x" == "make-test-assets" (
			SET ShouldMakeTestAssetsFolder=1
			SET IsOption=1
		)

		IF "%%x" == "dont-make-test-assets" (
			SET ShouldMakeTestAssetsFolder=0
			SET IsOption=0
		)


		IF "%%x" == "make-assets" (
			SET ShouldMakeAssetsFolder=1
			SET IsOption=1
		)

		IF "%%x" == "dont-make-assets" (
			SET ShouldMakeAssetsFolder=0
			SET IsOption=1
		)


		IF "%%x" == "help" (
			ECHO build[.bat] [debug, release] [test, no-test] [make-test-assets, dont-make-test-assets] [make-assets, dont-make-assets] [help]

			CD "!StartPath!"
			ENDLOCAL
			EXIT /B 0
		)

		IF "!IsOption!" == "0" (
			SET OtherArgs=!OtherArgs! %%x
		)
	)

	IF "!IsInvalidCommandLine!" == "1" (
		ECHO Invalid command line arguments were provided, exiting with error...

		CD "!StartPath!"
		ENDLOCAL
		EXIT /B 1
	)

	SET CompilerFlags=
	IF "!UseDebug!" == "1" (
		SET CompilerFlags=!CompilerFlags! !DebugCompilerFlags!
	) ELSE (
		SET CompilerFlags=!CompilerFlags! !ReleaseCompilerFlags!
	)

	IF "!DoTesting!" == "1" (
		SET CompilerFlags=!CompilerFlags! !TestingCompilerFlags!
	)

	SET FilesToCompile=
	FOR /F "delims=" %%x IN ('DIR /B /S "!RootPath!\src"') DO (
		SET FilesToCompile=!FilesToCompile! "%%x"
	)

	DEL /F /S /Q "!RootPath!\obj" >NUL 2>&1
	MKDIR "!RootPath!\obj"

	REM I hate this idiom(?) in CL, but if a command line option can take a
	REM file or directory, any input given to it that lacks two backslashes
	REM at the end of it is assumed to be a file.
	CL ^
		/I"!RootPath!\include" ^
		/Fo"!RootPath!\obj\\" ^
		/Fd"!RootPath!\obj\\" ^
		/Fe"!RootPath!\obj\main.exe" ^
		!CompilerFlags! ^
		!FilesToCompile! ^
		!OtherArgs!

	IF %ERRORLEVEL% NEQ 0 (
		ECHO Compiling failed, exiting with error...

		CD "!StartPath!"
		ENDLOCAL
		EXIT /B 1
	)

	MKDIR "!RootPath!\obj\result"
	COPY "!RootPath!\obj\main.exe" "!RootPath!\obj\result\main.exe"

	IF "!ShouldMakeAssetsFolder!" == "1" (
		MKDIR "!RootPath!\obj\result\assets"
		XCOPY /S /E /I /Y "!RootPath!\assets" "!RootPath!\obj\result\assets"
	)

	IF "!DoTesting!" == "1" (
	IF "!ShouldMakeTestAssetsFolder!" == "1" (
		XCOPY /S /E /I /Y "!RootPath!\test-assets" "!RootPath!\obj\result\assets"
	)
	)
CD "!StartPath!"
ENDLOCAL
