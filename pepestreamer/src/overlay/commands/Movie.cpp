#include "Movie.h"

namespace pepestreamer::overlay::commands {

Movie::Movie(const std::string &name, const std::string &filename) : name(name), filename(filename) {}

std::string Movie::Name() const { return name; }

void Movie::Apply(FilterGraph *graph) { graph->Create(name.c_str(), avfilter_get_by_name("movie"), filename.c_str()); }

} // namespace pepestreamer::overlay::commands