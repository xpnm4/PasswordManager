cmake_minimum_required(VERSION 3.16)
project(PasswordManager VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt6 automatic code generation features
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Include directories
include_directories(include include/crypto)

# Common sources for both targets
set(COMMON_SOURCES
    src/PasswordManager.cpp
    src/PasswordGenerator.cpp
    src/Encryptor.cpp
    src/AuthManager.cpp
    src/FileStorage.cpp
    src/crypto/aes.c
)

# Compiler definitions - Set AES256 for tiny-AES-c
add_compile_definitions(AES256=1)

# 1. CLI target (Console application)
add_executable(password_manager_cli
    src/main_cli.cpp
    ${COMMON_SOURCES}
)

# 2. GUI target (Windows desktop application, no background console)
add_executable(password_manager_gui WIN32
    src/main_gui.cpp
    src/MainWindow.cpp
    include/MainWindow.h
    ${COMMON_SOURCES}
)

# Link libraries
target_link_libraries(password_manager_gui PRIVATE Qt6::Widgets)
