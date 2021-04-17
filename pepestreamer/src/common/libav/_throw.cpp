#include "_throw.h"
#include "../../Common.h"

#include <iostream>
#include <stdexcept>

namespace pepestreamer {

ThrowTrait _throw;

void operator,(ThrowTrait, int ret) {
    if (ret < 0) {
        auto what = pepestreamer::avErrorStr(ret);

#ifdef DEBUG
        std::cerr << what << std::endl;
#endif // DEBUG

        throw std::runtime_error(what);
    }
}

} // namespace pepestreamer
