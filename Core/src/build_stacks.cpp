#include "precompiled.h"

#include "equality_context.h"
#include "printing-util.h"
#include "oriented_area.h"
#include "sbt-core.h"
#include "space.h"
#include "surface.h"

#define PRINT_STACKS(...) \
	do { \
		if (g_opts.flags & SBT_VERBOSE_STACKS) { \
			NOTIFY_MSG( __VA_ARGS__); \
		} \
	} \
	while (false);

extern sb_calculation_options g_opts;

namespace {

struct region;

struct space_face {
	area a;
	NT z;
	const orientation * dir;
	//direction_3 dir;
	bool sense;
	std::weak_ptr<space> sp;

	space_face(std::shared_ptr<space> s, const oriented_area & rep) : a(rep.area_2d()), z(rep.height()), dir(&rep.orientation()), sense(rep.sense()), sp(s) { 
		SBT_EXPENSIVE_ASSERT(a.is_valid(), "[Aborting - tried to create a space face with an invalid area.]\n");
	}

	space_face(std::shared_ptr<space> s) : sp(s) { }

	bool trim(const area & cut) {
		a -= cut;
		SBT_EXPENSIVE_ASSERT(a.is_valid(), "[Aborting - a space face trim resulted in an invalid area.]\n");
		return !a.is_empty();
	}
};

struct overlap {
	area a;
	std::weak_ptr<region> other_region;
	
	overlap(const area & a, std::shared_ptr<region> r) : a(a), other_region(r) { }
};

struct region {
	area a;
	NT z;
	std::weak_ptr<surface> backing_surface;
	std::weak_ptr<region> opposed;
	std::vector<overlap> overlaps;

	region(std::shared_ptr<surface> & s) : a(s->geometry().area_2d()), z(s->geometry().height()), backing_surface(s) { 
		SBT_ASSERT(!a.is_empty(), "[Aborting - created a stacking region with no area.]\n");
		SBT_EXPENSIVE_ASSERT(a.is_valid(), "[Aborting - created an invalid stacking region.]\n");
	}

	static void set_overlaps(std::shared_ptr<region> & a, std::shared_ptr<region> & b) {
		if (a != b && a->z == b->z) {
			if (a->backing_surface.lock()->geometry().sense() != b->backing_surface.lock()->geometry().sense()) {
				if (FLAGGED(SBT_VERBOSE_STACKS)) {
					NOTIFY_MSG("[intersecting regions]\n");
					a->a.print();
					NOTIFY_MSG("[and]\n");
					b->a.print();
				}
				area intr = a->a * b->a;
				if (!intr.is_empty()) {
					a->overlaps.push_back(overlap(intr, b));
					b->overlaps.push_back(overlap(intr, a));
				}
			}
		}
	}

	static void set_as_opposing(std::shared_ptr<region> a, std::shared_ptr<region> b) {
		a->opposed = b;
		b->opposed = a;
	}
};

struct stack {
private:
	space_face start;
	space_face end;
	area m_area;
	std::weak_ptr<region> m_curr_region;
	std::weak_ptr<surface> m_start_orig_surface;
	std::weak_ptr<surface> m_end_orig_surface;
	std::vector<NT> m_zs;
	std::vector<material_id_t> m_materials;

	std::shared_ptr<equality_context> c;

