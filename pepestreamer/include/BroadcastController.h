#pragma once
#include "../src/MediaFileSource.h"
#include "../src/Placeholder.h"
#include "../src/PlaceholderMediaSource.h"
#include "../src/RtmpStream.h"
#include "../src/codec/BaseDecoder.h"
#include "../src/codec/BaseEncoder.h"
#include "../src/overlay.h"
#include "../src/overlay/TestOverlay.h"
#include "../src/placeholder/BuildBlankMediaFragment.h"

#include <boost/atomic.hpp>
#include <boost/thread.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>
#include <string>
#include <vector>

namespace pepestreamer {

struct BroadcastOptions {
    RtmpStreamOptions stream;
    int32_t fps;
    int32_t gop;
    Rgb backgroundColor;
};

class BroadcastController {
    struct Timestamp {
        int64_t dts;
        int64_t pts;
        AVRational timebase;
    };

  public:
    void StartService(const BroadcastOptions &settings);
    void StopService();
    void Play(const std::string &filename);
    void Seek(int64_t offset);
    void Stop();
    std::unique_ptr<overlay::elements::TextElement, overlay::TestOverlay::TextElementDeleter>
    DrawText(const std::string &text,
             int16_t x,
             int16_t y,
             std::string fontColor,
             int16_t fontSize,
             const std::string &filename);
    std::unique_ptr<overlay::elements::ImageElement, overlay::TestOverlay::ImageElementDeleter>
    DrawImage(const std::string &filename, int16_t x, int16_t y);

  private:
    void WorkerLoop();
    bool IsCompatibleSource();
    void SwitchToPlaceholder();
    void ReadDecodedFrame(AVFrame *frame);
    void ReadEncodedPacket(AVPacket *pkt);
    void ReadFilteredFrame(AVFrame *frame);
    void SetupDecoder(AVCodecParameters *codecParameters);

  private:
    BroadcastOptions settings;

    std::unique_ptr<RtmpStream> out;
    std::unique_ptr<PacketSource> in;
    bool playingFile = false;

    int64_t uLast = 0;
    int64_t uOffset = 0;

    boost::thread worker;
    boost::atomic<bool> isWorking = false;

    boost::mutex m;

    std::unique_ptr<codec::BaseDecoder> dec;
    std::unique_ptr<codec::BaseEncoder> enc;

    std::unique_ptr<overlay::TestOverlay> overlay;

    PlaceholderPayload placeholder;
};

} // namespace pepestreamer
