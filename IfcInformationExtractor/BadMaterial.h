#pragma once

#include "Construction.h"

using namespace System;
using namespace System::Collections::Generic;

namespace IfcInformationExtractor { 

public ref class BadMaterial : Construction {
private:
	String ^ name;
	String ^ summary;
	ICollection<String ^> ^ sources;

public:
	BadMaterial(String ^ name, String ^ summary) : name(name), summary(summary), sources(gcnew List<String ^>()) { }

	property String ^ Name { virtual String ^ get() override { return name; } }
	property bool IsComposite { virtual bool get() override { return false; } } 
	property String ^ Summary { virtual String ^ get() override { 
		return summary + (sources->Count == 1 ? "1 element uses this construction." : String::Format(" {0} elements use this construction.", sources->Count));
	} }
	property String ^ Details { 
		virtual String ^ get() override { 
			String ^ res = "The following building elements use this construction:\n";
			for each(String ^ guid in sources) {
				res += guid + Environment::NewLine;
			}
			return res;
		}
	}

	void AddSource(String ^ source) { sources->Add(source); }
};

} // namespace IfcInformationExtractor