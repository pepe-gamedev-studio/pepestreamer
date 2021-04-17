#pragma once
#include "../Libav.h"

#include <functional>

namespace pepestreamer::codec {

class BaseEncoder {
  public:
    using ReadDelegate = std::function<void(AVPacket *)>;

  public:
    explicit BaseEncoder(AVCodecParameters *codecpar);

    void Open();

    AVCodecContext *const Context();

    void Push(AVFrame *frame);

    void Reader(ReadDelegate reader);

    ~BaseEncoder();

  protected:
    AVCodecContext *ctx;
    std::function<void(AVPacket *)> readNext;
    AVPacket *pkt;
};

} // namespace pepestreamer::codec
