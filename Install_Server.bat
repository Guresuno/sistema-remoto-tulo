@echo off
:: BatchGotAdmin - Elevar a Administrador automaticamente
:-------------------------------------
REM  --> Comprobar permisos
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"

REM --> Si hay un error, no tenemos privilegios de admin
if '%errorlevel%' NEQ '0' (
    echo Solicitando privilegios de administrador para instalar dependencias...
    goto UACPrompt
) else ( goto gotAdmin )

:UACPrompt
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    set params= %*
    echo UAC.ShellExecute "cmd.exe", "/c ""%~s0"" %params:"=""%", "", "runas", 1 >> "%temp%\getadmin.vbs"

    "%temp%\getadmin.vbs"
    del "%temp%\getadmin.vbs"
    exit /B

:gotAdmin
    pushd "%CD%"
    CD /D "%~dp0"
:--------------------------------------

title Instalador de Dependencias - TULO Server
color 0A

echo ===================================================
echo     INSTALADOR DE DEPENDENCIAS - TULO SERVER
echo ===================================================
echo.

echo [1/2] Comprobando instalacion de ViGEmBus (Mando Virtual)...
sc query "ViGEmBus" >nul 2>&1
if %errorlevel% equ 0 (
    echo [+] ViGEmBus ya se encuentra instalado en este sistema.
) else (
    echo [-] ViGEmBus no encontrado. Descargando instalador oficial...
    powershell -Command "Invoke-WebRequest -Uri 'https://github.com/nefarius/ViGEmBus/releases/download/v1.22.0/ViGEmBusSetup_x64.msi' -OutFile '%temp%\ViGEmBusSetup_x64.msi'"
    
    echo [*] Instalando ViGEmBus de forma silenciosa...
    msiexec /i "%temp%\ViGEmBusSetup_x64.msi" /quiet /qn /norestart
    
    echo [+] ViGEmBus instalado correctamente.
    del "%temp%\ViGEmBusSetup_x64.msi"
)

echo.
echo [2/2] Creando acceso directo en el Escritorio...
powershell -Command "$WshShell = New-Object -comObject WScript.Shell; $Shortcut = $WshShell.CreateShortcut(\"$Home\Desktop\TULO Server.lnk\"); $Shortcut.TargetPath = \"$PWD\TuloServer.exe\"; $Shortcut.WorkingDirectory = \"$PWD\"; $Shortcut.Save()"

echo.
echo ===================================================
echo   Instalacion completada! Ya puedes abrir el
echo   acceso directo 'TULO Server' en tu escritorio.
echo ===================================================
pause
