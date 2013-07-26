#include "precompiled.h"

#include "../../Edm Wrapper/edm_wrapper_native_interface.h"

#include "CreateGuid_64.h"
#include "geometry_common.h"
#include "ifc-to-cgal.h"
#include "model_operations.h"
#include "sbt-ifcadapter.h"
#include "unit_scaler.h"

using namespace ifc_interface;

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

ifc_object * create_point(
	model * m,
	const ipoint_3 & p,
	const unit_scaler & scaler)
{
	double x = scaler.length_out(CGAL::to_double(p.x()));
	double y = scaler.length_out(CGAL::to_double(p.y()));
	double z = scaler.length_out(CGAL::to_double(p.z()));
	return m->create_point(x, y, z);
}

ifc_object * create_direction(model * m, const idirection_3 & d) {
	double dx = CGAL::to_double(d.dx());
	double dy = CGAL::to_double(d.dy());
	double dz = CGAL::to_double(d.dz());
	return m->create_direction(dx, dy, dz);
}

ifc_object * create_a2p3d(
	model * m,
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

	auto res = m->create_object("IfcAxis2Placement3D", false);
	set_field(res, "Location", *create_point(m, origin, scaler));
	set_field(res, "Axis", *create_direction(m, z_axis));
	set_field(res, "RefDirection", *create_direction(m, refdir));
	return res;
}

ifc_object * create_plane(
	model * m, 
	const itransformation_3 & space_placement, 
	space_boundary * sb, 
	std::vector<ipoint_2> * points_2d, 
	const unit_scaler & scaler) 
{
	auto plane = m->create_object("IfcPlane", false);
	auto a2p3d = create_a2p3d(m, space_placement, sb, points_2d, scaler);
	set_field(plane, "Position", *a2p3d);
	return plane;
}

ifc_object * create_curve(
	model * m,
	const std::vector<ipoint_2> & points, 
	const unit_scaler & scaler) 
{
	std::vector<std::pair<double, double>> pts;
	for (auto p = points.begin(); p != points.end(); ++p) {
		double x = scaler.length_out(CGAL::to_double(p->x()));
		double y = scaler.length_out(CGAL::to_double(p->y()));
		pts.push_back(std::make_pair(x, y));
	}
	return m->create_curve(pts);
}

ifc_object * create_boundary(
	model * m, 
	const itransformation_3 & space_placement, 
	space_boundary * sb, 
	const unit_scaler & scaler) 
{
	auto res = m->create_object("IfcCurveBoundedPlane", false);
	std::vector<ipoint_2> points_2d;
	auto basis = create_plane(m, space_placement, sb, &points_2d, scaler);
	set_field(res, "BasisSurface", *basis);
	set_field(res, "OuterBoundary", *create_curve(m, points_2d, scaler));
	return res;
}

ifc_object * create_geometry(
	model * m, 
	const itransformation_3 & space_placement, 
	space_boundary * sb, 
	const unit_scaler & scaler) 
{
	auto res = m->create_object("IfcConnectionSurfaceGeometry", false);
	auto boundary = create_boundary(m, space_placement, sb, scaler);
	set_field(res, "SurfaceOnRelatingElement", *boundary);
	return res;
}

ifc_object * create_sb(
	model * m,
	const ifc_object & space,
	const ifc_object & element,
	space_boundary * sb,
	const unit_scaler & scaler,
	number_collection<iK> * c)
{
	auto space_trans = object_field(space, "ObjectPlacement");
	auto space_placement = build_transformation(space_trans, scaler, c);

	auto res = m->create_object("IfcRelSpaceBoundary", true);

	int level = get_level(*sb);

	set_field(res, "GlobalId", sb->global_id);
	set_field(res, "Name", (
		level == 3 ? "3rdLevel" :
		level == 4 ? "4thLevel" :
		level == 5 ? "5thLevel" : "2ndLevel"));
	if (level == 2 || level == 3 || level == 4 || level == 5) {
		set_field(res, "Description", level == 2 ? "2a" : "2b");
	}
	set_field(res, "RelatingSpace", space);
	set_field(res, "RelatedBuildingElement", element);
	auto virt_string = is_virtual(*sb) ? "VIRTUAL" : "PHYSICAL";
	set_field(res, "PhysicalOrVirtualBoundary", virt_string);
	auto ext_string = sb->is_external ? "EXTERNAL" : "INTERNAL";
	set_field(res, "InternalOrExternalBoundary", ext_string);
	auto geom = create_geometry(m, space_placement, sb, scaler);
	set_field(res, "ConnectionGeometry", *geom);
	return res;
}

