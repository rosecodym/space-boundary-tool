#include <cpp_edmi.h>

#include "EdmSession.h"

using namespace System;
using namespace System::Runtime::InteropServices;

typedef ConstructionManagement::ModelConstructions::ModelConstructionCollection ConstructionCollection;

namespace IfcInformationExtractor {

namespace {

void GetLocationInformation(cppw::Open_model * model, double % /*northAxis*/, double % latitude, double % longitude, double % elevation) {
	cppw::Instance site = model->get_set_of("IfcSite").get_(0);
	cppw::Select sel;
	if ((sel = site.get("RefLatitude")).is_set()) {
		cppw::List lat = sel;
		latitude = (cppw::Real)lat.get_(0) + (cppw::Real)lat.get_(1) / 60.0 + (cppw::Real)lat.get_(2) / 360.0;
	}
	if ((sel = site.get("RefLongitude")).is_set()) {
		cppw::List lng = sel;
		longitude = (cppw::Real)lng.get_(0) + (cppw::Real)lng.get_(1) / 60.0 + (cppw::Real)lng.get_(2) / 360.0;
	}
	if ((sel = site.get("RefElevation")).is_set()) {
		elevation = (cppw::Real)sel;
	}
}

ICollection<Space ^> ^ GetSpaces(cppw::Open_model * model) {
	if (model == __nullptr) {
		return nullptr;
	}
	cppw::Instance_set instances = model->get_set_of("IfcSpace", cppw::include_subtypes);
	IList<Space ^> ^ spaces = gcnew List<Space ^>();
	for (instances.move_first(); instances.move_next(); ) {
		spaces->Add(gcnew Space(instances.get()));
	}
	return spaces;
}

ICollection<Element ^> ^ GetElements(cppw::Open_model * model, ConstructionCollection ^ constructions) {
	if (model == __nullptr) {
		return nullptr;
	}
	cppw::Instance_set instances = model->get_set_of("IfcBuildingElement", cppw::include_subtypes);
	IList<Element ^> ^ elements = gcnew List<Element ^>();
	for (instances.move_first(); instances.move_next(); ) {
		elements->Add(gcnew Element(instances.get(), constructions));
	}
	return elements;
}

} // namespace

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
	catch (cppw::Error &) { /* who cares? */ }
}

char * EdmSession::convert_to_chars(char dst[], String ^ src, size_t size) {
	char * str = (char *)(Marshal::StringToHGlobalAnsi(src)).ToPointer();
	strncpy_s(dst, size, str, size - 1);
	Marshal::FreeHGlobal(IntPtr(str));
	return dst;
}

void EdmSession::LoadIfcFile(String ^ path) {
	try {
		currentIfcPath = path;
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
	res->SpacesByGuid = gcnew Dictionary<String ^, Space ^>();
	res->ElementsByGuid = gcnew Dictionary<String ^, Element ^>();

	res->Filename = currentIfcPath;

	GetLocationInformation(model, res->NorthAxis, res->Latitude, res->Longitude, res->Elevation);

	ICollection<Space ^> ^ spaces = GetSpaces(model);
	for each(Space ^ s in spaces) {
		res->SpacesByGuid[s->Guid] = s;
	}

	ConstructionCollection ^ constructions = gcnew ConstructionCollection();
	ICollection<Element ^> ^ elements = GetElements(model, constructions);
	for each(Element ^ e in elements) {
		res->ElementsByGuid[e->Guid] = e;
	}
	res->ConstructionMappingSources = constructions->MappingSources;

	return res;
}

} // namespace IfcInformationExtractor