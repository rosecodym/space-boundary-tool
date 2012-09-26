#include "precompiled.h"

#include "geometry_common.h"
#include "ifc-to-face.h"
#include "ifc-to-cgal.h"
#include "number_collection.h"
#include "sbt-ifcadapter.h"
#include "unit_scaler.h"
#include "util.h"
#include "wrapped_nef_operations.h"

#include "ifc-to-solid.h"

extern sb_calculation_options g_opts;

namespace {

void ifc_to_brep(exact_brep * b, const cppw::Instance & inst, const unit_scaler & s, number_collection<K> * c) {
	assert(inst.is_kind_of("IfcConnectedFaceSet"));
	cppw::Set faces = inst.get("CfsFaces");
	for (faces.move_first(); faces.move_next(); ) {
		b->faces.push_back(ifc_to_face((cppw::Instance)faces.get_(), s, c));
	}
}

void ifc_to_ext(exact_extruded_area_solid * e,const cppw::Instance & inst, const unit_scaler & s, number_collection<K> * c) {
	assert(inst.is_instance_of("IfcExtrudedAreaSolid"));
	e->area = ifc_to_face((cppw::Instance)inst.get("SweptArea"), s, c);
	e->extrusion_depth = c->request_height(s.length_in(inst.get("Depth")));
	cppw::Instance dir = inst.get("ExtrudedDirection");
	cppw::List ratios = dir.get("DirectionRatios");
	e->ext_dir = c->request_direction(ratios.get_(0), ratios.get_(1), (cppw::Integer)dir.get("Dim") == 3 ? ratios.get_(2) : 0.0);

	if (!normal_matches_extrusion(e->area, e->ext_dir)) {
		std::reverse(e->area.outer_boundary.vertices.begin(), e->area.outer_boundary.vertices.end());
	}
}

void transform_according_to(exact_solid * s, const cppw::Select & trans, const unit_scaler & scaler, number_collection<K> * c) {
	if (!trans.is_set()) {
		return;
	}
	if (s->rep_type() == REP_BREP) {
		for (size_t i = 0; i < s->rep.as_brep->faces.size(); ++i) {
			transform_according_to(&s->rep.as_brep->faces[i], trans, scaler, c);
		}
	}
	else if (s->rep_type() == REP_EXT) {
		transform_according_to(&s->rep.as_ext->area, trans, scaler, c);
		s->rep.as_ext->ext_dir = s->rep.as_ext->ext_dir.transform(build_transformation(trans, scaler, c));
	}
	else {
		g_opts.error_func("[Internal solid rep was of an unknown type!]\n");
		return;
	}
}

} // namespace

int ifc_to_solid(exact_solid * s, const cppw::Instance & inst, const unit_scaler & scaler, number_collection<K> * c) {
	
	if (inst.is_kind_of("IfcProduct")) {
		int res = ifc_to_solid(s, (cppw::Instance)inst.get("Representation"), scaler, c);
		if (res == 0) {
			transform_according_to(s, inst.get("ObjectPlacement"), scaler, c);
		}
		return res;
	}

	else if (inst.is_instance_of("IfcProductDefinitionShape")) {
		cppw::List reps = inst.get("Representations");
		for (reps.move_first(); reps.move_next(); ) {
			if (((cppw::Instance)reps.get_()).get("RepresentationIdentifier") == "Body") {
				return ifc_to_solid(s, (cppw::Instance)reps.get_(), scaler, c);
			}
		}
		g_opts.error_func("[Error - no 'Body' representation identifier.]\n");
		return 2;
	}

	else if (inst.is_instance_of("IfcShapeRepresentation") && (((cppw::String)inst.get("RepresentationIdentifier")) == "Body")) {
		cppw::Set rep_items = inst.get("Items");
		return ifc_to_solid(s, (cppw::Instance)rep_items.get_(0), scaler, c);
	}

	else if (inst.is_instance_of("IfcFacetedBrep")) {
		s->set_rep_type(REP_BREP);
		ifc_to_brep(s->rep.as_brep, (cppw::Instance)inst.get("Outer"), scaler, c);
		return 0;
	}

	else if (inst.is_instance_of("IfcExtrudedAreaSolid")) {
		s->set_rep_type(REP_EXT);
		ifc_to_ext(s->rep.as_ext, inst, scaler, c);
		transform_according_to(s, inst.get("Position"), scaler, c);
		return 0;
	}

	else if (inst.is_instance_of("IfcMappedItem")) {
		cppw::Instance mapping_source = inst.get("MappingSource");
		cppw::Instance mapped_rep = mapping_source.get("MappedRepresentation");
		int res = ifc_to_solid(s, mapped_rep, scaler, c);
		if (res == 0) {
			transform_according_to(s, mapping_source.get("MappingOrigin"), scaler, c);
			transform_according_to(s, inst.get("MappingTarget"), scaler, c);
		}
		return res;
	}

	else if (inst.is_instance_of("IfcBooleanClippingResult")) {
		wrapped_nef_operations::solid_from_boolean_result(s, inst, scaler, c);
		return 0;
	}

	else if (inst.is_instance_of("IfcFaceBasedSurfaceModel")) {
		cppw::Set faceSet = inst.get("FbsmFaces");
		s->set_rep_type(REP_BREP);
		ifc_to_brep(s->rep.as_brep, (cppw::Instance)faceSet.get_(0), scaler, c);
		return 0;
	}

	else {
		g_opts.error_func("[Error - a solid wasn't represented in a known way.]\n");
		return 1;
	}
}