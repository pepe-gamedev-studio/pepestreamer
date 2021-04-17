#include "RtmpStream.h"

#include <boost/log/trivial.hpp>
#include <exception>

namespace pepestreamer {

void RtmpStream::Open(RtmpStreamOptions settings) {
    this->settings = settings;

    SetupFormatContext();
    SetupVideoStream();
    SetupAudioStream();

    if (!(formatContext->oformat->flags & AVFMT_NOFILE)) {
        _throw, avio_open2(&formatContext->pb, settings.url.c_str(), AVIO_FLAG_WRITE, nullptr, nullptr);
    }

    startTime = av_gettime();
    _throw, avformat_write_header(formatContext, nullptr);
}

void RtmpStream::Write(AVPacket &packet) {
    bool isVideo = packet.stream_index == 0;

    int64_t upts = av_rescale_q(packet.pts,
                                packet.stream_index == 0 ? videoStream->time_base : audioStream->time_base,
                                AVRational{1, AV_TIME_BASE});

    _throw, av_interleaved_write_frame(formatContext, &packet);

    int64_t elapsed = av_gettime() - startTime;
    if (isVideo && upts > elapsed) {
        if (upts - elapsed > 2000000) {
            BOOST_LOG_TRIVIAL(warning) << "Abnormal sleep " << (upts - elapsed) / 1000 << " ms";
        }

        av_usleep(static_cast<uint32_t>(upts - elapsed));
    }
}

AVRational RtmpStream::Timebase(int64_t stream) { return stream ? audioStream->time_base : videoStream->time_base; }

void RtmpStream::SetupFormatContext() {
    _throw, avformat_alloc_output_context2(&formatContext, nullptr, "flv", nullptr);
}

void RtmpStream::SetupVideoStream() {
    AVCodec *codec = avcodec_find_encoder(settings.videoCodecParameters->codec_id);

    if (!codec)
        throw std::runtime_error("Failed to find encoder for video stream");

    videoStream = avformat_new_stream(formatContext, codec);
    _throw, avcodec_parameters_copy(videoStream->codecpar, settings.videoCodecParameters);
    videoStream->avg_frame_rate = {settings.fps, 1};

    BOOST_LOG_TRIVIAL(info) << "[RtmpStream] video stream has been configured id=" << videoStream->index;
}

void RtmpStream::SetupAudioStream() {
    AVCodec *codec = avcodec_find_encoder(settings.audioCodecParameters->codec_id);

    if (!codec)
        throw std::runtime_error("Failed to find encoder for audio stream");

    audioStream = avformat_new_stream(formatContext, codec);
    _throw, avcodec_parameters_copy(audioStream->codecpar, settings.audioCodecParameters);

    BOOST_LOG_TRIVIAL(info) << "[RtmpStream] audio stream has been configured id=" << audioStream->index;
}

RtmpStream::~RtmpStream() {}

} // namespace pepestreamer
