#include "TextElement.h"

#include <boost/format.hpp>

namespace pepestreamer::overlay::elements {
TextElement::TextElement(std::weak_ptr<commands::DrawText> state, std::unique_ptr<FilterGraph> &graph)
    : state(state), graph(&graph) {}

void TextElement::Text(const std::string &text) {
    auto s = state.lock();

    s->text = text;

    std::string args = (boost::format("text=%1%") % text).str();

    (*graph)->SendCommand(s->Name().c_str(), "reinit", args.c_str());
}

const std::string &TextElement::Text() const { return state.lock()->text; }

void TextElement::Position(int16_t x, int16_t y) {
    auto s = state.lock();

    s->x = x;
    s->y = y;

    std::string args = (boost::format("x=%1%:y=%2%") % x % y).str();

    (*graph)->SendCommand(s->Name().c_str(), "reinit", args.c_str());
}

int16_t TextElement::X() const { return state.lock()->x; }

int16_t TextElement::Y() const { return state.lock()->x; }

void TextElement::FontColor(const std::string &color) {
    auto s = state.lock();

    s->fontColor = color;

    std::string args = (boost::format("fontcolor=%1%") % color).str();

    (*graph)->SendCommand(s->Name().c_str(), "reinit", args.c_str());
}

void TextElement::FontSize(int16_t size) {
    auto s = state.lock();

    s->fontSize = size;

    std::string args = (boost::format("fontsize=%1%") % size).str();

    (*graph)->SendCommand(s->Name().c_str(), "reinit", args.c_str());
}

void TextElement::FontFilename(const std::string &filename) {
    auto s = state.lock();

    s->filename = filename;

    std::string args = (boost::format("fontfile=%1%") % filename).str();

    (*graph)->SendCommand(s->Name().c_str(), "reinit", args.c_str());
}

std::weak_ptr<commands::DrawText> &TextElement::Command() { return state; }

} // namespace pepestreamer::overlay::elements