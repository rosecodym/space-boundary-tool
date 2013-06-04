// Copyright (c) 2005  INRIA (France).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/releases/CGAL-4.1-branch/Surface_mesh_parameterization/include/CGAL/Parameterization_polyhedron_adaptor_3.h $
// $Id: Parameterization_polyhedron_adaptor_3.h 68566 2012-04-16 15:24:14Z lrineau $
//
//
// Author(s)     : Laurent Saboret, Pierre Alliez, Bruno Levy


#ifndef CGAL_PARAMETERIZATION_POLYHEDRON_ADAPTOR3_H
#define CGAL_PARAMETERIZATION_POLYHEDRON_ADAPTOR3_H

#include <CGAL/iterator.h>
#include <CGAL/circulator.h>
#include <CGAL/Polyhedron_3.h>

#include <CGAL/surface_mesh_parameterization_assertions.h>
#include <CGAL/Convertible_iterator_project.h>
#include <CGAL/Convertible_circulator_project.h>

#include <list>

namespace CGAL {


/// Parameterization_polyhedron_adaptor_3 is an adaptor class to access to a Polyhedron
/// 3D mesh using the ParameterizationPatchableMesh_3 interface.
/// Among other things, this concept defines the accessor to the (u,v) values
/// computed by parameterizations methods.
///
/// Note that these interfaces are decorators that add "on the fly"
/// the necessary fields to unmodified CGAL data structures (using STL maps).
/// For performance reasons, it is recommended to use CGAL data structures
/// enriched with the proper fields.
///
/// A ParameterizationMesh_3 surface consists of vertices,
/// facets and an incidence relation on them.
/// No notion of edge is requested.
///
/// ParameterizationMesh_3 meshes can have any genus, arity or number of components.
///
/// It can have have any number of borders. Its "main border"
/// will be the mesh's longest border (if there is at least one border).
///
/// It has also the ability to support patches and virtual seams.
/// "Patches" are a subset of a 3D mesh. "Virtual seams" are the ability
/// to behave exactly as if the surface was cut following a certain path.
///
/// @heading Is Model for the Concepts:
/// Model of ParameterizationPatchableMesh_3 concept, whose purpose is to allow
/// the Surface_mesh_parameterization package to access meshes in a uniform manner.
///
/// @heading Design Pattern:
/// Parameterization_polyhedron_adaptor_3 is an Adaptor [GHJV95]: it changes the
/// Polyhedron interface to match the ParameterizationPatchableMesh_3 concept.

template<class Polyhedron_3_>
class Parameterization_polyhedron_adaptor_3
{
// Forward references
public:
    class                                   Halfedge_info;
    class                                   Vertex_info;

private:
    struct                                  Less;
    struct                                  Project_halfedge_vertex;
    struct                                  Project_vertex_handle_vertex;
    struct                                  Project_opposite_halfedge_vertex;

// Private types
private:

    /// Halfedge
    typedef typename Polyhedron_3_::Halfedge Halfedge;
    typedef typename Polyhedron_3_::Halfedge_handle
                                            Halfedge_handle;
    typedef typename Polyhedron_3_::Halfedge_const_handle
                                            Halfedge_const_handle;
    typedef typename Polyhedron_3_::Halfedge_iterator
                                            Halfedge_iterator;
    typedef typename Polyhedron_3_::Halfedge_const_iterator
                                            Halfedge_const_iterator;
    typedef typename Polyhedron_3_::Halfedge_around_vertex_circulator
                                            Halfedge_around_vertex_circulator;
    typedef typename Polyhedron_3_::Halfedge_around_vertex_const_circulator
                                            Halfedge_around_vertex_const_circulator;
    typedef typename Polyhedron_3_::Halfedge_around_facet_circulator
                                            Halfedge_around_facet_circulator;
    typedef typename Polyhedron_3_::Halfedge_around_facet_const_circulator
                                            Halfedge_around_facet_const_circulator;

    /// Additional info attached to halfedges
    typedef typename std::map<Halfedge_const_handle,
                              Halfedge_info,
                              Less>         Halfedge_info_map;
    /// Additional info attached to vertices
    typedef typename std::map<typename Polyhedron_3_::Vertex_const_handle,
                              Vertex_info,
                              Less>         Vertex_info_map;


// Public types
public:

    //*******************************************************************
    /// @name INTERFACE SPECIFIC TO Parameterization_polyhedron_adaptor_3
    //*******************************************************************
    //@{

    /// Export template parameter.
    typedef Polyhedron_3_                  Polyhedron;

    /// Additional info attached to halfedges.
    class Halfedge_info
    {
    public:
        typedef typename Polyhedron_3_::Traits::Kernel::Point_2
                                            Point_2;

    private:
        int m_tag;                  ///< general purpose tag
        bool m_is_parameterized;    ///< is parameterized?
        int m_seaming;              ///< seaming status
        Point_2 m_uv;               ///< texture coordinates
        int m_index;                ///< unique index

