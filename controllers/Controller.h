
#include <cstdint>

struct Controller {

    bool is_strobing = false;

    virtual bool read_input() = 0;
    virtual void set_strobe(bool) = 0;

};