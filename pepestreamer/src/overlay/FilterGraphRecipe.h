#pragma once
#include <memory>
#include <vector>

#include "FilterGraphConstructCommand.h"

namespace pepestreamer::overlay {

using FilterGraphRecipe = std::vector<std::shared_ptr<FilterGraphConstructCommand>>;

}