@echo off
echo Converting itemdesc txt files to JSON...

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0convert.ps1"

echo.
echo Done!
pause