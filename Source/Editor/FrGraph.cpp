/*=============================================================================
    FrGraph.cpp: Path graph builder.
    Copyright Sep.2016 Vlad Gordienko.
	Totally rewritten Jan.2017.
=============================================================================*/

#include "Editor.h"

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
	TVector				Location;
	TVector				Normal;
	FBrushComponent*	Floor;
	Int32				iEdges[TPathNode::NUM_EDGES];
	Int32				iGroup;

	// Pin initialization.
	TPin()
	{
		Flags		= PIN_None;
		Location	= TVector( 0.f, 0.f );
		Normal		= TVector( 0.f, 1.f );
		Floor		= nullptr;
		iGroup		= -1;
		for( Int32 i=0; i<TPathNode::NUM_EDGES; i++ )
			iEdges[i]	= -1;
	}

	// Pin to Node convertation.
	TPathNode ToPathNode() const
	{
		TPathNode Node;
		Node.Location		= Location;
		Node.Weight			= 0;
		Node.Marker			= nullptr;
		mem::copy( Node.iEdges, iEdges, sizeof(iEdges) );
		return Node;
	}

	// Return true, if we can link this node.
	Bool CanAddEdge() const
	{
		for( Int32 i=0; i<TPathNode::NUM_EDGES; i++ )
			if( iEdges[i] == -1 )
				return true;
		return false;
	}

	// Add a new edge from the pin.
	Bool AddEdge( Int32 iEdge )
	{
		for( Int32 i=0; i<TPathNode::NUM_EDGES; i++ )
			if( iEdges[i] == -1 )
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
	TRect				Bounds;
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
	CNavigator*					Navigator;
	Array<FBrushComponent*>		BrushList;		
	Array<TPin>					Pins;
	Array<TPinGroup>			Groups;

	// CPathBuilder interface.
	CPathBuilder( FLevel* InLevel );
	~CPathBuilder();
	void BuildNetwork( IProgressIndicator* Indicator );

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
	FBrushComponent* TestPoint( const TVector& P );
	FBrushComponent* TestLine
	( 
		Bool bFast,
		const TVector& From, 
		const TVector& To, 
		TVector& Hit, 
		TVector& Normal 
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
		TRect		AABB1	= Group1.Bounds;
		TRect		AABB2	= Group2.Bounds;
		Bool		bOnLeft	= false;

		if( AABB2.Max.X < AABB1.Min.X+0.5f )
		{
			// Group2 lies on left of Group1.
			bOnLeft	= true;
		}
		else if( AABB2.Min.X > AABB1.Max.X-0.5f )
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
		TVector HitPoint, HitNormal;
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
		TVector	PathDelta	= Best2.Location - Best1.Location;
		Int32	NumSteps	= Max( 1, ::Floor(PathDelta.Size() / WALK_STEP) );
		TVector	Walk		= Best1.Location;
		PathDelta			*= 1.f / NumSteps;

		for( Int32 k=0; k<NumSteps; k++ )
		{
			Floor	= TestLine
			(
				false,
				Walk,
				Walk - TVector( 0.f, WALK_HEIGHT ),
				HitPoint,
				HitNormal
			);

			if( Floor && ((HitPoint-Walk).SizeSquared() > PIN_BASE) )
				Floor	= nullptr;

			if( !Floor )
				break;

			Walk	+= PathDelta;
		}

		if( Floor )
		{
			// Floor detected along entire path. Path is
			// walkable.
			TPathEdge	Edge;
			Edge.iStart		= iBest1;
			Edge.iFinish	= iBest2;
			Edge.PathType	= PATH_Walk;
			Edge.Cost		= ::Floor(Abs(Best1.Location.X-Best2.Location.X));

			// Add to navigator.
			if( Best1.CanAddEdge() ) Best1.AddEdge(Navigator->Edges.push(Edge));
			Exchange( Edge.iStart, Edge.iFinish );
			if( Best2.CanAddEdge() ) Best2.AddEdge(Navigator->Edges.push(Edge));

			// Link groups.
			Group1.iLinked.push(g2);
			Group2.iLinked.push(g1);
		}
	}
}


