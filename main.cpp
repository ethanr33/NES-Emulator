
#include <iostream>
#include <chrono>
#include <optional>
#include <string>
#include "Bus.h"
#include "Helpers.h"
#include "RomPicker.h"

int main(int argc, char** argv) {

    std::string rom_file;
    if (argc >= 2) {
        rom_file = argv[1];
    } else {
        std::optional<std::string> picked = pick_rom_interactively("roms");
        if (!picked) {
            return 0;
        }
        rom_file = *picked;
    }

    Bus nes = Bus();
    Cartridge* game = new Cartridge(rom_file);

    nes.insert_cartridge(game);
    nes.reset();

    auto start = std::chrono::high_resolution_clock::now();
    int frame_count_start = 0;
    int cur_cycles = 0;    

    while (nes.ppu->ui->window->isOpen()) {

        sf::Event event;
        while (cur_cycles % 300000 == 0 && nes.ppu->ui->window->pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                nes.ppu->ui->window->close();
            }
        }

        if (nes.ppu->frames_elapsed > frame_count_start + 30) {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;
            nes.ppu->ui->window->setTitle("NES Emulator frame time: " + std::to_string(elapsed.count() / 30));
            start = std::chrono::high_resolution_clock::now();
            frame_count_start = nes.ppu->frames_elapsed;

        }

        nes.tick();
        cur_cycles++;
    }

    return 0;
}
