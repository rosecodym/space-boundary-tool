#include "precompiled.h"

#include "cgal-typedefs.h"

class equality_context {
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
	double tolerance;
	std::map<double, NT> cached;
	NT _zero;
	NT _one;

	void init_constants();

	equality_context(const equality_context & src);
	equality_context & operator = (const equality_context & src);

public:

	equality_context(double tol) : tolerance(tol) { init_constants(); }

	const NT & zero() const { return _zero; }
	const NT & one() const { return _one; }

	NT request_coordinate(double d);

	static bool is_zero(double d, double eps) { return d < eps && d > -eps; }
	static bool is_zero(const NT & n, double eps) { return is_zero(CGAL::to_double(n), eps); }
	static bool are_equal(const NT & a, const NT & b, double eps) { return is_zero(a - b, eps); }
	static bool is_zero_squared(double d, double eps) { return d < eps * eps && d > -eps * eps; }
	static bool is_zero_squared(const NT & n, double eps) { return is_zero_squared(CGAL::to_double(n), eps); }

	bool is_zero(double d) const { return is_zero(d, tolerance); }
	bool is_zero(const NT & n) const { return is_zero(CGAL::to_double(n)); }
	bool are_equal(const NT & a, const NT & b) const { return is_zero(a - b); }
	bool is_zero_squared(double d) const { return is_zero_squared(d, tolerance); }
	bool is_zero_squared(const NT & n) const { return is_zero_squared(CGAL::to_double(n), tolerance); }
	bool is_zero_squared(const CGAL::Nef_polynomial<NT> & n) const { return is_zero_squared(CGAL::to_double(n), tolerance); }

	bool are_effectively_parallel(const vector_3 & a, const vector_3 & b) const { return is_zero_squared(CGAL::cross_product(a, b).squared_length()); }
	bool are_effectively_collinear(const point_3 & a, const point_3 & b, const point_3 & c) const { return a == c || is_zero_squared(CGAL::squared_distance(b, line_3(a, c))); }
};