#include "precompiled.h"

#include <gtest/gtest.h>

#include "sbt-core.h"

sb_calculation_options g_opts;
char g_msgbuf[256];

void do_nothing(char *) { }

void print(char * msg) { printf(msg); }

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	g_opts = create_default_options();
	g_opts.notify_func = g_opts.warn_func = g_opts.error_func = &do_nothing;
	return RUN_ALL_TESTS();
}