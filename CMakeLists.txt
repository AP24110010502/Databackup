cmake_minimum_required(VERSION 3.14)
project(DataBackupPlanner VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Compiler warnings ──────────────────────────────────────────────────────────
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ── Includes ──────────────────────────────────────────────────────────────────
include_directories(${PROJECT_SOURCE_DIR}/include)

# ── Source files ──────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/File.cpp
    src/BackupManager.cpp
    src/RecoverySystem.cpp
)

# ── Main executable ───────────────────────────────────────────────────────────
add_executable(DataBackupPlanner ${SOURCES})

# ── Tests executable (optional) ───────────────────────────────────────────────
if(EXISTS "${PROJECT_SOURCE_DIR}/tests/test_main.cpp")
    add_executable(RunTests
        tests/test_main.cpp
        src/File.cpp
        src/BackupManager.cpp
        src/RecoverySystem.cpp
    )
endif()

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS DataBackupPlanner DESTINATION bin)
