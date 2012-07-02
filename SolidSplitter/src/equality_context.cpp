#include "precompiled.h"

#include "exceptions.h"

#include "equality_context.h"

void equality_context::init_constants()
{
	_zero = request_coordinate(0.0);
	_one = request_coordinate(1.0);
}

NT equality_context::request_coordinate(double d) {

	typedef equality_context::interval_wrapper::inner_interval interval;
	typedef CGAL::Interval_skip_list<interval> interval_skip_list;

	auto res = cached.find(d);
	if (res != cached.end()) {
		return res->second;
	}

	std::vector<interval_wrapper> ints;
	intervals.find_intervals(d, std::back_inserter(ints));

	if (ints.size() == 0) {
		auto i = interval_wrapper(d, tolerance);
		intervals.insert(i);
		cached[d] = i.actual;
		return i.actual;
	}

	else if (ints.size() == 1) {
		interval_wrapper & i = ints.front();
		cached[d] = i.actual;
		return i.actual;
	}

	else {
		throw bad_input_exception(bad_input_exception::NON_TRANSITIVE_SOLID);
	}
	
}