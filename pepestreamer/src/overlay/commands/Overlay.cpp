#include "Overlay.h"

#include <boost/format.hpp>

namespace pepestreamer::overlay::commands {

Overlay::Overlay(const std::string &name, int16_t x, int16_t y) : name(name), x(x), y(y) {}

std::string Overlay::Name() const { return name; }

void Overlay::Apply(FilterGraph *graph) {
    auto args = (boost::format("x=%1%:y=%2%:eval=init") % x % y).str();

    graph->Create(name.c_str(), avfilter_get_by_name("overlay"), args.c_str());
}

} // namespace pepestreamer::overlay::commands