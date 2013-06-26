#pragma once

#include <cpp_edmi.h>

using namespace System;

namespace IfcInterface {

private ref class Edm {
public:
	static Edm ^ Instance() { return instance->Value; }

private:
	Edm() {
		native_ = new cppw::EDM();
		native_->std_output(false);
	}

	static Edm() {
		const char * const LICENSE_KEY = "set key here";
		sdai::tSdaiSelect mySelect;
		mySelect.value.stringVal = sdai::SdaiString(LICENSE_KEY);
		mySelect.type = sdai::sdaiSTRING;
		edmiSetInternalDataBN("EDM_LICENSE", "", "", 0, &mySelect);
		instance = gcnew Lazy<Edm ^>(gcnew Func<Edm ^>(Create));
	}

	static Edm ^ Create() { return gcnew Edm(); }

	cppw::EDM * native_;

	static initonly Lazy<Edm ^> ^ instance;
};

} // namespace IfcInterface