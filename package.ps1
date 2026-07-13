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

# Copy executables
Copy-Item "TuloServer.exe" -Destination $ServerDir
Copy-Item "TuloClient.exe" -Destination $ClientDir

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