    public:
        /// Default constructor.
        Halfedge_info()
        {
            m_tag = -1;             // uninitialized
            m_uv = Point_2(0, 0);
            m_index = -1;           // uninitialized
            m_seaming = -1;         // uninitialized
            m_is_parameterized = false;
        }

        // Default destructor, copy constructor and operator =() are fine

        /// Access to general purpose tag.
        int tag() const { return m_tag; }
        void tag(int tag) { m_tag = tag; }

        /// Access to 'seaming status' field.
        int seaming() const { return m_seaming; }
        void seaming(int seaming) { m_seaming = seaming; }

        /// Access to texture coordinates.
        Point_2 uv() const { return m_uv; }
        void uv(Point_2 uv) { m_uv = uv; }

        /// Access to "parameterized?" field.
        bool is_parameterized() const { return m_is_parameterized; }
        void is_parameterized(bool is)  { m_is_parameterized = is; }

        /// Access to 'index' field.
        int index() const { return m_index; }
        void index(int i) { m_index = i; }
    };

    /// Additional info attached to vertices.
    class Vertex_info
    {
    private:
        int m_tag;                  ///< general purpose tag
        int m_seaming;              ///< seaming status
        int m_index;                ///< unique index

    public:
        /// Default constructor.
        Vertex_info()
        {
            m_index = -1;           // uninitialized
            m_tag = -1;             // uninitialized
            m_seaming = -1;         // uninitialized
        }

        // Default destructor, copy constructor and operator =() are fine.

        /// Access to 'index' field.
        int index() const { return m_index; }
        void index(int i) { m_index = i; }

        /// Access to 'tag' field.
        int tag() const { return m_tag; }
        void tag(int tag) { m_tag = tag; }

        /// Access to 'seaming status' field.
        int seaming() const { return m_seaming; }
        void seaming(int seaming) { m_seaming = seaming; }
    };

    //@} // end of INTERFACE SPECIFIC TO Parameterization_polyhedron_adaptor_3

    //*******************************************************************
    /// @name ParameterizationMesh_3 INTERFACE
    //*******************************************************************
    //@{

    /// Number type to represent coordinates.
    typedef typename Polyhedron::Traits::Kernel::FT NT;

    /// 2D point that represents (u,v) coordinates computed
    /// by parameterization methods. Must provide X() and Y() methods.
    typedef typename Polyhedron::Traits::Kernel::Point_2
                                            Point_2;
    /// 3D point that represents vertices coordinates. Must provide X() and Y() methods.
    typedef typename Polyhedron::Traits::Kernel::Point_3
                                            Point_3;
    /// 2D vector. Must provide X() and Y() methods.
    typedef typename Polyhedron::Traits::Kernel::Vector_2
                                            Vector_2;
    /// 3D vector. Must provide X() and Y() methods.
    typedef typename Polyhedron::Traits::Kernel::Vector_3
                                            Vector_3;

    /// Opaque type representing a facet of the 3D mesh. No methods are expected.
    typedef typename Polyhedron::Facet      Facet;
    /// Handle to a facet. Model of the Handle concept.
    typedef typename Polyhedron::Facet_handle Facet_handle;
    typedef typename Polyhedron::Facet_const_handle
                                            Facet_const_handle;
    /// Iterator over all mesh facets. Model of the ForwardIterator concept.
    typedef typename Polyhedron::Facet_iterator
                                            Facet_iterator;
    typedef typename Polyhedron::Facet_const_iterator
                                            Facet_const_iterator;

    /// Opaque type representing a vertex of the 3D mesh. No methods are expected.
    typedef typename Polyhedron::Vertex     Vertex;
    /// Handle to a vertex. Model of the Handle concept.
    typedef typename Polyhedron::Vertex_handle
                                            Vertex_handle;
    typedef typename Polyhedron::Vertex_const_handle
                                            Vertex_const_handle;
    /// Iterator over all vertices of a mesh. Model of the ForwardIterator concept.
    typedef typename Polyhedron::Vertex_iterator
                                            Vertex_iterator;
    typedef typename Polyhedron::Vertex_const_iterator
                                            Vertex_const_iterator;
    /// Iterator over vertices of the mesh "main border".
    /// Model of the ForwardIterator concept.
    typedef CGAL::Convertible_iterator_project<typename std::list<Vertex_handle>::iterator,
                                               Project_vertex_handle_vertex,
                                               Vertex_const_handle,
                                               Vertex_handle>
                                            Border_vertex_iterator;
    typedef CGAL::Convertible_iterator_project<typename std::list<Vertex_handle>::const_iterator,
                                               Project_vertex_handle_vertex,
                                               Vertex_const_handle>
                                            Border_vertex_const_iterator;
    /// Counter-clockwise circulator over a facet's vertices.
    /// Model of the BidirectionalCirculator concept.
    typedef CGAL::Convertible_circulator_project<Halfedge_around_facet_circulator,
                                                 Project_halfedge_vertex,
                                                 Vertex&,
                                                 Vertex*,
                                                 Vertex_const_handle,
                                                 Vertex_handle>
                                            Vertex_around_facet_circulator;
    typedef CGAL::Convertible_circulator_project<Halfedge_around_facet_const_circulator,
                                                 Project_halfedge_vertex,
                                                 const Vertex&,
                                                 const Vertex*,
                                                 Vertex_const_handle>
                                            Vertex_around_facet_const_circulator;
    /// Clockwise circulator over the vertices incident to a vertex.
    /// Model of the BidirectionalCirculator concept.
    typedef CGAL::Convertible_circulator_project<Halfedge_around_vertex_circulator,
                                                 Project_opposite_halfedge_vertex,
                                                 Vertex&,
                                                 Vertex*,
                                                 Vertex_const_handle,
                                                 Vertex_handle>
                                            Vertex_around_vertex_circulator;
    typedef CGAL::Convertible_circulator_project<Halfedge_around_vertex_const_circulator,
                                                 Project_opposite_halfedge_vertex,
                                                 const Vertex&,
                                                 const Vertex*,
                                                 Vertex_const_handle>
                                            Vertex_around_vertex_const_circulator;

