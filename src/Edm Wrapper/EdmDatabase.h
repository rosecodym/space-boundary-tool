#pragma once

#include <cpp_edmi.h>

#include "Edm.h"
#include "EdmException.h"

using namespace System;
using namespace System::IO;
using namespace System::Reflection;

namespace IfcInterface {

private ref class EdmDatabase {
public:
	cppw::Open_model * LoadModel(String ^ path);

	static EdmDatabase ^ Instance() { 
		try { return instance->Value; }
		catch (cppw::Error & e) { throw gcnew EdmException(e.message.data()); }
	}

private:
	static const char * const DB_NAME = "db";
	static const char * const DB_PASS = "pass";
	static const char * const REPO_NAME = "repo";

	EdmDatabase();
	!EdmDatabase() {
		delete repo_;
		if (db_handler_) { 
			manager_->close(*db_handler_);
			delete db_handler_;
		}
		ClearDB();
		delete db_path_;
		delete manager_;
	}
	~EdmDatabase() { this->!EdmDatabase(); }

	static EdmDatabase() {
		const char * const LICENSE_KEY = "set key here";
		sdai::tSdaiSelect mySelect;
		mySelect.value.stringVal = sdai::SdaiString(LICENSE_KEY);
		mySelect.type = sdai::sdaiSTRING;
		edmiSetInternalDataBN("EDM_LICENSE", "", "", 0, &mySelect);
		Func<EdmDatabase ^> ^ create = gcnew Func<EdmDatabase ^>(Create);
		instance = gcnew Lazy<EdmDatabase ^>(create);
	}

	void ClearDB() {
		cppw::String db_name(DB_NAME);
		try {
			if (manager_->does_db_exist(*db_path_, db_name)) {
				manager_->remove_db(*db_path_, db_name, cppw::String(DB_PASS));
			}
		}
		catch (cppw::Error &) { }
	}

	static EdmDatabase ^ Create() { return gcnew EdmDatabase(); }

	// Owning via bare-pointers is a no-no, but these objects are singletons
	// and last the lifetime of the program, so who cares.
	cppw::EDM * manager_;
	cppw::String * db_path_;
	cppw::Database_handler * db_handler_;
	cppw::Open_repository * repo_;

	static initonly Lazy<EdmDatabase ^> ^ instance;
};

} // namespace IfcInterface