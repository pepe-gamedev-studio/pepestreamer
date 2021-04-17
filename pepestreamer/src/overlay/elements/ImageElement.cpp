#include "ImageElement.h"

#include <string>

namespace pepestreamer::overlay::elements {

ImageElement::ImageElement(std::weak_ptr<commands::Overlay> state, std::unique_ptr<FilterGraph> &graph)
    : state(state), graph(&graph) {}

void ImageElement::X(int16_t x) {
    auto s = state.lock();

    s->x = x;

    (*graph)->SendCommand(s->Name().c_str(), "x", std::to_string(x).c_str());
}

void ImageElement::Y(int16_t y) {
    auto s = state.lock();

    s->y = y;

    (*graph)->SendCommand(s->Name().c_str(), "y", std::to_string(y).c_str());
}

int16_t ImageElement::X() const { return state.lock()->x; }

int16_t ImageElement::Y() const { return state.lock()->y; }

std::weak_ptr<commands::Overlay> &ImageElement::Command() { return state; }

} // namespace pepestreamer::overlay::elements