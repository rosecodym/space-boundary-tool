#include "precompiled.h"

#include "area.h"
#include "equality_context.h"

#include "stackable.h"

namespace stacking {

namespace impl {

area stackable::stackable_area() const {
	struct v : public boost::static_visitor<area> {
		area operator () (space_face * f) const { return f->face_area(); }
		area operator () (const block * b) const { return b->base_area(); }
	};
	return boost::apply_visitor(v(), m_data);
}

double stackable::thickness() const {
	struct v : public boost::static_visitor<double> { 
		double operator () (space_face *) const { return 0; }
		double operator () (const block * b) const { 
			return b->heights().second ? abs(CGAL::to_double(b->heights().first) - CGAL::to_double(*b->heights().second)) : 0;
		}
	};
	return boost::apply_visitor(v(), m_data);
}

boost::optional<space_face *> stackable::as_space_face() const {
	struct v : public boost::static_visitor<boost::optional<space_face *>> {
		boost::optional<space_face *> operator () (space_face * f) const { return f; }
		boost::optional<space_face *> operator () (const block *) const { return boost::optional<space_face *>(); }
	};
	return boost::apply_visitor(v(), m_data);
}

boost::optional<stackable_connection> stackable_connection::do_connect(stackable a, stackable b, double height_eps) {
	struct v : public boost::static_visitor<boost::optional<stackable_connection>> {
		double eps;
		v(double eps) : eps(eps) { }
		boost::optional<stackable_connection> operator () (space_face * f1, space_face * f2) const {
			if (f1 != f2 &&
				f1->sense() != f2->sense() &&
				equality_context::are_equal(f1->height(), f2->height(), eps))
			{
				area intr = f1->face_area() * f2->face_area();
				if (!intr.is_empty()) {
					return stackable_connection((CGAL::to_double(f1->height()) + CGAL::to_double(f2->height())) / 2, std::move(intr));
				}
			}
			return boost::optional<stackable_connection>();
		}
		boost::optional<stackable_connection> operator () (space_face * f, const block * b) const {
			boost::optional<double> matching_height;
			if (f->sense() == b->sense()) {
				if (b->heights().second && equality_context::are_equal(f->height(), *b->heights().second, eps)) {
					matching_height = (CGAL::to_double(f->height()) + CGAL::to_double(*b->heights().second)) / 2;
				}
			}
			else {
				if (equality_context::are_equal(f->height(), b->heights().first, eps)) {
					matching_height = (CGAL::to_double(f->height()) + CGAL::to_double(b->heights().first)) / 2;
				}
			}
			if (matching_height) {
				area intr = f->face_area() * b->base_area();
				if (!intr.is_empty()) {
					return stackable_connection(*matching_height, std::move(intr));
				}
			}
			return boost::optional<stackable_connection>();
		}
		boost::optional<stackable_connection> operator () (const block * b, space_face * f) const { return (*this)(f, b); }
		boost::optional<stackable_connection> operator () (const block * b1, const block * b2) const {
			auto b1_heights = b1->heights();
			auto b2_heights = b2->heights();
			boost::optional<double> matching_height;
			if (b1->sense() == b2->sense()) {
				if (b1_heights.second && equality_context::are_equal(*b1_heights.second, b2_heights.first, eps)) {
					matching_height = (CGAL::to_double(*b1_heights.second) + CGAL::to_double(b2_heights.first)) / 2;
				}
				else if (b2_heights.second && equality_context::are_equal(b1_heights.first, *b2_heights.second, eps)) {
					matching_height = (CGAL::to_double(b1_heights.first) + CGAL::to_double(*b2_heights.second)) / 2;
				}
			}
			else {
				if (equality_context::are_equal(b1_heights.first, b2_heights.first, eps)) {
					matching_height = (CGAL::to_double(b1_heights.first) + CGAL::to_double(b2_heights.first)) / 2;
				}
				else if (b1_heights.second && b2_heights.second && equality_context::are_equal(*b1_heights.second, *b2_heights.second, eps)) {
					matching_height = (CGAL::to_double(*b1_heights.second) + CGAL::to_double(*b2_heights.second)) / 2;
				}
			}
			if (matching_height) {
				area intr = b1->base_area() * b2->base_area();
				if (!intr.is_empty()) {
					return stackable_connection(*matching_height, std::move(intr));
				}
			}
			return boost::optional<stackable_connection>();
		}
	};
	return boost::apply_visitor(v(height_eps), a.data(), b.data());
}

} // namespace impl

} // namespace stacking