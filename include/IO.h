
#include "controllers/StandardController.h"

struct IO {

    uint8_t read_from_cpu();
    uint8_t write_from_cpu();

    Controller* port1_controller = new StandardController();
    Controller* port2_controller = nullptr;

    bool strobe_mode;

    void connect_controller(Controller*, uint8_t);

    uint8_t read_from_cpu(uint16_t);
    void write_from_cpu(uint16_t, uint8_t);

};