    //@} // end of ParameterizationMesh_3 INTERFACE


// Public operations
public:

    //*******************************************************************
    /// @name INTERFACE SPECIFIC TO Parameterization_polyhedron_adaptor_3
    //*******************************************************************
    //@{

    /// Create an adaptator for an existing Polyhedron_3 mesh.
    /// The input mesh can be of any genus.
    /// It can have have any number of borders. Its "main border"
    /// will be the mesh's longest border (if there is at least one border).
    Parameterization_polyhedron_adaptor_3(Polyhedron& mesh)
        // Store reference to adapted mesh
      : m_polyhedron(mesh)
    {
        typedef typename Halfedge_info_map::value_type Halfedge_info_pair;
        typedef typename Vertex_info_map::value_type Vertex_info_pair;

        // Allocate extra info for each halfedge
        Halfedge_iterator he;
        for (he = m_polyhedron.halfedges_begin(); he != m_polyhedron.halfedges_end(); he++)
            m_halfedge_info.insert( Halfedge_info_pair(he, Halfedge_info()) );

        // Allocate extra info for each vertex
        Vertex_iterator vtx;
        for (vtx = m_polyhedron.vertices_begin(); vtx != m_polyhedron.vertices_end(); vtx++)
            m_vertex_info.insert( Vertex_info_pair(vtx, Vertex_info()) );

        // Extract mesh's longest border
        m_main_border = extract_longest_border(mesh);

#ifndef CGAL_NDEBUG
        // Index vertices right away to ease debugging
        index_mesh_vertices();
#endif
    }

    // Default destructor, copy constructor and operator =() are fine

    /// Get the adapted mesh.
    Polyhedron&       get_adapted_mesh()       { return m_polyhedron; }
    const Polyhedron& get_adapted_mesh() const { return m_polyhedron; }

    /// Get halfedge from source and target vertices.
    /// Will assert if such a halfedge doesn't exist.
    typename Polyhedron::Halfedge_const_handle get_halfedge(
        Vertex_const_handle source, Vertex_const_handle target) const
    {
        CGAL_surface_mesh_parameterization_precondition(source != NULL);
        CGAL_surface_mesh_parameterization_precondition(target != NULL);

        Halfedge_around_vertex_const_circulator cir     = target->vertex_begin(),
                                                cir_end = cir;
        CGAL_For_all(cir, cir_end)
            if (cir->opposite()->vertex() == source)
                return cir;

        // we should not get here
        CGAL_error();
        return NULL;
    }
    typename Polyhedron::Halfedge_handle get_halfedge(
        Vertex_handle source, Vertex_handle target)
    {
        Halfedge_const_handle halfedge = get_halfedge((Vertex_const_handle)source,
                                                      (Vertex_const_handle)target);
        return const_cast<Halfedge*>(&*halfedge);
    }

    /// Access to additional info attached to halfedges.
    const Halfedge_info* info(Halfedge_const_handle halfedge) const
    {
        typename Halfedge_info_map::const_iterator it = m_halfedge_info.find(halfedge);
        CGAL_surface_mesh_parameterization_assertion(it != m_halfedge_info.end());
        return &it->second;
    }
    Halfedge_info* info(Halfedge_const_handle halfedge)
    {
        typename Halfedge_info_map::iterator it = m_halfedge_info.find(halfedge);
        CGAL_surface_mesh_parameterization_assertion(it != m_halfedge_info.end());
        return &it->second;
    }

    /// Access to additional info attached to vertices.
    const Vertex_info* info(Vertex_const_handle vertex) const
    {
        typename Vertex_info_map::const_iterator it = m_vertex_info.find(vertex);
        CGAL_surface_mesh_parameterization_assertion(it != m_vertex_info.end());
        return &it->second;
    }
    Vertex_info* info(Vertex_const_handle vertex)
    {
        typename Vertex_info_map::iterator it = m_vertex_info.find(vertex);
        CGAL_surface_mesh_parameterization_assertion(it != m_vertex_info.end());
        return &it->second;
    }

    //@} // end of INTERFACE SPECIFIC TO Parameterization_polyhedron_adaptor_3

