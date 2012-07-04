#include "precompiled.h"

#include "equality_context.h"
#include "printing-macros.h"
#include "printing-util.h"
#include "sbt-core.h"
#include "surface.h"

// HERE BE DRAGONS

#define PRINT_LEVELS(...) \
	do { \
		IF_FLAGGED(SBT_VERBOSE_LEVELS) { \
			sprintf(g_msgbuf, __VA_ARGS__); \
			NOTIFY_MSG(g_msgbuf); \
		} \
	} \
	while (false);

namespace {

typedef NT q_NT;
typedef NT r_NT;
typedef point_3 q_point_3;
typedef point_3 r_point_3;
typedef nef_polyhedron_3 q_nef_polyhedron_3;
typedef polyhedron_3 q_polyhedron_3;

q_point_3 convert_to_q(const r_point_3 & rp, std::shared_ptr<equality_context> & context) {
	return context->snap(rp);
}

class stack_geometry_builder : public CGAL::Modifier_base<q_polyhedron_3::HDS> {

public:

	typedef std::vector<q_point_3> point_sequence;

private:

	point_sequence points;
	std::vector<std::vector<size_t>> point_indices;

	stack_geometry_builder(const stack_geometry_builder & src);
	stack_geometry_builder & operator = (const stack_geometry_builder & src);

public:

	stack_geometry_builder(const std::vector<r_point_3> & base, const std::vector<r_point_3> & target, std::shared_ptr<equality_context> & context) {
		assert(base.size() == target.size());
		boost::transform(base, std::back_inserter(points), [&context](const r_point_3 & p) { return convert_to_q(p, context); });
		boost::transform(target, std::back_inserter(points), [&context](const r_point_3 & p) { return convert_to_q(p, context); });
		std::vector<size_t> face;
		for (size_t i = 0; i < base.size(); ++i) {
			face.push_back(i);
		}
		point_indices.push_back(std::move(face));
		for (size_t i = 0; i < base.size(); ++i) {
			face.clear(); // strictly speaking i don't know if you're allowed to do this to a moved-from container but i don't foresee any problems
			size_t j = i + 1 == base.size() ? 0 : i + 1;
			face.push_back(j);
			face.push_back(i);
			face.push_back(i + base.size());
			face.push_back(j + base.size());
			point_indices.push_back(std::move(face));
		}
		face.clear();
		for (size_t i = base.size(); i < points.size(); ++i) {
			face.push_back(i);
		}
		std::reverse(face.begin(), face.end());
		point_indices.push_back(std::move(face));
	}

	void operator () (q_polyhedron_3::HDS & hds) {
		CGAL::Polyhedron_incremental_builder_3<q_polyhedron_3::HDS> b(hds, true);
		b.begin_surface(points.size(), point_indices.size());
		for (auto p = points.begin(); p != points.end(); ++p) {
			b.add_vertex(*p);
		}
		for (auto f = point_indices.begin(); f != point_indices.end(); ++f) {
			b.add_facet(f->begin(), f->end());
		}
		b.end_surface();
	}

};

struct stack {
	std::pair<std::shared_ptr<surface>, std::shared_ptr<surface>> boundaries;
	q_nef_polyhedron_3 geometry;
	r_NT height;

	stack(std::shared_ptr<surface> a, std::shared_ptr<surface> b, std::shared_ptr<equality_context> context)
		: boundaries(std::make_pair(a, b)), height(context->snap_height(abs(a->geometry().height() - b->geometry().height())))
	{
		if (FLAGGED(SBT_EXPENSIVE_CHECKS) && !surface::oppose(a, b)) {
			ERROR_MSG("Space boundaries %s/%s and %s/%s form a stack but do not oppose.\n",
				a->guid().c_str(),
				a->get_space().lock()->global_id().c_str(),
				b->guid().c_str(),
				b->get_space().lock()->global_id().c_str());
			abort();
		}

		if (FLAGGED(SBT_VERBOSE_LEVELS)) {
			NOTIFY_MSG("[creating stack from p = %f]\n", CGAL::to_double(a->geometry().height()));
			polygon_2 poly = a->geometry().area_2d().to_single_polygon();
			PRINT_POLYGON(poly);
			NOTIFY_MSG("[and p = %f]\n", CGAL::to_double(b->geometry().height()));
			poly = b->geometry().area_2d().to_single_polygon();
			PRINT_POLYGON(poly);
		}

		polygon_2 a_area = context->snap(a->geometry().area_2d().to_single_polygon());
		polygon_2 b_area = context->snap(b->geometry().area_2d().to_single_polygon());

		size_t rot = 0;
		auto p = b_area.vertices_begin();
		while (*b_area.vertices_begin() != *p) {
			++p;
			++rot;
		}

		std::vector<r_point_3> a_points(a->geometry().to_3d().front().outer());
		std::vector<r_point_3> b_points(b->geometry().to_3d().front().outer());
		//std::reverse(b_points.begin(), b_points.end());
		std::rotate(b_points.begin(), b_points.begin() + rot, b_points.end());

		q_polyhedron_3 poly;
		stack_geometry_builder builder(a_points, b_points, context);
		poly.delegate(builder);
		geometry = q_nef_polyhedron_3(poly).interior();

		boundaries.first->set_level(2);
		boundaries.second->set_level(2);
	}

