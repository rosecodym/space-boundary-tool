#pragma once

#include "precompiled.h"

#if 0

#include "element.h"
#include "misc-util.h"
#include "oriented_area.h"
#include "sbt-core.h"
#include "space.h"

extern sb_calculation_options g_opts;

class surface {

private:

	std::string global_id;
	oriented_area m_geometry;

	int lvl;

	std::vector<std::pair<material_id_t, NT>> material_layers;

	const element * e;
	std::weak_ptr<surface> other_side;
	const space * bounded_space;
	std::weak_ptr<surface> parent;

	surface(const surface & src);
	surface & operator = (const surface & src);

public:
	surface(const oriented_area & geometry, const element & e);
	surface(oriented_area && geometry, const element & e);
	surface(std::shared_ptr<surface> s, std::shared_ptr<equality_context> new_c);
	surface(const oriented_area & geometry, const space * bounded_space); // for virtuals

	template <class LayerIterator>
	surface(std::shared_ptr<surface> src, const polygon_2 & new_area, std::weak_ptr<space> bounded_space, LayerIterator layers_begin, LayerIterator layers_end)
		: global_id(util::misc::new_guid_as_string()),
		m_geometry(src->geometry().reverse(), area(new_area)),
		lvl(0),
		material_layers(layers_begin, layers_end),
		e(src->e),
		other_side(src->other_side),
		bounded_space(bounded_space),
		parent(src->parent)
	{ }

	const std::string &									guid() const { return global_id; }
	const oriented_area &								geometry() const { return m_geometry; }
	std::string											element_id() const { return is_virtual() ? "" : e->source_id(); }
	material_id_t										surface_material() const { return e->material(); }
	std::weak_ptr<surface>								opposite() const { return other_side; }
	std::weak_ptr<surface>								containing_boundary() const { return parent; }
	const std::vector<std::pair<material_id_t, NT>> &	materials() const { return material_layers; }
	const space *										get_space() const { return bounded_space; }
	bool												lies_on_outside() const { return bounded_space == nullptr || get_space()->is_outside_space(); }
	int													get_level() const { return lvl; }
	bool												is_virtual() const { return e == nullptr; }
	bool												is_fenestration() const { return !is_virtual() && e->is_fenestration(); }

	void												set_level(int l) { lvl = l; }
	void												set_space(const space * s) { bounded_space = s; }

	static void set_contains(std::shared_ptr<surface> parent, std::shared_ptr<surface> child) {
		child->parent = std::weak_ptr<surface>(parent);
		child->set_space(parent->get_space());
	}

	static bool share_space(std::weak_ptr<surface> a, std::weak_ptr<surface> b) {
		return a.lock()->bounded_space == b.lock()->bounded_space;
	}

	static bool are_parallel(std::weak_ptr<surface> a, std::weak_ptr<surface> b) {
		return oriented_area::are_parallel(a.lock()->m_geometry, b.lock()->m_geometry);
	}

	static bool oppose(std::weak_ptr<surface> a, std::weak_ptr<surface> b) {
		if (a.lock()->other_side.lock().get() == b.lock().get()) {
			return true;
		}
		return false;
	}

	static void set_other_sides(std::shared_ptr<surface> a, std::shared_ptr<surface> b) {
		a->other_side = std::weak_ptr<surface>(b);
		b->other_side = std::weak_ptr<surface>(a);
	}

	// DEPRECATED
	surface(const oriented_area & g, std::shared_ptr<element> & e)
		: global_id(util::misc::new_guid_as_string()), m_geometry(g), lvl(0), e(e.get()) { }

};

#endif