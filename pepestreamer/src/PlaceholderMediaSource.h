#pragma once
#include "Libav.h"
#include "PacketSource.h"

#include <vector>

namespace pepestreamer {

struct PlaceholderPayload {
    std::vector<AVPacket *> packets;
    AVRational videoTimebase;
    AVRational audioTimebase;
};

class PlaceholderMediaSource : public PacketSource {
  public:
    explicit PlaceholderMediaSource(const PlaceholderPayload &placeholder);
    bool ReadNext(AVPacket &pkt) override;
    AVRational Timebase(int64_t stream) override;

  private:
    const PlaceholderPayload *placeholder;
    int64_t uLast;
    int64_t uOffset;
    size_t nextIdx = 0;
};

} // namespace pepestreamer
