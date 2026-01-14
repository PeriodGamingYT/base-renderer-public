@ECHO OFF

REM This Batch script is meant to aid setting up the environment
REM variables for the CL compiler in Visual Studio. If you
REM are using a different compiler that isn't from Visual Studio
REM then this script won't help you

REM Default parameters
SET VisualStudioStartPath=C:\Program Files\Microsoft Visual Studio
SET VisualStudioBatchPathFragment=Common7\Tools\VsDevCmd.bat

SET DefaultVisualStudioVersionYear=2022
SET DefaultVisualStudioVersionKind=Community
SET DefaultVisualStudioArch=x64

REM Flag parameters
SET IsVersionYearUpcoming=0
SET VisualStudioVersionYear=%DefaultVisualStudioVersionYear%

SET IsVersionKindUpcoming=0
SET VisualStudioVersionKind=%DefaultVisualStudioVersionKind%

SET IsVisualStudioArchUpcoming=0
SET VisualStudioArch=%DefaultVisualStudioArch%

SET IsInvalidCommandLine=0

CLS
SETLOCAL ENABLEDELAYEDEXPANSION
PUSHD %~dp0\..
	FOR %%x IN (%*) DO (
		IF "!IsVersionYearUpcoming!" == "1" (
			SET VisualStudioVersionYear=%%x
			SET IsVersionYearUpcoming=0
		)

		IF "%%x" == "vs-year" (
			SET IsVersionYearUpcoming=1
		)


		IF "!IsVersionKindUpcoming!" == "1" (
			SET VisualStudioVersionKind=%%x
			SET IsVersionKindUpcoming=0
		)

		IF "%%x" == "vs-kind" (
			SET IsVersionKindUpcoming=1
		)


		IF "!IsVisualStudioArchUpcoming!" == "1" (
			SET VisualStudioArch=%%x
			SET IsVisualStudioArchUpcoming=0
		)

		IF "%%x" == "vs-arch" (
			SET IsVisualStudioArchUpcoming=1
		)


		IF "%%x" == "help" (
			ECHO vs-setup[.bat] [vs-year year] [vs-kind kind] [vs-arch arch] [help]
			ENDLOCAL
			POPD
			EXIT /B 0
		)
	)

	IF "!IsVersionYearUpcoming!" == "1" (
		ECHO Year of Visual Studio wasn't provided with vs-year
		SET IsInvalidCommandLine=1
	)

	IF "!IsVersionKindUpcoming!" == "1" (
		ECHO Kind of Visual Studio wasn't provided with vs-kind
		ECHO The kind of Visual Studio should be Community, Professional, or Enterprise
		SET IsInvalidCommandLine=1
	)

	IF "!IsVisualStudioArchUpcoming!" == "1" (
		ECHO Architecture of Visual Studio wasn't provided with vs-arch
		SET IsInvalidCommandLine=1
	)

	IF "!IsInvalidCommandLine!" == "1" (
		ECHO Invalid command line arguments were provided, shutting down with error...
		ENDLOCAL
		POPD
		EXIT /B 1
	)


	ECHO Attempting to load Visual Studio !VisualStudioVersionYear! !VisualStudioVersionKind! for !VisualStudioArch!...

	REM I had to put these on seperate lines in order to
	REM make the path more understandable, but don't
	REM try to indent the path parts/fragments forward
	REM lest the VisualStudioBatchPath will have tabs
	REM and not work!
	SET VisualStudioBatchPath=^
%VisualStudioStartPath%\^
!VisualStudioVersionYear!\^
!VisualStudioVersionKind!\^
%VisualStudioBatchPathFragment%

REM Again with stupid indentation edge cases in Batch.
REM UGH! Don't program in Batch if you can, it's a
REM horrible language!
ENDLOCAL & ^
SET VisualStudioArch=%VisualStudioArch% & ^
SET VisualStudioBatchPath=%VisualStudioBatchPath%

IF NOT EXIST "%VisualStudioBatchPath%" (
	ECHO The path determined for the Visual Studio environment variable setup script, being:
	ECHO %VisualStudioBatchPath%
	ECHO doesn't exist.
	ECHO:
	ECHO You can try to consult vs-setup[.bat] to learn what your options for setup are.
	ECHO:
	ECHO Shutting down with error...

	POPD
	EXIT /B 1
)

CALL "%VisualStudioBatchPath%" ^
	-startdir=none ^
	-arch=%VisualStudioArch% ^
	-host_arch=%VisualStudioArch%

POPD
