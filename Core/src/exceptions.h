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
	virtual sbt_return_t code() const { return SBT_FAILED_ALLOCATION; }
	virtual const char * what() const throw() { return "Failed malloc."; }
};