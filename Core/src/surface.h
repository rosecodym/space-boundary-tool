#pragma once

#include "precompiled.h"

#include "CreateGuid_64.h"
#include "element.h"
#include "oriented_area.h"

class space;

class surface {
private:
	std::string m_guid;
	oriented_area m_geometry;
	const element * m_element; // virtual space boundaries don't have elements
	const space & m_space;
	const surface * m_other_side;
	const surface * m_parent;
	bool m_external;

	static std::string new_guid_as_string() {
		char buf[24] = "ERROR CREATING GUID";
		CreateCompressedGuidString(buf, 23);
		return std::string(buf);
	}

public:
	surface(const oriented_area & geometry, const element & e, const space & bounded_space, bool external)
		: m_guid(new_guid_as_string()), m_geometry(geometry), m_element(&e), m_space(bounded_space), m_other_side(nullptr), m_parent(nullptr), m_external(external) { }
	surface(oriented_area && geometry, const element & e, const space & bounded_space, bool external)
		: m_guid(new_guid_as_string()), m_geometry(std::move(geometry)), m_element(&e), m_space(bounded_space), m_other_side(nullptr), m_parent(nullptr), m_external(external) { }
	// for virtuals
	surface(const oriented_area & geometry, const space & bounded_space)
		: m_guid(new_guid_as_string()), m_geometry(geometry), m_element(nullptr), m_space(bounded_space), m_other_side(nullptr), m_parent(nullptr), m_external(false) { }
	surface(oriented_area && geometry, const space & bounded_space)
		: m_guid(new_guid_as_string()), m_geometry(geometry), m_element(nullptr), m_space(bounded_space), m_other_side(nullptr), m_parent(nullptr), m_external(false) { }

	const std::string & global_id() const { return m_guid; }
	const oriented_area & geometry() const { return m_geometry; }
	const space & bounded_space() const { return m_space; }

	bool is_virtual() const { return m_element != nullptr; }
	bool is_fenestration() const { return !is_virtual() && m_element->is_fenestration(); }
	bool is_external() const { return m_external; }
	bool has_other_side() const { return m_other_side != nullptr; }
	bool shares_space_with_other_side() const { return has_other_side() && &m_space == &m_other_side->m_space; }

	void set_parent(surface * parent) { m_parent = parent; }

	static void set_other_sides(surface * a, surface * b) {
		a->m_other_side = b;
		b->m_other_side = a;
	}
};