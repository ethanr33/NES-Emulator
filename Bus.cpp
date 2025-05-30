
#include "Bus.h"

Bus::Bus() {
    cpu = new CPU();
    ppu = new PPU();
    io = new IO();
    apu = new APU();
    cpu->attach_bus(this);
    ppu->attach_bus(this);
}

Bus::Bus(bool ui_disabled) {
    cpu = new CPU();
    ppu = new PPU(ui_disabled);
    io = new IO();
    apu = new APU();
    cpu->attach_bus(this);
    ppu->attach_bus(this);
}

uint8_t Bus::read_cpu(uint16_t address) {
    uint8_t data;

    if (cartridge->read_cpu(address, data)) {
        return data;
    }

    if (address >= RAM_MIRROR_START && address <= RAM_MIRROR_END) {
        return cpu_RAM.at(address & 0x7FF);
    }

    if (address >= PPU_REG_MIRROR_START && address <= PPU_REG_MIRROR_END) {
        return ppu->read_from_cpu(address);
    }

    if (address >= APU_IO_REG_START && address <= APU_IO_REG_END) {
        // TODO: Implement APU / IO registers

        if (address == 0x4016 || address == 0x4017) {
            // Reroute to IO
            uint8_t status = io->read_from_cpu(address);
            return status;
        } else {
            // Reroute to APU
            return apu->read_from_cpu(address);
        }

    }

    return cpu_RAM.at(address);
}

void Bus::write_cpu(uint16_t address, uint8_t val) {
    if (cartridge->write_cpu(address, val)) {
        return;
    }

    if (address >= RAM_MIRROR_START && address <= RAM_MIRROR_END) {
        cpu_RAM.at(address & 0x7FF) = val;
        return;
    }

    if (address >= PPU_REG_MIRROR_START && address <= PPU_REG_MIRROR_END) {
        ppu->write_from_cpu(address, val);
        return;
    }

    // OAM DMA register
    if (address == 0x4014) {
        ppu->load_OAMDMA(val);
        return;
    }

    if (address == 0x4016) {
        // Reroute to IO
        io->write_from_cpu(address, val);
        return;
    }

    if (address >= APU_IO_REG_START && address <= APU_IO_REG_END) {
        // Reroute to APU
        apu->write_from_cpu(address, val);
    }

    
}

void Bus::insert_cartridge(Cartridge* new_cartridge) {
    cartridge = new_cartridge;
    ppu->load_cartridge(new_cartridge);
}

void Bus::reset() {
    cpu->reset();
    ppu->reset();
}

void Bus::tick() {

    if (num_ticks % 3 == 0) {
        cpu->tick();
        num_cpu_cycles++;
    }

    if (num_ticks % 6 == 0) {
        apu->tick();
    }

    ppu->tick();

    num_ticks++;
}

void Bus::halt() {
    throw std::runtime_error("Execution halted at " + std::to_string(cpu->program_counter));
}

void Bus::set_nmi_line(bool is_line_low) {
    if (!is_nmi_suppressed) {
        is_nmi_line_low = is_line_low;
    } else {
        is_nmi_line_low = false;
    }
}

bool Bus::get_nmi_line_status() const {
    if (is_nmi_suppressed) {
        return false;
    }
    return is_nmi_line_low;
}

void Bus::set_nmi_suppression_status(bool new_status) {
    is_nmi_suppressed = new_status;
}