	stack(const stack & src, std::shared_ptr<region> r, const area & a)
		: start(src.start),
		end(src.end),
		m_area(a),
		m_curr_region(r),
		m_start_orig_surface(src.m_start_orig_surface.expired() ? r->backing_surface : src.m_start_orig_surface),
		m_end_orig_surface(r->backing_surface),
		m_zs(src.m_zs),
		m_materials(src.m_materials),
		c(src.c)
	{
		if (m_zs.size() != m_materials.size() + 1) {
			ERROR_MSG("[Aborting - a stack's material list and z list became unsynchronized (in constructor from region).\n");
			throw core_exception(SBT_ASSERTION_FAILED);
		} 
	}
	stack(const stack & src, const space_face & sface, const area & a)
		: start(src.start),
		end(sface),
		m_area(a),
		m_start_orig_surface(src.m_start_orig_surface),
		m_end_orig_surface(src.m_end_orig_surface),
		m_zs(src.m_zs),
		m_materials(src.m_materials),
		c(src.c)
	{
		if (m_zs.size() != m_materials.size() + 1) {
			ERROR_MSG("[Aborting - a stack's material list and z list became unsynchronized (in constructor from space face).\n");
			throw core_exception(SBT_ASSERTION_FAILED);
		}
		if (!m_curr_region.expired()) {
			m_end_orig_surface = m_curr_region.lock()->backing_surface;
		}
	}

	void add_material_at(material_id_t mat, NT z) {
		if (m_zs.size() != m_materials.size() + 1) {
			ERROR_MSG("[Aborting - a stack's material list and z list became unsynchronized (while adding material).\n");
			throw core_exception(SBT_ASSERTION_FAILED);
		}
		m_zs.push_back(c->snap_height(z));
		m_materials.push_back(mat);
	}

public:
	stack(const space_face & face, std::shared_ptr<equality_context> & c) 
		: start(face), 
		end(nullptr),
		m_area(face.a),
		c(c)
	{ 
		m_zs.push_back(c->snap_height(face.z)); 
	}

	NT curr_z() const { return m_zs.back(); }
	bool has_area() const { return !m_area.is_empty(); }
	bool needs_initial_region() const { return m_curr_region.expired(); }
	bool is_full_stack() const { return !(m_materials.empty() || m_end_orig_surface.expired()); } // theoretically only one of these checks should be sufficient...
	const area & a() const { return m_area; }
	std::weak_ptr<region> curr_region() const { return m_curr_region; }
	const std::vector<NT> & zs() const { return m_zs; }
	const std::vector<material_id_t> & materials() const { return m_materials; }
	std::weak_ptr<surface> start_orig_surface() const { return m_start_orig_surface; }
	std::weak_ptr<surface> end_orig_surface() const { return m_end_orig_surface; }
	const space_face & start_face() const { return start; }
	const space_face & end_face() const { return end; }
	std::weak_ptr<space> start_space() const { return start_face().sp; }
	std::weak_ptr<space> end_space() const { return end_face().sp; }
	bool sense() const { return start.sense; }

	bool traverse_to_opposing() {
		PRINT_STACKS("[traversing element %s ", m_curr_region.lock()->backing_surface.lock()->element_id().c_str());
		if (!m_curr_region.expired() && !m_curr_region.lock()->opposed.expired()) {
			m_curr_region = m_curr_region.lock()->opposed;
			m_end_orig_surface = m_curr_region.lock()->backing_surface;
			add_material_at(m_curr_region.lock()->backing_surface.lock()->surface_material(), m_curr_region.lock()->z);
			PRINT_STACKS("to %f]\n", CGAL::to_double(m_zs.back()));
			return true;
		}
		PRINT_STACKS("to nowhere! (%s)]\n", m_curr_region.expired() ? "nothing to traverse" : "no opposite");
		m_zs.resize(1);
		m_materials.clear();
		m_curr_region.reset();
		return false;
	}

	stack split_off(std::shared_ptr<region> r, const area & other_area) {
		m_area -= other_area;
		PRINT_STACKS("[preparing to create new stack after split]\n");
		return stack(*this, r, other_area);
	}

	stack finalize_with(const space_face & sface, const area & other_area) {
		m_area -= other_area;
		return stack(*this, sface, other_area);
	}

	void finalize_fully(std::shared_ptr<space> & s) {
		end = space_face(s);
	}

