
#include "Controller.h"

struct StandardController : Controller {
    uint8_t controller_state = 0;

    bool read_input() override;
    void set_strobe(bool) override;
};