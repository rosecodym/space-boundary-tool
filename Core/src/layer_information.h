#pragma once

#include "precompiled.h"

#include "element.h"

class layer_information {
private:
	NT m_height_a;
	boost::optional<NT> m_height_b;
	const element * e;

public:
	layer_information(const NT & height_a, const element & e) 
		: m_height_a(height_a), e(&e) { }
	layer_information(const NT & height_a, const NT & height_b, const element & e) 
		: m_height_a(height_a), m_height_b(height_b), e(&e) { }

	layer_information(layer_information && src) { *this = std::move(src); }

	layer_information & operator = (layer_information && src) {
		if (&src != this) {
			m_height_a = std::move(src.m_height_a);
			m_height_b = std::move(src.m_height_b);
			e = src.e;
		}
		return *this;
	}

	bool has_both_sides() const { return m_height_b.is_initialized(); }
	NT height_a() const { return m_height_a; }
	NT height_b() const { return *m_height_b; }
	const element & layer_element() const { return *e; }
};