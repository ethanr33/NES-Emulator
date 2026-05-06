cmake_minimum_required(VERSION 3.31)
project(NES_Emulator)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

# Add main executable
add_executable(nesemu)
add_executable(debug)
add_executable(chr_dump)
add_executable(debugger)

file(GLOB_RECURSE NESEMU-SOURCES "./src/*.cpp")
file(GLOB_RECURSE NESEMU-HEADERS "./include/*.h")

target_sources(nesemu
   PRIVATE
      ${NESEMU-SOURCES}
      main.cpp
   PUBLIC 
      FILE_SET HEADERS
      BASE_DIRS
         include 
      FILES 
         ${NESEMU_HEADERS}
)

target_sources(debug
   PRIVATE 
      ${NESEMU-SOURCES}
      main.cpp
   PUBLIC 
      FILE_SET HEADERS
      BASE_DIRS
         include 
      FILES 
         ${NESEMU_HEADERS}
)

target_sources(chr_dump
   PRIVATE 
      ${NESEMU-SOURCES}
      chr_dump.cpp
   PUBLIC 
      FILE_SET HEADERS
      BASE_DIRS
         include 
      FILES 
         ${NESEMU_HEADERS}
)

target_sources(debugger
   PRIVATE 
      ${NESEMU-SOURCES}
      debug.cpp
   PUBLIC 
      FILE_SET HEADERS
      BASE_DIRS
         include 
      FILES 
         ${NESEMU_HEADERS}
)

target_link_libraries(nesemu PRIVATE sfml-system sfml-window sfml-graphics)
target_link_libraries(debug PRIVATE sfml-system sfml-window sfml-graphics)
target_link_libraries(chr_dump PRIVATE sfml-system sfml-window sfml-graphics)
target_link_libraries(debugger PRIVATE sfml-system sfml-window sfml-graphics)

target_compile_features(nesemu PRIVATE cxx_std_17)
target_compile_features(debug PRIVATE cxx_std_17)
target_compile_features(chr_dump PRIVATE cxx_std_17)
target_compile_features(debugger PRIVATE cxx_std_17)

set_target_properties(nesemu PROPERTIES
   COMPILE_FLAGS "-O3 -flto"
   LINK_FLAGS "-O3 -flto" 
)

# Ensure that debug symbols are included
set_target_properties(debug PROPERTIES
   COMPILE_FLAGS "-g -Wall -Wextra -fsanitize=address"
   LINK_FLAGS "-g -Wall -Wextra -fsanitize=address"
)

set_target_properties(debugger PROPERTIES
   COMPILE_FLAGS "-g"
   LINK_FLAGS "-g"
)

set_target_properties(chr_dump PROPERTIES
   COMPILE_FLAGS "-g"
   LINK_FLAGS "-g"
)