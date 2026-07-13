@echo off
title Instalador - TULO Client
color 0B

echo ===================================================
echo           INSTALADOR - TULO CLIENT
echo ===================================================
echo.

echo [1/1] Creando acceso directo en el Escritorio...
powershell -Command "$WshShell = New-Object -comObject WScript.Shell; $Shortcut = $WshShell.CreateShortcut(\"$Home\Desktop\TULO Client.lnk\"); $Shortcut.TargetPath = \"$PWD\TuloClient.exe\"; $Shortcut.WorkingDirectory = \"$PWD\"; $Shortcut.Save()"

echo.
echo ===================================================
echo   Instalacion completada! Ya puedes abrir el
echo   acceso directo 'TULO Client' en tu escritorio.
echo ===================================================
pause
