
#include <iostream>
#include <SFML/Window.hpp>

#include "StandardController.h"

bool StandardController::read_input() {

    bool key_pressed;

    if (is_strobing) {
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
    }

    //std::cout << "Checking if input " << controller_state << " is pressed" << std::endl;

    switch (controller_state) {
        case 0:
            // Requesting to read A
            controller_state++;
            key_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
            break;
        case 1:
            // Requesting to read B
            controller_state++;
            key_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::B);
            break;
        case 2:
            // Requesting to read Select
            controller_state++;
            key_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::O);
            break;
        case 3:
            // Requesting to read Start
            controller_state++;
            key_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P);
            break;
        case 4:
            // Requesting to read Up
            controller_state++;
            key_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);
            break;
        case 5:
            // Requesting to read Down
            controller_state++;
            key_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);

            if (key_pressed) {
                std::cout << "Down is pressed" << std::endl;
            }
            break;
        case 6:
            // Requesting to read Left
            controller_state++;
            key_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
            break;
        case 7:
            // Requesting to read Right
            controller_state++;
            key_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
            break;
        default:
            key_pressed = true;
            break;
    }

    //std::cout << "Input  " << controller_state - 1 << ": " << key_pressed << std::endl;

    return key_pressed;
}

void StandardController::set_strobe(bool strobe_status) {
    is_strobing = strobe_status;

    if (is_strobing) {
        controller_state = 0;
    }
}