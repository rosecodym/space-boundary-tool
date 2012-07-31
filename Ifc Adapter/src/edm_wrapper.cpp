#include "precompiled.h"

#include <tchar.h>

#include "sbt-ifcadapter.h"

#include "edm_wrapper.h"

extern sb_calculation_options g_opts;

const char * const REPO_NAME = "repo";
const char * const MODEL_NAME = "model";
const char * const DEFAULT_DB_NAME = "db";
const char * const DEFAULT_DB_PASS = "pass";

const char * const LICENSE_KEY = "EDM LICENSE KEY";

bool edm_wrapper::license_inited = false;

namespace {

	void print_last_error() {
		DWORD code = GetLastError();
		LPVOID msg;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			code,
			0,
			(LPTSTR)&msg,
			0,
			NULL);
		_ftprintf(stderr, (TCHAR *)msg);
		LocalFree(msg);
		assert(false);
	}

	cppw::String make_cppw_string(TCHAR * str) {
#ifdef UNICODE
		char t[MAX_PATH];
		size_t l = (wcslen(str) + 1) * 2;
		WideCharToMultiByte(CP_ACP, 0, str, (int)l, t, (int)l, NULL, NULL);
		return cppw::String(t);
#else
		return cppw::String(str);
#endif
	}

	void init_license(const char * const key) {
		sdai::tSdaiSelect mySelect;
		mySelect.value.stringVal = sdai::SdaiString(key);
		mySelect.type = sdai::sdaiSTRING;
		sdai::EdmiError rstat = edmiSetInternalDataBN("EDM_LICENSE", "", "", 0, &mySelect);
		if (rstat != 0) {
			char buf[256];
			sprintf(buf, "An error occured while setting up the EDM license: %s\n", sdai::edmiGetErrorText(rstat));
			g_opts.error_func(buf);
		}
	}

	TCHAR * get_temp_path(TCHAR path[MAX_PATH]) {
		DWORD pathlen = GetTempPath(MAX_PATH, path);
		if (pathlen == 0 || pathlen > MAX_PATH - 16) {
			return NULL;
		}
		return path;
	}

	void unpack_schema(TCHAR scratch_path[MAX_PATH], TCHAR schema_path[MAX_PATH]) {
		LPCTSTR an_address_in_this_module = (LPCTSTR)&unpack_schema;
		HMODULE curr_module;
		if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, an_address_in_this_module, &curr_module) == 0) {
			print_last_error();
			return;
		}
		HRSRC rsc = FindResource(curr_module, MAKEINTRESOURCE(101), _T("express_schema"));
		if (rsc == NULL) {
			print_last_error();
			return;
		}
		HGLOBAL hnd = LoadResource(curr_module, rsc);
		if (hnd == NULL) {
			print_last_error();
			return;
		}
		LPVOID lock = LockResource(hnd);
		DWORD size = SizeofResource(curr_module, rsc);
		char * schema;
		schema = (char *)calloc(size, sizeof(char));
		memcpy(schema, lock, size);

		_tcsncpy(schema_path, scratch_path, _tcslen(scratch_path) + 1);
		_tcsncat(schema_path, _T("IFC2x3_final.exp"), 16);

		HANDLE outfile = CreateFile(
			schema_path,
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_TEMPORARY,
			NULL);
		if (outfile == INVALID_HANDLE_VALUE) {
			print_last_error();
			free(schema);
			return;
		}

		DWORD bytes_written;
		if (!WriteFile(outfile, schema, size, &bytes_written, NULL)) {
			print_last_error();
			free(schema);
			return;
		}

		CloseHandle(outfile);
	}

}

void edm_wrapper::setup_schema() {
	cppw::String path = make_cppw_string(schema_path);
	g_opts.notify_func("Loading the schema...");
	try {
		clear_db();
		manager.create_db(dbpath, dbname, dbpass);
		data = manager.open_db(dbpath, dbname, dbpass);
		data->create_repository(REPO_NAME);
	}
	catch (cppw::Error & e) {
		char buf[256];
		sprintf(buf, "\nError while setting up the schema: %s\n", e.message.data());
		g_opts.error_func(buf);
	}
	cppw::Compile_results res = cppw::Express_compiler(path).compile();
	if (res.errors) {
		clear_db();
		throw "Couldn't compile the schema.";
	}
	g_opts.notify_func("done.\n");
}

cppw::Open_model & edm_wrapper::load_ifc_file(const std::string & filename) {
	if (repository != nullptr || model != nullptr) {
		throw "Can't open two models.";
	}
	cppw::Step_reader(filename.c_str(), REPO_NAME, MODEL_NAME).read();
	repository = new cppw::Open_repository(data->get_repository(REPO_NAME), cppw::RW_access);
	model = new cppw::Open_model(repository->get_model(MODEL_NAME), cppw::RW_access);
	return *model;
}

void edm_wrapper::write_ifc_file(const std::string & filename) {
	cppw::Step_writer(model->get_model(), filename.c_str()).write();
}

edm_wrapper::edm_wrapper()
	: dbname(DEFAULT_DB_NAME), dbpass(DEFAULT_DB_PASS), repository(nullptr), model(nullptr) 
{
	if (!license_inited) {
		init_license(LICENSE_KEY);
		license_inited = true;
	}
	manager.std_output(false);
	TCHAR temp_path[MAX_PATH];
	get_temp_path(temp_path);
	unpack_schema(temp_path, schema_path);
	dbpath = make_cppw_string(temp_path);
	setup_schema();
}

edm_wrapper::~edm_wrapper() { 
	if (model != nullptr) {
		delete model;
		model = nullptr;
	}
	if (repository != nullptr) {
		delete repository;
		repository = nullptr;
	}
	manager.close(data); 
	clear_db(); 
	DeleteFile(schema_path);
}

void edm_wrapper::clear_db() {
	if (manager.does_db_exist(dbpath, dbname)) {
		manager.remove_db(dbpath, dbname, dbpass);
	}
}