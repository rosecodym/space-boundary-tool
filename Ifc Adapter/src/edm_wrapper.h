#pragma once

#include "precompiled.h"

#include <cpp_edmi.h>

extern const char * const REPO_NAME;
extern const char * const MODEL_NAME;
extern const char * const DEFAULT_DB_NAME;
extern const char * const DEFAULT_DB_PASS;

class edm_wrapper {
	cppw::EDM manager;
	cppw::Database_handler data;
	cppw::Open_repository * repository;
	cppw::Open_model * model;

	cppw::String dbpath; // not const because it can't be populated in all the initializer lists :c++:
	const cppw::String dbname;
	const cppw::String dbpass;

	TCHAR schema_path[MAX_PATH];

	void clear_db();

	void setup_schema();
	
	edm_wrapper(const edm_wrapper & other);
	edm_wrapper & operator = (const edm_wrapper & other);

	static bool license_inited;

public:
	edm_wrapper();
	~edm_wrapper();

	cppw::Open_model & load_ifc_file(const std::string & filename);
	void write_ifc_file(const std::string & filename);

};