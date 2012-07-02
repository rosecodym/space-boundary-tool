#include "precompiled.h"

#include "geometry_common.h"
#include "ifc-to-face.h"
#include "ifc-to-cgal.h"
#include "sbt-ifcadapter.h"
#include "unit_scaler.h"
#include "util.h"
#include "wrapped_nef_operations.h"

#include "ifc-to-solid.h"

extern sb_calculation_options g_opts;

namespace {

void ifc_to_brep(exact_brep * b, const cppw::Instance & inst, const unit_scaler & s) {
	assert(inst.is_kind_of("IfcConnectedFaceSet"));
	cppw::Set faces = inst.get("CfsFaces");
	for (faces.move_first(); faces.move_next(); ) {
		b->faces.push_back(ifc_to_face((cppw::Instance)faces.get_(), s));
	}
}

void ifc_to_ext(exact_extruded_area_solid * e,const cppw::Instance & inst, const unit_scaler & s) {
	assert(inst.is_instance_of("IfcExtrudedAreaSolid"));
	e->area = ifc_to_face((cppw::Instance)inst.get("SweptArea"), s);
	e->extrusion_depth = g_numbers.request_height(s.length_in(inst.get("Depth")));
	cppw::Instance dir = inst.get("ExtrudedDirection");
	cppw::List ratios = dir.get("DirectionRatios");
	e->ext_dir = g_numbers.request_direction(ratios.get_(0), ratios.get_(1), (cppw::Integer)dir.get("Dim") == 3 ? ratios.get_(2) : 0.0);

	if (!normal_matches_extrusion(e->area, e->ext_dir)) {
		std::reverse(e->area.outer_boundary.vertices.begin(), e->area.outer_boundary.vertices.end());
	}
}

void transform_according_to(exact_solid * s, const cppw::Select & trans, const unit_scaler & scaler) {
	if (!trans.is_set()) {
		return;
	}
	if (s->rep_type() == REP_BREP) {
		for (size_t i = 0; i < s->rep.as_brep->faces.size(); ++i) {
			transform_according_to(&s->rep.as_brep->faces[i], trans, scaler);
		}
	}
	else if (s->rep_type() == REP_EXT) {
		transform_according_to(&s->rep.as_ext->area, trans, scaler);
		s->rep.as_ext->ext_dir = s->rep.as_ext->ext_dir.transform(build_transformation(trans, scaler));
	}
	else {
		g_opts.error_func("[Internal solid rep was of an unknown type!]\n");
		return;
	}
}

} // namespace

void ifc_to_solid(exact_solid * s, const cppw::Instance & inst, const unit_scaler & scaler) {
	
	if (inst.is_kind_of("IfcProduct")) {
		ifc_to_solid(s, (cppw::Instance)inst.get("Representation"), scaler);
		transform_according_to(s, inst.get("ObjectPlacement"), scaler);
	}

	else if (inst.is_instance_of("IfcProductDefinitionShape")) {
		cppw::List reps = inst.get("Representations");
		for (reps.move_first(); reps.move_next(); ) {
			if (((cppw::Instance)reps.get_()).get("RepresentationIdentifier") == "Body") {
				ifc_to_solid(s, (cppw::Instance)reps.get_(), scaler);
				return;
			}
		}
		g_opts.error_func("[Error - no 'Body' representation identifier.]\n");
		return;
	}

	else if (inst.is_instance_of("IfcShapeRepresentation") && (((cppw::String)inst.get("RepresentationIdentifier")) == "Body")) {
		cppw::Set rep_items = inst.get("Items");
		ifc_to_solid(s, (cppw::Instance)rep_items.get_(0), scaler);
	}

	else if (inst.is_instance_of("IfcFacetedBrep")) {
		s->set_rep_type(REP_BREP);
		ifc_to_brep(s->rep.as_brep, (cppw::Instance)inst.get("Outer"), scaler);
	}

	else if (inst.is_instance_of("IfcExtrudedAreaSolid")) {
		s->set_rep_type(REP_EXT);
		ifc_to_ext(s->rep.as_ext, inst, scaler);
		transform_according_to(s, inst.get("Position"), scaler);
	}

	else if (inst.is_instance_of("IfcMappedItem")) {
		cppw::Instance mapping_source = inst.get("MappingSource");
		cppw::Instance mapped_rep = mapping_source.get("MappedRepresentation");
		ifc_to_solid(s, mapped_rep, scaler);
		transform_according_to(s, mapping_source.get("MappingOrigin"), scaler);
		transform_according_to(s, inst.get("MappingTarget"), scaler);
	}

	else if (inst.is_instance_of("IfcBooleanClippingResult")) {
		wrapped_nef_operations::solid_from_boolean_result(s, inst, scaler);
	}

	else if (inst.is_instance_of("IfcFaceBasedSurfaceModel")) {
		cppw::Set faceSet = inst.get("FbsmFaces");
		//if (faceSet.size() != 1) {
		//	g_opts.error_func("[Aborting - an IfcFaceBasedSurfaceModel had more than one face set.]\n");
		//	exit(IFCADAPT_UNSUPPORTED_INPUT);
		//}
		s->set_rep_type(REP_BREP);
		ifc_to_brep(s->rep.as_brep, (cppw::Instance)faceSet.get_(0), scaler);
	}

	else {
		g_opts.error_func("[Error - a solid wasn't represented in a known way.]\n");
		return;
	}

}