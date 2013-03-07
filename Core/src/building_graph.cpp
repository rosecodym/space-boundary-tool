#include "precompiled.h"

#include "building_graph.h"

#include "equality_context.h"
#include "space.h"
#include "space_face.h"

namespace traversal {

namespace impl {

const geometry_2d::area & bg_vertex_data::a() const {
	struct v : public boost::static_visitor<const geometry_2d::area &> {
		const geometry_2d::area & operator () (space_face * sf) const {
			return sf->face_area();
		}
		const geometry_2d::area & operator () (const block * b) const {
			return b->base_area();
		}
	};
	return boost::apply_visitor(v(), data_);
}

double bg_vertex_data::thickness() const {
	struct v : public boost::static_visitor<double> {
		double operator () (space_face *) const { return 0; }
		double operator () (const block * b) const {
			if (b->heights().second) {
				double h1 = CGAL::to_double(b->heights().first);
				double h2 = CGAL::to_double(*b->heights().second);
				return abs(h1 - h2);
			}
			else { return 0; }
		}
	};
	return boost::apply_visitor(v(), data_);
}

std::string bg_vertex_data::identifier() const {
	struct v : public boost::static_visitor<const std::string &> {
		const std::string & operator () (space_face * sf) const {
			return sf->bounded_space()->global_id();
		}
		const std::string & operator () (const block * b) const {
			return b->material_layer().layer_element().name();
		}
	};
	return boost::apply_visitor(v(), data_);
}

bool bg_vertex_data::represents_halfblock() const {
	struct v : public boost::static_visitor<bool> {
		bool operator () (space_face *) const { return false; }
		bool operator () (const block * b) const {
			return !b->heights().second;
		}
	};
	return boost::apply_visitor(v(), data_);
}

boost::optional<double> bg_vertex_data::do_connect(
	bg_vertex_data a, 
	bg_vertex_data b, 
	const equality_context & c) 
{
	typedef boost::optional<double> height_maybe;

	struct v : public boost::static_visitor<height_maybe> {
		const equality_context & c_;
		v(const equality_context & c) : c_(c) { }

		static double avg(const NT & a, const NT & b) {
			return (CGAL::to_double(a) + CGAL::to_double(b)) / 2;
		}

		height_maybe operator () (space_face * f1, space_face * f2) const {
			if (f1 != f2 &&
				f1->sense() != f2->sense() &&
				c_.are_equal(f1->height(), f2->height()))
			{
				if (!(f1->face_area() * f2->face_area()).is_empty()) {
					return avg(f1->height(), f2->height());
				}
			}
			return height_maybe();
		}

		height_maybe operator () (space_face * f, const block * b) const {
			height_maybe matching_height;
			if (f->sense() == b->sense()) {
				if (b->heights().second &&
					c_.are_equal(f->height(), *b->heights().second))
				{
					matching_height = avg(f->height(), *b->heights().second);
				}
			}
			else {
				if (c_.are_equal(f->height(), b->heights().first)) {
					matching_height = avg(f->height(), b->heights().first);
				}
			}
			if (matching_height && 
				!(f->face_area() * b->base_area()).is_empty())
			{
				return matching_height;
			}
			else { return height_maybe(); }
		}

		height_maybe operator () (const block * b, space_face * f) const {
			return (*this)(f, b);
		}

		height_maybe operator () (const block * b1, const block * b2) const {
			auto b1_hs = b1->heights();
			auto b2_hs = b2->heights();
			height_maybe matching_height;
			if (b1->sense() == b2->sense()) {
				if (b1_hs.second && 
					c_.are_equal(*b1_hs.second, b2_hs.first))
				{
					matching_height = avg(*b1_hs.second, b2_hs.first);
				}
				else if (
					b2_hs.second && 
					c_.are_equal(b1_hs.first, *b2_hs.second))
				{
					matching_height = avg(b1_hs.first, *b2_hs.second);
				}
			}
			else {
				if (c_.are_equal(b1_hs.first, b2_hs.first)) {
					matching_height = avg(b1_hs.first, b2_hs.first);
				}
				else if (
					b1_hs.second && 
					b2_hs.second && 
					c_.are_equal(*b1_hs.second, *b2_hs.second))
				{
					matching_height = avg(*b1_hs.second, *b2_hs.second);
				}
			}
			if (matching_height &&
				!(b1->base_area() * b2->base_area()).is_empty())
			{
				return matching_height;
			}
			else { return height_maybe(); }
		}
	};
	return boost::apply_visitor(v(c), a.data_, b.data_);
}

space_face * bg_vertex_data::represents_space_face() const {
	struct v : public boost::static_visitor<space_face *> {
		space_face * operator () (space_face * sf) const { return sf; }
		space_face * operator () (const block *) const { return nullptr; }
	};
	return boost::apply_visitor(v(), data_);
}

typedef boost::optional<layer_information> layer_maybe;

layer_maybe bg_vertex_data::to_layer() const {
	struct v : public boost::static_visitor<layer_maybe> {
		layer_maybe operator () (space_face *) const {
			return layer_maybe();
		}
		layer_maybe operator () (const block * b) const {
			return b->material_layer();
		}
	};
	return boost::apply_visitor(v(), data_);
}

} // namespace impl

} // namespace traversal