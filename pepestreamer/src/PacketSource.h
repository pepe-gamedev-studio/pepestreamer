#pragma once
#include "Libav.h"

namespace pepestreamer {

struct PacketSource {
  virtual bool ReadNext(AVPacket &packet) = 0;
  virtual AVRational Timebase(int64_t stream) = 0;
  virtual ~PacketSource() = default;
};

} // namespace pepestreamer