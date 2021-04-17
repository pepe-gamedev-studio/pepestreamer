#pragma once
#include <string>

namespace pepestreamer {

struct ThrowTrait {};

std::string avErrorStr(int error);

void operator,(ThrowTrait, int ret);

extern ThrowTrait _throw;

} // namespace pepestreamer
