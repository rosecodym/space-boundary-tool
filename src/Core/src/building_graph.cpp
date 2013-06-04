#include "precompiled.h"

#include "building_graph.h"

#include "equality_context.h"
#include "space.h"
#include "space_face.h"

namespace traversal {

namespace impl {

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
	double height_eps) 
{
	typedef boost::optional<double> height_maybe;

	struct v : public boost::static_visitor<height_maybe> {
		double height_eps_;
		// This predicate is lazy because it's expensive and only necessary if
		// the heights match.
		std::function<bool(void)> areas_match_;
		v(
			double height_eps,
			const std::function<bool(void)> & areas_match) 
			: height_eps_(height_eps),
			  areas_match_(areas_match)
		{ }

		static double avg(const NT & a, const NT & b) {
			return (CGAL::to_double(a) + CGAL::to_double(b)) / 2;
		}

		height_maybe operator () (space_face * f1, space_face * f2) const {
			if (f1 != f2 &&
				f1->sense() != f2->sense() &&
				equality_context::are_equal(
					f1->height(), 
					f2->height(), 
					height_eps_))
			{
				if (areas_match_()) { return avg(f1->height(), f2->height()); }
			}
			return height_maybe();
		}

		height_maybe operator () (space_face * f, const block * b) const {
			height_maybe matching_height;
			if (f->sense() == b->sense()) 
			{
				if (b->heights().second &&
					equality_context::are_equal(
						f->height(), 
						*b->heights().second, 
						height_eps_))
				{
					matching_height = avg(f->height(), *b->heights().second);
				}
			}
			else {
				if (equality_context::are_equal(
						f->height(), 
						b->heights().first, 
						height_eps_)) 
				{
					matching_height = avg(f->height(), b->heights().first);
				}
			}
			if (matching_height && areas_match_())
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
					equality_context::are_equal(
						*b1_hs.second, 
						b2_hs.first,
						height_eps_))
				{
					matching_height = avg(*b1_hs.second, b2_hs.first);
				}
				else if (
					b2_hs.second && 
					equality_context::are_equal(
						b1_hs.first, 
						*b2_hs.second,
						height_eps_))
				{
					matching_height = avg(b1_hs.first, *b2_hs.second);
				}
			}
			else {
				if (equality_context::are_equal(
						b1_hs.first, 
						b2_hs.first,
						height_eps_)) 
				{
					matching_height = avg(b1_hs.first, b2_hs.first);
				}
				else if (
					b1_hs.second && 
					b2_hs.second && 
					equality_context::are_equal(
						*b1_hs.second, 
						*b2_hs.second,
						height_eps_))
				{
					matching_height = avg(*b1_hs.second, *b2_hs.second);
				}
			}
			if (matching_height && areas_match_())
			{
				return matching_height;
			}
			else { return height_maybe(); }
		}
	};
	auto areas_match = [&]() { return !(a.a() * b.a()).is_empty(); };
	return boost::apply_visitor(v(height_eps, areas_match), a.data_, b.data_);
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