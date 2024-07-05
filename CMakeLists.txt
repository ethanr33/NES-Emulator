cmake_minimum_required(VERSION 3.22)
project(NES_Emulator)

# Add main executable
add_executable(main main.cpp Bus.cpp CPU.cpp PPU.cpp Cartridge.cpp Helpers.cpp mappers/Mapper.cpp mappers/Mapper000.cpp)

# Add debug executable
# add_executable(debug debug.cpp NES.cpp CPU.cpp Helpers.cpp)

# Add test executable
# add_executable(cputest tests/cputest.cpp CPU.cpp Helpers.cpp)
