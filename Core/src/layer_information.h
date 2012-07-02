#pragma once

#include "precompiled.h"

#include "element.h"

class layer_information {
private:
	NT m_p_a;
	boost::optional<NT> m_p_b;
	const element * e;

public:
	layer_information(const NT & p_a, const element & e) 
		: m_p_a(p_a), e(&e) { }
	layer_information(const NT & p_a, const NT & p_b, const element & e) 
		: m_p_a(p_a), m_p_b(p_b), e(&e) { }

	layer_information(layer_information && src) { *this = std::move(src); }

	layer_information & operator = (layer_information && src) {
		if (&src != this) {
			m_p_a = std::move(src.m_p_a);
			m_p_b = std::move(src.m_p_b);
			e = src.e;
		}
		return *this;
	}

	bool has_both_sides() const { return m_p_b.is_initialized(); }
	NT p_a() const { return m_p_a; }
	NT p_b() const { return *m_p_b; }
	const element & layer_element() const { return *e; }
};