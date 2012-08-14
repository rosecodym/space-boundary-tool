#include <cpp_edmi.h>

#include "EdmSession.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace IfcInformationExtractor {

static EdmSession::EdmSession() {
	sdai::tSdaiSelect mySelect;
	mySelect.value.stringVal = sdai::SdaiString(LICENSE_KEY);
	mySelect.type = sdai::sdaiSTRING;
	edmiSetInternalDataBN("EDM_LICENSE", "", "", 0, &mySelect);
}

EdmSession::EdmSession(String ^ schemaPath, Action<String ^> ^ notify) 
	: manager(new cppw::EDM()), data(__nullptr), db_path(__nullptr), repository(__nullptr), model(__nullptr)
{
	try {
		manager->std_output(false);
		cppw::String db_name(DB_NAME);
		cppw::String db_pass(DB_PASS);
		cppw::String repo_name(REPO_NAME);
		char temp_path[MAX_PATH];
		get_temp_path(temp_path);
		db_path = new cppw::String(temp_path);
		char schema_path[MAX_PATH];
		convert_to_chars(schema_path, schemaPath, MAX_PATH);
		clear_db();
		manager->create_db(*db_path, db_name, db_pass);
		data = new cppw::Database_handler(manager->open_db(*db_path, db_name, db_pass));
		(*data)->create_repository(repo_name);
		cppw::Compile_results res = cppw::Express_compiler(schema_path).compile();
		if (res.errors) {
			clear_db();
		}
	}
	catch (cppw::Error & e) {
		char buf[256];
		sprintf(buf, "Execution aborted - couldn't establish an EDM session (%s).\n", e.message.data());
		notify(gcnew String(buf));
	}
}

EdmSession::!EdmSession() {
	if (data) { manager->close(*data); }
	clear_db();
	delete model;
	delete repository;
	delete db_path;
	delete data;
	delete manager;
}

void EdmSession::clear_db() { 
	try {
		if (manager->does_db_exist(*db_path, cppw::String(DB_NAME))) { 
			manager->remove_db(*db_path, cppw::String(DB_NAME), cppw::String(DB_PASS)); 
		} 
	}
	catch (cppw::Error & e) {
		fprintf(stderr, "Error cleaning up the EDM DB: %s\n", e.message.data());
		exit(1);
	}
}

ICollection<Element ^> ^ EdmSession::GetElements() {
	if (model == __nullptr) {
		return nullptr;
	}
	cppw::Instance_set instances = model->get_set_of("IfcBuildingElement", cppw::include_subtypes);
	IList<Element ^> ^ elements = gcnew List<Element ^>();
	for (instances.move_first(); instances.move_next(); ) {
		elements->Add(gcnew Element(instances.get()));
	}
	return elements;
}

char * EdmSession::convert_to_chars(char dst[], String ^ src, size_t size) {
	char * str = (char *)(Marshal::StringToHGlobalAnsi(src)).ToPointer();
	strncpy_s(dst, size, str, size - 1);
	Marshal::FreeHGlobal(IntPtr(str));
	return dst;
}

void EdmSession::LoadIfcFile(String ^ path) {
	try {
		cppw::String repo_name(REPO_NAME);
		cppw::String model_name(MODEL_NAME);
		char ifc_path[MAX_PATH];
		convert_to_chars(ifc_path, path, MAX_PATH);
		cppw::Step_reader(ifc_path, repo_name, model_name).read();
		repository = new cppw::Open_repository((*data)->get_repository(repo_name), cppw::RW_access);
		model = new cppw::Open_model(repository->get_model(model_name), cppw::RW_access);
	}
	catch (cppw::Error & e) {
		throw gcnew EdmException(e.message.data());
	}
}

BuildingInformation ^ EdmSession::GetBuildingInformation() {
	if (model == __nullptr) {
		return nullptr;
	}
	BuildingInformation ^ res = gcnew BuildingInformation();
	res->ElementsByGuid = gcnew Dictionary<String ^, Element ^>();
	ICollection<Element ^> ^ elements = GetElements();
	for each(Element ^ e in elements) {
		res->ElementsByGuid[e->Guid] = e;
	}

	List<Construction ^> ^ constructions = gcnew List<Construction ^>(); // my kingdom for c++/cli lambdas~~~
	for each(Element ^ e in res->ElementsByGuid->Values) {
		constructions->Add(e->AssociatedConstruction);
	}
	res->Constructions = gcnew SortedSet<Construction ^>(constructions);

	return res;
}

} // namespace IfcInformationExtractor