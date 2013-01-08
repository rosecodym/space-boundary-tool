#pragma once

#include "precompiled.h"

#include "area.h"
#include "block.h"
#include "equality_context.h"
#include "space_face.h"

namespace stacking {

namespace impl {

struct stackable {
public:
	typedef boost::variant<space_face *, const block *> data_t;
private:
	data_t m_data;
	area local_area;
public:
	stackable() { }
	explicit stackable(space_face * f) : m_data(f), local_area(f->face_area()) { }
	explicit stackable(const block * b) : m_data(b), local_area(b->base_area()) { }
	area stackable_area() const { return local_area; }
	double thickness() const;
	boost::optional<space_face *> as_space_face() const;
	const block * as_block() const { return boost::get<const block *>(m_data); }
	std::string identifier() const;
	const data_t & data() const { return m_data; }
};

struct stackable_connection {
	double connection_height;
	area connection_area; // this isn't actually used right now
	stackable_connection() { }
	stackable_connection(double height, area && a) : connection_height(height), connection_area(std::move(a)) { }
	static boost::optional<stackable_connection> do_connect(stackable a, stackable b, double height_eps);
};

} // namespace impl

} // namespace stacking