#pragma once

#include <cpp_edmi.h>

using namespace System;

namespace IfcInformationExtractor {

public ref class Construction abstract : IComparable<Construction ^> {
public:
	property String ^ Name { virtual String ^ get() abstract; }
	property bool IsComposite { virtual bool get() abstract; }

	virtual Int32 CompareTo(Construction ^ other) {
		if (other == nullptr) { return 1; }
		if (this->IsComposite == other->IsComposite) {
			return this->Name->CompareTo(other->Name);
		}
		else {
			return this->IsComposite ? 1 : -1;
		}
	}
};

} // namespace IfcInformationExtractor