    //*******************************************************************
    /// @name ParameterizationMesh_3 INTERFACE
    //*******************************************************************
    //@{

    // MESH INTERFACE

    /// Indicate if the mesh matches the ParameterizationMesh_3 concept.
    bool is_valid() const {
        return m_polyhedron.is_valid();
    }

    /// Get iterator over first vertex of mesh.
    Vertex_iterator  mesh_vertices_begin() {
        return m_polyhedron.vertices_begin();
    }
    Vertex_const_iterator  mesh_vertices_begin() const {
        return m_polyhedron.vertices_begin();
    }

    /// Get iterator over past-the-end vertex of mesh.
    Vertex_iterator  mesh_vertices_end() {
        return m_polyhedron.vertices_end();
    }
    Vertex_const_iterator  mesh_vertices_end() const {
        return m_polyhedron.vertices_end();
    }

    /// Count the number of vertices of the mesh.
    int  count_mesh_vertices() const {
        int index = 0;
        for (Vertex_const_iterator it=mesh_vertices_begin(); it!=mesh_vertices_end(); it++)
            index++;
        return index;
    }

    // Index vertices of the mesh from 0 to count_mesh_vertices()-1
    void  index_mesh_vertices ()
    {
#ifdef DEBUG_TRACE
        fprintf(stderr,"  index Parameterization_polyhedron_adaptor vertices:\n");
#endif
        int index = 0;
        for (Vertex_iterator it=mesh_vertices_begin(); it!=mesh_vertices_end(); it++)
        {
            // Point_3 position = get_vertex_position(it);
/*#ifdef DEBUG_TRACE
            fprintf(stderr, "    %d=(%f,%f,%f)\n",
                            index,
                            (float)position.x(),
                            (float)position.y(),
                            (float)position.z());
#endif*/
            set_vertex_index(it, index++);
        }
#ifdef DEBUG_TRACE
        fprintf(stderr,"    ok\n");
#endif
    }

    /// Get iterator over first vertex of mesh's "main border".
    Border_vertex_iterator  mesh_main_border_vertices_begin() {
        return Border_vertex_iterator(m_main_border.begin());
    }
    Border_vertex_const_iterator  mesh_main_border_vertices_begin() const {
        return Border_vertex_const_iterator(m_main_border.begin());
    }

    /// Get iterator over past-the-end vertex of mesh's "main border".
    Border_vertex_iterator  mesh_main_border_vertices_end() {
        return Border_vertex_iterator(m_main_border.end());
    }
    Border_vertex_const_iterator  mesh_main_border_vertices_end() const {
        return Border_vertex_const_iterator(m_main_border.end());
    }

    /// Return the border containing seed_vertex.
    /// Return an empty list if not found.
    std::list<Vertex_handle> get_border(Vertex_handle seed_vertex)
    {
        std::list<Vertex_handle> border;    // returned list

        Halfedge_around_vertex_circulator pHalfedge = seed_vertex->vertex_begin();
        Halfedge_around_vertex_circulator end       = pHalfedge;

        // if isolated vertex
        if (pHalfedge == NULL) {
            border.push_back(seed_vertex);
            return border;
        }

        // Get seed_vertex' border halfedge
        Halfedge_handle  seed_halfedge = NULL;
        CGAL_For_all(pHalfedge,end) {
            if(pHalfedge->is_border()) {
                seed_halfedge = pHalfedge;
                break;
            }
        }

        // if inner vertex
        if (seed_halfedge == NULL)
            return border;                  // return empty list

        // Add seed vertex
        border.push_back(seed_vertex);

        // fill border
        int size = 1;
        Halfedge_handle current_halfedge = seed_halfedge;
        do
        {
            // Stop if end of loop
            Halfedge_handle next_halfedge = current_halfedge->next();
            Vertex_handle next_vertex = next_halfedge->vertex();
            if(next_vertex == seed_vertex)
                break;

            // Add vertex
            border.push_back(next_vertex);

            current_halfedge = next_halfedge;
            size++;
        }
        while(1);

        return border;
    }

    /// Get iterator over first facet of mesh.
    Facet_iterator  mesh_facets_begin() {
        return m_polyhedron.facets_begin();
    }
    Facet_const_iterator  mesh_facets_begin() const {
        return m_polyhedron.facets_begin();
    }

    /// Get iterator over past-the-end facet of mesh.
    Facet_iterator  mesh_facets_end() {
        return m_polyhedron.facets_end();
    }
    Facet_const_iterator  mesh_facets_end() const {
        return m_polyhedron.facets_end();
    }

    /// Count the number of facets of the mesh.
    int  count_mesh_facets() const {
        int index = 0;
        for (Facet_const_iterator it=mesh_facets_begin(); it!=mesh_facets_end(); it++)
            index++;
        return index;
    }

    /// Return true of all mesh's facets are triangles.
    bool  is_mesh_triangular() const {
        for (Facet_const_iterator it = mesh_facets_begin(); it != mesh_facets_end(); it++)
            if (count_facet_vertices(it) != 3)
                return false;
        return true;            // mesh is triangular if we reach this point
    }

