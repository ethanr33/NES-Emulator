
#include "APU.h"

#include <stdexcept>
#include <string>

APU::APU() {

}

uint8_t APU::read_from_cpu(uint16_t address) {
    switch (address) {
        case 0x4015:
            // unimplemented
            return 0;
        default:
            throw std::runtime_error("Attempted to read APU from invalid address " + std::to_string(address));
    }
}

void APU::write_from_cpu(uint16_t address, uint8_t val) {
    switch (address) {
        case 0x4000:
            pulse_channel_1.duty = (val & 0xCF) >> 6;
            pulse_channel_1.length_counter_halted = val & 0x20;
            pulse_channel_1.is_volume_constant = val & 0x10;
            pulse_channel_1.volume = val & 0x0F;
            break;
        case 0x4001:
            pulse_channel_1.sweep_enabled = val & 0x80;
            pulse_channel_1.sweep_period = val & 0x70;
            pulse_channel_1.is_sweep_negated = val & 0x08;
            pulse_channel_1.sweep_shift = val & 0x07;
            break;
        case 0x4002:
            pulse_channel_1.timer = (pulse_channel_1.timer & 0x700) | val;
            break;
        case 0x4003:
            pulse_channel_1.sound_length = val >> 5;
            pulse_channel_1.timer = (pulse_channel_1.timer & 0x0FF) | ((val & 0x7) << 8);
            break;
        case 0x4004:
            pulse_channel_2.duty = (val & 0xCF) >> 6;
            pulse_channel_2.length_counter_halted = val & 0x20;
            pulse_channel_2.is_volume_constant = val & 0x10;
            pulse_channel_2.volume = val & 0x0F;
            break;
        case 0x4005:
            pulse_channel_2.sweep_enabled = val & 0x80;
            pulse_channel_2.sweep_period = val & 0x70;
            pulse_channel_2.is_sweep_negated = val & 0x08;
            pulse_channel_2.sweep_shift = val & 0x07;
            break;
        case 0x4006:
            pulse_channel_2.timer = (pulse_channel_2.timer & 0x700) | val;
            break;
        case 0x4007:
            pulse_channel_1.sound_length = val >> 5;
            pulse_channel_1.timer = (pulse_channel_1.timer & 0x0FF) | ((val & 0x7) << 8);
            break;
        case 0x4008:
            break;
        case 0x4009:
            break;
        case 0x400A:
            break;
        case 0x400B:
            break;
        case 0x400C:
            break;
        case 0x400D:
            break;
        case 0x400E:
            break;
        case 0x400F:
            break;
        case 0x4010:
            break;
        case 0x4011:
            break;
        case 0x4012:
            break;
        case 0x4013:
            break;
        case 0x4015:
            apu_status.dmc_enable = val & 0x10;
            apu_status.noise_enable = val & 0x08;
            apu_status.triangle_enable = val & 0x04;
            apu_status.pulse_2_enable = val & 0x02;
            apu_status.pulse_1_enable = val & 0x01;
            break;
        case 0x4017:
            break;
        default:
            throw std::runtime_error("Attempted to write to APU from invalid address " + std::to_string(address));
            break;
    }
}

void APU::attach_bus(Bus* b) {
    bus = b;
}

void APU::tick() {

    // The frame counter keeps track of how many APU cycles have passed
    // Based on its mode, it will periodically clock each of the sound generators

    if (frame_counter.mode == 0) {
        if (frame_counter.cycles_elapsed == 7457) {

        } else if (frame_counter.cycles_elapsed == 14913) {
    
        } else if (frame_counter.cycles_elapsed == 22371) {
    
        } else if (frame_counter.cycles_elapsed == 29828) {

        } else if (frame_counter.cycles_elapsed == 29829) {
    
        } else if (frame_counter.cycles_elapsed == 29830) {
            // Counter resets to 0 on the next APU cycle
            // Set to -1 because this variable will be incremented at the end of the function
            frame_counter.cycles_elapsed = -1;
        }
    } else {
        if (frame_counter.cycles_elapsed == 7457) {

        } else if (frame_counter.cycles_elapsed == 14913) {
    
        } else if (frame_counter.cycles_elapsed == 22371) {
    
        } else if (frame_counter.cycles_elapsed == 29829) {
    
        } else if (frame_counter.cycles_elapsed == 37281) {
    
        } else if (frame_counter.cycles_elapsed == 37282) {
            // Counter resets to 0 on the next APU cycle
            // Set to -1 because this variable will be incremented at the end of the function
            frame_counter.cycles_elapsed = -1;
        }
    }

    frame_counter.cycles_elapsed++;
}