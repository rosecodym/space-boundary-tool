#pragma once

#include "AllIfcInformation.h"
#include "IfcElement.h"
#include "IfcSpace.h"
#include "UnitScaler.h"

using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;

#define MAX_PATH 260

namespace EdmWrapper {

public ref class EdmSession {
private:
	cppw::EDM * manager;
	cppw::Database_handler * data;
	cppw::String * db_path;
	cppw::Open_repository * repository;
	cppw::Open_model * model;

	UnitScaler ^ unitScaler;

	AllIfcInformation ^ ifcInfo;

	void clear_db();

	static const char * const DB_NAME = "db";
	static const char * const DB_PASS = "pass";
	static const char * const REPO_NAME = "repo";
	static const char * const MODEL_NAME = "model";

	static const char * const LICENSE_KEY = "EDM LICENSE KEY";

	static UnitScaler ^ GetUnits(cppw::Open_model * model);
	static Tuple<double, double, double, double> ^ GetLocationInformation(cppw::Open_model * model);
	static IEnumerable<IfcSpace ^> ^ GetSpaces(cppw::Open_model * model, UnitScaler ^ unitScaler);
	static IEnumerable<IfcElement ^> ^ GetElements(cppw::Open_model * model, IList<IfcConstruction ^> ^ constructions, UnitScaler ^ unitScaler);

	static char * convert_to_chars(char dst[], String ^ src, size_t size);

	static char * get_temp_path(char path[MAX_PATH]) { return convert_to_chars(path, Path::GetTempPath(), MAX_PATH); }

public:
	static EdmSession();
	EdmSession(String ^ schemaPath, Action<String ^> ^ notify);
	~EdmSession() { this->!EdmSession(); }
	!EdmSession();

	void LoadIfcFile(String ^ path);
	void WriteIfcFile(String ^ path);

	property AllIfcInformation ^ IfcInformation
	{
		AllIfcInformation ^ get() { return ifcInfo; }
	}
	
	void RemoveAllSpaceBoundariesAndVirtualElements();
	void AddSpaceBoundaries(IEnumerable<Sbt::CoreTypes::SpaceBoundary ^> ^ boundaries);

	ref class EdmException : public Exception {
	public:
		EdmException(const char * msg) : Exception(gcnew String(msg)) { }
	};

};

} // namespace EdmWrapper