    /// Count the number of halfedges of the mesh.
    int  count_mesh_halfedges() const {
        int index = 0;
        for (Halfedge_iterator pHalfedge = m_polyhedron.halfedges_begin();
             pHalfedge != m_polyhedron.halfedges_end();
             pHalfedge++)
        {
            index++;
        }
        return index;
    }

    // FACET INTERFACE

    /// Get circulator over facet's vertices.
    Vertex_around_facet_circulator  facet_vertices_begin(Facet_handle facet) {
        return Vertex_around_facet_circulator(facet->facet_begin());
    }
    Vertex_around_facet_const_circulator  facet_vertices_begin(Facet_const_handle facet) const {
        return Vertex_around_facet_const_circulator(facet->facet_begin());
    }

    /// Count the number of vertices of a facet.
    int  count_facet_vertices(Facet_const_handle facet) const {
        int index = 0;
        Vertex_around_facet_const_circulator cir     = facet_vertices_begin(facet),
                                             cir_end = cir;
        CGAL_For_all(cir, cir_end)
            index++;
        return index;
    }

    // VERTEX INTERFACE

    /// Get the 3D position of a vertex
    Point_3 get_vertex_position(Vertex_const_handle vertex) const {
        return vertex->point();
    }

    /// Get/set the 2D position (u/v pair) of a vertex. Default value is undefined.
    /// (stored in halfedges sharing the same vertex).
    Point_2  get_vertex_uv(Vertex_const_handle vertex) const {
        return get_corners_uv(vertex, NULL, NULL);
    }
    void  set_vertex_uv(Vertex_handle vertex, const Point_2& uv) {
        set_corners_uv(vertex, NULL, NULL, uv);
    }

    /// Get/set "is parameterized" field of vertex. Default value is undefined.
    /// (stored in halfedges sharing the same vertex).
    bool  is_vertex_parameterized(Vertex_const_handle vertex) const {
        return are_corners_parameterized(vertex, NULL, NULL);
    }
    void  set_vertex_parameterized(Vertex_handle vertex, bool parameterized) {
        set_corners_parameterized(vertex, NULL, NULL, parameterized);
    }

    /// Get/set vertex index. Default value is undefined.
    /// (stored in Polyhedron vertex for debugging purpose).
    int  get_vertex_index(Vertex_const_handle vertex) const {
        return info(vertex)->index();
    }
    void  set_vertex_index(Vertex_handle vertex, int index)
    {
        info(vertex)->index(index);
    }

    /// Get/set vertex' all purpose tag. Default value is undefined.
    /// (stored in halfedges sharing the same vertex).
    int  get_vertex_tag(Vertex_const_handle vertex) const {
        return get_corners_tag(vertex, NULL, NULL);
    }
    void set_vertex_tag(Vertex_handle vertex, int tag) {
        set_corners_tag(vertex, NULL, NULL, tag);
    }

    /// Return true if a vertex belongs to ANY mesh's border.
    bool  is_vertex_on_border(Vertex_const_handle vertex) const
    {
        Halfedge_around_vertex_const_circulator pHalfedge = vertex->vertex_begin();
        Halfedge_around_vertex_const_circulator end       = pHalfedge;
        if(pHalfedge == NULL) // isolated vertex
            return true;
        CGAL_For_all(pHalfedge,end)
            if(pHalfedge->is_border())
                return true;
        return false;
    }

    /// Return true if a vertex belongs to the UNIQUE mesh's main border,
    /// i.e. the mesh's LONGEST border.
    bool  is_vertex_on_main_border(Vertex_const_handle vertex) const {
        return std::find(m_main_border.begin(),
                         m_main_border.end(),
                         (Vertex*)&*vertex) != m_main_border.end();
    }

    /// Get circulator over the vertices incident to 'vertex'.
    /// 'start_position' defines the optional initial position of the circulator.
    Vertex_around_vertex_circulator vertices_around_vertex_begin(
                            Vertex_handle vertex,
                            Vertex_handle start_position = Vertex_handle())
    {
        if (start_position == NULL)
            return Vertex_around_vertex_circulator(vertex->vertex_begin());
        else
            return Vertex_around_vertex_circulator(
            		Halfedge_around_vertex_circulator(
                        	get_halfedge(start_position, vertex)));
    }
    Vertex_around_vertex_const_circulator vertices_around_vertex_begin(
                            Vertex_const_handle vertex,
                            Vertex_const_handle start_position = Vertex_const_handle()) const
    {
        if (start_position == NULL)
            return Vertex_around_vertex_const_circulator(vertex->vertex_begin());
        else
            return Vertex_around_vertex_const_circulator(
            		Halfedge_around_vertex_const_circulator(
                        	get_halfedge(start_position, vertex)));
    }

    //@} // end of ParameterizationMesh_3 INTERFACE

    //*******************************************************************
    /// @name ParameterizationPatchableMesh_3 INTERFACE
    //*******************************************************************
    //@{

    // VERTEX INTERFACE

