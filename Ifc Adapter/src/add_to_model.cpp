#include "precompiled.h"

#include "CreateGuid_64.h"
#include "geometry_common.h"
#include "ifc-to-cgal.h"
#include "model_operations.h"
#include "sbt-ifcadapter.h"
#include "unit_scaler.h"

namespace {

bool is_virtual(const space_boundary & b) {
	return b.is_virtual != 0;
}

int get_level(const space_boundary & b) {
	return
		b.is_external								 ? 2 :
		b.opposite == nullptr						 ? 3 :
		b.bounded_space == b.opposite->bounded_space ? 4 : 2;
}

cppw::Application_instance create_point(
	cppw::Open_model & model, 
	const ipoint_2 & p, 
	const unit_scaler & scaler) 
{
	cppw::Application_instance point = model.create("IfcCartesianPoint");
	cppw::List coords(point.create_aggregate("Coordinates"));
	coords.add(scaler.length_out(CGAL::to_double(p.x())));
	coords.add(scaler.length_out(CGAL::to_double(p.y())));
	return point;
}

cppw::Application_instance create_point(
	cppw::Open_model & model, 
	const ipoint_3 & p, 
	const unit_scaler & scaler) 
{
	cppw::Application_instance point = model.create("IfcCartesianPoint");
	cppw::List coords(point.create_aggregate("Coordinates"));
	coords.add(scaler.length_out(CGAL::to_double(p.x())));
	coords.add(scaler.length_out(CGAL::to_double(p.y())));
	coords.add(scaler.length_out(CGAL::to_double(p.z())));
	return point;
}

cppw::Application_instance create_direction(
	cppw::Open_model & model, 
	const idirection_3 & d) 
{
	cppw::Application_instance direction = model.create("IfcDirection");
	cppw::List ratios(direction.create_aggregate("DirectionRatios"));
	ratios.add(CGAL::to_double(d.dx()));
	ratios.add(CGAL::to_double(d.dy()));
	ratios.add(CGAL::to_double(d.dz()));
	return direction;
}

cppw::Application_instance create_a2p3d(
	cppw::Open_model & model, 
	const itransformation_3 & space_placement, 
	space_boundary * sb, 
	std::vector<ipoint_2> * points_2d, 
	const unit_scaler & scaler) 
{
	auto ortho_dir = iplane_3(
		from_c_point<iK>(sb->geometry.vertices[0]),
		from_c_point<iK>(sb->geometry.vertices[1]),
		from_c_point<iK>(sb->geometry.vertices[2])).orthogonal_direction();

	auto flatten = build_flatten(ortho_dir);

	double z = 0.0;
	bool set_z = false;
	for (size_t i = 0; i < sb->geometry.vertex_count; ++i) {
		point p = sb->geometry.vertices[i];
		auto p3flat = from_c_point<iK>(p).transform(flatten);
		points_2d->push_back(ipoint_2(p3flat.x(), p3flat.y()));
		if (!set_z) {
			z = CGAL::to_double(p3flat.z());
			set_z = true;
		}
	}

	ipolygon_2 as_poly(points_2d->begin(), points_2d->end());
	if ((CGAL::orientation(as_poly[0], as_poly[1], as_poly[2]) == CGAL::RIGHT_TURN) != as_poly.is_clockwise_oriented())
	{
		// i'm sure there's a more elegant way to do this
		ortho_dir = -ortho_dir;
		flatten = build_flatten(ortho_dir);
		points_2d->clear();
		set_z = false;
		for (size_t i = 0; i < sb->geometry.vertex_count; ++i) {
			point p = sb->geometry.vertices[i];
			auto p3flat = from_c_point<iK>(p).transform(flatten);
			points_2d->push_back(ipoint_2(p3flat.x(), p3flat.y()));
			if (!set_z) {
				z = CGAL::to_double(p3flat.z());
				set_z = true;
			}
		}
	}

	// This code makes sure that if the ortho_dir is -zhat then it will
	// actually be written that way. That was the original idea, anyway - I
	// don't know if this fix is still needed but I'm not going to touch it
	// until I do.
	if (ortho_dir == idirection_3(0, 0, -1)) {
		itransformation_3 flip(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, -1, 0);
		flatten = flatten * flip;
		z *= -1;
		for (auto p = points_2d->begin(); p != points_2d->end(); ++p) {
			*p = p->transform(itransformation_2(1, 0, 0, 0, -1, 0));
		}
	}

	auto unflatten = flatten.inverse();

	ipoint_3 origin = 
		ipoint_3(0, 0, z)
		.transform(unflatten)
		.transform(space_placement.inverse());
	idirection_3 z_axis = 
		idirection_3(0, 0, 1)
		.transform(unflatten)
		.transform(space_placement.inverse());
	idirection_3 refdir = 
		idirection_3(1, 0, 0)
		.transform(unflatten)
		.transform(space_placement.inverse());

	cppw::Application_instance a2p3d = model.create("IfcAxis2Placement3D");
	a2p3d.put("Location", create_point(model, origin, scaler));
	a2p3d.put("Axis", create_direction(model, z_axis));
	a2p3d.put("RefDirection", create_direction(model, refdir));

	return a2p3d;
}

cppw::Application_instance create_plane(
	cppw::Open_model & model, 
	const itransformation_3 & space_placement, 
	space_boundary * sb, 
	std::vector<ipoint_2> * points_2d, 
	const unit_scaler & scaler) 
{
	cppw::Application_instance plane = model.create("IfcPlane");
	plane.put("Position", create_a2p3d(model, space_placement, sb, points_2d, scaler));
	return plane;
}

cppw::Application_instance create_curve(
	cppw::Open_model & model, 
	const std::vector<ipoint_2> & points, 
	const unit_scaler & scaler) 
{
	cppw::Application_instance curve = model.create("IfcPolyline");
	cppw::List pts = curve.create_aggregate("Points");
	for (auto p = points.begin(); p != points.end(); ++p) {
		pts.add(create_point(model, *p, scaler));
	}
	return curve;
}

cppw::Application_instance create_boundary(
	cppw::Open_model & model, 
	const itransformation_3 & space_placement, 
	space_boundary * sb, 
	const unit_scaler & scaler) 
{
	cppw::Application_instance boundary = model.create("IfcCurveBoundedPlane");
	std::vector<ipoint_2> points_2d;
	boundary.put("BasisSurface", create_plane(model, space_placement, sb, &points_2d, scaler));
	boundary.put("OuterBoundary", create_curve(model, points_2d, scaler));
	return boundary;
}

cppw::Application_instance create_geometry(
	cppw::Open_model & model, 
	const itransformation_3 & space_placement, 
	space_boundary * sb, 
	const unit_scaler & scaler) 
{
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
	number_collection<iK> * c)
{

	auto space_placement = 
		build_transformation(space.get("ObjectPlacement"), scaler, c);

	cppw::Application_instance inst = model.create("IfcRelSpaceBoundary");

	int level = get_level(*sb);

	inst.put("GlobalId", sb->global_id);
	inst.put("OwnerHistory", ownerhistory);
	inst.put("Name", (
		level == 3 ? "3rdLevel" :
		level == 4 ? "4thLevel" :
		level == 5 ? "5thLevel" : "2ndLevel"));
	if (level == 2 || level == 3 || level == 4 || level == 5) {
		inst.put("Description", level == 2 ? "2a" : "2b");
	}
	inst.put("RelatingSpace", space);
	inst.put("RelatedBuildingElement", element);
	inst.put("PhysicalOrVirtualBoundary", is_virtual(*sb) ? "VIRTUAL" : "PHYSICAL");
	inst.put("InternalOrExternalBoundary", sb->is_external ? "EXTERNAL" : "INTERNAL");
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
	app.put("Version", "1.5.5");
	app.put("ApplicationFullName", "Space Boundary Tool");
	app.put("ApplicationIdentifier", "SBT");
	inst.put("LastModifyingApplication", app);
	inst.put("CreationDate", (int)time(NULL));
	return inst;
}

void create_necessary_virtual_elements(cppw::Open_model * model, space_boundary ** sbs, int sb_count, const cppw::Instance & ownerhistory) {
	std::map<space_boundary *, space_boundary *> virtuals;
	for (int i = 0; i < sb_count; ++i) {
		if (is_virtual(*sbs[i])) {
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
		strncpy(pair->first->element_name, guidbuf, ELEMENT_NAME_MAX_LEN);
		strncpy(pair->second->element_name, guidbuf, ELEMENT_NAME_MAX_LEN);
	}
}

} // namespace

ifcadapter_return_t add_to_model(
	cppw::Open_model & model,
	size_t sb_count, 
	space_boundary ** sbs,
	void (*msg_func)(char *),
	number_collection<iK> * c) 
{
	unit_scaler scaler = unit_scaler::identity_scaler;

	msg_func("Preparing to add space boundaries to the model.\n");
	cppw::Instance ownerhistory = create_owner_history(&model);

	create_necessary_virtual_elements(&model, sbs, sb_count, ownerhistory);
	msg_func("Created virtual elements.\n");

	std::map<std::string, int> element_map; // we have to use indices because Instances aren't default-constructable
	auto es = model.get_set_of("IfcElement", cppw::include_subtypes);
	for (int i = 0; i < es.count(); ++i) {
		if (es.get(i).is_kind_of("IfcBuildingElement") || es.get(i).is_kind_of("IfcVirtualElement")) {
			element_map[((cppw::String)es.get(i).get("GlobalId")).data()] = i;
		}
	}
	msg_func("Created element map.\n");

	std::map<std::string, int> space_map;
	auto ss = model.get_set_of("IfcSpace");
	for (int i = 0; i < ss.count(); ++i) {
		space_map[((cppw::String)ss.get(i).get("GlobalId")).data()] = i;
	}
	msg_func("Created space map.\n");
	
	msg_func("Adding space boundaries to model");
	int added_count = 0;
	for (size_t i = 0; i < sb_count; ++i) {
		try {
			// Space boundaries with no geometry shouldn't exist but I don't
			// want everything to crash if they do.
			if (sbs[i]->geometry.vertex_count > 0) {
				create_sb(
					model, 
					ownerhistory, 
					ss.get(space_map[sbs[i]->bounded_space->id]), 
					es.get(element_map[sbs[i]->element_name]), 
					sbs[i], 
					scaler, 
					c);
				++added_count;
				msg_func(".");
			}
		}
		catch (...) {
			// There used to be some error handling code here that was
			// literally worse than nothing so I'm taking it out as a quick
			// stopgap.
		}
	}
	msg_func("done.\n");

	char buf[256];
	sprintf(buf, "Added %i space boundaries to the model.\n", added_count);
	msg_func(buf);

	return IFCADAPT_OK;
}
