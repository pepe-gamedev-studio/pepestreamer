#include "BuildBlankMediaFragment.h"
#include <functional>

namespace pepestreamer::placeholder {

std::vector<AVPacket *> BlankMediaFragmentGenerator::Generate(const BlankMediaFragmentOptions &options) {
    this->options = options;

    SetupVideoEncoder();
    SetupAudioEncoder();

    PrepareVideoFrame();
    PrepareAudioFrame();

    for (videoFrame->pts = audioFrame->pts = 0; videoFrame->pts < options.gop; ++videoFrame->pts) {
        videoEncoder->Push(videoFrame);

        int64_t uVideoPts = av_rescale_q(videoFrame->pts, videoEncoder->Context()->time_base, {1, AV_TIME_BASE});

        for (AVCodecContext *ctx = audioEncoder->Context();
             av_rescale_q(audioFrame->pts, ctx->time_base, {1, AV_TIME_BASE}) <= uVideoPts;
             audioFrame->pts += ctx->frame_size) {
            audioEncoder->Push(audioFrame);
        }
    }

    videoEncoder->Push(nullptr);
    audioEncoder->Push(nullptr);

    std::vector<AVPacket *> ret = data;

    Cleanup();

    return ret;
}
void BlankMediaFragmentGenerator::SetupVideoEncoder() {
    videoEncoder = std::make_unique<codec::BaseEncoder>(options.videoParameters);
    videoEncoder->Reader(std::bind(&BlankMediaFragmentGenerator::ReadNextVideoPacket, this, std::placeholders::_1));

    AVCodecContext *ctx = videoEncoder->Context();

    ctx->framerate = {options.fps, 1};
    ctx->time_base = {1, options.fps};
    ctx->gop_size = options.gop;

    videoEncoder->Open();
}

void BlankMediaFragmentGenerator::SetupAudioEncoder() {
    audioEncoder = std::make_unique<codec::BaseEncoder>(options.audioParameters);
    audioEncoder->Reader(std::bind(&BlankMediaFragmentGenerator::ReadNextAudioPacket, this, std::placeholders::_1));

    AVCodecContext *ctx = audioEncoder->Context();

    AVCodec *codec = avcodec_find_encoder(ctx->codec_id);

    ctx->sample_fmt = codec->sample_fmts[0];

    audioEncoder->Open();
}

void BlankMediaFragmentGenerator::ReadNextVideoPacket(AVPacket *pkt) {
    pkt->stream_index = 0;
    data.push_back(av_packet_clone(pkt));
}

void BlankMediaFragmentGenerator::ReadNextAudioPacket(AVPacket *pkt) {
    pkt->stream_index = 1;
    pkt->duration = 0;
    data.push_back(av_packet_clone(pkt));
}

void BlankMediaFragmentGenerator::PrepareVideoFrame() {
    videoFrame = av_frame_alloc();
    videoFrame->format = options.videoParameters->format;
    videoFrame->width = options.videoParameters->width;
    videoFrame->height = options.videoParameters->height;

    _throw, av_frame_get_buffer(videoFrame, 0);
    _throw, av_frame_make_writable(videoFrame);

    FillFramebuffer();
}

void BlankMediaFragmentGenerator::FillFramebuffer() {
    auto color = Rgb2Yuv(options.backgroundColor);

    memset(videoFrame->data[0], color[0], videoFrame->linesize[0] * videoFrame->height);
    memset(videoFrame->data[1], color[1], videoFrame->linesize[1] * videoFrame->height / 2);
    memset(videoFrame->data[2], color[2], videoFrame->linesize[2] * videoFrame->height / 2);
}

void BlankMediaFragmentGenerator::PrepareAudioFrame() {
    AVCodecContext *ctx = audioEncoder->Context();

    audioFrame = av_frame_alloc();
    audioFrame->sample_rate = ctx->sample_rate;
    audioFrame->format = ctx->sample_fmt;
    audioFrame->channel_layout = ctx->channel_layout;
    audioFrame->nb_samples = options.audioParameters->frame_size;

    _throw, av_frame_get_buffer(audioFrame, 0);
    _throw, av_frame_make_writable(audioFrame);

    FillAudioFrame();
}

void BlankMediaFragmentGenerator::FillAudioFrame() {
    av_frame_make_writable(audioFrame);

    for (int n = 0; n < audioEncoder->Context()->channels; ++n) {
        memset(audioFrame->extended_data[n], 0, audioFrame->linesize[0]);
    }
}

void BlankMediaFragmentGenerator::Cleanup() {
    videoEncoder.reset();
    audioEncoder.reset();

    av_frame_free(&videoFrame);
    av_frame_free(&audioFrame);
}

} // namespace pepestreamer::placeholder