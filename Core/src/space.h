#pragma once

#include "precompiled.h"

#include "misc-util.h"
#include "multiview_solid.h"

class equality_context;
struct space_info;

class space {
private:
	std::string guid;
	boost::optional<multiview_solid> m_geometry;
	space_info * m_original_info;
public:
	space(space_info * s, equality_context * c) : guid(s->id), m_geometry(multiview_solid(s->geometry, c)), m_original_info(s) { }

	std::vector<oriented_area>	get_faces(equality_context * c) const { return m_geometry->oriented_faces(c); }
	const std::string &			global_id() const { return guid; }
	space_info *				original_info() const { return m_original_info; }

	void subtract_geometry(const multiview_solid & g, equality_context * c) { 
		if (CGAL::do_overlap(g.bounding_box(), m_geometry->bounding_box())) {
			m_geometry->subtract(g, c);
		}
	}
};