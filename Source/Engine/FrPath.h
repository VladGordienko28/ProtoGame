/*=============================================================================
    FrPath.h: AI navigation network.
    Copyright Sep.2016 Vlad Gordienko.
	Totally rewritten Jan.2017.
=============================================================================*/

/*-----------------------------------------------------------------------------
    Path graph structures.
-----------------------------------------------------------------------------*/

//
// A path type associative with the edges
// of the navigation graph.
//
enum EPathType
{
	PATH_None,			// Bad path.			
	PATH_Walk,			// Path for walking, without abyss or obstacles.
	PATH_Jump,			// Jump over abyss or obstacle.
	PATH_Ladder,		// Ladder climb.
	PATH_Teleport,		// Immediately teleportation.
	PATH_Other,			// Something else.
	PATH_MAX
};


//
// A path graph node.
//
struct TPathNode
{
public:
	// Constants.
	enum { NUM_EDGES = 4 };

	// General info.
	TVector				Location;
	Integer				iEdges[NUM_EDGES];
	FBaseComponent*		Marker;	

	// Runtime info.
	Integer				iParent;
	Integer				Weight;
	
	// TPathNode interface.
	TPathNode();
	TColor GetDrawColor() const;
	friend void Serialize( CSerializer& S, TPathNode& V );
};


//
// A path edge, to connected two nodes. 
//
struct TPathEdge
{
public:
	// General info.
	Integer				iStart;
	Integer				iFinish;
	EPathType			PathType;
	Integer				Cost;
	Float				Height;
	
	// TPathEdge interface.
	TPathEdge();
	TColor GetDrawColor() const;
	friend void Serialize( CSerializer& S, TPathEdge& V );
};


/*-----------------------------------------------------------------------------
    CNavigator.
-----------------------------------------------------------------------------*/

//
// A path navigation network.
//
class CNavigator
{
public:
	// Variables.
	FLevel*				Level;
	TArray<TPathNode>	Nodes;
	TArray<TPathEdge>	Edges;

	// Friends.
	friend class FLevel;
	friend class FPuppetComponent;

	// CNavigator interface.
	CNavigator( FLevel* InLevel );
	~CNavigator();
	friend void Serialize( CSerializer& S, CNavigator*& V );

	// Path making.
	Bool CanPassThrough( FPuppetComponent* Seeker, const TPathEdge& Edge );
	Integer FindNearestNode( TVector P, Bool bTraceLine = false, Float Radius = 8.f );
	Bool MakePathTo( FPuppetComponent* Seeker, TVector Dest );
	Bool MakeRandomPath( FPuppetComponent* Seeker );

private:
	// BFS algorithm.
	void ClearPaths();
	Integer BreadthFirstSearch( FPuppetComponent* Seeker, Integer iStart, Integer iFinish );
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/