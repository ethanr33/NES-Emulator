
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

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // At 60 FPS each frame should be about 16666.6666... microseconds long
        if (elapsed_time > 16666) {
            nes.ppu->ui->window->setTitle("FPS: " + std::to_string(1000000 * (nes.ppu->frames_elapsed - frame_count_start) / (double) elapsed_time));
            start = std::chrono::high_resolution_clock::now();
            frame_count_start = nes.ppu->frames_elapsed;
        }

        nes.tick();
        cur_cycles++;
    }

    return 0;
}
