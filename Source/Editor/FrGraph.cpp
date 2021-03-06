/*=============================================================================
    FrGraph.cpp: Path graph builder.
    Copyright Sep.2016 Vlad Gordienko.
	Totally rewritten Jan.2017.
=============================================================================*/

#include "Editor.h"

namespace flu
{
namespace navi
{

/*-----------------------------------------------------------------------------
    Magic numbers.
-----------------------------------------------------------------------------*/

//
// Path building numbers.
//
#define	PIN_BASE			0.6f			// Height of pin over surface.
#define PIN_FALL_OFFSET		3.f				// Offset from pin to trace fall.
#define FALL_MAX_LEN		64.f			// Maximum length of fall.
#define PIN_SAME			1.5f			// Threshold for pins merging.
#define WALK_HEIGHT			1.2f			// Walk step height.
#define WALK_STEP			2.f				// Constant step size to check floor.
#define MAX_JUMP_X			16.f			// Maximum length of jump.
#define JUMP_WEIGHT			1.6f			// Jumping cost.
#define MAX_HULL_HEIGHT		64.f			// Ceiling maximum height.


/*-----------------------------------------------------------------------------
    TPin.
-----------------------------------------------------------------------------*/

//
// Pins flags.
//
#define PIN_None			0x0000
#define PIN_Left			0x0001
#define PIN_Right			0x0002
#define PIN_Middle			0x0004
#define PIN_Surface			0x0008
#define PIN_Fall			0x0010
#define PIN_Grouped			0x0020
#define PIN_Marker			0x0040


//
// Pin is a temporal spot during path building.
//
struct TPin
{
public:
	// Variables.
	UInt32				Flags;
	math::Vector		Location;
	math::Vector		Normal;
	FBrushComponent*	Floor;
	Int32				iEdges[MAX_EDGES_PER_NODE];
	Int32				iGroup;

	// Pin initialization.
	TPin()
	{
		Flags		= PIN_None;
		Location	= math::Vector( 0.f, 0.f );
		Normal		= math::Vector( 0.f, 1.f );
		Floor		= nullptr;
		iGroup		= -1;
		for( Int32 i=0; i<MAX_EDGES_PER_NODE; i++ )
			iEdges[i]	= INVALID_EDGE;
	}

	// Pin to Node convertation.
	PathNode ToPathNode() const
	{
		PathNode Node;
		Node.location		= Location;
		Node.marker			= nullptr;
		mem::copy( Node.iEdges, iEdges, sizeof(iEdges) );
		return Node;
	}

	// Return true, if we can link this node.
	Bool CanAddEdge() const
	{
		for( Int32 i=0; i<MAX_EDGES_PER_NODE; i++ )
			if( iEdges[i] == INVALID_EDGE )
				return true;
		return false;
	}

	// Add a new edge from the pin.
	Bool AddEdge( Int32 iEdge )
	{
		for( Int32 i=0; i<MAX_EDGES_PER_NODE; i++ )
			if( iEdges[i] == INVALID_EDGE )
			{
				iEdges[i]	= iEdge;
				return true;
			}
		return false;
	}
};


//
// A group of pins.
//
struct TPinGroup
{
public:
	// Variables.
	Array<Int32>		iPins;
	Array<Int32>		iLinked;
	math::Rect			Bounds;
	FBrushComponent*	Floor;

	// Group initialization.
	TPinGroup()
		:	iPins(),
			iLinked(),
			Bounds(),
			Floor(nullptr)
	{}
};


/*-----------------------------------------------------------------------------
    CPathBuilder.
-----------------------------------------------------------------------------*/

//
// A class to build paths on level.
//
class CPathBuilder
{
public:
	// Variables.
	FLevel*						Level;
	Navigator*					Navigator;
	Array<FBrushComponent*>		BrushList;		
	Array<TPin>					Pins;
	Array<TPinGroup>			Groups;

	// CPathBuilder interface.
	CPathBuilder( FLevel* InLevel );
	~CPathBuilder();
	void BuildNetwork( IProgressIndicator* Indicator );
	void DestroyNetwork();

	// Pins functions.
	void CreateSurfacePins();
	void CreateFallPins();
	void CreateMarkerPins();
	void MergePins();
	void GroupPins();

