#include "precompiled.h"

#include "block.h"
#include "exceptions.h"
#include "halfblocks_for_base.h"
#include "is_hexahedral_prismatoid.h"
#include "is_right_cuboid.h"
#include "link_halfblocks.h"
#include "oriented_area.h"
#include "report.h"
#include "surface_pair.h"

#include "build_blocks.h"

using namespace reporting;

namespace blocking {

namespace impl {

namespace {

std::vector<surface_pair::envelope_contribution> gather_contributions(
	const relations_grid & surf_rels, 
	size_t face_count,
	size_t base_index)
{
	std::vector<surface_pair::envelope_contribution> res;
	for (size_t i = 0; i < face_count; ++i) {
		auto c = surf_rels[base_index][i].contributes_to_envelope();
		if (c != surface_pair::NONE) { res.push_back(c); }
	}
	return res;
}

template <typename FinalHalfblockOutputIterator>
void general_case(
	const relations_grid & surf_rels, 
	size_t face_count,
	const element & e, 
	equality_context * c, 
	FinalHalfblockOutputIterator oi)
{
	typedef relations_grid::index_range array_range;
	typedef relations_grid::index_gen indices;
	typedef surface_pair::envelope_contribution ec;

	std::list<oriented_area> linkable;

	for (size_t i = 0; i < face_count; ++i) {
		auto contribs = gather_contributions(surf_rels, face_count, i);
		auto parallel_count = boost::count_if(
			contribs, 
			[](ec contr) { return contr == surface_pair::PARALLEL; });
		if (parallel_count >= 1) {
			if (contribs.size() > 1) {
				halfblocks_for_base(
					surf_rels, 
					i, 
					c, 
					std::back_inserter(linkable));
			}
			else {
				linkable.push_back(surf_rels[i][i].base());
			}
		}
		else { *oi++ = block(surf_rels[i][i].base(), e); }
	}

	link_halfblocks(std::move(linkable), e, oi);
}

} // namespace

std::vector<block> build_blocks_for(
	const element & e,
	equality_context * c, 
	double max_block_thickness) 
{
	std::vector<block> res;

	std::vector<oriented_area> faces = e.faces(c);
	report_progress(boost::format("(%u faces): ") % faces.size());

	if (faces.size() <= 4) {
		report_progress("element is a tetrahedron. ");
		boost::transform(faces, std::back_inserter(res), [&res, &e](const oriented_area & f) {
			return block(f, e);
		});
		return res;
	}

	relations_grid surface_relationships = surface_pair::build_relations_grid(
		faces,
		c, 
		max_block_thickness);

	if (is_right_cuboid(
			surface_relationships, 
			faces.size(), 
			e, 
			max_block_thickness, 
			std::back_inserter(res)) ||
		is_hexahedral_prismatoid(
			surface_relationships, 
			faces.size(), 
			e, 
			max_block_thickness,
			std::back_inserter(res)))
	{
		return res;
	}
	else {
		report_progress("element requires an envelope calculation. ");
		general_case(
			surface_relationships, 
			faces.size(), 
			e, 
			c, 
			std::back_inserter(res));
	}

	return res;
}

} // namespace impl

std::vector<block> build_blocks(
	const std::vector<element> & elements,
	equality_context * c, 
	double max_block_thickness)
{
	typedef boost::format fmt;
	std::vector<block> res;
	report_progress(fmt(
		"Building blocks for %u elements.\n") % elements.size());
	boost::for_each(elements, [=, &res](const element & e) {
		bool stack_overflow = false;
		try {
			int block_count = 0;
			report_progress(fmt("Building blocks for element %s ") % e.name());
			boost::transform(
				impl::build_blocks_for(e, c, max_block_thickness), 
				std::back_inserter(res), 
				[&block_count](const block & b) -> block { 
					++block_count; 
					return b; 
				});
			report_progress(fmt("%i blocks created.\n") % block_count);
		}
		catch (stack_overflow_exception &) {
			// use as little stack space as possible in this catch block because the stack is still busted
			stack_overflow = true;
		}
		catch (std::exception & ex) {
			auto m = fmt("Element %s blocking failed: %s. It will be skipped."); 
			report_warning(m % e.name() % ex.what());
		}
		if (stack_overflow) {
			_resetstkoflw();
			report_warning("Internal error: stack overflow. Please report this"
				           "SBT bug.");
		}
	});
	return res;
}

} // namespace blocking