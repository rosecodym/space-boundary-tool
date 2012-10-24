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

class bad_brep_exception : public unsupported_geometry_exception {
public:
	virtual const char * condition() const throw() { return "bad b-rep"; }
};

class unknown_geometry_rep_exception : public sbt_exception { };
class invalid_face_exception : public sbt_exception { };