	bool is_entirely_outside() const {
		return boundaries.first->lies_on_outside() && boundaries.second->lies_on_outside();
	}

	int get_level() const {
		return boundaries.first->get_level();
	}

	void set_to_2() {
		boundaries.first->set_level(2);
		boundaries.second->set_level(2);
	}

	void set_to_3() {
		boundaries.first->set_level(3);
		boundaries.second->set_level(3);
	}

	static bool intersect(const stack & a, const stack & b) {
		//const oriented_area & geom_a1 = a.boundaries.first->geometry();
		//const oriented_area & geom_a2 = a.boundaries.second->geometry();
		//const oriented_area & geom_b1 = b.boundaries.first->geometry();
		//const oriented_area & geom_b2 = b.boundaries.second->geometry();
		return !(a.geometry * b.geometry).is_empty();
	}
};

typedef stack blockstack;

class bounds_interval {
private:
	NT m_low;
	NT m_high;
	blockstack * m_stack;
public:
	typedef NT Value;
	bounds_interval(NT low, NT high, blockstack & st) : m_low(low), m_high(high), m_stack(&st) { 
		SBT_ASSERT(low != high, "[Aborting - tried to create a degenerate bounds interval (at %f).]\n", CGAL::to_double(low));
	}
	Value inf() const { return m_low; }
	Value sup() const { return m_high; }
	const blockstack * associated_stack() const { return m_stack; }
	bool contains(Value x) const { return m_low <= x && m_high >= x; }
	bool contains_interval(Value i, Value s) const { return m_low <= i && m_high >= s; }
};

bool operator == (const bounds_interval & lhs, const bounds_interval & rhs) {
	return lhs.inf() == rhs.inf() && lhs.sup() == rhs.sup();
}

bool operator != (const bounds_interval & lhs, const bounds_interval & rhs) {
	return !(lhs == rhs);
}

bool operator < (const bounds_interval & lhs, const bounds_interval & rhs) {
	return lhs.associated_stack() < rhs.associated_stack();
}

std::tuple<bounds_interval, bounds_interval, bounds_interval> get_bounds_for(blockstack & st) {
	NT bounds[6];
	bool found[6] = { false, false, false, false, false, false };
	nef_vertex_handle v;
	CGAL_forall_vertices(v, st.geometry) {
		if (!found[0] || v->point().x() < bounds[0]) {
			bounds[0] = v->point().x();
			found[0] = true;
		}
		if (!found[1] || v->point().x() > bounds[1]) {
			bounds[1] = v->point().x();
			found[1] = true;
		}
		if (!found[2] || v->point().y() < bounds[2]) {
			bounds[2] = v->point().y();
			found[2] = true;
		}
		if (!found[3] || v->point().y() > bounds[3]) {
			bounds[3] = v->point().y();
			found[3] = true;
		}
		if (!found[4] || v->point().z() < bounds[4]) {
			bounds[4] = v->point().z();
			found[4] = true;
		}
		if (!found[5] || v->point().z() > bounds[5]) {
			bounds[5] = v->point().z();
			found[5] = true;
		}
	}
	return std::make_tuple(
		bounds_interval(bounds[0], bounds[1], st),
		bounds_interval(bounds[2], bounds[3], st),
		bounds_interval(bounds[4], bounds[5], st));
}

void identify_23s(std::vector<stack> & stacks) {
	PRINT_LEVELS("[entered identify_23s]\n");
	std::vector<std::pair<blockstack *, std::tuple<bounds_interval, bounds_interval, bounds_interval>>> with_bounds;
	CGAL::Interval_skip_list<bounds_interval> x_bounds;
	CGAL::Interval_skip_list<bounds_interval> y_bounds;
	CGAL::Interval_skip_list<bounds_interval> z_bounds;
	boost::for_each(stacks, [&with_bounds, &x_bounds, &y_bounds, &z_bounds](blockstack & st) {
		auto bounds = get_bounds_for(st);
		with_bounds.push_back(std::make_pair(&st, bounds));
		x_bounds.insert(std::get<0>(bounds));
		y_bounds.insert(std::get<1>(bounds));
		z_bounds.insert(std::get<2>(bounds));
	});
	boost::for_each(with_bounds, [&x_bounds, &y_bounds, &z_bounds](std::pair<blockstack *, std::tuple<bounds_interval, bounds_interval, bounds_interval>> & info) {
		PRINT_LEVELS("[checking stack from %f (%s) to %f (%s)]\n", 
			CGAL::to_double(info.first->boundaries.first->geometry().height()), 
			info.first->boundaries.first->guid().c_str(),
			CGAL::to_double(info.first->boundaries.second->geometry().height()),
			info.first->boundaries.second->guid().c_str());
		std::set<bounds_interval> x_ints;
		std::set<bounds_interval> y_ints;
		std::set<bounds_interval> z_ints;
		x_bounds.find_intervals(std::get<0>(info.second).inf(), std::inserter(x_ints, x_ints.begin()));
		x_bounds.find_intervals(std::get<0>(info.second).sup(), std::inserter(x_ints, x_ints.begin()));
		y_bounds.find_intervals(std::get<1>(info.second).inf(), std::inserter(y_ints, y_ints.begin()));
		y_bounds.find_intervals(std::get<1>(info.second).sup(), std::inserter(y_ints, y_ints.begin()));
		z_bounds.find_intervals(std::get<2>(info.second).inf(), std::inserter(z_ints, z_ints.begin()));
		z_bounds.find_intervals(std::get<2>(info.second).sup(), std::inserter(z_ints, z_ints.begin()));
		std::for_each(x_bounds.begin(), x_bounds.end(), [](const bounds_interval & intv) {
			PRINT_LEVELS("[xint: %s to %s | %f to %f]\n", 
				intv.associated_stack()->boundaries.first->guid().c_str(), 
				intv.associated_stack()->boundaries.second->guid().c_str(),
				CGAL::to_double(intv.inf()),
				CGAL::to_double(intv.sup()));
		});
		std::for_each(y_bounds.begin(), y_bounds.end(), [](const bounds_interval & intv) {
			PRINT_LEVELS("[yint: %s to %s | %f to %f]\n", 
				intv.associated_stack()->boundaries.first->guid().c_str(), 
				intv.associated_stack()->boundaries.second->guid().c_str(),
				CGAL::to_double(intv.inf()),
				CGAL::to_double(intv.sup()));
		});
		std::for_each(z_bounds.begin(), z_bounds.end(), [](const bounds_interval & intv) {
			PRINT_LEVELS("[zint: %s to %s | %f to %f]\n", 
				intv.associated_stack()->boundaries.first->guid().c_str(), 
				intv.associated_stack()->boundaries.second->guid().c_str(),
				CGAL::to_double(intv.inf()),
				CGAL::to_double(intv.sup()));
		});
		blockstack * this_stack = info.first;
		for (auto p = x_ints.begin(); p != x_ints.end(); ++p) {
			const blockstack * other_stack = p->associated_stack();
			if (other_stack != this_stack) {
				for (auto q = y_ints.begin(); q != y_ints.end(); ++q) {
					if (q->associated_stack() == other_stack) {
						for (auto r = z_ints.begin(); r != z_ints.end(); ++r) {
							if (r->associated_stack() == other_stack) {
								if (blockstack::intersect(*this_stack, *other_stack)) {
									if (this_stack->get_level() == 2 && this_stack->height >= other_stack->height) {
										PRINT_LEVELS("[this stack is 3]\n");
										this_stack->set_to_3();
									}
									if (other_stack->get_level() == 2 && other_stack->height >= this_stack->height) {
										PRINT_LEVELS("[other stack is 3]\n");
										const_cast<blockstack *>(other_stack)->set_to_3();
									}
								}
							}
						}
					}
				}
			}
		}
		NOTIFY_MSG(".");
	});
}

}

