#include "BroadcastController.h"
#include "../src/Common.h"

#include <algorithm>
#include <boost/format.hpp>
#include <functional>
#include <iostream>

namespace pepestreamer {

void BroadcastController::StartService(const BroadcastOptions &settings) {
    this->settings = settings;

    pepestreamer::placeholder::BlankMediaFragmentOptions options{settings.stream.videoCodecParameters,
                                                                 settings.stream.audioCodecParameters,
                                                                 settings.fps,
                                                                 settings.gop,
                                                                 settings.backgroundColor};

    placeholder::BlankMediaFragmentGenerator gen;

    placeholder.packets = gen.Generate(options);
    placeholder.videoTimebase = {1, settings.fps};
    placeholder.audioTimebase = {1, settings.stream.audioCodecParameters->sample_rate};

    SwitchToPlaceholder();

    const overlay::InputBufferParameters sourceParameters{
        settings.stream.videoCodecParameters->width,
        settings.stream.videoCodecParameters->height,
        static_cast<AVPixelFormat>(settings.stream.videoCodecParameters->format),
        {1, settings.fps},
        settings.stream.videoCodecParameters->sample_aspect_ratio};

    overlay = std::make_unique<overlay::TestOverlay>(sourceParameters, m);
    overlay->Reader(std::bind(&BroadcastController::ReadFilteredFrame, this, std::placeholders::_1));

    enc = std::make_unique<codec::BaseEncoder>(settings.stream.videoCodecParameters);
    enc->Reader(std::bind(&BroadcastController::ReadEncodedPacket, this, std::placeholders::_1));
    enc->Context()->time_base = dec->Context()->time_base;
    enc->Context()->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    enc->Open();

    out = std::make_unique<RtmpStream>();
    AVCodecParameters *params = avcodec_parameters_alloc();
    avcodec_parameters_from_context(params, enc->Context());
    out->Open({params, settings.stream.audioCodecParameters, settings.stream.url, settings.fps});

    isWorking = true;
    worker = boost::thread(&BroadcastController::WorkerLoop, this);
}

void BroadcastController::StopService() {
    isWorking = false;

    // avcodec_parameters_free(&outDefaultParams.video);
    // avcodec_parameters_free(&outDefaultParams.audio);
}

void BroadcastController::Play(const std::string &filename) {
    if (!isWorking)
        throw std::runtime_error("Calling `BroadcastController::Play` before stream opening is forbidden");

    boost::lock_guard<boost::mutex> lock(m);

    auto file = std::make_unique<MediaFileSource>(filename);

    SetupDecoder(file->formatContext->streams[0]->codecpar);

    in = std::unique_ptr<PacketSource>(std::move(file));

    uOffset = uLast;
    playingFile = true;

    if (IsCompatibleSource()) {

    } else {
        throw std::runtime_error("Not implemented yet");
    }
}

void BroadcastController::Seek(int64_t offset) {
    boost::lock_guard<boost::mutex> lock(m);

    if (!playingFile)
        return;

    int64_t uRelative = static_cast<int64_t>(offset * AV_TIME_BASE - (uLast - uOffset) -
                                             AV_TIME_BASE / static_cast<double>(settings.fps));

    uOffset -= uRelative;

    static_cast<MediaFileSource *>(in.get())->Seek(offset);
}

void BroadcastController::Stop() {
    boost::lock_guard<boost::mutex> lock(m);

    if (playingFile)
        SwitchToPlaceholder();
}

std::unique_ptr<overlay::elements::TextElement, overlay::TestOverlay::TextElementDeleter>
BroadcastController::DrawText(const std::string &text,
                              int16_t x,
                              int16_t y,
                              std::string fontColor,
                              int16_t fontSize,
                              const std::string &filename) {
    return overlay->DrawText(text, x, y, fontColor, fontSize, filename);
}

std::unique_ptr<overlay::elements::ImageElement, overlay::TestOverlay::ImageElementDeleter>
BroadcastController::DrawImage(const std::string &filename, int16_t x, int16_t y) {
    return overlay->DrawImage(filename, x, y);
}

void BroadcastController::WorkerLoop() {
    AVPacket pkt;
    while (isWorking) {
        boost::lock_guard<boost::mutex> lock(m);

        if (in->ReadNext(pkt)) {
            av_packet_rescale_ts(&pkt, in->Timebase(pkt.stream_index), out->Timebase(pkt.stream_index));

            if (pkt.dts < 0) {
                pkt.pts = 0;
                pkt.dts = 0;
            }

            int64_t offset = av_rescale_q(uOffset, {1, AV_TIME_BASE}, out->Timebase(pkt.stream_index));
            pkt.pts += offset;
            pkt.dts += offset;

            uLast = std::max(uLast, av_rescale_q(pkt.dts, out->Timebase(pkt.stream_index), {1, AV_TIME_BASE}));

            if (pkt.stream_index) {
                out->Write(pkt);
            } else {
                av_packet_rescale_ts(&pkt, out->Timebase(pkt.stream_index), dec->Context()->time_base);
                dec->Push(&pkt);
            }
        } else {
            SwitchToPlaceholder();
        }
    }

    out.release();
}

bool BroadcastController::IsCompatibleSource() {
    return true; /*in->videoCodecContext->codec_id == out->videoCodecContext->codec_id &&
             in->videoCodecContext->width == out->videoCodecContext->width &&
             in->videoCodecContext->height == out->videoCodecContext->height &&
             in->videoCodecContext->framerate.num == out->videoCodecContext->framerate.num &&
             in->videoCodecContext->framerate.den == out->videoCodecContext->framerate.den &&
             in->audioCodecContext->sample_rate == out->audioCodecContext->sample_rate &&
             in->audioCodecContext->sample_fmt == out->audioCodecContext->sample_fmt &&
             in->audioCodecContext->channel_layout == out->audioCodecContext->channel_layout;*/
}

void BroadcastController::SwitchToPlaceholder() {
    SetupDecoder(settings.stream.videoCodecParameters);

    uOffset = uLast;

    in = std::make_unique<PlaceholderMediaSource>(placeholder);

    playingFile = false;
}

void BroadcastController::ReadDecodedFrame(AVFrame *frame) {
    frame->pts = frame->best_effort_timestamp;
    overlay->Push(frame);
}

void BroadcastController::ReadEncodedPacket(AVPacket *pkt) {
    av_packet_rescale_ts(pkt, enc->Context()->time_base, out->Timebase(pkt->stream_index));

    // FIXME
    if (pkt->dts > pkt->pts) {
        pkt->pts = pkt->dts;
        std::cout << "FIXME\n";
    }

    if (pkt->dts < out->formatContext->streams[0]->cur_dts) {
        auto diff = out->formatContext->streams[0]->cur_dts - pkt->dts;
        auto uAdd = av_rescale_q(diff, out->Timebase(pkt->stream_index), {1, AV_TIME_BASE});

        pkt->pts += diff;
        pkt->dts += diff;

        uOffset += uAdd;
    }

    out->Write(*pkt);
}

void BroadcastController::ReadFilteredFrame(AVFrame *frame) { enc->Push(frame); }

void BroadcastController::SetupDecoder(AVCodecParameters *codecParameters) {
    dec = std::make_unique<codec::BaseDecoder>(codecParameters);
    dec->Reader(std::bind(&BroadcastController::ReadDecodedFrame, this, std::placeholders::_1));
    dec->Context()->framerate = {settings.fps, 1};
    dec->Open();
}

} // namespace pepestreamer
