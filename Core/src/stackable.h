#pragma once

#include "precompiled.h"

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
public:
	stackable() { }
	explicit stackable(space_face * f) : m_data(f) { }
	explicit stackable(const block * b) : m_data(b) { }
	area stackable_area() const;
	double thickness() const;
	boost::optional<space_face *> as_space_face() const;
	const block * as_block() const;
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