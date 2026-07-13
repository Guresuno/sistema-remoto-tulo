$ServerDir = "R:\TULO_Releases\TuloServer"
$ClientDir = "R:\TULO_Releases\TuloClient"

# Limpiar y crear directorios de releases
if (Test-Path "R:\TULO_Releases") {
    Remove-Item "R:\TULO_Releases" -Recurse -Force
}
New-Item -ItemType Directory -Path $ServerDir | Out-Null
New-Item -ItemType Directory -Path $ClientDir | Out-Null

# Agregar MSYS2 al PATH
$env:PATH = "R:\msys64\ucrt64\bin;" + $env:PATH

# Build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make

# Copy executables
Copy-Item "TuloServer.exe" -Destination $ServerDir
Copy-Item "TuloClient.exe" -Destination $ClientDir

# Copy installers
Copy-Item "..\Install_Server.bat" -Destination $ServerDir
Copy-Item "..\Install_Client.bat" -Destination $ClientDir

# Copiar dependencias del Cliente (FFmpeg, SDL2 y TODAS las DLLs recursivas)
$lddClient = R:\msys64\usr\bin\ldd.exe TuloClient.exe
foreach ($line in $lddClient) {
    if ($line -match "=>\s*(/[^\s]+)") {
        $msysPath = $matches[1]
        if ($msysPath -match "^/ucrt64/bin/(.+)") {
            $dllName = $matches[1]
            if (Test-Path "R:\msys64\ucrt64\bin\$dllName") {
                Copy-Item "R:\msys64\ucrt64\bin\$dllName" -Destination "$ClientDir"
            }
        }
    }
}

# Copiar dependencias del Servidor (Solo stdc++, gcc, winpthread, etc)
$lddServer = R:\msys64\usr\bin\ldd.exe TuloServer.exe
foreach ($line in $lddServer) {
    if ($line -match "=>\s*(/[^\s]+)") {
        $msysPath = $matches[1]
        if ($msysPath -match "^/ucrt64/bin/(.+)") {
            $dllName = $matches[1]
            if (Test-Path "R:\msys64\ucrt64\bin\$dllName") {
                Copy-Item "R:\msys64\ucrt64\bin\$dllName" -Destination "$ServerDir"
            }
        }
    }
}

# Create Zip files
cd "R:\TULO_Releases"
Compress-Archive -Path "TuloServer\*" -DestinationPath "TuloServer.zip" -Force
Compress-Archive -Path "TuloClient\*" -DestinationPath "TuloClient.zip" -Force

# Clean up unzipped folders
Remove-Item $ServerDir -Recurse -Force
Remove-Item $ClientDir -Recurse -Force

Write-Host "Releases created successfully with all required DLL dependencies in R:\TULO_Releases!"
