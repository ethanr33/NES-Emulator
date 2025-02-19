
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
    int cur_cycles = 0;

    int num_iterations = 0;
    

    while (true) {

        // sf::Event event;
        // while (nes.ppu->ui->window->pollEvent(event)) {
        //     if (event.type == sf::Event::Closed) {
        //         nes.ppu->ui->window->close();
        //     }
        // }

        nes.tick();
        cur_cycles++;
    }

    auto current = std::chrono::high_resolution_clock::now();

    // Calculate the elapsed time
    std::chrono::duration<double> elapsed = current - start;

    std::cout << "Ran " << cur_cycles << " cycles in " << elapsed.count() << std::endl;
    std::cout << "FPS: " << nes.ppu->frames_elapsed / elapsed.count() << std::endl;

    return 0;
}
