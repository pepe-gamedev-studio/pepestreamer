#pragma once
#include "../FilterGraphConstructCommand.h"

#include <string>

namespace pepestreamer::overlay::commands {

struct Overlay : FilterGraphConstructCommand {
    std::string name;
    int16_t x;
    int16_t y;

    explicit Overlay(const std::string &name, int16_t x, int16_t y);

    std::string Name() const override;

    void Apply(FilterGraph *graph) override;
};

} // namespace pepestreamer::overlay::commands
