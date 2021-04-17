#pragma once
#include "../ConfigurableFilterGraph.h"
#include "../commands/Overlay.h"

#include <boost/thread.hpp>
#include <memory>

namespace pepestreamer::overlay::elements {

class ImageElement {
  public:
    explicit ImageElement(std::weak_ptr<commands::Overlay> state, ConfigurableFilterGraph *graph, boost::mutex *m);

    int16_t X() const;

    int16_t Y() const;

    void Position(int16_t x, int16_t y);

    std::weak_ptr<commands::Overlay> &Command();

  private:
    std::weak_ptr<commands::Overlay> state;
    ConfigurableFilterGraph *graph;
    boost::mutex *m;
};

} // namespace pepestreamer::overlay::elements
