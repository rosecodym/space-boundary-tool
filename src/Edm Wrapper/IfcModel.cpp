#include "IfcModel.h"

namespace IfcInterface {

static IfcModel::IfcModel() {
	Func<EdmDatabase ^> ^ create = 
		gcnew Func<EdmDatabase ^>(EdmDatabase::Create);
	database = gcnew Lazy<EdmDatabase ^>(create);
}

} // namespace IfcInterface