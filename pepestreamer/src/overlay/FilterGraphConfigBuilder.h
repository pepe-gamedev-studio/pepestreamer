#pragma once
#include "FilterGraphConstructCommand.h"
#include "FilterGraphRecipe.h"

#include <type_traits>

namespace pepestreamer::overlay {

class ConfigurableFilterGraph;

class FilterGraphConfigBuilder {
  public:
    explicit FilterGraphConfigBuilder(ConfigurableFilterGraph &filterGraph);

    void Reset();

    template<typename T,
             typename... Args,
             std::enable_if_t<std::is_base_of_v<FilterGraphConstructCommand, T>, bool> = true>
    FilterGraphConfigBuilder &Add(Args... args) {
        target.push_back(std::make_shared<T>(args...));

        return *this;
    }

    FilterGraphConfigBuilder &Remove(const std::string &name);

    void Apply();

  private:
    ConfigurableFilterGraph *filterGraph;
    FilterGraphRecipe target;
};

} // namespace pepestreamer::overlay
