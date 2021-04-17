#include "FilterGraph.h"
#include "../Common.h"

#include <boost/format.hpp>

namespace pepestreamer::overlay {

FilterGraph::FilterGraph(const InputBufferParameters &sourceParameters) {
    const AVFilter *buffersrc = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");

    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();

    graph = avfilter_graph_alloc();

    auto buffFilterArgs = boost::format("video_size=%1%x%2%:pix_fmt=%3%:time_base=%4%/%5%:pixel_aspect=%6%/%7%") %
                          sourceParameters.width % sourceParameters.height % sourceParameters.pixelFormat %
                          sourceParameters.timebase.num % sourceParameters.timebase.den %
                          sourceParameters.sampleAspectRatio.num % sourceParameters.sampleAspectRatio.den;

    _throw, avfilter_graph_create_filter(&bufferSrcCtx, buffersrc, "in", buffFilterArgs.str().c_str(), nullptr, graph);
    _throw, avfilter_graph_create_filter(&bufferSinkCtx, buffersink, "out", nullptr, nullptr, graph);

    enum AVPixelFormat pix_fmts[] = {sourceParameters.pixelFormat, AV_PIX_FMT_NONE};
    av_opt_set_int_list(bufferSinkCtx, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);

    outputs->name = av_strdup("in");
    outputs->filter_ctx = bufferSrcCtx;
    outputs->pad_idx = 0;
    outputs->next = nullptr;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = bufferSinkCtx;
    inputs->pad_idx = 0;
    inputs->next = nullptr;
}

AVFilterContext *FilterGraph::In() const { return bufferSrcCtx; }

AVFilterContext *FilterGraph::Out() const { return bufferSinkCtx; }

AVFilterContext *FilterGraph::Create(char const *name, AVFilter const *type, char const *args) {
    AVFilterContext *ctx = nullptr;
    _throw, avfilter_graph_create_filter(&ctx, type, name, args, nullptr, graph);

    return ctx;
}

void FilterGraph::Insert(AVFilterLink *pos, AVFilterContext *filter, int filterSourcePad, int filterDestinationPad) {
    _throw, avfilter_insert_filter(pos, filter, filterSourcePad, filterDestinationPad);
}

void FilterGraph::Link(AVFilterContext *src, unsigned srcpad, AVFilterContext *dst, unsigned dstpad) {
    _throw, avfilter_link(src, srcpad, dst, dstpad);
}

AVFilterContext *FilterGraph::Find(const char *name) { return avfilter_graph_get_filter(graph, name); }

void FilterGraph::SendCommand(char const *filterName, char const *command, char const *args) {
    avfilter_graph_send_command(graph, filterName, command, args, nullptr, 0, 1);
}

void FilterGraph::Push(AVFrame *frame) {
    _throw, av_buffersrc_add_frame_flags(bufferSrcCtx, frame, AV_BUFFERSRC_FLAG_KEEP_REF);

    while (1) {
        AVFrame *filtered = av_frame_alloc();

        int ret = av_buffersink_get_frame(bufferSinkCtx, filtered);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        if (ret < 0)
            throw std::runtime_error("Failed to get filtered frame");

        if (readNext)
            readNext(filtered);

        av_frame_free(&filtered);
    }
}

void FilterGraph::Configure() {
    _throw, avfilter_graph_config(graph, nullptr);
    av_log(nullptr, 10, avfilter_graph_dump(graph, nullptr));
}

void FilterGraph::Reader(ReadDelegate handler) { readNext = handler; }

FilterGraph::~FilterGraph() { avfilter_graph_free(&graph); }

} // namespace pepestreamer::overlay