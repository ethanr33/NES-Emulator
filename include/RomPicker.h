#pragma once

#include <optional>
#include <string>

// Opens an SFML window listing .nes files in rom_directory. Returns the
// filename only (e.g. "game.nes") to match Cartridge's "roms/" + name loading.
std::optional<std::string> pick_rom_interactively(const std::string& rom_directory = "roms");