	void clear_area() { m_area.clear(); } // remove this
		
};

std::vector<space_face> get_faces_for(std::shared_ptr<space> & sp, equality_context * c) {
	if (FLAGGED(SBT_VERBOSE_STACKS)) { NOTIFY_MSG("\n"); }
	std::vector<space_face> results;
	boost::for_each(sp->get_faces(c), [&results, sp](const oriented_area & face) {
		if (!face.area_2d().is_empty()) {
			results.push_back(space_face(sp, face));
			if (!FLAGGED(SBT_VERBOSE_STACKS)) {
				NOTIFY_MSG(".");
			}
			else {
				face.print();
			}
		}
	});
	return results;
}

template <class RegionPairIter>
void set_all_opposites(RegionPairIter begin, RegionPairIter end) {
	for (auto p = begin; p != end; ++p) {
		if (p->second->opposed.expired() && !p->second->backing_surface.lock()->opposite().expired()) {
			for (auto q = p; q != end; ++q) {
				if (p->second->backing_surface.lock()->opposite().lock()->guid() == q->second->backing_surface.lock()->guid()) {
					p->second->opposed = q->second;
					q->second->opposed = p->second;
					break;
				}
			}
		}
	}
}

template <class RegionPairIter>
void set_region_overlaps(RegionPairIter begin, RegionPairIter end) {
	if (begin != end) {
		NOTIFY_MSG("  identifying overlaps");
		for (auto p = begin; p != end; ++p) {
			for (auto q = p; q != end; ++q) {
				region::set_overlaps(p->second, q->second);
			}
			NOTIFY_MSG(".");
		}
		NOTIFY_MSG("done\n");
	}
}

template <class SuI, class SpI>
std::vector<std::pair<std::vector<std::shared_ptr<surface>>, std::vector<space_face>>> organize_by_orientation(
	SuI surfaces_begin, 
	SuI surfaces_end, 
	SpI spaces_begin, 
	SpI spaces_end,
	std::shared_ptr<equality_context> & c) 
{
	std::vector<std::pair<std::vector<std::shared_ptr<surface>>, std::vector<space_face>>> results;
	
	for (auto p = spaces_begin; p != spaces_end; ++p) {
		if (!(*p)->is_outside_space()) {
			NOTIFY_MSG("Getting faces for space %s%s", (*p)->global_id().c_str(), (*p)->is_outside_space() ? " (outside space)" : "");
			std::shared_ptr<space> sp = *p;
			auto faces = get_faces_for(sp, c.get());
			NOTIFY_MSG("done\n");
			for (auto q = faces.begin(); q != faces.end(); ++q) {
				auto this_o = std::find_if(results.begin(), results.end(), [q](const std::pair<std::vector<std::shared_ptr<surface>>, std::vector<space_face>> & o) {
					return o.second.front().dir == q->dir;
				});
				if (this_o != results.end()) {
					this_o->second.push_back(*q);
				}
				else {
					results.push_back(std::make_pair(std::vector<std::shared_ptr<surface>>(), std::vector<space_face>()));
					results.back().second.push_back(*q);
				}
			}
		}
	}

	for (auto p = surfaces_begin; p != surfaces_end; ++p) {
		auto this_o = std::find_if(results.begin(), results.end(), [p, &c](const std::pair<std::vector<std::shared_ptr<surface>>, std::vector<space_face>> & o) {
			return o.second.front().dir == &(*p)->geometry().orientation();
		});
		if (this_o != results.end()) {
			if (!(*p)->geometry().area_2d().is_empty()) {
				this_o->first.push_back(std::shared_ptr<surface>(new surface(*p, c)));
			}
			else {
				ERROR_MSG("[Aborting - tried to match a surface with no area to a space face.]\n");
				throw core_exception(SBT_ASSERTION_FAILED);
			}
		}
		else {
			PRINT_STACKS("[dropping surface; not parallel to any space faces (z = %f)]\n", CGAL::to_double((*p)->geometry().height()));;
		}
		// if there's no match we assume it's not exposed
	}

	return results;
}

template <class StackOutIter>
void descend(stack * curr_stack, const std::multimap<NT, std::shared_ptr<region>> & regions, std::multimap<NT, space_face> * space_faces, std::shared_ptr<space> & outside_space, StackOutIter & oi) {
	auto sfaces_this_z = space_faces->equal_range(curr_stack->curr_z());
	if (FLAGGED(SBT_VERBOSE_STACKS)) {
		NOTIFY_MSG( "[descending from %f %c]\n", CGAL::to_double(curr_stack->curr_z()), curr_stack->sense() ? '+' : '-');
		NOTIFY_MSG( "[current area is:]\n");
		curr_stack->a().print_with(g_opts.notify_func);
		NOTIFY_MSG( "[%u space faces at this z]\n", std::distance(sfaces_this_z.first, sfaces_this_z.second));
		if (sfaces_this_z.first == sfaces_this_z.second) {
			NOTIFY_MSG( "[all the zs follow]\n");
			for (auto p = space_faces->begin(); p != space_faces->end(); ++p) {
				NOTIFY_MSG( "%f\n", CGAL::to_double(p->first));
			}
		}
	}
	for (auto p = sfaces_this_z.first; p != sfaces_this_z.second; ) {
		PRINT_STACKS("[checking face of space %s]\n", p->second.sp.lock()->global_id().c_str());
		space_face & face = p->second;
		if (curr_stack->sense() != face.sense) {
			IF_FLAGGED(SBT_VERBOSE_STACKS) {
				NOTIFY_MSG( "[intersecting face area]\n");
				face.a.print_with(g_opts.notify_func);
				NOTIFY_MSG( "[with current stack area]\n");
				curr_stack->a().print_with(g_opts.notify_func);
			}
			area intr = curr_stack->a() * face.a;
			if (!intr.is_empty()) {
				IF_FLAGGED(SBT_VERBOSE_STACKS) {
					NOTIFY_MSG( "[intersection was non-empty - here it is]\n");
					intr.print_with(g_opts.notify_func);
				}
				*oi++ = curr_stack->finalize_with(face, intr);
				PRINT_STACKS("[terminating]\n[subtracting intersection from face area]\n");
				face.a -= intr;
				PRINT_STACKS("[subtraction complete]\n");
				if (face.a.is_empty()) {
					PRINT_STACKS("[eliminating a face of %s (at %f)]\n", p->second.sp.lock()->global_id().c_str(), CGAL::to_double(p->second.z));
					auto q = p;
					++q;
					space_faces->erase(p);
					p = q;
				}
				else {
					IF_FLAGGED(SBT_VERBOSE_STACKS) {
						NOTIFY_MSG( "[remaining area follows]\n");
						face.a.print_with(g_opts.notify_func);
					}
					++p;
				}
			}
			else {
				PRINT_STACKS("[intersection was empty]\n");
				++p;
			}
		}
		else {
			++p;
		}
	}

	// necessary here?
	if (!curr_stack->has_area()) {
		return;
	}

	if (curr_stack->needs_initial_region()) {
		PRINT_STACKS("[searching for initial region]\n");
		auto regions_this_z = regions.equal_range(curr_stack->curr_z());
		PRINT_STACKS("[found %u regions at this z]\n", std::distance(regions_this_z.first, regions_this_z.second));
		if (regions_this_z.first == regions_this_z.second) {
			PRINT_STACKS("[all zs follow]\n");
			for (auto p = regions.begin(); p != regions.end(); ++p) {
				PRINT_STACKS("%f\n", CGAL::to_double(p->first));
			}
		}
		for (auto p = regions_this_z.first; p != regions_this_z.second; ++p ) {
			std::shared_ptr<region> r = p->second;
			SBT_EXPENSIVE_ASSERT(r->a.is_valid(), "[Aborting - tried to intersect with an invalid region.]\n");
			area intr = curr_stack->a() * r->a;
			if (FLAGGED(SBT_VERBOSE_STACKS)) {
				NOTIFY_MSG( "[intersecting current area]\n");
				curr_stack->a().print_with(g_opts.notify_func);
				NOTIFY_MSG( "[with region area]\n");
				r->a.print_with(g_opts.notify_func);
				NOTIFY_MSG( "[to get]\n");
				intr.print_with(g_opts.notify_func);
			}
			if (!intr.is_empty()) {
				stack other = curr_stack->split_off(r, intr);
				PRINT_STACKS("[finished stack splitting]\n");
				if (other.traverse_to_opposing()) {
					descend(&other, regions, space_faces, outside_space, oi);
					PRINT_STACKS("[descend call returned]\n");
				}
				else {
					PRINT_STACKS("[terminating - 5th level]\n");
					*oi++ = other;
				}
			}
		}
		if (curr_stack->has_area()) {
			return; // this happens if we're doing fenestration stacking and there's no fenestration to stack
		}
	}

	else {
		std::shared_ptr<region> curr_region = curr_stack->curr_region().lock();
		IF_FLAGGED(SBT_VERBOSE_STACKS) {
			NOTIFY_MSG(  "[evaluating %u overlaps; current area follows]\n", curr_region->overlaps.size());
			curr_stack->a().print_with(g_opts.notify_func);
		}
		for (auto p = curr_region->overlaps.begin(); p != curr_region->overlaps.end(); ++p) {
			IF_FLAGGED(SBT_VERBOSE_STACKS) {
				NOTIFY_MSG( "[overlap:]\n");
				p->a.print_with(g_opts.notify_func);
			}
			area intr_area = p->a * curr_stack->a();
			if (!intr_area.is_empty()) {
				PRINT_STACKS("[continuing]\n");
				stack other = curr_stack->split_off(p->other_region.lock(), intr_area);
				if (other.traverse_to_opposing()) {
					descend(&other, regions, space_faces, outside_space, oi);
					PRINT_STACKS("[descend call returned]\n");
				}
				else {
					PRINT_STACKS("[terminating - 5th level]\n");
					*oi++ = other;
				}
			}
			else {
				PRINT_STACKS("[no intersection with current stack area - not continuing]\n");
			}
		}

		// area, no space faces and no regions means we're on the building facade (i.e. we're done)
		if (!curr_stack->a().is_empty()) {
			curr_stack->finalize_fully(outside_space);
			*oi++ = *curr_stack;
		}
	}
}

template <class StackInputIter, class SurfOutputIter>
void stacks_to_surfaces(StackInputIter begin, StackInputIter end, std::shared_ptr<equality_context> & c, SurfOutputIter & oi) {
	if (begin != end) {
		NOTIFY_MSG("  converting stacks to surfaces");

		std::for_each(begin, end, [&oi, &c](const stack & s) {

			SBT_ASSERT(!s.a().is_empty(), "[Aborting - tried to make surfaces out of a stack with no area.]\n");
		
			std::vector<std::pair<material_id_t, NT>> layers;
			if (s.is_full_stack()) {
				std::vector<NT> thicknesses;
				std::adjacent_difference(s.zs().begin(), s.zs().end(), std::back_inserter(thicknesses), [](const NT & a, const NT & b) {
					return CGAL::abs(a - b);
				});
				for (size_t i = 1; i < thicknesses.size(); ++i) {
					layers.push_back(std::make_pair(s.materials()[i - 1], thicknesses[i]));
				}
			}

			std::vector<polygon_2> polys;
			PRINT_STACKS("[stack\n  z = %f to %f\n  space %s to %s\n  element %s to %s]\n", 
				CGAL::to_double(s.zs().front()),
				CGAL::to_double(s.zs().back()),
				s.start_space().lock()->global_id().c_str(),
				s.is_full_stack() ? s.end_space().lock()->global_id().c_str() : "nowhere",
				s.start_orig_surface().expired() ? "no element" : s.start_orig_surface().lock()->element_id().c_str(),
				s.end_orig_surface().expired() ? "no element" : s.end_orig_surface().lock()->element_id().c_str());
			PRINT_STACKS("[start surface sense:%c end surface sense:%c]\n",
				s.start_orig_surface().expired() ? '!' : s.start_orig_surface().lock()->geometry().sense() ? '+' : '-',
				s.end_orig_surface().expired() ? '!' : s.end_orig_surface().lock()->geometry().sense() ? '+' : '-');
			s.a().to_simple_polygons(std::back_inserter(polys));
			PRINT_STACKS("[got polygons]\n");

			for (auto p = polys.begin(); p != polys.end(); ++p) {
				if (s.start_orig_surface().expired()) {
					// it's virtual
					std::shared_ptr<surface> surf1(new surface(oriented_area(s.start_face().dir, s.start_face().z, *p, s.end_face().sense, c), s.start_space()));
					std::shared_ptr<surface> surf2(new surface(oriented_area(s.end_face().dir, s.end_face().z, *p, s.start_face().sense, c), s.end_space()));
					surface::set_other_sides(surf1, surf2);
					*oi++ = surf1;
					*oi++ = surf2;
				}
				else {
					std::shared_ptr<surface> surf1(new surface(s.start_orig_surface().lock(), *p, s.start_space(), layers.begin(), layers.end()));
					if (s.is_full_stack()) {
						std::shared_ptr<surface> surf2(new surface(s.end_orig_surface().lock(), *p, s.end_space(), layers.rbegin(), layers.rend()));
						surface::set_other_sides(surf1, surf2);
						*oi++ = surf1;
						*oi++ = surf2;
					}
					else {
						if (FLAGGED(SBT_EXPENSIVE_CHECKS) && surf1->is_fenestration()) {
							ERROR_MSG("Surface %s is a fenestration (%s) but bases a halfstack.\n",
								surf1->guid().c_str(),
								surf1->element_id().c_str());
							abort();
						}
						surf1->set_level(5);
						*oi++ = surf1;
					}
				}
			}

			NOTIFY_MSG(".");
		});

		NOTIFY_MSG("done\n");
	}
}

template <class SurfIn, class SFaceIn, class OutputIterator>
void build_stacks_for_group(SurfIn surfs_begin, SurfIn surfs_end, SFaceIn sfaces_begin, SFaceIn sfaces_end, std::shared_ptr<equality_context> & c, std::shared_ptr<space> & outside_space, OutputIterator & oi) {
	std::multimap<NT, std::shared_ptr<region>> regions;
	std::transform(surfs_begin, surfs_end, std::inserter(regions, regions.begin()), [&c](std::shared_ptr<surface> & s) {
		return std::make_pair(c->snap_height(s->geometry().height()), std::shared_ptr<region>(new region(s)));
	});

	set_all_opposites(regions.begin(), regions.end());
	PRINT_STACKS("[opposites set]\n");

	std::multimap<NT, space_face> space_faces;
	std::transform(sfaces_begin, sfaces_end, std::inserter(space_faces, space_faces.begin()), [surfs_begin, &c](const space_face & face) {
		return std::make_pair(c->snap_height(face.z), face);
	});
	
	set_region_overlaps(regions.begin(), regions.end());
	PRINT_STACKS("[region overlaps set]\n");

	std::vector<stack> stacks;

	NOTIFY_MSG("  building stacks");
	for (auto p = space_faces.begin(); p != space_faces.end(); ++p) {
		if (util::misc::space_passes_filter(p->second.sp.lock()->global_id(), g_opts)) {
			stack curr_stack(p->second, c);
			PRINT_STACKS("[beginning descent from space %s]\n", p->second.sp.lock()->global_id().c_str());
			descend(&curr_stack, regions, &space_faces, outside_space, std::back_inserter(stacks));
			NOTIFY_MSG(".");
		}
	}
	NOTIFY_MSG("done\n");

	stacks_to_surfaces(stacks.begin(), stacks.end(), c, oi);
}

template <class OutputIterator>
void build_stacks_for_orientation(
	const std::pair<std::vector<std::shared_ptr<surface>>, 
	std::vector<space_face>> & info, 
	std::shared_ptr<equality_context> & c, 
	std::shared_ptr<space> & outside_space, 
	OutputIterator oi) 
{

	std::vector<std::shared_ptr<surface>> fenestrations;
	std::vector<std::shared_ptr<surface>> nonfenestrations;
	for (auto p = info.first.begin(); p != info.first.end(); ++p) {
		((*p)->is_fenestration() ? fenestrations : nonfenestrations).push_back(*p);
	}

	if (!fenestrations.empty()) {
		NOTIFY_MSG("Building fenestration stacks for ");
		util::printing::print_dir(g_opts.notify_func, info.second.front().dir->direction());
		NOTIFY_MSG("\n");
		build_stacks_for_group(fenestrations.begin(), fenestrations.end(), info.second.begin(), info.second.end(), c, outside_space, oi);
	}
	
	// we don't check for nonfenestration emptiness because there could be only virtuals (and they would get incorrectly skipped in that case)
	NOTIFY_MSG("Building nonfenestration stacks for ");
	util::printing::print_dir(g_opts.notify_func, info.second.front().dir->direction());
	NOTIFY_MSG("\n");
	build_stacks_for_group(nonfenestrations.begin(), nonfenestrations.end(), info.second.begin(), info.second.end(), c, outside_space, oi);

}

} // namespace