//
// Sort jump links by cost.
//
Bool JumpLinksCmp( const TPathEdge& A, const TPathEdge& B )
{
	return A.Cost < B.Cost;
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
		Array<TPathEdge>	Links[2];

		// Try each pair from both groups.
		for( Int32 p1=0; p1<Group1.iPins.size(); p1++ )
		for( Int32 p2=0; p2<Group2.iPins.size(); p2++ )
		{
			Int32	iLower, iUpper;
			TPin&	Pin1 = Pins[Group1.iPins[p1]];
			TPin&	Pin2 = Pins[Group2.iPins[p2]];

			// Sort pins by height.
			if( Pin1.Location.Y > Pin2.Location.Y )
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
			TVector	Tangent	= -Upper.Normal.Cross();
			TVector	Dir		= Lower.Location - Upper.Location;
			TVector	JumpSpot, UnusedPoint, UnusedNormal;
			if( bLeft )
				JumpSpot	= Upper.Location + (Upper.Normal-Tangent) * PIN_BASE * 1.f;
			else
				JumpSpot	= Upper.Location + (Upper.Normal+Tangent) * PIN_BASE * 1.f;

			// Make sure, we can jump.
			if	(
					( bLeft ? Dir.X < 0.f : Dir.X > 0.f ) &&
					( Abs(Dir.X) <= MAX_JUMP_X ) &&
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
					TPathEdge	Edge;
					Edge.iStart		= iUpper;
					Edge.iFinish	= iLower;
					Edge.PathType	= PATH_Jump;
					Edge.Cost		= Floor(JUMP_WEIGHT*(Abs(Dir.X)+Abs(Dir.Y)));

					Links[bLeft].push(Edge);
				}
			}
		}

		// Add links to both directions.
		for( Int32 d=0; d<2; d++ )
			if( Links[d].size() > 0 )
			{
				// Sort links by distance.
				Links[d].sort(JumpLinksCmp);
				TPathEdge Edge	= Links[d][Max( (Links[d].size())/2-1, 0 )];

				if( Pins[Edge.iStart].CanAddEdge() ) Pins[Edge.iStart].AddEdge(Navigator->Edges.push(Edge));
				Exchange( Edge.iStart, Edge.iFinish );
				if( Pins[Edge.iStart].CanAddEdge() ) Pins[Edge.iStart].AddEdge(Navigator->Edges.push(Edge));
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
	for( Int32 iEdge=0; iEdge<Navigator->Edges.size(); iEdge++ )
	{
		TPathEdge& Edge = Navigator->Edges[iEdge];

		// Explore only walkable, since jumping is to unpredictable, and
		// others are available for all kind of AI.
		if( Edge.PathType != PATH_Walk )
			continue;

		// Walk along the edge and trace line up.
		Edge.Height			= MAX_HULL_HEIGHT;
		TVector	A			= Navigator->Nodes[Edge.iStart].Location,
				B			= Navigator->Nodes[Edge.iFinish].Location,
				Delta		= B - A,
				Walk		= A;
		Int32	NumSteps	= Max( 1, Floor(Delta.Size()) );

		Delta	*= 1.f/NumSteps;

		for( Int32 i=0; i<NumSteps; i++ )
		{
			TVector HitPoint, HitNormal;
			if( TestLine
				(
					true,
					Walk,
					TVector( Walk.X, Walk.Y+MAX_HULL_HEIGHT ),
					HitPoint,
					HitNormal
				) )
			{
				Float	TestHeight = Abs(HitPoint.Y - Walk.Y) + PIN_BASE;

				if( Edge.Height > TestHeight )
						Edge.Height	= TestHeight;
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
		TVector P2, P1 = Brush->Vertices[Brush->NumVerts-1];
		for( Int32 i=0; i<Brush->NumVerts; i++ )
		{
			// Information about current edge.
			P2	= Brush->Vertices[i];
			TVector	Tanget	= P2 - P1;
			Tanget.Normalize();
			TVector Normal	= Tanget.Cross();

			// Place only above walkable surfaces.
			if( IsWalkable(Normal) )
			{
				// Place left one.
				TVector Left	= P1 + (Normal+Tanget) * PIN_BASE + Brush->Location;
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
				TVector Right	= P2 + (Normal-Tanget) * PIN_BASE + Brush->Location;
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
		TVector	Tangent	= -Source.Normal.Cross();

		// Compute jump 'from' and 'to' points.
		assert(Source.Flags & (PIN_Left | PIN_Right));
		TVector From, To;
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
		To	= TVector( From.X, From.Y-FALL_MAX_LEN );

		// Trace line.
		TVector HitNormal, HitPoint;
		FBrushComponent* HitBrush;
		HitBrush	= TestLine( false, From, To, HitPoint, HitNormal );

		if	( 
				HitBrush && 
				HitBrush != Source.Floor && 
				IsWalkable(HitNormal) &&
				Abs(HitPoint.Y-From.Y) > PIN_BASE*1.5f
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

				if( (Pin2.Location-Pin1.Location).SizeSquared() <= PIN_SAME*PIN_SAME )
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
	return GBuilder->Pins[A].Location.X < GBuilder->Pins[B].Location.X;
}


//
// Groups sorting by ..Bounds.Min.X.
//
Bool GroupXCmp( const TPinGroup& A, const TPinGroup& B )
{
	return A.Bounds.Min.X < B.Bounds.Min.X;
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
			TVector	UnusedHit, UnusedNormal;
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

		Group.Bounds.Min	=
		Group.Bounds.Max	= Pins[Group.iPins[0]].Location;
		for( Int32 i=1; i<Group.iPins.size(); i++ )
			Group.Bounds += Pins[Group.iPins[i]].Location;
	}

	// Sort groupes to process pathbuilding from 
	// left to right.
	Groups.sort(GroupXCmp);

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
				TPathEdge	Edge;
				Edge.iStart		= Group.iPins[i];
				Edge.iFinish	= Group.iPins[i+1];
				Edge.PathType	= PATH_Walk;
				Edge.Cost		= ::Floor(Abs(Pins[Edge.iStart].Location.X-Pins[Edge.iFinish].Location.X));

				Pins[Edge.iStart].AddEdge(Navigator->Edges.push(Edge));
				Exchange( Edge.iStart, Edge.iFinish );
				Pins[Edge.iStart].AddEdge(Navigator->Edges.push(Edge));
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

	// Destroy old navigator.
	if( Level->Navigator )
		freeandnil(Level->Navigator);

	// Allocate new navigator.
	Navigator			= new CNavigator( Level );

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
}


//
// Builder destructor.
//
CPathBuilder::~CPathBuilder()
{
}


//
// Path Builder main function.
//
void CPathBuilder::BuildNetwork( IProgressIndicator* Indicator )
{
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
			Navigator->Nodes.push(Pins[i].ToPathNode());

		// Explore just created paths.
		Ind.UpdateDetails(L"Exploration...");
		Ind.SetProgress( 1, 2 );
			ExplorePaths();
	}

	// Save our righteous works.
	Level->Navigator	= Navigator;

	// Count stats.
	NumEdges		= Navigator->Edges.size();
	NumNodes		= Navigator->Nodes.size();
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
FBrushComponent* CPathBuilder::TestPoint( const TVector& P )
{
	for( Int32 b=0; b<BrushList.size(); b++ )
		if( BrushList[b]->Type == BRUSH_Solid )
		{
			// Transform point to Brush's local coords system.
			FBrushComponent* Brush = BrushList[b];
			TVector	Local	= P - Brush->Location;

			if( IsPointInsidePoly( Local, Brush->Vertices, Brush->NumVerts ) )
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
	const TVector& From, 
	const TVector& To, 
	TVector& OutHit, 
	TVector& OutNormal 
)
{
	FBrushComponent*	Result		= nullptr;
	Float	TestDist, BestDist		= 1000000.f;

	for( Int32 b=0; b<BrushList.size(); b++ )
	{
		FBrushComponent* Brush = BrushList[b];

		// To brush local coords system.
		TVector	Normal, Hit;
		TVector	LocalFrom	= From - Brush->Location,
				LocalTo		= To - Brush->Location;

		if( LineIntersectPoly( LocalFrom, LocalTo, Brush->Vertices, Brush->NumVerts, &Hit, &Normal ) )
		{
			if	(
					(Brush->Type == BRUSH_Solid) ||
					(Brush->Type == BRUSH_SemiSolid && IsWalkable(Normal))
				)
			{
				Hit			+= Brush->Location;
				TestDist	= (Hit - From).SizeSquared();

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


/*-----------------------------------------------------------------------------
    Global path routines.
-----------------------------------------------------------------------------*/

//
// Build navigation network for level.
//
void CEditor::BuildPaths( FLevel* Level )
{
	CPathBuilder Builder( Level );
	Builder.BuildNetwork( TaskDialog );
}


//
// Destroy a level's navigation network.
//
void CEditor::DestroyPaths( FLevel* Level )
{
	if( Level->Navigator )
		freeandnil(Level->Navigator);
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/