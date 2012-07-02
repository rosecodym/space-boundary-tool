#pragma once

#include "precompiled.h"

#include "sbt-core.h"

class core_exception : std::exception {
private:
	sbt_return_t m_code;
public:
	core_exception(sbt_return_t code) : m_code(code) { }
	sbt_return_t code() const { return m_code; }
};