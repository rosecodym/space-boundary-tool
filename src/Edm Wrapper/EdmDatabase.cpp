#include <cpp_edmi.h>

#include "EdmDatabase.h"

using namespace System::Reflection;
using namespace System::Runtime::InteropServices;

namespace IfcInterface {

namespace {

const int MAX_PATH = 260;

char * managedStringToNativeString(char dst[], String ^ src, size_t len) {
	char * str = (char *)(Marshal::StringToHGlobalAnsi(src)).ToPointer();
	strncpy(dst, str, len);
	Marshal::FreeHGlobal(IntPtr(str));
	return dst;
}

} // namespace

EdmDatabase::EdmDatabase() : manager_(new cppw::EDM()) {
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
		managedStringToNativeString(tmp_dir, tmpDir, MAX_PATH);
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
		managedStringToNativeString(schema_path, schemaPath, MAX_PATH);
		cppw::Express_compiler compiler(schema_path);
		cppw::Compile_results res = compiler.compile();
		if (res.errors) { 
			ClearDB(); 
			throw gcnew Exception("Error compiling the schema.");
		}
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

} // namespace IfcInterface