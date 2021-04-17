#include "PlaceholderMediaSource.h"
#include <algorithm>

namespace pepestreamer {
PlaceholderMediaSource::PlaceholderMediaSource(const PlaceholderPayload &placeholder) : placeholder(&placeholder) {}

bool PlaceholderMediaSource::ReadNext(AVPacket &pkt) {
    AVPacket *temp = av_packet_clone(placeholder->packets[nextIdx]);

    pkt = *temp;

    // Reset to maintain monotony
    if (uOffset && pkt.dts < 0) {
        pkt.pts = 0;
        pkt.dts = 0;
    }

    auto timebase = pkt.stream_index ? placeholder->audioTimebase : placeholder->videoTimebase;

    int64_t offset = av_rescale_q(uOffset, {1, AV_TIME_BASE}, timebase);

    pkt.pts += offset;
    pkt.dts += offset;

    uLast = std::max(uLast, av_rescale_q(pkt.dts, timebase, {1, AV_TIME_BASE}));

    if (nextIdx + 1 == placeholder->packets.size()) {
        uOffset = uLast;

        nextIdx = 0;
    } else {
        ++nextIdx;
    }

    return true;
}

AVRational PlaceholderMediaSource::Timebase(int64_t stream) {
    return stream ? placeholder->audioTimebase : placeholder->videoTimebase;
}

} // namespace pepestreamer