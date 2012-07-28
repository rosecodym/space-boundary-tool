#include "precompiled.h"

#include "misc-util.h"
#include "oriented_area.h"
#include "sbt-core.h"

#include "surface.h"

extern sb_calculation_options g_opts;

class equality_context;

surface::surface(const oriented_area & g, const element & e)
	: global_id(util::misc::new_guid_as_string()), m_geometry(g), lvl(0), e(&e), bounded_space(nullptr) 
{ }

surface::surface(oriented_area && g, const element & e)
	: global_id(util::misc::new_guid_as_string()), m_geometry(std::move(g)), lvl(0), e(&e), bounded_space(nullptr) 
{ }

surface::surface(std::shared_ptr<surface> s, std::shared_ptr<equality_context> new_c)
	: global_id(s->global_id), 
	m_geometry(s->m_geometry, new_c), 
	lvl(s->lvl), 
	material_layers(s->material_layers),
	e(s->e),
	other_side(s->other_side),
	bounded_space(s->bounded_space),
	parent(s->parent)
{ }

// for virtuals
surface::surface(const oriented_area & g, const space * bounded_space)
	: global_id(util::misc::new_guid_as_string()),
	m_geometry(g),
	lvl(2),
	e(nullptr),
	bounded_space(bounded_space)
{ }