#pragma once

#include "precompiled.h"

#include "sbt-core.h"

class sbt_exception : public std::exception {
public:
	virtual sbt_return_t code() const { return SBT_UNKNOWN; }
	virtual const char * what() const throw() { return "Unknown processing error."; }
};

class stack_overflow_exception : public sbt_exception {
public:
	virtual sbt_return_t code() const { return SBT_TOO_COMPLICATED; }
	virtual const char * what() const throw() { return "Stack overflow."; }
};

class failed_malloc_exception : public sbt_exception {
public:
	virtual sbt_return_t code() const { return SBT_FAILED_ALLOCATION; }
	virtual const char * what() const throw() { return "Failed malloc."; }
};

class unsupported_geometry_exception : public sbt_exception {
public:
	virtual sbt_return_t code() const { return SBT_UNSUPPORTED; }
	virtual const char * what() const throw() { std::string * str = new std::string((boost::format("Unsupported geometry (%s).") % condition()).str()); return str->c_str(); }
	virtual const char * condition() const throw() = 0;
};

class brep_with_voids_exception : public unsupported_geometry_exception {
public:
	virtual const char * condition() const throw() { return "b-rep with a face void"; }
};

class bad_geometry_exception : public sbt_exception { 
public:
	virtual const char * what() const throw() {
		std::string * str = new std::string(
			(boost::format("Bad geometry (%s).") % condition()).str());
		return str->c_str();
	}
	virtual const char * condition() const throw() = 0;
};

class bad_brep_exception : public bad_geometry_exception {
public:
	virtual const char * condition() const throw() { return "bad b-rep"; }
};

class invalid_face_exception : public bad_geometry_exception {
public:
	virtual const char * condition() const throw() { return "invalid face"; }
};

class parallel_ext_exception : public bad_geometry_exception {
public:
	virtual const char * condition() const throw() { 
		return "extruded area solid with extrusion direction parallel to base";
	}
};

// This isn't a bad_geometry_exception because it indicates an inconsistent
// program state, not bad geometry.
class unknown_geometry_rep_exception : public sbt_exception { };