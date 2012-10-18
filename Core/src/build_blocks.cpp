#include "precompiled.h"

#include "block.h"
#include "exceptions.h"
#include "halfblocks_for_base.h"
#include "is_hexahedral_prismatoid.h"
#include "is_right_cuboid.h"
#include "link_halfblocks.h"
#include "oriented_area.h"
#include "report.h"
#include "scan_for_degenerate_halfblocks.h"
#include "surface_pair.h"

#include "build_blocks.h"

using namespace reporting;

namespace blocking {

namespace impl {

namespace {

template <typename OutputIterator>
void blocks_from_envelope(const relations_grid & surface_relationships, size_t face_count, const element & e, equality_context * c, OutputIterator oi) {
	auto degenerate_indices = scan_for_degenerate_halfblocks(surface_relationships, face_count, e, oi);

	std::list<oriented_area> halfblocks;
	for (size_t i = 0; i < face_count; ++i) {
		if (degenerate_indices.find(i) == degenerate_indices.end()) {
			halfblocks_for_base(surface_relationships, i, c, std::back_inserter(halfblocks));
		}
	}

	link_halfblocks(std::move(halfblocks), e, oi);
}

} // namespace

std::vector<block> build_blocks_for(const element & e, equality_context * c) {
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

	relations_grid surface_relationships = build_relations_grid(faces, c);

	if (is_right_cuboid(surface_relationships, faces.size(), e, std::back_inserter(res)) ||
		is_hexahedral_prismatoid(surface_relationships, faces.size(), e, std::back_inserter(res)))
	{
		return res;
	}
	else {
		report_progress("element requires an envelope calculation. ");
		blocks_from_envelope(surface_relationships, faces.size(), e, c, std::back_inserter(res));
	}

	return res;
}

} // namespace impl

std::vector<block> build_blocks(const std::vector<element> & elements, equality_context * c) {
	std::vector<block> res;
	report_progress(boost::format("Building blocks for %u elements.\n") % elements.size());
	boost::for_each(elements, [&res, c](const element & e) {
		bool stack_overflow = false;
		try {
			int block_count = 0;
			report_progress(boost::format("Building blocks for element %s ") % e.source_id().c_str());
			boost::transform(impl::build_blocks_for(e, c), std::back_inserter(res), [&block_count](const block & b) -> block { ++block_count; return b; });
			report_progress(boost::format("%i blocks created.\n") % block_count);
		}
		catch (stack_overflow_exception &) {
			// use as little stack space as possible in this catch block because the stack is still busted
			stack_overflow = true;
		}
		if (stack_overflow) {
			_resetstkoflw();
			report_warning(boost::format("The geometry for building element %s is too complicated. Space boundaries for this element will not be generated.\n") % e.source_id().c_str());
		}
	});
	return res;
}

} // namespace blocking