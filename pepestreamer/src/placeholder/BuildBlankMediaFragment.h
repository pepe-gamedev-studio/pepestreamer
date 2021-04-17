#pragma once
#include "../Codec.h"
#include "../Common.h"
#include "../Libav.h"

#include <memory>
#include <vector>

namespace pepestreamer::placeholder {

struct BlankMediaFragmentOptions {
    AVCodecParameters *videoParameters;
    AVCodecParameters *audioParameters;
    int fps;
    int gop;
    Rgb backgroundColor;
};

/**
 * Generates a set of packets containing video and audio of duration `gop / fps` seconds.
 * The video contains a solid background of the specified color. The audio is silent.
 */
class BlankMediaFragmentGenerator {
  public:
    std::vector<AVPacket *> Generate(const BlankMediaFragmentOptions &options);

  private:
    void SetupVideoEncoder();
    void SetupAudioEncoder();
    void ReadNextVideoPacket(AVPacket *pkt);
    void ReadNextAudioPacket(AVPacket *pkt);
    void PrepareVideoFrame();
    void FillFramebuffer();
    void PrepareAudioFrame();
    void FillAudioFrame();
    void Cleanup();

  private:
    BlankMediaFragmentOptions options;
    std::unique_ptr<codec::BaseEncoder> videoEncoder;
    std::unique_ptr<codec::BaseEncoder> audioEncoder;
    AVFrame *videoFrame;
    AVFrame *audioFrame;
    std::vector<AVPacket *> data;
};

} // namespace pepestreamer::placeholder