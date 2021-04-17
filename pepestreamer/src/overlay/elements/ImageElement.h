#pragma once
#include "../commands/Overlay.h"
#include <memory>

namespace pepestreamer::overlay::elements {

class ImageElement {
  public:
    explicit ImageElement(std::weak_ptr<commands::Overlay> state, std::unique_ptr<FilterGraph> &graph);

    void X(int16_t x);

    void Y(int16_t y);

	int16_t X() const;

	int16_t Y() const;

    std::weak_ptr<commands::Overlay> &Command();

  private:
    std::weak_ptr<commands::Overlay> state;
    std::unique_ptr<FilterGraph> *graph;
};

} // namespace pepestreamer::overlay::elements
