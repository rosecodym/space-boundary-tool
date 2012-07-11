#include "precompiled.h"

#include <gtest/gtest.h>

#include "sbt-core.h"
#include "sbt-core-helpers.h"

sb_calculation_options g_opts;
char g_msgbuf[256];

void do_nothing(char *) { }

void print(char * msg) { printf(msg); }

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	g_opts = create_default_options();
	g_opts.equality_tolerance = 0.01;
	g_opts.notify_func = g_opts.warn_func = g_opts.error_func = &do_nothing;
	//g_opts.notify_func = &print;
	//g_opts.flags |= SBT_VERBOSE_STACKS;
	return RUN_ALL_TESTS();
}