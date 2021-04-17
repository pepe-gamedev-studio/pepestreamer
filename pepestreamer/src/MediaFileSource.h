#pragma once
#include "Common.h"
#include "Libav.h"
#include "PacketSource.h"

#include <string>

namespace pepestreamer {

class MediaFileSource : public PacketSource {
    using seconds = int64_t;

  public:
    MediaFileSource();
    MediaFileSource(const std::string &filename);
    void Open(const std::string &filename);
    void Seek(seconds offset);
    bool ReadNext(AVPacket &packet) override;
    AVRational Timebase(int64_t stream) override;
    ~MediaFileSource();

  public:
    AVFormatContext *formatContext = nullptr;
};

} // namespace pepestreamer
