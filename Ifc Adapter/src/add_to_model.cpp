#include "precompiled.h"

#include "cgal-typedefs.h"
#include "CreateGuid_64.h"
#include "geometry_common.h"
#include "ifc-to-cgal.h"
#include "model_operations.h"
#include "sbt-ifcadapter.h"
#include "unit_scaler.h"

namespace {

cppw::Application_instance create_point(cppw::Open_model & model, const point_2 & p, const unit_scaler & scaler) {
	cppw::Application_instance point = model.create("IfcCartesianPoint");
	cppw::List coords(point.create_aggregate("Coordinates"));
	coords.add(scaler.length_out(CGAL::to_double(p.x())));
	coords.add(scaler.length_out(CGAL::to_double(p.y())));
	return point;
}

cppw::Application_instance create_point(cppw::Open_model & model, const point_3 & p, const unit_scaler & scaler) {
	cppw::Application_instance point = model.create("IfcCartesianPoint");
	cppw::List coords(point.create_aggregate("Coordinates"));
	coords.add(scaler.length_out(CGAL::to_double(p.x())));
	coords.add(scaler.length_out(CGAL::to_double(p.y())));
	coords.add(scaler.length_out(CGAL::to_double(p.z())));
	return point;
}

cppw::Application_instance create_direction(cppw::Open_model & model, const direction_3 & d) {
	cppw::Application_instance direction = model.create("IfcDirection");
	cppw::List ratios(direction.create_aggregate("DirectionRatios"));
	ratios.add(CGAL::to_double(d.dx()));
	ratios.add(CGAL::to_double(d.dy()));
	ratios.add(CGAL::to_double(d.dz()));
	return direction;
}

cppw::Application_instance create_a2p3d(cppw::Open_model & model, const transformation_3 & space_placement, space_boundary * sb, std::vector<point_2> * points_2d, const unit_scaler & scaler) {
	direction_3 ortho_dir = plane_3(
		from_c_point(sb->geometry.vertices[0]),
		from_c_point(sb->geometry.vertices[1]),
		from_c_point(sb->geometry.vertices[2])).orthogonal_direction();

	transformation_3 flatten = build_flatten(ortho_dir);

	double z = 0.0;
	bool set_z = false;
	for (size_t i = 0; i < sb->geometry.vertex_count; ++i) {
		point_3 p3flat = from_c_point(sb->geometry.vertices[i]).transform(flatten);
		points_2d->push_back(point_2(p3flat.x(), p3flat.y()));
		if (!set_z) {
			z = CGAL::to_double(p3flat.z());
			set_z = true;
		}
	}

	polygon_2 as_poly(points_2d->begin(), points_2d->end());
	if ((CGAL::orientation(as_poly[0], as_poly[1], as_poly[2]) == CGAL::RIGHT_TURN) != as_poly.is_clockwise_oriented())
	{
		// i'm sure there's a more elegant way to do this
		ortho_dir = -ortho_dir;
		flatten = build_flatten(ortho_dir);
		points_2d->clear();
		set_z = false;
		for (size_t i = 0; i < sb->geometry.vertex_count; ++i) {
			point_3 p3flat = from_c_point(sb->geometry.vertices[i]).transform(flatten);
			points_2d->push_back(point_2(p3flat.x(), p3flat.y()));
			if (!set_z) {
				z = CGAL::to_double(p3flat.z());
				set_z = true;
			}
		}
	}

	// this next part is necessary because GST actually cares about the plane normal
	// normally, -zhat never occurs, so we have to do some tweaking for floors
	if (ortho_dir == direction_3(0, 0, -1)) {
		flatten = flatten * transformation_3(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0);
		z *= -1;
		for (auto p = points_2d->begin(); p != points_2d->end(); ++p) {
			*p = p->transform(transformation_2(1, 0, 0, 0, -1, 0));
		}
	}

	transformation_3 unflatten = flatten.inverse();

	point_3 origin = point_3(0, 0, z).transform(unflatten).transform(space_placement.inverse());
	direction_3 z_axis = direction_3(0, 0, 1).transform(unflatten).transform(space_placement.inverse());
	direction_3 refdir = direction_3(1, 0, 0).transform(unflatten).transform(space_placement.inverse());

	cppw::Application_instance a2p3d = model.create("IfcAxis2Placement3D");
	a2p3d.put("Location", create_point(model, origin, scaler));
	a2p3d.put("Axis", create_direction(model, z_axis));
	a2p3d.put("RefDirection", create_direction(model, refdir));

	return a2p3d;
}

cppw::Application_instance create_plane(cppw::Open_model & model, const transformation_3 & space_placement, space_boundary * sb, std::vector<point_2> * points_2d, const unit_scaler & scaler) {
	cppw::Application_instance plane = model.create("IfcPlane");
	plane.put("Position", create_a2p3d(model, space_placement, sb, points_2d, scaler));
	return plane;
}

cppw::Application_instance create_curve(cppw::Open_model & model, const std::vector<point_2> & points, const unit_scaler & scaler) {
	cppw::Application_instance curve = model.create("IfcPolyline");
	cppw::List pts = curve.create_aggregate("Points");
	for (auto p = points.begin(); p != points.end(); ++p) {
		pts.add(create_point(model, *p, scaler));
	}
	return curve;
}

cppw::Application_instance create_boundary(cppw::Open_model & model, const transformation_3 & space_placement, space_boundary * sb, const unit_scaler & scaler) {
	cppw::Application_instance boundary = model.create("IfcCurveBoundedPlane");
	std::vector<point_2> points_2d;
	boundary.put("BasisSurface", create_plane(model, space_placement, sb, &points_2d, scaler));
	boundary.put("OuterBoundary", create_curve(model, points_2d, scaler));
	return boundary;
}

cppw::Application_instance create_geometry(cppw::Open_model & model, const transformation_3 & space_placement, space_boundary * sb, const unit_scaler & scaler) {
	cppw::Application_instance geometry = model.create("IfcConnectionSurfaceGeometry");
	geometry.put("SurfaceOnRelatingElement", create_boundary(model, space_placement, sb, scaler));
	return geometry;
}

cppw::Application_instance create_sb(
	cppw::Open_model & model,
	const cppw::Instance & ownerhistory, 
	const cppw::Instance & space,
	const cppw::Instance & element,
	space_boundary * sb,
	const unit_scaler & scaler,
	number_collection * c)
{

	transformation_3 space_placement = build_transformation(space.get("ObjectPlacement"), scaler, c);

	cppw::Application_instance inst = model.create("IfcRelSpaceBoundary");

	inst.put("GlobalId", sb->global_id);
	inst.put("OwnerHistory", ownerhistory);
	inst.put("Name", (
		sb->level == 3 ? "3rdLevel" :
		sb->level == 4 ? "4thLevel" :
		sb->level == 5 ? "5thLevel" : "2ndLevel"));
	if (sb->level == 2 || sb->level == 3 || sb->level == 4 || sb->level == 5) {
		inst.put("Description", sb->level == 2 ? "2a" : "2b");
	}
	inst.put("RelatingSpace", space);
	inst.put("RelatedBuildingElement", element);
	inst.put("PhysicalOrVirtualBoundary", sb->is_virtual ? "VIRTUAL" : "PHYSICAL");
	inst.put("InternalOrExternalBoundary", is_external_sb(sb) ? "EXTERNAL" : "INTERNAL");
	inst.put("ConnectionGeometry", create_geometry(model, space_placement, sb, scaler));
	return inst;
}

cppw::Application_instance create_rel_aggregates(cppw::Open_model & model, cppw::Instance & ownerhistory, const char * name, const char * desc) {
	cppw::Application_instance inst = model.create("IfcRelAggregates");
	char guidbuf[128];
	CreateCompressedGuidString(guidbuf, 127);
	inst.put("GlobalId", cppw::String(guidbuf));
	inst.put("OwnerHistory", ownerhistory);
	inst.put("Name", cppw::String(name));
	inst.put("Description", cppw::String(desc));
	return inst;
}

cppw::Application_instance create_owner_history(cppw::Open_model * model) {
	cppw::Application_aggregate histories = model->get_set_of("IfcOwnerHistory");
	cppw::Application_instance inst = ((cppw::Application_instance)histories.get_(0)).clone();
	inst.put("ChangeAction", "ADDED");
	cppw::Application_instance app = model->create("IfcApplication");
	cppw::Application_instance org = model->create("IfcOrganization");
	org.put("Name", "Lawrence Berkeley National Laboratory");
	app.put("ApplicationDeveloper", org);
	app.put("Version", "1.3.0b");
	app.put("ApplicationFullName", "Space Boundary Tool");
	app.put("ApplicationIdentifier", "SBT");
	inst.put("LastModifyingApplication", app);
	inst.put("CreationDate", (int)time(NULL));
	return inst;
}

void create_necessary_virtual_elements(cppw::Open_model * model, space_boundary ** sbs, int sb_count, const cppw::Instance & ownerhistory) {
	std::map<space_boundary *, space_boundary *> virtuals;
	for (int i = 0; i < sb_count; ++i) {
		if (sbs[i]->is_virtual) {
			auto itr = virtuals.find(sbs[i]->opposite);
			if (itr == virtuals.end()) {
				virtuals[sbs[i]] = sbs[i]->opposite;
			}
		}
	}
	for (auto pair = virtuals.begin(); pair != virtuals.end(); ++pair) {
		cppw::Application_instance inst = model->create("IfcVirtualElement");
		char guidbuf[128];
		CreateCompressedGuidString(guidbuf, 127);
		inst.put("GlobalId", guidbuf);
		inst.put("OwnerHistory", ownerhistory);
		strncpy(pair->first->element_id, guidbuf, ELEMENT_ID_MAX_LEN);
		strncpy(pair->second->element_id, guidbuf, ELEMENT_ID_MAX_LEN);
	}
}

} // namespace

