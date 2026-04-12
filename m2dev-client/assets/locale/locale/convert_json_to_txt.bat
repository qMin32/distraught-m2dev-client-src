@echo off
echo Exporting itemdesc per language folders...
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0exporttxt.ps1" -JsonFile "%~dp0itemdesc.json" -OutRoot "%~dp0out"
echo.
echo Done!
pause