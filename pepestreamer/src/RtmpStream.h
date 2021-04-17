#pragma once
#include <boost/atomic.hpp>
#include <boost/thread.hpp>
#include <string>

#include "Common.h"
#include "Libav.h"

namespace pepestreamer {

struct RtmpStreamOptions {
    AVCodecParameters *videoCodecParameters;
    AVCodecParameters *audioCodecParameters;
    std::string url;
    int32_t fps;
};

class RtmpStream {
    friend class BroadcastController;

  public:
    void Open(RtmpStreamOptions settings);
    void Write(AVPacket &packet);
    AVRational Timebase(int64_t stream);
    ~RtmpStream();

  private:
    void SetupFormatContext();
    void SetupVideoStream();
    void SetupAudioStream();

  public:
    RtmpStreamOptions settings;

    int64_t startTime;

    AVFormatContext *formatContext;
    AVStream *videoStream;
    AVStream *audioStream;
};

} // namespace pepestreamer