	// Pins linking functions.
	void LinkWalkable();
	void LinkJumpable();
	void LinkSpecial();
	Bool IsLinked1( Int32 iGroup1, Int32 iGroup2 );
	Bool IsLinked2( Int32 iGroup1, Int32 iGroup2 );

	// Exploration.
	void ExplorePaths();

	// Tracing functions.
	FBrushComponent* TestPoint( const math::Vector& P );
	FBrushComponent* TestLine
	( 
		Bool bFast,
		const math::Vector& From, 
		const math::Vector& To, 
		math::Vector& Hit, 
		math::Vector& Normal 
	);
};


/*-----------------------------------------------------------------------------
    Coming Soon.
-----------------------------------------------------------------------------*/

void CPathBuilder::CreateMarkerPins()
{
}
void CPathBuilder::LinkSpecial()
{
}


/*-----------------------------------------------------------------------------
    Linking.
-----------------------------------------------------------------------------*/

//
// Link pins that are walkable.
//
void CPathBuilder::LinkWalkable()
{
	// Try to connect groups.
	for( Int32 g1=0; g1<Groups.size(); g1++ )
	for( Int32 g2=g1+1; g2<Groups.size(); g2++ )
	{
		TPinGroup&	Group1	= Groups[g1];
		TPinGroup&	Group2	= Groups[g2];
		math::Rect	AABB1	= Group1.Bounds;
		math::Rect	AABB2	= Group2.Bounds;
		Bool		bOnLeft	= false;

		if( AABB2.max.x < AABB1.min.x+0.5f )
		{
			// Group2 lies on left of Group1.
			bOnLeft	= true;
		}
		else if( AABB2.min.x > AABB1.max.x-0.5f )
		{
			// Group2 lies on right of Group1.
			bOnLeft	= false;
		}
		else
			continue;

		// Select 'best' pins to link them and their groups.
		Int32 iBest1	= Group1.iPins[bOnLeft ? 0 : Group1.iPins.size()-1];
		Int32 iBest2	= Group2.iPins[bOnLeft ? Group2.iPins.size()-1 : 0];
		TPin&	Best1	= Pins[iBest1];
		TPin&	Best2	= Pins[iBest2];

		// Trace line along walk dir.
		math::Vector HitPoint, HitNormal;
		FBrushComponent* Obstacle = TestLine
		(
			true,
			Best1.Location,
			Best2.Location,
			HitPoint,
			HitNormal
		);

		// Path is obstructed, no walking.
		if( Obstacle != nullptr )
			continue;

		// Walk along the path and check for floor.
		FBrushComponent*	Floor;
		math::Vector PathDelta	= Best2.Location - Best1.Location;
		Int32	NumSteps	= max( 1, math::floor(PathDelta.size() / WALK_STEP) );
		math::Vector Walk		= Best1.Location;
		PathDelta			*= 1.f / NumSteps;

		for( Int32 k=0; k<NumSteps; k++ )
		{
			Floor	= TestLine
			(
				false,
				Walk,
				Walk - math::Vector( 0.f, WALK_HEIGHT ),
				HitPoint,
				HitNormal
			);

			if( Floor && ((HitPoint-Walk).sizeSquared() > PIN_BASE) )
				Floor	= nullptr;

			if( !Floor )
				break;

			Walk	+= PathDelta;
		}

		if( Floor )
		{
			// Floor detected along entire path. Path is
			// walkable.
			PathEdge	Edge;
			Edge.iStartNode	= iBest1;
			Edge.iEndNode	= iBest2;
			Edge.type		= EPathType::Walk;
			Edge.cost		= math::floor(abs(Best1.Location.x-Best2.Location.x));

			// Add to navigator.
			if( Best1.CanAddEdge() ) Best1.AddEdge(Navigator->m_edges.push(Edge));
			exchange( Edge.iStartNode, Edge.iEndNode );
			if( Best2.CanAddEdge() ) Best2.AddEdge(Navigator->m_edges.push(Edge));

			// Link groups.
			Group1.iLinked.push(g2);
			Group2.iLinked.push(g1);
		}
	}
}

//
// Link pins that are jumpable and/or fallable.
//
void CPathBuilder::LinkJumpable()
{
	// Try to connect groups.
	for( Int32 g1=0; g1<Groups.size(); g1++ )
	for( Int32 g2=g1+1; g2<Groups.size(); g2++ )
	{
		TPinGroup&	Group1	= Groups[g1];
		TPinGroup&	Group2	= Groups[g2];

		// Don't connect group, if it already linked with
		// a walk or special paths.
		if( IsLinked1( g1, g2 ) )
			continue;

		// It's a list of edges to link groups.
		Array<PathEdge>	Links[2];

		// Try each pair from both groups.
		for( Int32 p1=0; p1<Group1.iPins.size(); p1++ )
		for( Int32 p2=0; p2<Group2.iPins.size(); p2++ )
		{
			Int32	iLower, iUpper;
			TPin&	Pin1 = Pins[Group1.iPins[p1]];
			TPin&	Pin2 = Pins[Group2.iPins[p2]];

			// Sort pins by height.
			if( Pin1.Location.y > Pin2.Location.y )
			{
				iUpper	= Group1.iPins[p1];
				iLower	= Group2.iPins[p2];
			}
			else
			{
				iUpper	= Group2.iPins[p2];
				iLower	= Group1.iPins[p1];
			}

			// Get those pins.
			TPin&	Upper	= Pins[iUpper];
			TPin&	Lower	= Pins[iLower];

			// Can we jump off from upper pin?
			if( !(Upper.Flags & (PIN_Left | PIN_Right)) )
				continue;
			Bool bLeft	= Upper.Flags & PIN_Left;

			// Compute initial jump spot.
			math::Vector	Tangent	= -Upper.Normal.cross();
			math::Vector	Dir		= Lower.Location - Upper.Location;
			math::Vector	JumpSpot, UnusedPoint, UnusedNormal;
			if( bLeft )
				JumpSpot	= Upper.Location + (Upper.Normal-Tangent) * PIN_BASE * 1.f;
			else
				JumpSpot	= Upper.Location + (Upper.Normal+Tangent) * PIN_BASE * 1.f;

			// Make sure, we can jump.
			if	(
					( bLeft ? Dir.x < 0.f : Dir.x > 0.f ) &&
					( abs(Dir.x) <= MAX_JUMP_X ) &&
					TestPoint(JumpSpot) == nullptr
				)
			{
				// Trace directory.
				FBrushComponent* Obstacle = TestLine
				(
					true, 
					JumpSpot,
					Lower.Location,
					UnusedPoint,
					UnusedNormal
				);

				// Add edge to temporal list of links.
				if( !Obstacle )
				{
					PathEdge	Edge;
					Edge.iStartNode	= iUpper;
					Edge.iEndNode	= iLower;
					Edge.type		= EPathType::Jump;
					Edge.cost		= math::floor(JUMP_WEIGHT*(abs(Dir.x)+abs(Dir.y)));

					Links[bLeft].push(Edge);
				}
			}
		}

		// Add links to both directions.
		for( Int32 d=0; d<2; d++ )
			if( Links[d].size() > 0 )
			{
				// Sort links by distance.
				Links[d].sort( []( const PathEdge& a, const PathEdge& b )->Bool
				{
					return a.cost < b.cost;
				} );

				PathEdge Edge	= Links[d][max( (Links[d].size())/2-1, 0 )];

				if( Pins[Edge.iStartNode].CanAddEdge() ) Pins[Edge.iStartNode].AddEdge(Navigator->m_edges.push(Edge));
				exchange( Edge.iStartNode, Edge.iEndNode );
				if( Pins[Edge.iStartNode].CanAddEdge() ) Pins[Edge.iStartNode].AddEdge(Navigator->m_edges.push(Edge));
			}

		// Link groups.
		if( Links[0].size()+Links[1].size() > 0 )
		{		
			Group1.iLinked.push(g2);
			Group2.iLinked.push(g1);
		}
	}
}

/*-----------------------------------------------------------------------------
    Path exploration.
-----------------------------------------------------------------------------*/

//
// Explore all paths on level and compute hull wide, to
// prevent tall AI walk through the narrow pass, and etc.
//
void CPathBuilder::ExplorePaths()
{
	for( Int32 iEdge=0; iEdge<Navigator->m_edges.size(); iEdge++ )
	{
		PathEdge& Edge = Navigator->m_edges[iEdge];

		// Explore only walkable, since jumping is to unpredictable, and
		// others are available for all kind of AI.
		if( Edge.type != EPathType::Walk )
			continue;

		// Walk along the edge and trace line up.
		Edge.breadth		= MAX_HULL_HEIGHT;
		math::Vector	A	= Navigator->m_nodes[Edge.iStartNode].location,
				B			= Navigator->m_nodes[Edge.iEndNode].location,
				Delta		= B - A,
				Walk		= A;
		Int32	NumSteps	= max( 1, math::floor(Delta.size()) );

		Delta	*= 1.f/NumSteps;

		for( Int32 i=0; i<NumSteps; i++ )
		{
			math::Vector HitPoint, HitNormal;
			if( TestLine
				(
					true,
					Walk,
					math::Vector( Walk.x, Walk.y+MAX_HULL_HEIGHT ),
					HitPoint,
					HitNormal
				) )
			{
				Float	TestHeight = abs(HitPoint.y - Walk.y) + PIN_BASE;

				if( Edge.breadth > TestHeight )
						Edge.breadth	= TestHeight;
			}
			Walk += Delta;
		}
	}
}


/*-----------------------------------------------------------------------------
    Pins placement & service.
-----------------------------------------------------------------------------*/

//
// Create pins above walkable brushes surfaces.
//
void CPathBuilder::CreateSurfacePins()
{
	for( Int32 b=0; b<BrushList.size(); b++ )
	{
		FBrushComponent* Brush = BrushList[b];

		// Winding the brush.
		math::Vector P2, P1 = Brush->Vertices[Brush->NumVerts-1];
		for( Int32 i=0; i<Brush->NumVerts; i++ )
		{
			// Information about current edge.
			P2	= Brush->Vertices[i];
			math::Vector	Tanget	= P2 - P1;
			Tanget.normalize();
			math::Vector Normal	= Tanget.cross();

			// Place only above walkable surfaces.
			if( phys::isWalkableSurface(Normal) )
			{
				// Place left one.
				math::Vector Left	= P1 + (Normal+Tanget) * PIN_BASE + Brush->Location;
				if( TestPoint(Left) == nullptr )
				{
					TPin	Pin;
					Pin.Flags		= PIN_Left | PIN_Surface;
					Pin.Location	= Left;
					Pin.Normal		= Normal;
					Pin.Floor		= Brush;
					Pins.push( Pin );
				}

				// Place right one.
				math::Vector Right	= P2 + (Normal-Tanget) * PIN_BASE + Brush->Location;
				if( TestPoint(Right) == nullptr )
				{
					TPin	Pin;
					Pin.Flags		= PIN_Right | PIN_Surface;
					Pin.Location	= Right;
					Pin.Normal		= Normal;
					Pin.Floor		= Brush;
					Pins.push( Pin );
				}
			}

			P1	= P2;
		}
	}
}


//
// Trace line downward from every pin and 
// try to create 'jump off target' pin.
//
void CPathBuilder::CreateFallPins()
{
	Int32 NumSource = Pins.size();

	for( Int32 i=0; i<NumSource; i++ )
	{
		TPin&	Source	= Pins[i];
		math::Vector	Tangent	= -Source.Normal.cross();

		// Compute jump 'from' and 'to' points.
		assert(Source.Flags & (PIN_Left | PIN_Right));
		math::Vector From, To;
		if( Source.Flags & PIN_Left )
		{
			// From left.
			From	= Source.Location - Tangent * PIN_FALL_OFFSET;
		}
		if( Source.Flags & PIN_Right )
		{
			// From right.
			From	= Source.Location + Tangent * PIN_FALL_OFFSET;
		}
		To	= math::Vector( From.x, From.y-FALL_MAX_LEN );

		// Trace line.
		math::Vector HitNormal, HitPoint;
		FBrushComponent* HitBrush;
		HitBrush	= TestLine( false, From, To, HitPoint, HitNormal );

		if	( 
				HitBrush && 
				HitBrush != Source.Floor && 
				phys::isWalkableSurface(HitNormal) &&
				abs(HitPoint.y-From.y) > PIN_BASE*1.5f
			)
		{
			// Create new pin.
			TPin	Pin;
			Pin.Flags		= PIN_Middle | PIN_Fall;
			Pin.Location	= HitPoint + HitNormal * PIN_BASE;
			Pin.Normal		= HitNormal;
			Pin.Floor		= HitBrush;
			Pins.push( Pin );
		}
	}
}


//
// Merge all nearst pins.
//
void CPathBuilder::MergePins()
{
	Bool bAgain = true;

	while( bAgain )
	{
		bAgain	= false;

		for( Int32 i=0; i<Pins.size(); i++ )
			for( Int32 j=i+1; j<Pins.size(); j++ )
			{
				TPin&	Pin1 = Pins[i];
				TPin&	Pin2 = Pins[j];

				if( (Pin2.Location-Pin1.Location).sizeSquared() <= PIN_SAME*PIN_SAME )
				{
					// Marge them.
					Pin1.Flags		|= Pin2.Flags;
					Pin1.Location	= (Pin1.Location + Pin2.Location) * 0.5001f;
					Pin1.Normal		= (Pin1.Normal + Pin2.Normal) * 0.5f;
					Pin1.Floor		= Pin1.Floor ? Pin1.Floor : Pin2.Floor;

					Pins.removeFast(j);
					bAgain	= true;
				}
			}
	}
}


// 
// Pins X-sort function.
// Little hack, but it local.
//
static CPathBuilder*	GBuilder;
Bool PinXCmp( const Int32& A, const Int32& B )
{
	return GBuilder->Pins[A].Location.x < GBuilder->Pins[B].Location.x;
}


//
// Group pins into groups.
//
void CPathBuilder::GroupPins()
{
	for( Int32 b=0; b<BrushList.size(); b++ )
	{
		FBrushComponent* Brush	= BrushList[b];

		// Collect pins being this brush.
		Array<Int32>	List;
		for( Int32 i=0; i<Pins.size(); i++ )
			if( Brush == Pins[i].Floor )
				List.push( i );

		if( List.size() == 0 )
			continue;

		// Sort pins for next their connection from left
		// to right.
		GBuilder	= this;
		List.sort(PinXCmp);

		// Start connect them and make groups.
		TPinGroup	Group;
		Group.Floor		= Brush;
		Group.iPins.push(List[0]);
		for( Int32 i=1; i<List.size(); i++ )
		{
			math::Vector	UnusedHit, UnusedNormal;
			Int32	iFrom	= List[i-1],
					iTo		= List[i];
			TPin&	From	= Pins[iFrom];
			TPin&	To		= Pins[iTo];

			// Try to walk.
			if( TestLine( true, From.Location, To.Location, UnusedHit, UnusedNormal ) )
			{
				// Hit obstacle, so create a new group.
				Groups.push(Group);

				Group.iPins.empty();
				Group.iPins.push(iTo);
			}
			else
			{
				// Path free, so pave the path.
				Group.iPins.push( iTo );
			}
		}
		Groups.push( Group );
	}

	// Build AABB bounds for all groups.
	for( Int32 g=0; g<Groups.size(); g++ )
	{
		TPinGroup& Group = Groups[g];

		Group.Bounds.min	=
		Group.Bounds.max	= Pins[Group.iPins[0]].Location;
		for( Int32 i=1; i<Group.iPins.size(); i++ )
			Group.Bounds += Pins[Group.iPins[i]].Location;
	}

	// Sort groupes to process pathbuilding from 
	// left to right.
	Groups.sort( []( const TPinGroup& a, const TPinGroup& b )->Bool
	{
		return a.Bounds.min.x < b.Bounds.min.x;
	} );

	// Assign to each pin it Group.
	for( Int32 i=0; i<Groups.size(); i++ )
	{
		TPinGroup& Group = Groups[i];
		for( Int32 j=0; j<Group.iPins.size(); j++ )
			Pins[Group.iPins[j]].iGroup	= i;
	}

	// Add inner group links.
	for( Int32 g=0; g<Groups.size(); g++ )
		if( Groups[g].iPins.size() > 1 )
		{
			TPinGroup& Group = Groups[g];

			for( Int32 i=0; i<Group.iPins.size()-1; i++ )
			{
				PathEdge	Edge;
				Edge.iStartNode	= Group.iPins[i];
				Edge.iEndNode	= Group.iPins[i+1];
				Edge.type		= EPathType::Walk;
				Edge.cost		= math::floor(abs(Pins[Edge.iStartNode].Location.x-Pins[Edge.iEndNode].Location.x));

				Pins[Edge.iStartNode].AddEdge(Navigator->m_edges.push(Edge));
				exchange( Edge.iStartNode, Edge.iEndNode );
				Pins[Edge.iStartNode].AddEdge(Navigator->m_edges.push(Edge));
			}
		}
}


/*-----------------------------------------------------------------------------
    CPathBuilder implementation.
-----------------------------------------------------------------------------*/

//
// Builder constructor.
//
CPathBuilder::CPathBuilder( FLevel* InLevel )
{
	assert(InLevel);
	Level	= InLevel;
	Navigator = &Level->m_navigator;
}


//
// Builder destructor.
//
CPathBuilder::~CPathBuilder()
{
}


//
// Path destructor
//
void CPathBuilder::DestroyNetwork()
{
	Navigator->m_edges.empty();
	Navigator->m_nodes.empty();
}


//
// Path Builder main function.
//
void CPathBuilder::BuildNetwork( IProgressIndicator* Indicator )
{
	// Destroy old navigator.
	DestroyNetwork();

	// Collect list of brushes.
	for( Int32 i=0; i<Level->Entities.size(); i++ )
	{
		FBaseComponent* Base = Level->Entities[i]->Base;

		if( Base->IsA(FBrushComponent::MetaClass) )
		{
			FBrushComponent* Brush	= (FBrushComponent*)Base;
			if( Brush->Type != BRUSH_NotSolid )
				BrushList.push( Brush );
		}
	}

	// Zero statistics.
	Int32	PinsNoMerge		= 0,
			NumEdges		= 0,
			NumNodes		= 0,
			NumGroups		= 0,
			LonelyGroups	= 0;

	{
		IProgressIndicator::THolder Ind( Indicator, L"Path Building" );

		// Place pins and conjure them.
		Ind.UpdateDetails(L"Placing Pins...");
		Ind.SetProgress( 0, 5 );
			CreateSurfacePins();
		Ind.SetProgress( 1, 5 );
			CreateMarkerPins();
		Ind.SetProgress( 2, 5 );
			CreateFallPins();
			PinsNoMerge	= Pins.size();
		Ind.SetProgress( 3, 5 );
			MergePins();
		Ind.SetProgress( 4, 5 );
			GroupPins();

		// Link them and allocate edges.
		Ind.UpdateDetails(L"Linking...");
		Ind.SetProgress( 0, 3 );
			LinkWalkable();
		Ind.SetProgress( 1, 3 );
			LinkSpecial();
		Ind.SetProgress( 2, 3 );
			LinkJumpable();

		// Turn pins into nodes.
		for( Int32 i=0; i<Pins.size(); i++ )
			Navigator->m_nodes.push( Pins[i].ToPathNode() );

		// Explore just created paths.
		Ind.UpdateDetails(L"Exploration...");
		Ind.SetProgress( 1, 2 );
			ExplorePaths();
	}

	// Validation
	if( Navigator->getNodesCount() > MAX_NODES || Navigator->getEdgesCount() > MAX_EDGES )
	{
		String message = String::format( 
			L"Failed to build navigation network, navigation graph is too large: %d/%d nodes, %d/%d edges",
			Navigator->getNodesCount(), MAX_NODES, Navigator->getEdgesCount(), MAX_EDGES );

		GEditor->GUIWindow->ShowMessage( *message, L"Path Builder" );
		error( *message );
		DestroyNetwork();
		return;
	}

	// Count stats.
	NumEdges		= Navigator->getEdgesCount();
	NumNodes		= Navigator->getNodesCount();
	NumGroups		= Groups.size();

	for( Int32 g=0; g<Groups.size(); g++ )
		if( Groups[g].iLinked.size() == 0 )
			LonelyGroups++;

	// Out info.
	debug( L"Path building in '%s'", *Level->GetFullName() );
	debug( L"Initially placed %d pins", PinsNoMerge );
	debug( L"%d sectors found. %d is unlinked", NumGroups, LonelyGroups );
	debug( L"Create %d nodes and %d edges", NumNodes, NumEdges );
}


/*-----------------------------------------------------------------------------
    Utility.
-----------------------------------------------------------------------------*/

//
// Return true, if two groupes linked directyly.
//
Bool CPathBuilder::IsLinked1( Int32 iGroup1, Int32 iGroup2 )
{
	TPinGroup& Group = Groups[iGroup1];
	for( Int32 i=0; i<Group.iLinked.size(); i++ )
		if( Group.iLinked[i] == iGroup2 )
			return true;

	return false;
}


//
// Return true, if two Groups linked directly, or via just one
// shared neighbor group.
//
Bool CPathBuilder::IsLinked2( Int32 iGroup1, Int32 iGroup2 )
{
	// Test directly.
	if( IsLinked1( iGroup1, iGroup2 ) )
		return true;

	// Slow, slow, slow test..
	TPinGroup& Group = Groups[iGroup1];
	for( Int32 i=0; i<Group.iLinked.size(); i++ )
		if( IsLinked1( Group.iLinked[i], iGroup2 ) )
			return true;

	return false;
}


//
// Test a point with level's geometry. Return brush where
// point resides in, or nullptr, if point outside of any brush.
//
FBrushComponent* CPathBuilder::TestPoint( const math::Vector& P )
{
	for( Int32 b=0; b<BrushList.size(); b++ )
		if( BrushList[b]->Type == BRUSH_Solid )
		{
			// Transform point to Brush's local coords system.
			FBrushComponent* Brush = BrushList[b];
			math::Vector	Local	= P - Brush->Location;

			if( math::isPointInsidePoly( Local, Brush->Vertices, Brush->NumVerts ) )
				return Brush;
		}

	// Nothing found.
	return nullptr;
}


//
// Test a line with a level's geometry. If line hits nothing
// return nullptr. Otherwise return brush with hit, hit
// location and hit normal. bFast - only check fact of hit,
// without addition information.
//
FBrushComponent* CPathBuilder::TestLine
( 
	Bool bFast,
	const math::Vector& From, 
	const math::Vector& To, 
	math::Vector& OutHit, 
	math::Vector& OutNormal 
)
{
	FBrushComponent*	Result		= nullptr;
	Float	TestDist, BestDist		= 1000000.f;

	for( Int32 b=0; b<BrushList.size(); b++ )
	{
		FBrushComponent* Brush = BrushList[b];

		// To brush local coords system.
		math::Vector	Normal, Hit;
		math::Vector	LocalFrom	= From - Brush->Location,
				LocalTo		= To - Brush->Location;

		if( math::isLineIntersectPoly( LocalFrom, LocalTo, Brush->Vertices, Brush->NumVerts, &Hit, &Normal ) )
		{
			if	(
					(Brush->Type == BRUSH_Solid) ||
					(Brush->Type == BRUSH_SemiSolid && phys::isWalkableSurface(Normal))
				)
			{
				Hit			+= Brush->Location;
				TestDist	= (Hit - From).sizeSquared();

				if( TestDist < BestDist )
				{
					OutHit		= Hit;
					OutNormal	= Normal;
					BestDist	= TestDist;
					Result		= Brush;

					if( bFast )
						return Result;
				}
			}
		}
	}

	return Result;
}


} // namespace navi
} // namespace flu

/*-----------------------------------------------------------------------------
    Global path routines.
-----------------------------------------------------------------------------*/

//
// Build navigation network for level.
//
void CEditor::BuildPaths( FLevel* Level )
{
	navi::CPathBuilder Builder( Level );
	Builder.BuildNetwork( TaskDialog );
}


//
// Destroy a level's navigation network.
//
void CEditor::DestroyPaths( FLevel* Level )
{
	navi::CPathBuilder Builder( Level );
	Builder.DestroyNetwork();
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/