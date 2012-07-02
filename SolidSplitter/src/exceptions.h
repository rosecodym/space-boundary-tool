#include "precompiled.h"

class polyloop_clean_failure_exception : public std::exception {
public:
	const enum clean_failure_reason {
		NO_INITIAL,
		CLEANED_TO_TWO
	} reason;
	polyloop_clean_failure_exception(clean_failure_reason r) : reason(r) { }
	const char * what() const throw() { return "Couldn't clean up a polyloop."; }
};

class bad_input_exception : public std::exception {
public:
	const enum split_failure_reason {
		SOLID_WITH_VOIDS,
		NON_TRANSITIVE_SOLID,
		NO_BASE_PLANE
	} reason;
	bad_input_exception(split_failure_reason r) : reason(r) { }
	const char * what() const throw() { return "Couldn't split a solid."; }
};