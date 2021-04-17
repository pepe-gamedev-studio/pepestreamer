#pragma once
#include "../Libav.h"
#include "FilterGraph.h"
#include "FilterGraphConfigBuilder.h"
#include "FilterGraphConstructCommand.h"
#include "FilterGraphRecipe.h"
#include "commands/Link.h"

#include <algorithm>
#include <memory>
#include <vector>

namespace pepestreamer::overlay {

class ConfigurableFilterGraph {
    friend class FilterGraphConfigBuilder;

  public:
    using ReadDelegate = std::function<void(AVFrame *)>;

  public:
    explicit ConfigurableFilterGraph(const InputBufferParameters &sourceParameters);

    void Push(AVFrame *frame);

    void Reader(ReadDelegate reader);

    template<typename T>
    std::weak_ptr<T> FindRecipeItem(const std::string &name) {
        auto found = std::find_if(graphRecipe.begin(),
                                  graphRecipe.end(),
                                  [&](std::shared_ptr<FilterGraphConstructCommand> &x) { return x->Name() == name; });

        return found == graphRecipe.end() ? std::weak_ptr<T>() : std::dynamic_pointer_cast<T>(*found);
    }

    std::vector<std::weak_ptr<commands::Link>> GetFilterLinks(const std::weak_ptr<FilterGraphConstructCommand> &filter);

    FilterGraphConfigBuilder Configure();

    std::unique_ptr<FilterGraph> &ManagedGraph();

  private:
    void SetupGraph();

  private:
    InputBufferParameters sourceParameters;
    std::unique_ptr<FilterGraph> graph;
    std::unique_ptr<FilterGraph> backGraph;
    FilterGraphRecipe graphRecipe;

    ReadDelegate readNext;
};

} // namespace pepestreamer::overlay