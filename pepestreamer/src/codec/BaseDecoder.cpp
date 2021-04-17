#include "BaseDecoder.h"

namespace pepestreamer::codec {

BaseDecoder::BaseDecoder(AVCodecParameters *codecpar) : frame(av_frame_alloc()) {
    AVCodec *decoder = avcodec_find_decoder(codecpar->codec_id);

    if (decoder == nullptr)
        throw std::runtime_error("Failed to found decoder");

    ctx = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(ctx, codecpar);
}

void BaseDecoder::Open() { avcodec_open2(ctx, nullptr, nullptr); }

AVCodecContext *BaseDecoder::Context() { return ctx; }

void BaseDecoder::Push(AVPacket *pkt) {
    _throw, avcodec_send_packet(ctx, pkt);

    int ret = 0;
    while (ret >= 0) {
        ret = avcodec_receive_frame(ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0)
            throw std::runtime_error("Unspecified error while avcodec_receive_frame");

        assert(readNext);

        readNext(frame);
    }
}

void BaseDecoder::Reader(ReadDelegate reader) { readNext = reader; }

BaseDecoder::~BaseDecoder() {
    av_frame_free(&frame);
    avcodec_free_context(&ctx);
}

} // namespace pepestreamer::codec