ifcadapter_return_t add_to_model(
	cppw::Open_model & model,
	size_t sb_count, 
	space_boundary ** sbs,
	void (*msg_func)(char *),
	const unit_scaler & scaler,
	number_collection * c) 
{
	msg_func("Adding space boundaries to model");
	cppw::Instance ownerhistory = create_owner_history(&model);

	create_necessary_virtual_elements(&model, sbs, sb_count, ownerhistory);

	std::map<std::string, int> element_map; // we have to use indices because Instances aren't default-constructable
	auto es = model.get_set_of("IfcElement", cppw::include_subtypes);
	for (int i = 0; i < es.count(); ++i) {
		if (es.get(i).is_kind_of("IfcBuildingElement") || es.get(i).is_kind_of("IfcVirtualElement")) {
			element_map[((cppw::String)es.get(i).get("GlobalId")).data()] = i;
		}
	}

	std::map<std::string, int> space_map;
	auto ss = model.get_set_of("IfcSpace");
	for (int i = 0; i < ss.count(); ++i) {
		space_map[((cppw::String)ss.get(i).get("GlobalId")).data()] = i;
	}

	int added_count = 0;
	for (size_t i = 0; i < sb_count; ++i) {
		if (!sbs[i]->lies_on_outside) {
			try {
				create_sb(model, ownerhistory, ss.get(space_map[sbs[i]->bounded_space->id]), es.get(element_map[sbs[i]->element_id]), sbs[i], scaler, c);
				++added_count;
				msg_func(".");
			}
			catch (...) {
				char buf[128];
				sprintf(buf, "Failed to add space boundary %s/%s/%s to the model.\n",
					sbs[i]->global_id,
					sbs[i]->element_id,
					sbs[i]->bounded_space->id);
				fprintf(stderr, buf);
			}
		}
	}
	msg_func("done.\n");

	char buf[256];
	sprintf(buf, "Added %i space boundaries to the model.\n", added_count);
	msg_func(buf);

	return IFCADAPT_OK;
}