    /// Get/set vertex seaming flag. Default value is undefined.
    int  get_vertex_seaming(Vertex_const_handle vertex) const {
        return info(vertex)->seaming();
    }
    void set_vertex_seaming(Vertex_handle vertex, int seaming) {
        info(vertex)->seaming(seaming);
    }

    // EDGE INTERFACE

    /// Get/set oriented edge's seaming flag, i.e. position of the oriented edge
    /// w.r.t. to the UNIQUE main border.
    int  get_halfedge_seaming(Vertex_const_handle source, Vertex_const_handle target) const {
        return info(get_halfedge(source, target))->seaming();
    }
    void set_halfedge_seaming(Vertex_handle source, Vertex_handle target, int seaming) {
        info(get_halfedge(source, target))->seaming(seaming);
    }

    // CORNER INTERFACE

    /// Get/set the 2D position (= (u,v) pair) of corners at the "right"
    /// of the prev_vertex -> vertex -> next_vertex line.
    /// Default value is undefined.
    /// (stored in incident halfedges).
    Point_2 get_corners_uv(Vertex_const_handle vertex,
                           Vertex_const_handle prev_vertex,
                           Vertex_const_handle next_vertex) const
    {
        // if inner vertex
        if (prev_vertex == NULL && next_vertex == NULL)
        {
            // get (u,v) pair from any incident halfedge
            return info(vertex->halfedge())->uv();
        }
        else // if seam vertex
        {
            CGAL_surface_mesh_parameterization_precondition(prev_vertex != NULL);
            CGAL_surface_mesh_parameterization_precondition(next_vertex != NULL);

            // get (u,v) pair from first inner halfedge (clockwise)
            Halfedge_around_vertex_const_circulator cir(
                                get_halfedge(next_vertex, vertex) );
            return info(cir)->uv();
        }
    }
    void set_corners_uv(Vertex_handle vertex,
                        Vertex_const_handle prev_vertex,
                        Vertex_const_handle next_vertex,
                        const Point_2& uv)
    {
        // if inner vertex
        if (prev_vertex == NULL && next_vertex == NULL)
        {
            // Loop over all incident halfedges
            Halfedge_around_vertex_circulator cir     = vertex->vertex_begin(),
                                              cir_end = cir;
            CGAL_For_all(cir, cir_end)
                info(cir)->uv(uv);
        }
        else // if seam vertex
        {
            CGAL_surface_mesh_parameterization_precondition(prev_vertex != NULL);
            CGAL_surface_mesh_parameterization_precondition(next_vertex != NULL);

            // first inner halfedge (for a clockwise rotation)
            Halfedge_around_vertex_circulator cir(
                                get_halfedge((Vertex*)&*next_vertex, vertex) );

            // past-the-end inner halfedge (for a clockwise rotation)
            Halfedge_around_vertex_circulator cir_end(
                                get_halfedge((Vertex*)&*prev_vertex, vertex) );

            // Loop over incident halfedges at the "right"
            // of the prev_vertex -> vertex -> next_vertex line
            CGAL_For_all(cir, cir_end)
                info(cir)->uv(uv);
        }
    }

    /// Get/set "is parameterized" field of corners at the "right"
    /// of the prev_vertex -> vertex -> next_vertex line.
    /// Default value is undefined.
    /// (stored in incident halfedges).
    bool are_corners_parameterized(Vertex_const_handle vertex,
                                   Vertex_const_handle prev_vertex,
                                   Vertex_const_handle next_vertex) const
    {
        // if inner vertex
        if (prev_vertex == NULL && next_vertex == NULL)
        {
            // get "is parameterized" field from any incident halfedge
            return info(vertex->halfedge())->is_parameterized();
        }
        else // if seam vertex
        {
            CGAL_surface_mesh_parameterization_precondition(prev_vertex != NULL);
            CGAL_surface_mesh_parameterization_precondition(next_vertex != NULL);

            // get "is parameterized" field from first inner halfedge (clockwise)
            Halfedge_around_vertex_const_circulator cir(
                                get_halfedge(next_vertex, vertex) );
            return info(cir)->is_parameterized();
        }
    }
    void set_corners_parameterized(Vertex_handle vertex,
                                   Vertex_const_handle prev_vertex,
                                   Vertex_const_handle next_vertex,
                                   bool parameterized)
    {
        // if inner vertex
        if (prev_vertex == NULL && next_vertex == NULL)
        {
            // Loop over all incident halfedges
            Halfedge_around_vertex_circulator cir     = vertex->vertex_begin(),
                                              cir_end = cir;
            CGAL_For_all(cir, cir_end)
                info(cir)->is_parameterized(parameterized);
        }
        else // if seam vertex
        {
            CGAL_surface_mesh_parameterization_precondition(prev_vertex != NULL);
            CGAL_surface_mesh_parameterization_precondition(next_vertex != NULL);

            // first inner halfedge (for a clockwise rotation)
            Halfedge_around_vertex_circulator cir(
                                get_halfedge((Vertex*)&*next_vertex, vertex) );

            // past-the-end inner halfedge (for a clockwise rotation)
            Halfedge_around_vertex_circulator cir_end(
                                get_halfedge((Vertex*)&*prev_vertex, vertex) );

            // Loop over incident halfedges at the "right"
            // of the prev_vertex -> vertex -> next_vertex line
            CGAL_For_all(cir, cir_end)
                info(cir)->is_parameterized(parameterized);
        }
    }

