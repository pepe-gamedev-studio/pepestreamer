#include "ConfigurableFilterGraph.h"

#include <boost/algorithm/string.hpp>

namespace pepestreamer::overlay {

ConfigurableFilterGraph::ConfigurableFilterGraph(const InputBufferParameters &sourceParameters)
    : sourceParameters(sourceParameters) {
    SetupGraph();
}

void ConfigurableFilterGraph::Push(AVFrame *frame) {
    if (backGraph) {
        graph = std::move(backGraph);

        graph->Reader(readNext);

        graph->Configure();
    }

    graph->Push(frame);
}

void ConfigurableFilterGraph::Reader(ReadDelegate reader) {
    readNext = reader;

    graph->Reader(readNext);
}

std::vector<std::weak_ptr<commands::Link>>
ConfigurableFilterGraph::GetFilterLinks(const std::weak_ptr<FilterGraphConstructCommand> &filter) {
    std::vector<std::shared_ptr<FilterGraphConstructCommand>> filtered;

    std::string filterName = filter.lock()->Name();
    std::copy_if(graphRecipe.begin(),
                 graphRecipe.end(),
                 std::back_inserter(filtered),
                 [&](std::shared_ptr<FilterGraphConstructCommand> &x) {
                     const std::string name = x->Name();

                     return name.size() > filterName.size() && (boost::algorithm::starts_with(name, filterName) ||
                                                                boost::algorithm::ends_with(name, filterName));
                 });

    std::vector<std::weak_ptr<commands::Link>> result;
    result.reserve(filtered.size());

    std::transform(filtered.begin(),
                   filtered.end(),
                   std::back_inserter(result),
                   [](const std::shared_ptr<FilterGraphConstructCommand> &x) {
                       return std::dynamic_pointer_cast<commands::Link>(x);
                   });

    return result;
}

FilterGraphConfigBuilder ConfigurableFilterGraph::Configure() { return FilterGraphConfigBuilder(*this); }

std::unique_ptr<FilterGraph> &ConfigurableFilterGraph::ManagedGraph() { return graph; }

void ConfigurableFilterGraph::SetupGraph() {
    graph = std::make_unique<FilterGraph>(sourceParameters);

    graph->Reader(readNext);
}

} // namespace pepestreamer::overlay