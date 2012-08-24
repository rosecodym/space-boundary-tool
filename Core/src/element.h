#pragma once

#include "precompiled.h"

#include "equality_context.h"
#include "multiview_solid.h"
#include "oriented_area.h"
#include "printing-macros.h"
#include "sbt-core.h"

class element {
private:
	std::string source_guid;
	multiview_solid m_geometry;
	element_type m_type;
	material_id_t m_material;

	element(const element & info, multiview_solid && new_geometry) 
		: source_guid(info.source_guid), 
		m_geometry(std::move(new_geometry)),
		m_type(info.m_type),
		m_material(info.m_material)
	{ }

public:
	element(const element_info * e, equality_context * c)
		: source_guid(e->id),
		m_geometry(e->geometry, c),
		m_type(e->type),
		m_material(e->material)
	{ }

	element(element && src)
		: source_guid(src.source_guid),
		m_geometry(std::move(src.m_geometry)),
		m_type(src.m_type),
		m_material(src.m_material)
	{ }

	element & operator = (element && src) {
		if (&src != this) {
			source_guid = std::move(src.source_guid);
			m_geometry = std::move(src.m_geometry);
			m_type = src.m_type;
			m_material = src.m_material;
		}
		return *this;
	}

	const std::string &			source_id() const { return source_guid; }
	element_type				type() const { return m_type; }
	material_id_t				material() const { return m_material; }
	bool						is_fenestration() const { return m_type == DOOR || m_type == WINDOW; }
	const multiview_solid &		geometry() const { return m_geometry; }
	std::vector<oriented_area>	faces(equality_context * c) const { return m_geometry.oriented_faces(c); }
	bbox_3						bounding_box() const { return m_geometry.bounding_box(); }

	void subtract_geometry_of(const element & other, equality_context * c) {
		PRINT_ELEMENTS("Subtracting element %s from element %s.\n", other.source_id().c_str(), this->source_id().c_str());
		m_geometry.subtract(other.m_geometry, c); 
		PRINT_ELEMENTS("Element subtraction completed.\n");
	}

	template <typename OutputIterator>
	static void explode_to_single_volumes(element && src, equality_context * c, OutputIterator oi) {
		NOTIFY_MSG("Checking element %s for multiple volumes: ", src.source_id().c_str());
		if (src.m_geometry.is_single_volume()) {
			NOTIFY_MSG("element is a single volume.\n");
			*oi++ = std::move(src);
		}
		else {
			NOTIFY_MSG("element is multiple volumes. Converting to single volumes");
			auto single_volumes = src.m_geometry.as_single_volumes(c);
			for (auto v = single_volumes.begin(); v != single_volumes.end(); ++v) {
				*oi++ = element(src, std::move(*v));
				NOTIFY_MSG(".");
			}
			NOTIFY_MSG("done.\n");
		}
	}
};
