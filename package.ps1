$ErrorActionPreference = "Stop"

$ReleaseDir = "R:\TULO_Releases"
$ServerDir = "$ReleaseDir\TuloServer"
$ClientDir = "$ReleaseDir\TuloClient"

# Create directories
New-Item -ItemType Directory -Force -Path $ServerDir | Out-Null
New-Item -ItemType Directory -Force -Path $ClientDir | Out-Null

# Build
$env:PATH = "R:\msys64\ucrt64\bin;" + $env:PATH
cd R:\StreamEngine\build
cmake -G "MinGW Makefiles" ..
mingw32-make

# Copiar dependencias de MinGW para el Cliente
Copy-Item "R:\msys64\ucrt64\bin\libgcc_s_seh-1.dll" -Destination "$ClientDir"
Copy-Item "R:\msys64\ucrt64\bin\libstdc++-6.dll" -Destination "$ClientDir"
Copy-Item "R:\msys64\ucrt64\bin\libwinpthread-1.dll" -Destination "$ClientDir"

# Dependencias FFmpeg y SDL2
$ffmpegDlls = @("avcodec-61.dll", "avutil-59.dll", "swscale-8.dll", "swresample-5.dll", "SDL2.dll", "libx264-165.dll", "zlib1.dll", "liblzma-5.dll", "libiconv-2.dll", "libbz2-1.dll")
foreach ($dll in $ffmpegDlls) {
    if (Test-Path "R:\msys64\ucrt64\bin\$dll") {
        Copy-Item "R:\msys64\ucrt64\bin\$dll" -Destination "$ClientDir"
    }
}

# Copy executables
Copy-Item "TuloServer.exe" -Destination $ServerDir
Copy-Item "TuloClient.exe" -Destination $ClientDir

# Copy installers
Copy-Item "..\Install_Server.bat" -Destination $ServerDir
Copy-Item "..\Install_Client.bat" -Destination $ClientDir

# Copy Standard MinGW dependencies
$Dlls = @("libgcc_s_seh-1.dll", "libstdc++-6.dll", "libwinpthread-1.dll")
foreach ($dll in $Dlls) {
    $dllPath = "R:\msys64\ucrt64\bin\$dll"
    if (Test-Path $dllPath) {
        Copy-Item $dllPath -Destination $ServerDir
        Copy-Item $dllPath -Destination $ClientDir
    }
}

# Zip them up
Compress-Archive -Path "$ServerDir\*" -DestinationPath "$ReleaseDir\TuloServer.zip" -Force
Compress-Archive -Path "$ClientDir\*" -DestinationPath "$ReleaseDir\TuloClient.zip" -Force

Write-Host "Releases created successfully in $ReleaseDir!"
