#pragma once
#include <array>

namespace pepestreamer {

using Rgb = std::array<uint8_t, 3>;
using Yuv = std::array<uint8_t, 3>;

Yuv Rgb2Yuv(const Rgb &color);

} // namespace pepestreamer
