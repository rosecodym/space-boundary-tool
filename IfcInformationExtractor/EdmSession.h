#pragma once

#include <cpp_edmi.h>

#include "BuildingInformation.h"

using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;

#define MAX_PATH 260

namespace IfcInformationExtractor {

public ref class EdmSession {
private:
	cppw::EDM * manager;
	cppw::Database_handler * data;
	cppw::String * db_path;
	cppw::Open_repository * repository;
	cppw::Open_model * model;

	void clear_db();

	ICollection<Element ^> ^ GetElements();

	static const char * const DB_NAME = "db";
	static const char * const DB_PASS = "pass";
	static const char * const REPO_NAME = "repo";
	static const char * const MODEL_NAME = "model";

	static const char * const LICENSE_KEY = "EDM LICENSE KEY";

	static char * convert_to_chars(char dst[], String ^ src, size_t size);

	static char * get_temp_path(char path[MAX_PATH]) { return convert_to_chars(path, Path::GetTempPath(), MAX_PATH); }

	static EdmSession();
public:
	EdmSession(String ^ schemaPath, Action<String ^> ^ notify);
	~EdmSession() { this->!EdmSession(); }
	!EdmSession();

	void LoadIfcFile(String ^ path);
	BuildingInformation ^ GetBuildingInformation();

	ref class EdmException : public Exception {
	public:
		EdmException(const char * msg) : Exception(gcnew String(msg)) { }
	};

};

} // namespace IfcInformationExtractor