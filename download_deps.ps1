# download_deps.ps1
# Downloads headers and source files for nlohmann/json, PicoSHA2, and tiny-AES-c

$ErrorActionPreference = "Stop"

# Create target directories
New-Item -ItemType Directory -Force -Path "D:\PasswordManager\include\nlohmann"
New-Item -ItemType Directory -Force -Path "D:\PasswordManager\include\crypto"
New-Item -ItemType Directory -Force -Path "D:\PasswordManager\src\crypto"

Write-Host "Downloading nlohmann/json..."
Invoke-WebRequest -Uri "https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp" -OutFile "D:\PasswordManager\include\nlohmann\json.hpp"

Write-Host "Downloading PicoSHA2..."
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/okdshin/PicoSHA2/master/picosha2.h" -OutFile "D:\PasswordManager\include\crypto\picosha2.h"

Write-Host "Downloading tiny-AES-c (aes.h)..."
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/kokke/tiny-AES-c/master/aes.h" -OutFile "D:\PasswordManager\include\crypto\aes.h"

Write-Host "Downloading tiny-AES-c (aes.c)..."
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/kokke/tiny-AES-c/master/aes.c" -OutFile "D:\PasswordManager\src\crypto\aes.c"

Write-Host "All external dependencies downloaded successfully!" -ForegroundColor Green
