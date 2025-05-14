
#include <iostream>
#include <chrono>
#include <optional>
#include "Bus.h"
#include "Helpers.h"

int main(int argc, char** argv) {

    if (argc != 2) {
        std::cout << "Invalid usage: ./main <rom file name>" << std::endl;
        return 1;
    }

    Bus nes = Bus();
    Cartridge* game = new Cartridge(argv[1]);

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
