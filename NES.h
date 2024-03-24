#pragma once
#include "CPU.h"

class NES {
    
    CPU cpu;

    public:

        CPU get_cpu() const;
        void initialize();
        void load_program();
};