// Copyright (c) 2010 INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Mesh_3/include/CGAL/Mesh_3/parameters_defaults.h $
// $Id: parameters_defaults.h 67117 2012-01-13 18:14:48Z lrineau $
//
//
// Author(s)     : Stephane Tayeb
//
//******************************************************************************
// File Description : defines constants (default values) for parameters of
// Mesh_3 global functions
//******************************************************************************

#ifndef CGAL_MESH_3_PARAMETERS_DEFAULTS_H
#define CGAL_MESH_3_PARAMETERS_DEFAULTS_H

namespace CGAL {
namespace parameters { namespace default_values {

// exude_mesh_3  
const double exude_sliver_bound = 0.;

// perturb_mesh_3
const double perturb_sliver_bound = 0.;

// lloyd_optimize_mesh_3
const double lloyd_freeze_ratio = 0.01;
const double lloyd_convergence_ratio = 0.02;

// odt_optimize_mesh_3
const double odt_freeze_ratio = 0.01;
const double odt_convergence_ratio = 0.02;

} } // end namespace parameters::default_values
} // end namespace CGAL

#endif // CGAL_MESH_3_PARAMETERS_DEFAULTS_H
