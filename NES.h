#pragma once

#include <string>
#include "CPU.h"

using std::string;

class NES {
    
    CPU cpu;

    public:

        CPU get_cpu() const;
        void initialize();
        bool load_program(const string&);
};