#include <cpp_edmi.h>

#include "EdmDatabase.h"

#include "length_units_per_meter.h"

using namespace System::Runtime::InteropServices;
using namespace System::Text::RegularExpressions;

namespace IfcInterface {

namespace {

const int MAX_PATH = 260;

inline char * managed_string_to_native(char dst[], String ^ src, size_t len) {
	char * str = (char *)(Marshal::StringToHGlobalAnsi(src)).ToPointer();
	strncpy(dst, str, len);
	Marshal::FreeHGlobal(IntPtr(str));
	return dst;
}

} // namespace

EdmDatabase::EdmDatabase() 
	: manager_(new cppw::EDM()), 
	  db_path_(__nullptr),
	  db_handler_(__nullptr),
	  repo_(__nullptr)
{
	String ^ schemaPath = System::IO::Path::GetTempFileName();
	String ^ resName = "IFC2X3_final.exp";
	Assembly ^ assembly = Assembly::GetExecutingAssembly();
	// According to the internet, simple scope blocks will mimic the
	// "using" C# construct. I'm skeptical but can't find a better answer.
	{
		Stream ^ in = assembly->GetManifestResourceStream(resName);
		{
			FileStream ^ out = gcnew FileStream(schemaPath, FileMode::Create);
			in->CopyTo(out);
			out->Close();
		}
		in->Close();
	}
	try {
		String ^ tmpDir = System::IO::Path::GetTempPath();
		char tmp_dir[MAX_PATH];
		managed_string_to_native(tmp_dir, tmpDir, MAX_PATH);
		db_path_ = new cppw::String(tmp_dir);
		ClearDB();
		cppw::String db_name(DB_NAME);
		cppw::String db_pass(DB_PASS);
		manager_->create_db(*db_path_, db_name, db_pass);
		db_handler_ = new cppw::Database_handler(manager_->open_db(
			*db_path_, 
			db_name, 
			db_pass));
		char schema_path[MAX_PATH];
		managed_string_to_native(schema_path, schemaPath, MAX_PATH);
		cppw::Express_compiler compiler(schema_path);
		cppw::Compile_results res = compiler.compile();
		if (res.errors) { 
			ClearDB(); 
			throw gcnew Exception("Error compiling the schema.");
		}
		(*db_handler_)->create_repository(cppw::String(REPO_NAME));
		repo_ = new cppw::Open_repository(
			(*db_handler_)->get_repository(cppw::String(REPO_NAME)),
			cppw::RW_access);
	}
	catch (cppw::Error & e) {
		char buf[256];
		sprintf(
			buf, 
			"Couldn't establish the EDM database (%s).\n", 
			e.message.data());
		throw gcnew Exception(gcnew String(buf));
	}
}

cppw::Open_model * EdmDatabase::LoadModel(String ^ path)
{
	String ^ modelName = System::IO::Path::GetFileNameWithoutExtension(path);
	String ^ patt = gcnew String("[^a-zA-Z]");
	modelName = Regex::Replace(modelName, patt, String::Empty);
	char pathBuf[MAX_PATH];
	char modelNameBuf[MAX_PATH];
	managed_string_to_native(pathBuf, path, MAX_PATH);
	managed_string_to_native(modelNameBuf, modelName, MAX_PATH);
	cppw::Step_reader(pathBuf, cppw::String(REPO_NAME), modelNameBuf).read();
	return new cppw::Open_model(
		repo_->get_model(modelNameBuf), 
		cppw::RW_access);
}

} // namespace IfcInterface