/*
   This code is a modified version of an algorithm
   forming part of the software program Finite
   Element Method Magnetics (FEMM), authored by
   David Meeker. The original software code is
   subject to the Aladdin Free Public Licence
   version 8, November 18, 1999. For more information
   on FEMM see www.femm.info. This modified version
   is not endorsed in any way by the original
   authors of FEMM.

   This software has been modified to use the C++
   standard template libraries and remove all Microsoft (TM)
   MFC dependent code to allow easier reuse across
   multiple operating system platforms.

   Date Modified: 2011 - 11 - 10
   By: Richard Crozier
   Contact: richard.crozier@yahoo.co.uk

   Additional changes:
   Copyright 2016-2017 Johannes Zarl-Zierl <johannes.zarl-zierl@jku.at>
   Contributions by Johannes Zarl-Zierl were funded by Linz Center of
   Mechatronics GmbH (LCM)
*/

#ifndef FMESHER_H
#define FMESHER_H

#include "nosebl.h"
#include "femmcomplex.h"
#include "IntPoint.h"

#include "CArcSegment.h"
#include "CBlockLabel.h"
#include "CBoundaryProp.h"
#include "CCircuit.h"
#include "CNode.h"
#include "CPointProp.h"
#include "CSegment.h"
#include "femmenums.h"
#include "FemmProblem.h"

#include <memory>
#include <vector>
#include <string>

#ifndef LineFraction
#define LineFraction 500.0
#endif

// Default mesh size is the diagonal of the geometry's
// bounding box divided by BoundingBoxFraction
#ifndef BoundingBoxFraction
#define BoundingBoxFraction 100.0
#endif

// forward definition
struct triangulateio;

namespace fmesher
{

// FMesher Class

class FMesher
{

protected:


// Attributes
public:

    enum loaderrors { F_FILE_OK, F_FILE_UNKNOWN_TYPE, F_FILE_NOT_OPENED, F_FILE_MALFORMED};

    explicit FMesher();
    explicit FMesher(std::string);
    explicit FMesher(std::shared_ptr<femm::FemmProblem> p);

    femm::EditMode d_EditMode;
    std::shared_ptr<femm::FemmProblem> problem;
	bool    Verbose;

	std::string BinDir;

	// lists of nodes, segments, and block labels for undo purposes...
    std::vector< std::unique_ptr<femm::CNode> >       undonodelist;
    std::vector< std::unique_ptr<femm::CSegment> >    undolinelist;
    std::vector< std::unique_ptr<femm::CArcSegment> > undoarclist;
    std::vector< std::unique_ptr<femm::CBlockLabel> > undolabellist;

	// vectors containing the mesh information
    std::vector< std::unique_ptr<femm::IntPoint> >      meshline;
    std::vector< std::unique_ptr<femm::IntPoint> >      greymeshline;
    std::vector< std::unique_ptr<femm::CNode> >	meshnode;

    // used to echo start of input file to output
    std::vector< std::string > probdescstrings;

// Operations
public:

    static femm::FileType GetFileType(std::string PathName);
    bool SaveFEMFile(std::string PathName); ///< \deprecated
    bool WriteTriangulationFiles(const struct triangulateio &out, std::string Pathname);

    //void downstr(char *s);

	int ClosestNode(double x, double y);
	int ClosestBlockLabel(double x, double y);
	int ClosestSegment(double x, double y);
	int ClosestArcSegment(double x, double y);
    int GetLineArcIntersection(femm::CSegment &seg, femm::CArcSegment &arc, CComplex *p);
    double ShortestDistanceFromArc(CComplex p, femm::CArcSegment &arc);

    // Core functions
	bool LoadMesh(std::string PathName);
	int DoNonPeriodicBCTriangulation(std::string PathName);
	int DoPeriodicBCTriangulation(std::string PathName);
    /**
     * @brief Create an undo point.
     */
    void UpdateUndo();
    /**
     * @brief Revert data to the undo point.
     */
	void Undo();
	//bool OldOnOpenDocument(LPCTSTR lpszPathName);
	bool HasPeriodicBC();
    void UnselectAll();
	double ShortestDistance(double p, double q, int segm);


    /**
     * @brief EnforcePSLG
     * Makes sure that there are no:
     * 1. no duplicate points
     * 2. no intersections between line segments, lines and arcs, or arcs
     * 3. no duplicate block labels
     * 4. no overlapping lines or arcs.
     *
     * @param tol tolerance
     *
     * \internal
     * We do this by cleaning out the various lists, and rebuilding them
     * using the ``add'' functions that ensure that things come out right.
     */
    void EnforcePSLG(double tol=0);
	void FancyEnforcePSLG(double tol);

    // pointer to function to call when issuing warning messages
    void (*WarnMessage)(const char*);

    // pointer to function to use for triangle to issue warning messages
    int (*TriMessage)(const char * format, ...);

