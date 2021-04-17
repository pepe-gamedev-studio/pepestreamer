#pragma once
#include "../Common.h"
#include "../Libav.h"

#include <cassert>
#include <functional>
#include <stdexcept>

namespace pepestreamer::codec {

class BaseDecoder {
  public:
    using ReadDelegate = std::function<void(AVFrame *)>;

  public:
    explicit BaseDecoder(AVCodecParameters *codecpar);

    void Open();

    AVCodecContext *Context();

    void Push(AVPacket *pkt);

    void Reader(ReadDelegate reader);

    ~BaseDecoder();

  protected:
    ReadDelegate readNext;
    AVCodecContext *ctx;
    AVFrame *frame;
};

} // namespace pepestreamer::codec