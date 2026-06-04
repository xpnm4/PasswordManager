# setup_env.ps1
# Automates environment setup for C++ and Qt on D: drive

$ErrorActionPreference = "Stop"

# Create directories
Write-Host "Creating directories on D:..."
New-Item -ItemType Directory -Force -Path "D:\DevTools"
New-Item -ItemType Directory -Force -Path "D:\PasswordManager"

# 1. Install MinGW 11.2.0 (mislabeled as tools_mingw90 in aqt)
Write-Host "Installing MinGW 11.2.0 compiler to D:\DevTools..."
python -m aqt install-tool -O D:\DevTools windows desktop tools_mingw90 qt.tools.win64_mingw900

# 2. Install CMake
Write-Host "Installing CMake to D:\DevTools..."
python -m aqt install-tool -O D:\DevTools windows desktop tools_cmake qt.tools.cmake

# 3. Install Qt 6.7.2 framework
Write-Host "Installing Qt 6.7.2 win64_mingw framework to D:\DevTools..."
python -m aqt install-qt -O D:\DevTools windows desktop 6.7.2 win64_mingw

# 4. Generate local env.ps1 helper script
Write-Host "Generating local env.ps1..."
$envScript = @"
`$env:Path = "D:\DevTools\Tools\CMake\bin;D:\DevTools\Tools\mingw1120_64\bin;`$env:Path"
`$env:QT_DIR = "D:\DevTools\6.7.2\mingw_64"
`$env:CMAKE_PREFIX_PATH = "D:\DevTools\6.7.2\mingw_64"
Write-Host "C++ and Qt environment paths configured for this session." -ForegroundColor Green
"@
Set-Content -Path "D:\PasswordManager\env.ps1" -Value $envScript

Write-Host "Environment setup complete! Use 'D:\PasswordManager\env.ps1' to load paths." -ForegroundColor Green
