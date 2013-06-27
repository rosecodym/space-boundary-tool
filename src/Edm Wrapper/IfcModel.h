#pragma once

#include <cpp_edmi.h>

#include "EdmDatabase.h"
#include "IfcElement.h"
#include "IfcSpace.h"

using namespace System;

namespace IfcInterface {

public ref class IfcModel {
public:
	IfcModel(String ^ pathToFile);
	!IfcModel() {
		delete repo_;
		delete model_;
	}
	~IfcModel() { this->!IfcModel(); }
	
	property double Elevation { double get(); }
	property double NorthAxis { double get(); }
	property double Latitude { double get(); }
	property double Longitude { double get(); }
	property ICollection<IfcElement ^> ^ Elements
	{
		ICollection<IfcElement ^> ^ get();
	}
	property ICollection<IfcSpace ^> ^ Spaces 
	{ 
		ICollection<IfcSpace ^> ^ get(); 
	}

private:
	initonly EdmDatabase ^ database_;
	// The possibility of leaking memory by owning with bare pointers here has
	// been judged to be worth the tradeoff of not having to create a managed
	// smart pointer type. These objects simply aren't used very much.
	cppw::Open_repository * repo_;
	cppw::Open_model * model_;
};

} // namespace IfcInterface