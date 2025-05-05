
#pragma once

#include <cstdint>

#include "Bus.h"

struct Pulse_Channel {

    // Updated at 4000/4004
    uint8_t duty;
    bool length_counter_halted;
    bool is_volume_constant;
    uint8_t volume;

    // Updated at 4001/4005
    bool sweep_enabled;
    uint8_t sweep_period;
    bool is_sweep_negated;
    uint8_t sweep_shift;

    // Updated at 4002/4006 and 4003/4007
    uint16_t timer;
    uint8_t sound_length;

    Pulse_Channel() : duty(0), length_counter_halted(0), is_volume_constant(0), volume(0), sweep_enabled(0), sweep_period(0), is_sweep_negated(0), sweep_shift(0), timer(0), sound_length(0)  {}

};

struct Triangle_Channel {

    // Updated at 4008
    bool length_counter_halted;
    uint8_t linear_counter_reload;

    // Updated at 400A/400B
    uint16_t timer;
    uint8_t sound_length;

    Triangle_Channel() : length_counter_halted(0), linear_counter_reload(0), timer(0), sound_length(0) {}

};


struct Noise_Channel {

    // Updated at 400C
    bool length_counter_halted;
    bool is_volume_constant;
    uint8_t volume;

    // Updated at 400E
    bool loop_noise;
    uint8_t noise_period;
    
    // Updated at 400F
    uint8_t sound_length;

};

struct DMC_Channel {

    // Updated at 4010
    bool irq_enable;
    bool is_looping;
    uint8_t frequency;

    // Updated at 4011
    uint8_t load_counter;

    // Updated at 4012
    uint8_t sample_address;

    // Updated at 4013
    uint8_t sample_length;
};

struct APU_Status {
    bool dmc_enable;
    bool noise_enable;
    bool triangle_enable;
    bool pulse_2_enable;
    bool pulse_1_enable;

    APU_Status() : dmc_enable(0), noise_enable(0), triangle_enable(0), pulse_2_enable(0), pulse_1_enable(0) {}
};

struct Frame_Counter {

    bool mode;
    bool irq_inhibited;

    uint64_t cycles_elapsed;

    Frame_Counter() : mode(0), irq_inhibited(0), cycles_elapsed(0) {}

};

struct APU {
    APU();

    Bus* bus;

    Pulse_Channel pulse_channel_1;
    Pulse_Channel pulse_channel_2;

    Triangle_Channel triangle_channel;

    DMC_Channel dmc_channel;

    APU_Status apu_status;

    Frame_Counter frame_counter;

    void attach_bus(Bus*);

    void tick_pulse_channel_1();
    void tick_pulse_channel_2();
    void tick_triangle_channel();

    void tick();

    uint8_t read_from_cpu(uint16_t);
    void write_from_cpu(uint16_t, uint8_t);
};