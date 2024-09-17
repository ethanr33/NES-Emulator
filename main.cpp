
#include <iostream>
#include <chrono>
#include "Bus.h"
#include "Helpers.h"

int main() {

    Bus nes = Bus();
    Cartridge* game = new Cartridge("nestest.nes");

    nes.insert_cartridge(game);
    nes.reset();

    const int MICROS_PER_PPU_TICK = 86;

    auto start = std::chrono::steady_clock::now();

    while (nes.ppu->ui->window->isOpen()) {

        sf::Event event;
        while (nes.ppu->ui->window->pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                nes.ppu->ui->window->close();
        }

        auto curr_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(curr_time - start).count();

        //if (elapsed >= MICROS_PER_PPU_TICK) {
            nes.tick();
        //}
        
    }

    return 0;
}
