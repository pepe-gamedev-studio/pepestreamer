#pragma once
#include "../FIlterGraph.h"
#include "../FilterGraphConstructCommand.h"

#include <string>

namespace pepestreamer::overlay::commands {

struct Movie : FilterGraphConstructCommand {
    std::string name;
    std::string filename;

    explicit Movie(const std::string &name, const std::string &filename);

    std::string Name() const override;

    void Apply(FilterGraph *graph) override;
};

} // namespace pepestreamer::overlay::commands
