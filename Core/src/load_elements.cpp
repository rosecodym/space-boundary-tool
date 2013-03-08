#include "precompiled.h"

#include "element.h"
#include "exceptions.h"
#include "guid_filter.h"
#include "report.h"

#include "load_elements.h"

using namespace reporting;

std::vector<element> load_elements(
	element_info ** infos, 
	size_t count, 
	equality_context * c, 
	const guid_filter & filter) 
{
	typedef boost::format fmt;

	report_progress(fmt("Beginning loading for %u elements.\n") % count);

	std::vector<element> complex_elements;
	for (size_t i = 0; i < count; ++i) {
		try {
			if (filter(infos[i]->name)) {
				complex_elements.push_back(element(infos[i], c));
			}
		}
		catch (unsupported_geometry_exception & ex) {
			report_warning(fmt(
				"Element %s has unsupported geometry (%s). It will be "
				"ignored.\n") 
				% infos[i]->name % ex.condition());
		}
		catch (unknown_geometry_rep_exception & /*ex*/) {
			report_warning(fmt(
				"Element %s has an unknown internal geometry representation "
				"type. It will be ignored. Please report this SBT bug.\n") 
				% infos[i]->name);
		}
		catch (bad_geometry_exception & ex) {
			report_warning(fmt(
				"Element %s has bad geometry (%s). It will be ignored.\n")
				% infos[i]->name % ex.condition());
		}
	}

	typedef std::vector<element>::iterator element_iterator;
	using CGAL::Box_intersection_d::Box_with_handle_d;
	typedef Box_with_handle_d<double, 3, element_iterator> element_box;

	std::vector<element_box> walls;
	std::vector<element_box> slabs;
	std::vector<element_box> columns;

	for (auto e = complex_elements.begin(); e != complex_elements.end(); ++e) {
		element_box box(e->bounding_box(), e);
		if (e->type() == WALL) { walls.push_back(box); }
		else if (e->type() == SLAB) { slabs.push_back(box); }
		else if (e->type() == COLUMN) { columns.push_back(box); }
	}

	report_progress(fmt(
		"Got bounding boxes (%u walls, %u slabs, %u columns).\n") 
		% walls.size() % slabs.size() % columns.size());

	bool need_wall_column_check = !walls.empty() && !columns.empty();
	bool need_slab_column_check = !slabs.empty() && !columns.empty();

	if (need_wall_column_check) {
		for (auto w = walls.begin(); w != walls.end(); ++w) {

			report_progress(fmt("Resolving wall %s") % w->handle()->name());
			bool performed_any_subtractions = false;

			if (need_wall_column_check) {
				boost::for_each(columns, [&, w, c](const element_box & col) {
					// The share_plane_opposite check will yield false 
					// negatives in extremely pathological cases (if two 
					// objects share an opposite face but intersect elsewhere) 
					// but I'm not worried about that.
					if (CGAL::do_overlap(w->bbox(), col.bbox()) &&
						!element::share_plane_opposite(
							*w->handle(), 
							*col.handle(),
							c))
					{
						w->handle()->subtract_geometry_of(*col.handle(), c);
						report_progress(".");
						performed_any_subtractions = true;
					}
				});
			}

			if (performed_any_subtractions) {
				report_progress("done.\n");
			}
			else {
				report_progress(" - no resolution necessary.\n");
			}
		}
	}

	if (need_slab_column_check) {
		for (auto col = columns.begin(); col != columns.end(); ++col) {

			report_progress(fmt(
				"Resolving column %s") % col->handle()->name());
			bool performed_any_subtractions = false;

			boost::for_each(slabs, [&, col, c](const element_box & s) {
				// The share_plane_opposite check will yield false negatives in
				// extremely pathological cases (if two objects share an
				// opposite face but intersect elsewhere) but I'm not worried
				// about that.
				if (CGAL::do_overlap(
						col->bbox(), 
						s.handle()->bounding_box()) &&
					!element::share_plane_opposite(
						*col->handle(),
						*s.handle(),
						c))
				{
					col->handle()->subtract_geometry_of(*s.handle(), c);
					report_progress(".");
					performed_any_subtractions = true;
				}
			});

			if (performed_any_subtractions) {
				report_progress("done.\n");
			}
			else {
				report_progress(" - no resolution necessary.\n");
			}
		}
	}

	std::vector<element> res;
	for (auto e = complex_elements.begin(); e != complex_elements.end(); ++e) {
		element::explode_to_single_volumes(
			std::move(*e), 
			c, 
			std::back_inserter(res));
	}

	return res;
}