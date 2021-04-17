#include "../../Libav.h"

#include <string>

namespace pepestreamer {

std::string avErrorStr(int error) {
    const size_t buffSize = 256;
    std::string ret(buffSize, '\0');

    av_strerror(error, ret.data(), buffSize);

    ret.resize(ret.find('\0'));

    return ret;
}

} // namespace pepestreamer
