#pragma once

#include <cpp_edmi.h>

using namespace System;

namespace IfcInterface {

public ref class EdmException : public Exception {
internal:
	EdmException(const char * msg) : Exception(gcnew String(msg)) { }
};

} // namespace IfcAdapter