    /// Get/set index of corners at the "right"
    /// of the prev_vertex -> vertex -> next_vertex line.
    /// Default value is undefined.
    /// (stored in incident halfedges).
    int get_corners_index(Vertex_const_handle vertex,
                          Vertex_const_handle prev_vertex,
                          Vertex_const_handle next_vertex) const
    {
        // if inner vertex
        if (prev_vertex == NULL && next_vertex == NULL)
        {
            // get index from any incident halfedge
            return info(vertex->halfedge())->index();
        }
        else // if seam vertex
        {
            CGAL_surface_mesh_parameterization_precondition(prev_vertex != NULL);
            CGAL_surface_mesh_parameterization_precondition(next_vertex != NULL);

            // get index from first inner halfedge (clockwise)
            Halfedge_around_vertex_const_circulator cir(
                                get_halfedge(next_vertex, vertex) );
            return info(cir)->index();
        }
    }
    void set_corners_index(Vertex_handle vertex,
                           Vertex_const_handle prev_vertex,
                           Vertex_const_handle next_vertex,
                           int index)
    {
        // if inner vertex
        if (prev_vertex == NULL && next_vertex == NULL)
        {
            // Loop over all incident halfedges
            Halfedge_around_vertex_circulator cir     = vertex->vertex_begin(),
                                              cir_end = cir;
            CGAL_For_all(cir, cir_end)
                info(cir)->index(index);
        }
        else // if seam vertex
        {
            CGAL_surface_mesh_parameterization_precondition(prev_vertex != NULL);
            CGAL_surface_mesh_parameterization_precondition(next_vertex != NULL);

            // first inner halfedge (for a clockwise rotation)
            Halfedge_around_vertex_circulator cir(
                                get_halfedge((Vertex*)&*next_vertex, vertex) );

            // past-the-end inner halfedge (for a clockwise rotation)
            Halfedge_around_vertex_circulator cir_end(
                                get_halfedge((Vertex*)&*prev_vertex, vertex) );

            // Loop over incident halfedges at the "right"
            // of the prev_vertex -> vertex -> next_vertex line
            CGAL_For_all(cir, cir_end)
                info(cir)->index(index);
        }
    }

    /// Get/set all purpose tag of corners at the "right"
    /// of the prev_vertex -> vertex -> next_vertex line.
    /// Default value is undefined.
    /// (stored in incident halfedges).
    int get_corners_tag(Vertex_const_handle vertex,
                        Vertex_const_handle prev_vertex,
                        Vertex_const_handle next_vertex) const
    {
        // if inner vertex
        if (prev_vertex == NULL && next_vertex == NULL)
        {
            // get tag from any incident halfedge
            return info(vertex->halfedge())->tag();
        }
        else // if seam vertex
        {
            CGAL_surface_mesh_parameterization_precondition(prev_vertex != NULL);
            CGAL_surface_mesh_parameterization_precondition(next_vertex != NULL);

            // get tag from first inner halfedge (clockwise)
            Halfedge_around_vertex_const_circulator cir(
                                get_halfedge(next_vertex, vertex) );
            return info(cir)->tag();
        }
    }
    void set_corners_tag(Vertex_handle vertex,
                         Vertex_const_handle prev_vertex,
                         Vertex_const_handle next_vertex,
                         int tag)
    {
        // if inner vertex
        if (prev_vertex == NULL && next_vertex == NULL)
        {
            // Loop over all incident halfedges
            Halfedge_around_vertex_circulator cir     = vertex->vertex_begin(),
                                              cir_end = cir;
            CGAL_For_all(cir, cir_end)
                info(cir)->tag(tag);
        }
        else // if seam vertex
        {
            CGAL_surface_mesh_parameterization_precondition(prev_vertex != NULL);
            CGAL_surface_mesh_parameterization_precondition(next_vertex != NULL);

            // first inner halfedge (for a clockwise rotation)
            Halfedge_around_vertex_circulator cir(
                                get_halfedge((Vertex*)&*next_vertex, vertex) );

            // past-the-end inner halfedge (for a clockwise rotation)
            Halfedge_around_vertex_circulator cir_end(
                                get_halfedge((Vertex*)&*prev_vertex, vertex) );

            // Loop over incident halfedges at the "right"
            // of the prev_vertex -> vertex -> next_vertex line
            CGAL_For_all(cir, cir_end)
                info(cir)->tag(tag);
        }
    }

    //@} // end of ParameterizationPatchableMesh_3 INTERFACE


// Private operations
private:

