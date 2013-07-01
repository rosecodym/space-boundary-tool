#pragma once

#include <cpp_edmi.h>

using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;

namespace IfcInterface {

namespace Internal {

inline char * managed_string_to_native(char dst[], String ^ src, size_t len) {
	char * str = (char *)(Marshal::StringToHGlobalAnsi(src)).ToPointer();
	strncpy(dst, str, len);
	Marshal::FreeHGlobal(IntPtr(str));
	return dst;
}

double length_units_per_meter(const cppw::Open_model & m);

} // namespace Internal

} // namespace IfcInterface