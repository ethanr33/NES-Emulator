
#include <stdexcept>
#include <iostream>


#include "IO.h"

void IO::connect_controller(Controller* controller, uint8_t port_num) {
    if (port_num == 1) {
        port1_controller = controller;
    } else if (port_num == 2) {
        port2_controller = controller;
    }
}

uint8_t IO::read_from_cpu(uint16_t address) {
    if (address == 0x4016) {
        if (port1_controller == nullptr) {
            // If no controller is connected, a read returns 0
            return 0;
        }

        bool key_status = port1_controller->read_input();

        return key_status;
    } else if (address == 0x4017) {
        if (port2_controller == nullptr) {
            // If no controller is connected, a read returns 0
            return 0;
        }

        return port2_controller->read_input();
    } else {
        // Invalid read!
        throw std::runtime_error("Attempted to read from address unreadable by IO");
    }
}

void IO::write_from_cpu(uint16_t address, uint8_t val) {
    if (address == 0x4016) {
        // Toggle strobes for both controllers (if they are connected)

        if (port1_controller != nullptr) {
            port1_controller->set_strobe(val & 1);
        }

        if (port2_controller != nullptr) {
            port2_controller->set_strobe(val & 1);
        }
    } else {
        // Invalid write!
        throw std::runtime_error("Attempted to write to address unwriteable by IO");
    }
}