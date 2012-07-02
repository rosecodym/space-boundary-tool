#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <cstdio>

#include <boost/program_options.hpp>

#include "../Core/src/sbt-core.h"
#include "../Core/src/sbt-core-helpers.h"
#include "../Ifc Adapter/src/sbt-ifcadapter.h"

namespace bpo = boost::program_options;

void notify_print(char * msg) {
	printf(msg);
}

void warn_print(char * msg) {
	fprintf(stderr, msg);
}

void error_print(char * msg) {
	fprintf(stderr, msg);
}

int main(int argc, char * argv[]) {

	std::string output_file_name;
	std::vector<std::string> space_filter;
	std::vector<std::string> element_filter;

	sb_calculation_options opts = create_default_options();

	bpo::options_description visible("options");
	visible.add_options()
		("output-file,o", bpo::value<std::string>(&output_file_name))
		("check-wall-slab-intersections", "check for wall/slab intersections")
		("space-verify-timeout", bpo::value<int>(&opts.space_verification_timeout)->default_value(15), "spend at most this many seconds correcting space geometry (-1 for infinite)")
		("validate", "perform data integrity checks")
		("space-filter,s", bpo::value<std::vector<std::string>>(&space_filter), "filter to space guid")
		("element-filter,e", bpo::value<std::vector<std::string>>(&element_filter), "filter to element guid")
		("force-brute-force-element-resolution", "force the use of the brute force element intersection checking algorithm")
		("help", "describe usage");
	
	bpo::options_description all("all options");
	all.add_options()
		("input-ifc-file", bpo::value<std::string>())
		("verbose-blocking", "verbose blocking")
		("verbose-stacking", "verbose stacking")
		("verbose-spaces", "verbose space generation")
		("verbose-geometry", "verbose miscellaneous geometry operations")
		("verbose-levels", "verbose level resolution")
		("verbose-combinatorics", "verbose combinatoric relations setup")
		("verbose-elements", "verbose element loading")
		("verbose-fenestrations", "verbose fenestration assignment")
		("equality-tolerance", bpo::value<double>(&opts.equality_tolerance), "equality tolerance");

	all.add(visible);

	bpo::positional_options_description pos;
	pos.add("input-ifc-file", -1);
	
	bpo::variables_map vm;

	try {
		bpo::store(bpo::command_line_parser(argc, argv).options(all).positional(pos).run(), vm);
		bpo::notify(vm);
	}
	catch (bpo::error &) {
		std::cout << visible << std::endl;
		return 2;
	}

	if (vm.count("help")) {
		std::cout << visible << std::endl;
		return 0;
	}

	if (!vm.count("input-ifc-file")) {
		std::cout << "Please specify an input file.\n";
		return 1;
	}

	opts.flags = SBT_NONE;
	opts.space_filter_count = 0;
	opts.element_filter_count = 0;
	opts.notify_func = &notify_print;
	opts.warn_func = &warn_print;
	opts.error_func = & error_print;

	if (!vm.count("check-wall-slab-intersections"))	{ opts.flags |= SBT_SKIP_WALL_SLAB_CHECK; }
	if (true || vm.count("validate"))				{ opts.flags |= SBT_EXPENSIVE_CHECKS; }
	if (vm.count("verbose-blocking"))				{ opts.flags |= SBT_VERBOSE_BLOCKS; }
	if (vm.count("verbose-stacking"))				{ opts.flags |= SBT_VERBOSE_STACKS; }
	if (vm.count("verbose-spaces"))					{ opts.flags |= SBT_VERBOSE_SPACES; }
	if (vm.count("verbose-geometry"))				{ opts.flags |= SBT_VERBOSE_GEOMETRY; }
	if (vm.count("verbose-levels"))					{ opts.flags |= SBT_VERBOSE_LEVELS; }
	if (vm.count("verbose-combinatorics"))			{ opts.flags |= SBT_VERBOSE_COMBINATORICS; }
	if (vm.count("verbose-elements"))				{ opts.flags |= SBT_VERBOSE_ELEMENTS; }
	if (vm.count("verbose-fenestrations"))			{ opts.flags |= SBT_VERBOSE_FENESTRATIONS; }
	if (vm.count("force-brute-force-element-resolution")) { opts.flags |= SBT_FORCE_BRUTE_FORCE_ELEMENT_CHECK; }

	opts.space_filter_count = space_filter.size();
	opts.space_filter = (char **)malloc(sizeof(char *) * opts.space_filter_count);
	for (size_t i = 0; i < opts.space_filter_count; ++i) {
		opts.space_filter[i] = (char *)malloc(space_filter[i].length() + 1);
		strncpy(opts.space_filter[i], space_filter[i].c_str(), space_filter[i].length());
		opts.space_filter[i][space_filter[i].length()] = '\0';
	}

	opts.element_filter_count = element_filter.size();
	opts.element_filter = (char **)malloc(sizeof(char *) * opts.element_filter_count);
	for (size_t i = 0; i < opts.element_filter_count; ++i) {
		opts.element_filter[i] = (char *)malloc(element_filter[i].length() + 1);
		strncpy(opts.element_filter[i], element_filter[i].c_str(), element_filter[i].length());
		opts.element_filter[i][element_filter[i].length()] = '\0';
	}

	std::string infile = vm["input-ifc-file"].as<std::string>();
	std::string outfile = vm.count("output-file") ? output_file_name : infile + ".sbt.ifc";
	sb_counts counts;
	
	if (add_to_ifc_file(infile.c_str(), infile.c_str(), opts, &counts) == IFCADAPT_OK) {
		printf("Found:\n");
		printf("  2nd-level physical, external boundaries: %d\n", counts.level_2_physical_external[0]);
		printf("  2nd-level physical, internal boundaries: %d\n", counts.level_2_physical_internal[0]);
		printf("  3rd-level external boundaries: %d\n", counts.level_3_external[0]);
		printf("  3rd-level internal boundaries: %d\n", counts.level_3_internal[0]);
		printf("  4th-level boundaries: %d\n", counts.level_4[0]);
		printf("  5th-level boundaries: %d\n", counts.level_5[0]);
		printf("  virtual boundaries: %d\n", counts.virt[0]);
		printf("  total boundaries: %d\n",
			counts.level_2_physical_external[0] + counts.level_2_physical_internal[0] + 
			counts.level_3_external[0] + counts.level_3_internal[0] + 
			counts.level_4[0] + counts.level_5[0] + counts.virt[0]);

		free_sb_counts(counts);
	}

	for (size_t i = 0; i < opts.space_filter_count; ++i) {
		free(opts.space_filter[i]);
	}
	free(opts.space_filter);

	for (size_t i = 0; i < opts.element_filter_count; ++i) {
		free(opts.element_filter[i]);
	}
	free(opts.element_filter);

	return 0;
}