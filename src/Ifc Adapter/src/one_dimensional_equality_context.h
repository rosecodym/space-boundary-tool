#pragma once

#include "precompiled.h"

template <typename NT>
class one_dimensional_equality_context : public boost::noncopyable {
private:

	class interval_wrapper {
	public:

		typedef CGAL::Interval_skip_list_interval<double> inner_interval;
		typedef inner_interval::Value Value;

		inner_interval inner;
		double instanced_low;
		double instanced_high;
		NT actual;

		Value inf() const { return inner.inf(); }
		Value sup() const { return inner.sup(); }
		bool contains(Value v) const { return inner.contains(v); }
		bool contains_interval(Value i, Value s) const { return inner.contains_interval(i, s); }
		bool operator == (const interval_wrapper & rhs) const { return inner == rhs.inner; }
		bool operator != (const interval_wrapper & rhs) const { return inner != rhs.inner; }

		interval_wrapper(double d, double tol) : inner(inner_interval(d - tol, d + tol)), instanced_low(d), instanced_high(d), actual(d) { }
		interval_wrapper(inner_interval inner, double low, double high, NT actual) : inner(inner), instanced_low(low), instanced_high(high), actual(actual) { }

	};

	CGAL::Interval_skip_list<interval_wrapper> intervals;
	double eps;
	std::map<double, NT> cached;

public:

	one_dimensional_equality_context(double epsilon) : eps(epsilon) { request(0.0); request(1.0); }

	NT request(double d) {
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

	bool is_zero(double d) const { return is_zero(d, eps); }
	bool is_zero(const NT & n) const { return is_zero(n, eps); }
	bool is_zero_squared(double d) const { return is_zero_squared(d, eps); }
	bool is_zero_squared(const NT & n) const { return is_zero_squared(n, eps); }
	bool are_equal(const NT & a, const NT & b) const { return are_equal(a, b, eps); }

	NT snap(const NT & x) { return request(CGAL::to_double(x)); }
	void snap(NT * n) { *n = snap(*n); }
	
	static bool is_zero(double d, double eps) { return d < eps && d > -eps; }
	static bool is_zero(const NT & n, double eps) { return is_zero(CGAL::to_double(n), eps); }
	static bool is_zero_squared(double d, double eps) { return d < eps * eps && d > -eps * eps; }
	static bool is_zero_squared(const NT & n, double eps) { return is_zero_squared(CGAL::to_double(n), eps); }
	static bool are_equal(const NT & a, const NT & b, double eps) { return is_zero(a - b, eps); }
};

// one_dimensional_equality_context<double> needs an explicit specialization
// because otherwise a bunch of its overloaded member functions are ambiguous.
// Currently, this specialization is only used in a kind of dummy role (i.e.
// its synchronization of "known" numbers isn't really useful) but I'm not
// going to no-op the corresponding functions because I don't know what future
// use cases will be and I'd rather not surprise anyone.

template <>
class one_dimensional_equality_context<double> : public boost::noncopyable {
private:

	class interval_wrapper {
	public:
		typedef CGAL::Interval_skip_list_interval<double> inner_interval;
		typedef inner_interval::Value Value;

		inner_interval inner;
		double instanced_low;
		double instanced_high;
		double actual;

		Value inf() const { return inner.inf(); }
		Value sup() const { return inner.sup(); }
		bool contains(Value v) const { return inner.contains(v); }
		bool contains_interval(Value i, Value s) const { 
			return inner.contains_interval(i, s); 
		}
		bool operator == (const interval_wrapper & rhs) const { 
			return inner == rhs.inner; 
		}
		bool operator != (const interval_wrapper & rhs) const { 
			return inner != rhs.inner; 
		}

		interval_wrapper(double d, double tol) 
			: inner(inner_interval(d - tol, d + tol)), 
			  instanced_low(d), instanced_high(d), actual(d) 
		{ }
		interval_wrapper(
			inner_interval inner, 
			double low, 
			double high, 
			double actual) 
			: inner(inner), 
			  instanced_low(low), 
			  instanced_high(high), 
			  actual(actual) 
		{ }
	};

	CGAL::Interval_skip_list<interval_wrapper> intervals;
	double eps;
	std::map<double, double> cached;

public:

	one_dimensional_equality_context(double epsilon) : eps(epsilon) 
	{ 
		request(0.0); request(1.0); 
	}

	double request(double d) {
		typedef interval_wrapper::inner_interval interval;
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
			auto nearest = boost::min_element(
				ints, 
				[d](const interval_wrapper & a, const interval_wrapper & b) { 
					return 
						abs(CGAL::to_double(a.actual) - d) < 
						abs(CGAL::to_double(b.actual) - d);
				});
			cached[d] = nearest->actual;
			return nearest->actual;
		}
	}

	bool is_zero(double d) const { return is_zero(d, eps); }
	bool is_zero_squared(double d) const { return is_zero_squared(d, eps); }
	bool are_equal(double a, double b) const { return are_equal(a, b, eps); }

	double snap(double x) { return request(x); }
	void snap(double * n) { *n = snap(*n); }
	
	static bool is_zero(double d, double eps) { return d < eps && d > -eps; }
	static bool is_zero_squared(double d, double eps) { 
		return d < eps * eps && d > -eps * eps; 
	}
	static bool are_equal(double a, double b, double eps) { 
		return is_zero(a - b, eps); 
	}
};