namespace operations {

std::vector<std::shared_ptr<surface>> build_stacks(const std::vector<std::shared_ptr<surface>> & surfaces, const std::vector<std::shared_ptr<space>> & spaces, std::shared_ptr<equality_context> & c) {
	std::vector<std::shared_ptr<surface>> results;

	std::shared_ptr<space> outside_space = *std::find_if(spaces.begin(), spaces.end(), [](const std::shared_ptr<space> & s) { return s->is_outside_space(); });

	auto by_orientation = organize_by_orientation(surfaces.begin(), surfaces.end(), spaces.begin(), spaces.end(), /*stacks_main_context*/ c);

	std::for_each(by_orientation.begin(), by_orientation.end(), [&results, &outside_space, &c](std::pair<std::vector<std::shared_ptr<surface>>, std::vector<space_face>> & info) {
		build_stacks_for_orientation(info, c, outside_space, std::back_inserter(results));
	});

	IF_FLAGGED(SBT_VERBOSE_STACKS) {
		for (auto p = results.begin(); p != results.end(); ++p) {
			if ((*p)->get_space().expired()) {
				ERROR_MSG("[Aborting - a stacked surface doesn't have a face (element %s, z = %f).]\n",
					(*p)->element_id().c_str(),
					CGAL::to_double((*p)->geometry().height()));
				throw core_exception(SBT_ASSERTION_FAILED);
			}
			NOTIFY_MSG( 
				"stacked surface %s:\n  space: %s\n  opposes: %s\n  element: %s\n  normal: %s%c\n  p: %f\n",
				(*p)->guid().c_str(),
				(*p)->get_space().lock()->global_id().c_str(),
				(!(*p)->opposite().expired() ? (*p)->opposite().lock()->guid().c_str() : "[none]"),
				(*p)->element_id().c_str(),
				(*p)->geometry().orientation().to_string().c_str(),
				(*p)->geometry().sense() ? '+' : '-',
				CGAL::to_double((*p)->geometry().height()));
			(*p)->geometry().area_2d().print_with(g_opts.notify_func);
		}
	}

	return results;
}

} // namespace operations