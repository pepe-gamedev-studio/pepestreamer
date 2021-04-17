#include "MediaFileSource.h"
#include "Common.h"

#include <boost/format.hpp>
#include <boost/log/trivial.hpp>
#include <exception>

namespace pepestreamer {

MediaFileSource::MediaFileSource() {}

MediaFileSource::MediaFileSource(const std::string &filename) { Open(filename); }

void MediaFileSource::Open(const std::string &filename) {
    _throw, avformat_open_input(&formatContext, filename.c_str(), nullptr, nullptr);
    _throw, avformat_find_stream_info(formatContext, nullptr);

    BOOST_LOG_TRIVIAL(info) << "[MediaFileSource] " << filename << " opened";
}

void MediaFileSource::Seek(seconds offset) {
    av_seek_frame(formatContext, 0, av_rescale_q(offset, {1, 1}, formatContext->streams[0]->time_base), 0);
    av_seek_frame(formatContext, 1, av_rescale_q(offset, {1, 1}, formatContext->streams[1]->time_base), 0);
}

bool MediaFileSource::ReadNext(AVPacket &packet) { return av_read_frame(formatContext, &packet) != AVERROR_EOF; }

AVRational MediaFileSource::Timebase(int64_t stream) { return formatContext->streams[stream]->time_base; }

MediaFileSource::~MediaFileSource() { avformat_close_input(&formatContext); }

} // namespace pepestreamer
