#pragma once
#include <string>

namespace pepestreamer::overlay {

class NameGenerator {
  public:
    explicit NameGenerator(const std::string &prefix);
    std::string Generate();

  private:
    std::string prefix;
    int64_t id = 0;
};

} // namespace pepestreamer::overlay