#include "DrawText.h"

#include <boost/format.hpp>
#include <stdexcept>

namespace pepestreamer::overlay::commands {
DrawText::DrawText(const std::string &name,
                   const std::string &text,
                   int16_t x,
                   int16_t y,
                   const std::string &fontColor,
                   int16_t fontSize,
                   const std::string &filename)
    : name(name), x(x), y(y), text(text), fontColor(fontColor), fontSize(fontSize), filename(filename) {
    ValidateColor();
}

std::string DrawText::Name() const { return name; }

void DrawText::Apply(FilterGraph *graph) {
    auto args = (boost::format("fontfile=%1%:text='%2%':fontcolor=%3%:fontsize=%4%:x=%5%:y=%6%") % filename % text %
                 fontColor % fontSize % x % y)
                    .str();

    graph->Create(name.c_str(), avfilter_get_by_name("drawtext"), args.c_str());
}

void DrawText::ValidateColor() {

    uint8_t rgba[4];

    if (av_parse_color(rgba, fontColor.c_str(), -1, nullptr) == AVERROR(EINVAL))
        throw std::runtime_error("Invalid color value");
}

} // namespace pepestreamer::overlay::commands