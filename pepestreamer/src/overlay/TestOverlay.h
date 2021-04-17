#pragma once
#include "../Libav.h"
#include "ConfigurableFilterGraph.h"
#include "NameGenerator.h"
#include "elements/ImageElement.h"
#include "elements/TextElement.h"

#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>
#include <functional>
#include <memory>
#include <string>

namespace pepestreamer::overlay {

class TestOverlay {
  public:
    using ReadDelegate = std::function<void(AVFrame *)>;
    using TextElementDeleter = std::function<void(elements::TextElement *)>;
    using ImageElementDeleter = std::function<void(elements::ImageElement *)>;

  public:
    explicit TestOverlay(const InputBufferParameters &sourceParameters, boost::mutex &m);

    void Push(AVFrame *frame);

    std::unique_ptr<elements::TextElement, TextElementDeleter> DrawText(const std::string &text,
                                                                        int16_t x,
                                                                        int16_t y,
                                                                        std::string fontColor,
                                                                        int16_t fontSize,
                                                                        const std::string &filename);

    std::unique_ptr<elements::ImageElement, ImageElementDeleter>
    DrawImage(const std::string &filename, int16_t x, int16_t y);

    void Reader(ReadDelegate reader);

  private:
    ConfigurableFilterGraph graph;
    const std::string in = "in";
    const std::string out = "out";
    const std::string backgroundImage = "background";
    const std::string canvasOverlay = "canvas";

    NameGenerator textNameGen;
    NameGenerator imageNameGen;
    NameGenerator overlayNameGen;

    boost::mutex *m;
};

} // namespace pepestreamer::overlay
