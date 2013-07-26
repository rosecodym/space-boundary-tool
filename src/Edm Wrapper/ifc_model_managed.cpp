#include <cpp_edmi.h>

#include "edm_wrapper_native_interface.h"

#include "EdmDatabase.h"
#include "ifc_model_internals.h"

namespace ifc_interface {

model::model(const char * path) :  d_(new internals)
{
	try {
		IfcInterface::EdmDatabase ^ db = IfcInterface::EdmDatabase::Instance();
		d_->m = db->LoadModel(gcnew String(path));
	}
	catch (cppw::Error & e) { d_->last_error = e.message.data(); }
}

} // namespace ifc_interface