@ECHO OFF

REM Setup Visual Studio here, since doing it inside
REM of the SETLOCAL/ENDLOCAL will guarantee that
REM the environment variables that were set up
REM will be guaranteed to be lost after ENDLOCAL
PUSHD %~dp0\..
	IF "%VCINSTALLDIR%" == "" (
		ECHO Visual Studio environment variables weren't detected, loading them now...
		CALL bat\vs-setup.bat

		IF "%VCINSTALLDIR%" == "" (
			ECHO Visual Studio failed to setup, shutting down with error...
			EXIT /B 1
		)
	)
POPD
