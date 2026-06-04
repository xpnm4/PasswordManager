# build_project.ps1
# Loads development environment paths and compiles the C++ Password Manager project

$ErrorActionPreference = "Stop"

# Load local environment paths
if (Test-Path "D:\PasswordManager\env.ps1") {
    Write-Host "Loading environment settings..."
    . D:\PasswordManager\env.ps1
} else {
    Write-Warning "env.ps1 not found. Attempting build with default system PATH."
}

# Configure build directory
Write-Host "Configuring CMake project..."
cmake -B build -S . -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# Compile
Write-Host "Compiling targets (CLI & GUI)..."
cmake --build build --config Release

Write-Host "Compilation successful! Executables are located in 'D:\PasswordManager\build\':" -ForegroundColor Green
Write-Host "  - Command-line version: D:\PasswordManager\build\password_manager_cli.exe" -ForegroundColor Green
Write-Host "  - Qt GUI desktop version: D:\PasswordManager\build\password_manager_gui.exe" -ForegroundColor Green
