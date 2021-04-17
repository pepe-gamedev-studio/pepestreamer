#pragma once
#include "../FilterGraphConstructCommand.h"

#include <string>

namespace pepestreamer::overlay::commands {

struct DrawText : FilterGraphConstructCommand {
    std::string name;
    int16_t x;
    int16_t y;
    std::string text;
    std::string fontColor;
    int16_t fontSize;
    std::string filename;

    explicit DrawText(const std::string &name,
                      const std::string &text,
                      int16_t x,
                      int16_t y,
                      const std::string &fontColor,
                      int16_t fontSize,
                      const std::string &filename);

    std::string Name() const override;

    void Apply(FilterGraph *graph) override;

  private:
    void ValidateColor();
};

} // namespace pepestreamer::overlay::commands
