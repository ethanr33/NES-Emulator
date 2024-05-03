#pragma once

#include <string>
#include "CPU.h"

using std::string;

class NES {
    
    CPU* cpu;

    const int CPU_CLOCK_FREQUENCY = 1760000;

    public:
        NES();
        CPU* get_cpu() const;
        void initialize();
        bool load_program(const string&);
};