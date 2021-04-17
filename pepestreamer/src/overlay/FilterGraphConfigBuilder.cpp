#include "FilterGraphConfigBuilder.h"
#include "ConfigurableFilterGraph.h"

#include <algorithm>

namespace pepestreamer::overlay {

FilterGraphConfigBuilder::FilterGraphConfigBuilder(ConfigurableFilterGraph &filterGraph) : filterGraph(&filterGraph) {
    target = filterGraph.graphRecipe;
}

void FilterGraphConfigBuilder::Reset() { target.clear(); }

void FilterGraphConfigBuilder::Apply() {
    filterGraph->backGraph = std::make_unique<FilterGraph>(filterGraph->sourceParameters);

    for (auto state : target) {
        state->Apply(filterGraph->backGraph.get());
    }

    filterGraph->graphRecipe = target;
}

FilterGraphConfigBuilder &FilterGraphConfigBuilder::Remove(const std::string &name) {
    auto removed =
        std::find_if(target.begin(), target.end(), [&](const std::shared_ptr<FilterGraphConstructCommand> &cmd) {
            return cmd->Name() == name;
        });

    if (removed != target.end())
        target.erase(removed);

    return *this;
}

} // namespace pepestreamer::overlay