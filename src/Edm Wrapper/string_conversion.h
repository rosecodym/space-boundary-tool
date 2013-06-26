#pragma once

using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;

namespace IfcInterface {

inline char * managed_string_to_native(char dst[], String ^ src, size_t len) {
	char * str = (char *)(Marshal::StringToHGlobalAnsi(src)).ToPointer();
	strncpy(dst, str, len);
	Marshal::FreeHGlobal(IntPtr(str));
	return dst;
}

} // namespace IfcInterface