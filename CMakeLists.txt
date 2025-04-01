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

# find_package(SFML 2 REQUIRED COMPONENTS graphics system windows)

# Add main executable
add_executable(main main.cpp Bus.cpp CPU.cpp PPU.cpp Cartridge.cpp Helpers.cpp UI.cpp mappers/Mapper.cpp mappers/Mapper000.cpp mappers/Mapper001.cpp IO.cpp controllers/StandardController.cpp)

# Add debug executable
add_executable(debug main.cpp Bus.cpp CPU.cpp PPU.cpp Cartridge.cpp Helpers.cpp UI.cpp mappers/Mapper.cpp mappers/Mapper000.cpp mappers/Mapper001.cpp IO.cpp controllers/StandardController.cpp)

# Add debug executable
# add_executable(chr_dump chr_dump.cpp Bus.cpp CPU.cpp PPU.cpp Cartridge.cpp Helpers.cpp UI.cpp mappers/Mapper.cpp mappers/Mapper000.cpp mappers/Mapper001.cpp IO.cpp controllers/StandardController.cpp)

# Add debugger executable
# add_executable(debugger debug.cpp Bus.cpp CPU.cpp PPU.cpp Cartridge.cpp Helpers.cpp UI.cpp mappers/Mapper.cpp mappers/Mapper000.cpp mappers/Mapper001.cpp IO.cpp controllers/StandardController.cpp)

target_link_libraries(main PRIVATE sfml-system sfml-window sfml-graphics)
target_compile_features(main PRIVATE cxx_std_17)

target_link_libraries(debug PRIVATE sfml-system sfml-window sfml-graphics)
target_compile_features(debug PRIVATE cxx_std_17)

# target_link_libraries(chr_dump PRIVATE sfml-system sfml-window sfml-graphics)
# target_compile_features(chr_dump PRIVATE cxx_std_17)

# target_link_libraries(debugger PRIVATE sfml-system sfml-window sfml-graphics)
# target_compile_features(debugger PRIVATE cxx_std_17)

set_target_properties(main PROPERTIES
   COMPILE_FLAGS "-O3"
   LINK_FLAGS "-O3"
)

# Ensure that debug symbols are included
set_target_properties(debug PROPERTIES
   COMPILE_FLAGS "-g"
   LINK_FLAGS "-g"
)

# set_target_properties(debugger PROPERTIES
#    COMPILE_FLAGS "-g"
#    LINK_FLAGS "-g"
# )

# set_target_properties(chr_dump PROPERTIES
#    COMPILE_FLAGS "-g"
#    LINK_FLAGS "-g"
# )