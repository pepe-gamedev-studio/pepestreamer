#include "color.h"

namespace pepestreamer {

uint8_t Clip(uint8_t x) { return x > 255 ? 255 : x < 0 ? 0 : x; }

Yuv Rgb2Yuv(const Rgb &color) {
    return {Clip(((66 * color[0] + 129 * color[1] + 25 * color[2] + 128) >> 8) + 16),
            Clip(((-38 * color[0] - 74 * color[1] + 112 * color[2] + 128) >> 8) + 128),
            Clip(((112 * color[0] - 94 * color[1] - 18 * color[2] + 128) >> 8) + 128)};
}

} // namespace pepestreamer