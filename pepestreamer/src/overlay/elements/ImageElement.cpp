#include "ImageElement.h"

#include <string>

namespace pepestreamer::overlay::elements {

ImageElement::ImageElement(std::weak_ptr<commands::Overlay> state, ConfigurableFilterGraph *graph, boost::mutex *m)
    : state(state), graph(graph), m(m) {}

int16_t ImageElement::X() const { return state.lock()->x; }

int16_t ImageElement::Y() const { return state.lock()->y; }

void ImageElement::Position(int16_t x, int16_t y) {
    auto s = state.lock();
    s->x = x;
    s->y = y;

    {
        boost::lock_guard<boost::mutex> lock(*m);

        graph->Configure().Apply();
    }
}

std::weak_ptr<commands::Overlay> &ImageElement::Command() { return state; }

} // namespace pepestreamer::overlay::elements