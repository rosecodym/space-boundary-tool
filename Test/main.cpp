#include <cstdlib>

#include "C:/Users/Cody/Documents/Visual Studio 2010/Projects/Space Boundary Tool/Ifc Adapter/src/sbt-ifcadapter.h"

int main(int argc, char * argv[]) {

	add_to_ifc_file(argv[1], argc != 3 ? SBT_NONE : atoi(argv[2]));

	return 0;
}