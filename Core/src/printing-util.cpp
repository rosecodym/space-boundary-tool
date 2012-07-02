#include "precompiled.h"

#include "printing-util.h"

namespace util {

namespace printing {

void print_nef_polygon(void (*msg_func)(char *), const nef_polygon_2 & nef) {
	nef_polygon_2::Explorer e = nef.explorer();
	for (auto f = e.faces_begin(); f != e.faces_end(); ++f) {
		if (f->mark()) {
			msg_func("[face]\n");
			auto p = e.face_cycle(f);
			auto end = p;
			CGAL_For_all(p, end) {
				if (e.is_standard(p->vertex())) {
					char buf[256];
					sprintf(buf, "%f\t%f\n", CGAL::to_double(p->vertex()->point().x()), CGAL::to_double(p->vertex()->point().y()));
					msg_func(buf);
				}
				else {
					msg_func("[non-standard point]\n");
				}
			}
			for (auto h = e.holes_begin(f); h != e.holes_end(f); ++h) {
				msg_func("[hole]\n");
				nef_polygon_2::Explorer::Halfedge_around_face_const_circulator curr = h;
				auto end = curr;
				CGAL_For_all(curr, end) {
					if (e.is_standard(curr->vertex())) {
						char buf[256];
						sprintf(buf, "%f\t%f\n", CGAL::to_double(curr->vertex()->point().x()), CGAL::to_double(curr->vertex()->point().y()));
						msg_func(buf);
					}
					else {
						msg_func("[non-standard point]\n");
					}
				}
			}
		}
	}
}

} // namespace printing

} // namespace util