    /**
     * @brief Add a CNode to the problem description.
     * If necessary, adjust existing CSegments or CArcSegments.
     * The method also ensures that a new node can't be put atop of an existing node or a block label.
     * @param x node position x
     * @param y node position y
     * @param d minimum distance to next node or label.
     * @return \c true, if the node could be added, \c false otherwise.
     */
    bool AddNode(double x, double y, double d);
    /**
     * @brief Add a CNode to the problem description.
     * If necessary, adjust existing CSegments or CArcSegments.
     * The method also ensures that a new node can't be put atop of an existing node or a block label.
     *
     * In contrast to the AddNode(double,double,double), this signature needs an existing CNode rvalue reference that is invalidated by this call.
     * Normally, you want to call the other variant.
     *
     * @param node the node to add.
     * @param d minimum distance to next node or label.
     * @return \c true, if the node could be added, \c false otherwise.
     */
    bool AddNode(std::unique_ptr<femm::CNode> &&node, double d);
    /**
     * @brief Add a line (CSegment) to the problem description
     * The method checks for intersections, splits existing lines/arcs accordingly,
     * and adds nodes at intersections as needed.
     * It is ensured that a new line can not be added twice, and that no degenerate lines are added (with start point == end point)
     * @param n0 index of start node
     * @param n1 index of end node
     * @param tol tolerance, i.e. minimum distance between nodes
     * @return \c true, if the line could be added, \c false otherwise.
     */
    bool AddSegment(int n0, int n1, double tol=0.);
    /**
     * @brief Add a line (CSegment) to the problem description
     * The method checks for intersections, splits existing lines/arcs accordingly,
     * and adds nodes at intersections as needed.
     * It is ensured that a new line can not be added twice, and that no degenerate lines are added (with start point == end point)
     * @param n0 index of start node
     * @param n1 index of end node
     * @param parsegm
     * @param tol tolerance, i.e. minimum distance between nodes
     * @return \c true, if the line could be added, \c false otherwise.
     */
    bool AddSegment(int n0, int n1, const femm::CSegment *parsegm, double tol=0.);
    /**
     * @brief Add a block label to the problem description.
     * The method ensures that a block label can not be added on top
     * of another label, node or line.
     * @param x x-coordinate
     * @param y y-coordinate
     * @param d minimum distance to next node or label.
     * @return \c true if the label could be added or a block label already exists at that position, \c false otherwise.
     */
    bool AddBlockLabel(double x, double y, double d);
    /**
     * @brief Add a block label to the problem description.
     * The method ensures that a block label can not be added on top
     * of another label, node or line.
     *
     * In contrast to the AddBlockLabel(double,double,double), this signature needs an existing CBlockLabel rvalue reference that is invalidated by this call.
     * Normally, you want to call the other variant.
     *
     * @param label the label to add
     * @param d minimum distance to next node or label.
     * @return \c true if the label could be added or a block label already exists at that position, \c false otherwise.
     */
    bool AddBlockLabel(std::unique_ptr<femm::CBlockLabel> &&label, double d);
    /**
     * @brief Add an arc segment to the problem description.
     * This method takes care of intersections with other nodes or lines, and splits the arc segment if necessary.
     * No degenerate arc segments (with start point == end point) can be added.
     * @param asegm the proposed arc segment.
     * @param tol tolerance, i.e. minimum distance between arc segment and nodes
     * @return \c true if the arc segment could be added, \c false otherwise.
     */
    bool AddArcSegment(femm::CArcSegment &asegm, double tol=0.);

    /**
     * @brief Mirror copies of the selected objects about the given line.
     * The line is defined by two points along the line.
     * @param x0 x coordinate of point 0
     * @param y0 y coordinate of point 0
     * @param x1 x coordinate of point 1
     * @param y1 y coordinate of point 1
     * @param selector
     */
    void MirrorCopy(double x0, double y0, double x1, double y1, femm::EditMode selector);

    /**
     * @brief Rotate copies of the selected objects of the selected type.
     * Each copy is rotated by the given amount compared to the previous copy.
     * @param c center
     * @param dt (incremental) angle in degrees
     * @param ncopies number of copies
     * @param selector
     */
    void RotateCopy(CComplex c, double dt, int ncopies, femm::EditMode selector);
    /**
     * @brief Rotate the selected objects of the selected type.
     * @param c center
     * @param t angle
     * @param selector
     */
    void RotateMove(CComplex c, double t, femm::EditMode selector);

    /**
     * @brief Scale the selected objects relative to a base point
     * @param bx base-point x
     * @param by base-point y
     * @param sf scale factor
     * @param selector
     */
    void ScaleMove(double bx, double by, double sf, femm::EditMode selector);

    /**
     * @brief Translate copies of the selected objects of the selected type.
     * Each copy is translated by the given amount compared to the previous copy.
     * @param incx offset
     * @param incy offset
     * @param ncopies number of copies
     * @param selector
     */
    void TranslateCopy(double incx, double incy, int ncopies, femm::EditMode selector);
    /**
     * @brief Translate the selected objects of the selected type.
     * @param dx
     * @param dy
     * @param selector
     */
    void TranslateMove(double dx, double dy, femm::EditMode selector);
    /**
     * @brief Delete all selected segments.
     * @return \c true, if any segments were deleted, \c false otherwise.
     */
    bool DeleteSelectedSegments();
    /**
     * @brief Delete all selected arc segments
     * @return \c true, if any segments were deleted, \c false otherwise.
     */
    bool DeleteSelectedArcSegments();
    /**
     * @brief Delete all selected BlockLabels
     * @return \c true, if any blocks were deleted, \c false otherwise.
     */
    bool DeleteSelectedBlockLabels();
    /**
     * @brief Delete all selected nodes.
     * If a node is part of a line or arc, that line or arc is also deleted.
     * @return \c true, if any node was deleted, \c false otherwise.
     */
    bool DeleteSelectedNodes();

    /**
     * @brief Check whether the given node is suitable for creating a radius.
     * @param n node index
     * @return \c true if the node can be replaced with a radius, \c false otherwise
     */
    bool CanCreateRadius(int n);
    /**
     * @brief Create an arc in place of the given node.
     * @param n the node index
     * @param r the arc radius
     * @return \c true if the arc could be created, \c false otherwise
     */
    bool CreateRadius(int n, double r);
private:

    virtual bool Initialize(femm::FileType t);
	void addFileStr (char * q);

};

} // namespace femm

#endif