namespace operations {

std::vector<std::shared_ptr<surface>> resolve_levels(const std::vector<std::shared_ptr<surface>> & surfaces) {
	PRINT_LEVELS("[entered resolve_levels]\n");

	std::shared_ptr<equality_context> context(new equality_context(g_opts.equality_tolerance));

	std::set<std::shared_ptr<surface>> halfstacks;
	std::vector<blockstack> stacks;

	for (auto s = surfaces.begin(); s != surfaces.end(); ++s) {

		if ((*s)->is_fenestration() || (*s)->is_virtual()) {
			if (FLAGGED(SBT_EXPENSIVE_CHECKS) && (*s)->opposite().expired()) {
				ERROR_MSG("Space boundary %s/%s is %s but has no opposite.\n",
					(*s)->guid().c_str(),
					(*s)->element_id().c_str(),
					(*s)->is_fenestration() ? "a fenestration" : "virtual");
				abort();
			}
			(*s)->set_level(2);
			continue;
		}

		if ((*s)->opposite().expired()) {
			(*s)->set_level(5);
			continue;
		}

		if (surface::share_space(*s, (*s)->opposite())) {
			(*s)->set_level(4);
			continue;
		}

		auto other = halfstacks.find((*s)->opposite().lock());
		if (other != halfstacks.end()) {
			stacks.push_back(blockstack(*s, *other, context));
		}
		else {
			halfstacks.insert(*s);
		}
	}

	if (~g_opts.flags & SBT_SKIP_3RD_LEVEL_CHECK) {
		identify_23s(stacks);
	}
	else {
		NOTIFY_MSG("3rd-level check skipped.\n");
	}

	return surfaces;
}

} // namespace operations