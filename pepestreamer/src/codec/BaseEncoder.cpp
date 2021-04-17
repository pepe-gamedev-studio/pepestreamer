#include "BaseEncoder.h"
#include "../Common.h"

#include <cassert>
#include <stdexcept>

namespace pepestreamer::codec {

BaseEncoder::BaseEncoder(AVCodecParameters *codecpar) : pkt(av_packet_alloc()) {
    AVCodec *encoder = avcodec_find_encoder(codecpar->codec_id);

    if (encoder == nullptr)
        throw std::runtime_error("Failed to found encoder");

    ctx = avcodec_alloc_context3(encoder);
    avcodec_parameters_to_context(ctx, codecpar);
}

void BaseEncoder::Open() { _throw, avcodec_open2(ctx, nullptr, nullptr); }

AVCodecContext *const BaseEncoder::Context() { return ctx; }

void BaseEncoder::Reader(ReadDelegate reader) { readNext = reader; }

void BaseEncoder::Push(AVFrame *frame) {
    _throw, avcodec_send_frame(ctx, frame);

    int ret = 0;
    while (ret >= 0) {
        ret = avcodec_receive_packet(ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0)
            throw std::runtime_error("Unspecified error while avcodec_receive_frame");

        assert(readNext);

        readNext(pkt);
    }
}

BaseEncoder::~BaseEncoder() {
    av_packet_free(&pkt);
    avcodec_free_context(&ctx);
}

} // namespace pepestreamer::codec
