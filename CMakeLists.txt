cmake_minimum_required(VERSION 3.16)
project(MyChess VERSION 1.0.0 LANGUAGES CXX)

# =========================
# C++ Standard & Compiler Options
# =========================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -fconstexpr-steps=30000000 -fconstexpr-backtrace-limit=0)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -flto")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /DDEBUG")
endif()

# =========================
# Default Build Type
# =========================
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# =========================
# Source & Header Files
# =========================
file(GLOB_RECURSE CHESS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
file(GLOB_RECURSE CHESS_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

# =========================
# Create Executable
# =========================
add_executable(my_chess ${CHESS_SOURCES} ${CHESS_HEADERS})

# Include directories for headers
target_include_directories(my_chess PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nnue
)

# =========================
# Custom "run" Target
# =========================
add_custom_target(run
    COMMAND my_chess
    DEPENDS my_chess
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running chess engine"
)

# =========================
# Optional: Tests
# =========================
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_executable(chess_tests
        ${CHESS_SOURCES}
    )
    target_include_directories(chess_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    add_test(NAME ChessTests COMMAND chess_tests)
endif()

# =========================
# Install Configuration
# =========================
install(TARGETS my_chess
    RUNTIME DESTINATION bin
    COMPONENT Runtime
)

# =========================
# Packaging
# =========================
set(CPACK_PACKAGE_NAME "MyChess")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A chess engine")
set(CPACK_PACKAGE_VENDOR "Ruslan Sharafetdinov")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
else()
    set(CPACK_GENERATOR "TGZ;DEB")
endif()

include(CPack)

# =========================
# Build Info
# =========================
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Sources found: ${CHESS_SOURCES}")
