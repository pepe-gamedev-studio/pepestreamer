#pragma once
#include "../commands/DrawText.h"
#include <memory>

namespace pepestreamer::overlay::elements {

class TextElement {
  public:
    explicit TextElement(std::weak_ptr<commands::DrawText> state, std::unique_ptr<FilterGraph> &graph);

    void Text(const std::string &text);

	const std::string &Text() const;

    void Position(int16_t x, int16_t y);

	int16_t X() const;

	int16_t Y() const;

    void FontColor(const std::string &color);

    void FontSize(int16_t size);

    void FontFilename(const std::string &filename);

    std::weak_ptr<commands::DrawText> &Command();

  private:
  private:
    std::weak_ptr<commands::DrawText> state;
    std::unique_ptr<FilterGraph> *graph;
};

} // namespace pepestreamer::overlay::elements
