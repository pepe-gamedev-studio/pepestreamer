#include "Link.h"
#include <cassert>

namespace pepestreamer::overlay::commands {

Link::Link(const std::string &source, int32_t sourcePad, const std::string &destination, int32_t destinationPad)
    : source(source), sourcePad(sourcePad), destination(destination), destinationPad(destinationPad) {}

std::string Link::Name() const { return source + "->" + destination; }

void Link::Apply(FilterGraph *graph) {
    if (auto src = graph->Find(source.c_str())) {
        if (auto dst = graph->Find(destination.c_str())) {
            graph->Link(src, sourcePad, dst, destinationPad);
        } else {
            throw std::runtime_error("Failed to find source");
        }
    } else {
        throw std::runtime_error("Failed to find destination");
    }
}

std::string Link::GetName(AVFilterLink *link) {
    assert(link && link->src && link->dst);

    return std::string(link->src->name) + "->" + std::string(link->dst->name);
}

std::string Link::GetName(const std::string &source, const std::string &destination) {
    return source + "->" + destination;
}

} // namespace pepestreamer::overlay::commands