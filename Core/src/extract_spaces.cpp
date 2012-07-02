#include "precompiled.h"

#include "guid_filter.h"
#include "printing-macros.h"
#include "space.h"

namespace operations {

std::vector<std::shared_ptr<space>> extract_spaces(
	space_info ** infos, size_t count, 
	std::shared_ptr<equality_context> context, 
	std::function<point_3(point_3)> corrector,
	const guid_filter & passes_filter) 
{
	std::vector<std::shared_ptr<space>> results;
	NOTIFY_MSG("Setting up spaces");
	for (size_t i = 0; i < count; ++i) {
		if (passes_filter(infos[i]->id)) {
			PRINT_SPACES("Getting space %s..", infos[i]->id);
			results.push_back(std::shared_ptr<space>(new space(infos[i], context, corrector)));
			NOTIFY_MSG(".");
			PRINT_SPACES("done.\n");
		}
	}
	results.push_back(std::shared_ptr<space>(new space(context))); // for outside space. i'm not checking for whether this is necessary right now.
	NOTIFY_MSG("done.\n");
	return results;
}

}