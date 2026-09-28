@echo off
rem Deletes generated folders and regenerates Visual Studio project files for A1.uproject.
setlocal

set "PROJECT_DIR=%~dp0"
set "PROJECT_FILE=%PROJECT_DIR%A1.uproject"
set "ENGINE_DIR=D:\Program Files\Epic Games\UE_5.4"
set "UBT=%ENGINE_DIR%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe"

if not exist "%UBT%" (
	echo [ERROR] UnrealBuildTool not found: "%UBT%"
	goto :Fail
)

tasklist /FI "IMAGENAME eq UnrealEditor.exe" 2>nul | find /I "UnrealEditor.exe" >nul
if not errorlevel 1 (
	echo [ERROR] UnrealEditor is running. Close it first.
	goto :Fail
)

pushd "%PROJECT_DIR%"

for %%D in (Intermediate Saved DerivedDataCache) do (
	if exist "%%D" (
		echo Deleting %%D ...
		rmdir /s /q "%%D"
		if exist "%%D" (
			echo [ERROR] Failed to delete %%D. A file may be locked.
			popd
			goto :Fail
		)
	)
)

if exist "A1.sln" (
	echo Deleting A1.sln ...
	del /q "A1.sln"
)

echo Generating project files ...
"%UBT%" -projectfiles -project="%PROJECT_FILE%" -game -rocket -progress
if errorlevel 1 (
	echo [ERROR] Project file generation failed.
	popd
	goto :Fail
)

popd
echo Done.
endlocal
exit /b 0

:Fail
endlocal
exit /b 1
