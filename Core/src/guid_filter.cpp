#include "precompiled.h"

#include "guid_filter.h"

guid_filter create_guid_filter(char ** guids, size_t count) {
	if (count == 0 || guids == nullptr) {
		return [](const char * /*guid*/) { return true; };
	}
	std::set<std::string> ok_guids(guids, guids + count);
	return [ok_guids](const char * guid) { return ok_guids.find(guid) != ok_guids.end(); };
}