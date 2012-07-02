#include "precompiled.h"

#include <boost/range/algorithm.hpp>

#include "one_dimensional_equality_context.h"

NT one_dimensional_equality_context::request(double d) {

	typedef one_dimensional_equality_context::interval_wrapper::inner_interval interval;
	typedef CGAL::Interval_skip_list<interval> interval_skip_list;

	auto res = cached.find(d);
	if (res != cached.end()) {
		return res->second;
	}

	std::vector<interval_wrapper> ints;
	intervals.find_intervals(d, std::back_inserter(ints));

	if (ints.size() == 0) {
		auto i = interval_wrapper(d, eps);
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
		auto nearest = boost::min_element(ints, [d](const interval_wrapper & a, const interval_wrapper & b) { 
			return abs(CGAL::to_double(a.actual) - d) < abs(CGAL::to_double(b.actual) - d);
		});
		cached[d] = nearest->actual;
		return nearest->actual;
	}
	
}