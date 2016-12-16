#include "CArcSegment.h"

#include <cstdlib>
#include <cmath>
#include "fullmatrix.h"
#include "femmcomplex.h"
#include "femmconstants.h"

#define ElementsPerSkinDepth 10

using namespace std;
using namespace femm;

// CArcSegment construction
CArcSegment::CArcSegment()
    : n0(0)
    , n1(0)
    , ArcLength(90.)
    , MaxSideLength(-1)
    , Hidden(false)
    , InGroup(0)
    , IsSelected(false)
    , BoundaryMarker(-1)
    , InConductor(-1)
    , BoundaryMarkerName("<None>")
    , InConductorName("<None>")
    , NormalDirection(true)
    , cnt(0)
{
}

void CArcSegment::ToggleSelect()
{
    IsSelected = !IsSelected;
}
