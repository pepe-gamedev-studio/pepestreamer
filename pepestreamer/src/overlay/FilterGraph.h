#pragma once
#include "../Libav.h"

#include <functional>

namespace pepestreamer::overlay {

struct InputBufferParameters {
    int64_t width;
    int64_t height;
    AVPixelFormat pixelFormat;
    AVRational timebase;
    AVRational sampleAspectRatio;
};

class FilterGraph {
  public:
    using ReadDelegate = std::function<void(AVFrame *)>;

  public:
    explicit FilterGraph(const InputBufferParameters &sourceParameters);

    AVFilterContext *In() const;

    AVFilterContext *Out() const;

    AVFilterContext *Create(char const *name, AVFilter const *type, char const *args);

    void Insert(AVFilterLink *pos, AVFilterContext *filter, int filterSourcePad, int filterDestinationPad);

    void Link(AVFilterContext *src, unsigned srcpad, AVFilterContext *dst, unsigned dstpad);

    AVFilterContext *Find(const char *name);

    void SendCommand(char const *filterName, char const *command, char const *args);

    void Push(AVFrame *frame);

    void Configure();

    void Reader(ReadDelegate handler);

    ~FilterGraph();

  private:
    AVFilterGraph *graph;
    AVFilterContext *bufferSrcCtx = nullptr;
    AVFilterContext *bufferSinkCtx = nullptr;
    ReadDelegate readNext;
};

} // namespace pepestreamer::overlay
