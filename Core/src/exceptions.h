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
	sbt_return_t code() const { return SBT_TOO_COMPLICATED; }
	const char * what() const throw() { return "Stack overflow."; }
};