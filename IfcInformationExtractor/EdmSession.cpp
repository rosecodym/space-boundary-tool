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

double GetMetersPerLengthUnit(const cppw::Instance & inst) {
	if (inst.is_kind_of("IfcSIUnit")) {
		cppw::Select sel = inst.get("Prefix");
		if (sel.is_set()) {
			cppw::String prefix = sel;
			if (prefix == "MEGA") { return 1e6; }
			else if (prefix == "KILO") { return 1e3; }
			else if (prefix == "HECTO") { return 1e2; }
			else if (prefix == "DECA") { return 1e1; }
			else if (prefix == "DECI") { return 1e-1; }
			else if (prefix == "CENTI") { return 1e-2; }
			else if (prefix == "MILLI") { return 1e-3; }
			else if (prefix == "MICRO") { return 1e-6; }
		}
	}
	else if (inst.is_kind_of("IfcConversionBasedUnit")) {
		cppw::Instance conversionFactor = inst.get("ConversionFactor");
		double factor = (cppw::Real)conversionFactor.get("ValueComponent");
		cppw::Instance unitComponent = conversionFactor.get("UnitComponent");
		return factor * GetMetersPerLengthUnit(unitComponent);
	}
	return 1.0;	
}

double GetMetersPerLengthUnit(const cppw::Open_model & model) {
	cppw::Set projects = model.get_set_of("IfcProject");
	cppw::Instance project = projects.get_(0);
	cppw::Instance unitAssignment = project.get("UnitsInContext");
	double res = 1.0;
	cppw::Set units = unitAssignment.get("Units");
	for (units.move_first(); units.move_next(); ) {
		cppw::Instance unit = units.get_();
		if (unit.is_kind_of("IfcNamedUnit")) {
			cppw::String type = unit.get("UnitType");
			if (type == "LENGTHUNIT") {
				res = GetMetersPerLengthUnit(unit);
			}
		}
	}
	return res;
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

	double mplu = GetMetersPerLengthUnit(*model);
	ConstructionCollection ^ constrs = gcnew ConstructionCollection(mplu);
	ICollection<Element ^> ^ elements = GetElements(model, constrs);
	for each(Element ^ e in elements) {
		res->ElementsByGuid[e->Guid] = e;
	}
	res->ConstructionMappingSources = constrs->MappingSources;

	return res;
}

} // namespace IfcInformationExtractor