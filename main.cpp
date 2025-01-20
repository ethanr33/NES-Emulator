
#include <iostream>
#include <chrono>
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

    int num_ticks = 0;

    while (nes.ppu->ui->window->isOpen()) {

        sf::Event event;
        while (nes.ppu->ui->window->pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                nes.ppu->ui->window->close();
        }

        nes.tick();
        
    }

    return 0;
}