ifc_object * create_rel_aggregates(
	model * m, 
	const char * name, 
	const char * desc) 
{
	auto res = m->create_object("IfcRelAggregates", true);
	char guidbuf[128];
	CreateCompressedGuidString(guidbuf, 127);
	set_field(res, "GlobalId", guidbuf);
	set_field(res, "Name", name);
	set_field(res, "Description", desc);
	return res;
}

std::vector<ifc_object *> create_necessary_virtual_elements(
	model * m, 
	space_boundary ** sbs, 
	int sb_count)
{
	std::map<space_boundary *, space_boundary *> virtuals;
	for (int i = 0; i < sb_count; ++i) {
		if (is_virtual(*sbs[i])) {
			auto itr = virtuals.find(sbs[i]->opposite);
			if (itr == virtuals.end()) {
				virtuals[sbs[i]] = sbs[i]->opposite;
			}
		}
	}
	std::vector<ifc_object *> res;
	for (auto pair = virtuals.begin(); pair != virtuals.end(); ++pair) {
		auto inst = m->create_object("IfcVirtualElement", true);
		char guidbuf[128];
		CreateCompressedGuidString(guidbuf, 127);
		set_field(inst, "GlobalId", guidbuf);
		strncpy(pair->first->element_name, guidbuf, ELEMENT_NAME_MAX_LEN);
		strncpy(pair->second->element_name, guidbuf, ELEMENT_NAME_MAX_LEN);
		res.push_back(inst);
	}
	return res;
}

} // namespace

ifcadapter_return_t add_to_model(
	model * m,
	size_t sb_count, 
	space_boundary ** sbs,
	void (*msg_func)(char *),
	number_collection<iK> * c) 
{
	unit_scaler scaler = unit_scaler::identity_scaler;

	msg_func("Preparing to add space boundaries to the model.\n");
	auto version_string = "1.5.9";
	m->set_new_owner_history(
		"Space Boundary Tool",
		"SBT",
		version_string,
		"Lawrence Berkeley National Laboratory");

	auto virts = create_necessary_virtual_elements(m, sbs, sb_count);
	msg_func("Created virtual elements.\n");
	
	msg_func("Adding space boundaries to model");
	std::map<std::string, const ifc_object *> by_guid;
	auto ifc_spaces = m->spaces();
	for (auto s = ifc_spaces.begin(); s != ifc_spaces.end(); ++s) {
		by_guid[string_field(**s, "GlobalId")] = *s;
	}
	auto ifc_bldg_elems = m->building_elements();
	for (auto e = ifc_bldg_elems.begin(); e != ifc_bldg_elems.end(); ++e) {
		by_guid[string_field(**e, "GlobalId")] = *e;
	}
	for (auto v = virts.begin(); v != virts.end(); ++v) {
		by_guid[string_field(**v, "GlobalId")] = *v;
	}
	int added_count = 0;
	for (size_t i = 0; i < sb_count; ++i) {
		try {
			// Space boundaries with no geometry shouldn't exist but I don't
			// want everything to crash if they do.
			if (sbs[i]->geometry.vertex_count > 0) {
				auto sp = by_guid.find(sbs[i]->bounded_space->id);
				assert(sp != by_guid.end());
				auto el = by_guid.find(sbs[i]->element_name);
				assert(el != by_guid.end());
				create_sb(m, *sp->second, *el->second, sbs[i], scaler, c);
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
