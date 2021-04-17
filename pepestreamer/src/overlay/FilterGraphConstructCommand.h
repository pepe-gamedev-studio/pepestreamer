#pragma once
#include "../Libav.h"
#include "FilterGraph.h"

#include <string>

namespace pepestreamer::overlay {

struct FilterGraphConstructCommand {
    virtual std::string Name() const = 0;
    virtual void Apply(FilterGraph *graph) = 0;
    virtual ~FilterGraphConstructCommand() = default;
};

} // namespace pepestreamer::overlay