#pragma once
#include "../FilterGraphConstructCommand.h"

#include <string>

namespace pepestreamer::overlay::commands {

struct Link : FilterGraphConstructCommand {
    std::string source;
    int32_t sourcePad;

    std::string destination;
    int32_t destinationPad;

    explicit Link(const std::string &source, int32_t sourcePad, const std::string &destination, int32_t destinationPad);

    std::string Name() const override;

    void Apply(FilterGraph *graph) override;

    static std::string GetName(AVFilterLink *link);

    static std::string GetName(const std::string &source, const std::string &destination);
};

} // namespace pepestreamer::overlay::commands