    /// Extract mesh's longest border.
    std::list<Vertex_handle> extract_longest_border(Polyhedron& )
    {
        std::list<Vertex_handle> longest_border;    // returned list
        double                   max_len = 0;       // length of longest_border

        // Tag all vertices as unprocessed
        const int tag_free = 0;
        const int tag_done = 1;
        for (Vertex_iterator it=mesh_vertices_begin(); it!=mesh_vertices_end(); it++)
             set_vertex_tag(it, tag_free);

        // find all closed borders and keep longest one
        int nb = 0;
        while (1)
        {
            // Find a border tagged as "free" and tag it as "processed"
            std::list<Vertex_handle> border = find_free_border(tag_free, tag_done);
            if(border.empty())
                break;

            // compute  total len of 'border'
            double len = 0.0;
            typename std::list<Vertex_handle>::const_iterator it;
            for(it = border.begin(); it != border.end(); it++)
            {
                // Get next iterator (looping)
                typename std::list<Vertex_handle>::const_iterator next = it;
                next++;
                if (next == border.end())
                    next = border.begin();

                Vector_3 vect = get_vertex_position(*next) - get_vertex_position(*it);
                len += std::sqrt(vect*vect);
            }

            // Keep 'border' if longer
            if (len > max_len)
            {
                longest_border = border;
                max_len = len;
            }

            nb++;
        }

        return longest_border;
    }

    /// Find a border tagged as "free" and tag it as "processed".
    /// Return an empty list if not found.
    std::list<Vertex_handle> find_free_border(int tag_free, int tag_done)
    {
        std::list<Vertex_handle> border;    // returned list

        // get any border vertex with "free" tag
        Vertex_handle seed_vertex = NULL;
        for (Vertex_iterator pVertex = mesh_vertices_begin();
             pVertex != mesh_vertices_end();
             pVertex++)
        {
            if (is_vertex_on_border(pVertex) && get_vertex_tag(pVertex) == tag_free) {
                seed_vertex = pVertex;
                break;
            }
        }
        if (seed_vertex == NULL)
            return border;                  // return empty list

        // Get the border containing seed_vertex
        border = get_border(seed_vertex);

        // Tag border vertices as "processed"
        typename std::list<Vertex_handle>::iterator it;
        for(it = border.begin(); it != border.end(); it++)
            set_vertex_tag(*it, tag_done);

        return border;
    }


// Fields
private:

    /// The adapted mesh (cannot be NULL).
    Polyhedron&                 m_polyhedron;

    /// Additional info attached to halfedges.
    Halfedge_info_map           m_halfedge_info;
    /// Additional info attached to vertices.
    Vertex_info_map             m_vertex_info;

    /// Main border of a topological disc inside m_polyhedron (may be empty).
    std::list<Vertex_handle>    m_main_border;


// Private types
private:

    /// Functor for operator< for classes lacking this operator.
    struct Less
    {
        /// functor for operator< on Polyhedron::Halfedge_const_handle items
        bool operator()(const Halfedge_const_handle& _Left,
                        const Halfedge_const_handle& _Right) const
        {
            // apply operator< to pointers
            return (&*_Left < &*_Right);
        }

        /// functor for operator< on Polyhedron::Vertex_const_handle items
        bool operator()(const Vertex_const_handle& _Left,
                        const Vertex_const_handle& _Right) const
        {
            // apply operator< to pointers
            return (&*_Left < &*_Right);
        }
    };

    /// Utility class to generate the Vertex_around_facet_circulator type.
    struct Project_halfedge_vertex {
        typedef Halfedge                            argument_type;
        typedef typename Parameterization_polyhedron_adaptor_3::Vertex
                                                    Vertex;
        typedef Vertex                              result_type;

        /// Get the target vertex of a halfedge
        Vertex&       operator()(Halfedge& h)       const {
            return *(h.vertex());
        }
        const Vertex& operator()(const Halfedge& h) const {
            return *(h.vertex());
        }
    };

    /// Utility class to generate the Border_vertex_iterator type.
    struct Project_vertex_handle_vertex {
        typedef Vertex_handle                       argument_type;
        typedef typename Parameterization_polyhedron_adaptor_3::Vertex
                                                    Vertex;
        typedef Vertex                              result_type;

        /// Convert Vertex_handle to Vertex
        Vertex&       operator()(Vertex_handle& vh)       const { return *vh; }
        const Vertex& operator()(const Vertex_handle& vh) const { return *vh; }
    };

    /// This class is used to generate the Vertex_around_vertex_circulator type.
    struct Project_opposite_halfedge_vertex {
        typedef Halfedge                            argument_type;
        typedef typename Parameterization_polyhedron_adaptor_3::Vertex
                                                    Vertex;
        typedef Vertex                              result_type;

        /// Get the source vertex of a halfedge
        Vertex&       operator()(Halfedge& h)       const {
            return *(h.opposite()->vertex());
        }
        const Vertex& operator()(const Halfedge& h) const {
            return *(h.opposite()->vertex());
        }
    };

}; // Parameterization_polyhedron_adaptor_3


} //namespace CGAL

#endif //CGAL_SURFACE_MESH_PARAMETERIZATION_POLYHEDRON_ADAPTOR3_H
