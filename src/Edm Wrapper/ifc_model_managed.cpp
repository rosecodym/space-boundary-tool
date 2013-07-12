#include <cpp_edmi.h>

#include "edm_wrapper_native_interface.h"

#include "EdmDatabase.h"
#include "ifc_model_internals.h"

namespace ifc_interface {

model::model(const char * path) : d_(new internals)
{
	IfcInterface::EdmDatabase ^ db = IfcInterface::EdmDatabase::Instance();
	d_->m = db->LoadModel(gcnew String(path));	
	cppw::Instance_set elems = 
		d_->m->get_set_of("IfcBuildingElement", cppw::include_subtypes);
	for (elems.move_first(); elems.move_next(); ) {
		cppw::Instance inst = elems.get();
		std::string guid(((cppw::String)inst.get("GlobalId")).data());
		ifc_object obj(inst, this);
		d_->building_elements.insert(std::make_pair(guid, obj)).first;
	}	
	cppw::Instance_set spaces = d_->m->get_set_of("IfcSpace");
	for (spaces.move_first(); spaces.move_next(); ) {
		cppw::Instance inst = spaces.get();
		std::string guid(((cppw::String)inst.get("GlobalId")).data());
		ifc_object obj(inst, this);
		d_->spaces.insert(std::make_pair(guid, obj)).first;
	}
}

} // namespace ifc_interface