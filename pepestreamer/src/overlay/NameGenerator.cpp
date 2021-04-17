#include "NameGenerator.h"

namespace pepestreamer::overlay {

NameGenerator::NameGenerator(const std::string &prefix) : prefix(prefix + '_') {}

std::string NameGenerator::Generate() { return prefix + std::to_string(id++); }

} // namespace pepestreamer::overlay