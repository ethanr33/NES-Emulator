cmake_minimum_required(VERSION 3.22)
project(NES_Emulator)

find_package(Catch2 3 REQUIRED)

# Add main executable
add_executable(main main.cpp CPU.cpp)

# Add debug executable
add_executable(debug debug.cpp NES.cpp CPU.cpp)

# Add test executable
add_executable(cputest tests/cputest.cpp CPU.cpp)

# Link Catch2 to test executable
target_link_libraries(cputest Catch2::Catch2WithMain)

# Enable testing
include(Catch)
catch_discover_tests(cputest)
