cmake_minimum_required(VERSION 3.12)

# Project name and version
project(MyChess VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fconstexpr-steps=30000000 -fconstexpr-backtrace-limit=0")

# Set build type to Release by default (for -O2 optimization)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Find all source files in src/ and src/nnue/
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/nnue/*.cpp"
)

# Create the executable
add_executable(my_chess ${SOURCES})

# Set output name to match your makefile
set_target_properties(my_chess PROPERTIES OUTPUT_NAME "my_chess.exe")

# Include directories (if you have headers in src/)
target_include_directories(my_chess PRIVATE src)

# Custom target to run the program (equivalent to 'make run')
add_custom_target(run
    COMMAND my_chess.exe
    DEPENDS my_chess
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running the chess engine"
)
