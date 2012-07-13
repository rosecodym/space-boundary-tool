#include "precompiled.h"

#include "operations.h"
#include "printing-util.h"
#include "sbt-core.h"
#include "surface.h"

#define PRINT_FEN(...) \
	do { \
		if (g_opts.flags & SBT_VERBOSE_FENESTRATIONS) { \
			NOTIFY_MSG( __VA_ARGS__); \
		} \
	} \
	while (false);

extern sb_calculation_options g_opts;

namespace operations {

namespace {

const double OPENINGS_HEIGHT_TOLERANCE_ADJUSTMENT_FACTOR = 2; // kludge

// i *believe* that upstream flattening makes this approximate check necessary
// theoretically, removing that will make this irrelevant, but until then, here we are
bool contains_approximately(surface * container, surface * fenestration) {
	return
		&container->geometry().orientation() == &fenestration->geometry().orientation() &&
		equality_context::are_equal(container->geometry().height(), fenestration->geometry().height(), g_opts.equality_tolerance * OPENINGS_HEIGHT_TOLERANCE_ADJUSTMENT_FACTOR) &&
		container->geometry().area_2d() >= fenestration->geometry().area_2d();
}

} // namespace

std::vector<std::shared_ptr<surface>> assign_openings(const std::vector<std::shared_ptr<surface>> & surfaces) {

	std::vector<std::shared_ptr<surface>> results;

	for (auto p = surfaces.begin(); p != surfaces.end(); ++p) {
		if ((*p)->is_fenestration()) {
			for (auto q = surfaces.begin(); q != surfaces.end(); ++q) {
				if (q != p && 
					!(*q)->is_fenestration() && 
					contains_approximately(q->get(), p->get()))
				{
					surface::set_contains(*q, *p);
					results.push_back(*p);
					PRINT_FEN("[assigned %s to %s @ %f]\n[%s]\n", (*p)->guid().c_str(), (*q)->guid().c_str(), CGAL::to_double((*p)->geometry().height()), (*p)->guid().c_str());
					PRINT_FEN("[%s]\n", (*q)->guid().c_str());
					break;
				}
				else if (g_opts.flags & SBT_VERBOSE_FENESTRATIONS) {
					if (q == p) { NOTIFY_MSG( "[assignment failed - self]\n"); }
					else if ((*q)->is_fenestration()) { NOTIFY_MSG( "[assignment failed - host candidate is a fenestration]\n"); }
					else if ((*q)->geometry().height() != (*p)->geometry().height()) { NOTIFY_MSG( "[assignment failed height %f doesn't match height %f]\n", CGAL::to_double((*q)->geometry().height()), CGAL::to_double((*p)->geometry().height())); }
					else if ((*q)->geometry().orientation() != (*p)->geometry().orientation()) { NOTIFY_MSG( "[assignment failed - nonparallel]\n"); }
					else if (!((*q)->geometry().area_2d() >= (*p)->geometry().area_2d())) { 
						NOTIFY_MSG( "[assignment failed - area mismatch follows (>= returned %s)]\n", (*q)->geometry().area_2d() >= (*p)->geometry().area_2d() ? "true" : "false");
					}
					else { 
						NOTIFY_MSG( "[assignment failed - unknown!]\n"); 
					}
				}
			}
			if (FLAGGED(SBT_EXPENSIVE_CHECKS) && (*p)->containing_boundary().expired()) {
				ERROR_MSG("Fenestration surface %s/%s/%s (%s%c @ %f) could not be assigned a containing surface.\n",
					(*p)->guid().c_str(),
					(*p)->element_id().c_str(),
					(*p)->get_space() == nullptr ? "[no space]" : (*p)->get_space()->global_id().c_str(),
					(*p)->geometry().orientation().to_string().c_str(),
					(*p)->geometry().sense() ? '+' : '-',
					CGAL::to_double((*p)->geometry().height()));
				ERROR_MSG("Surface area:\n");
				(*p)->geometry().area_2d().print();
				if ((*p)->opposite().expired()) {
					ERROR_MSG("Surface has no opposite!\n");
				}
				else {
					const surface & opp = *(*p)->opposite().lock();
					ERROR_MSG("Opposite is %s/%s/%s (%s%c @ %f).\n",
						opp.guid().c_str(),
						opp.element_id().c_str(),
						opp.get_space() == nullptr ? "[no space]" : opp.get_space()->global_id().c_str(),
						opp.geometry().orientation().to_string().c_str(),
						opp.geometry().sense() ? '+' : '-',
						CGAL::to_double(opp.geometry().height()));
				}
				abort();
			}
			NOTIFY_MSG(".");
		}
		else {
			results.push_back(*p);
		}
	}

	return results;

} // namespace operations

}