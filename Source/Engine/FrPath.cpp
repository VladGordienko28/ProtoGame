/*=============================================================================
    FrPath.cpp: AI navigation network implementation.
    Copyright Sep.2016 Vlad Gordienko.
	Totally rewritten Jan.2017.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
	Path making.
-----------------------------------------------------------------------------*/

//
// General purpose function for path making. All others methods, just
// computes Goal to reach. This function return true if path was
// successfully created and set FPuppetComponent::Goal*** fields,
// to follow it. Otherwise return false and reset those fields.
//
Bool CNavigator::MakePathTo( FPuppetComponent* Seeker, TVector InDest )
{
	// Reset seeker's goal info.
	Seeker->GoalStart	= Seeker->Body->Location;
	Seeker->Goal		= InDest;
	Seeker->GoalReach	= PATH_None;
	Seeker->GoalHint	= 0.f;
	Seeker->iGoalNode	= 
	Seeker->iHoldenNode	= -1;

	// Get seeker's nearest node.
	Int32	iStart	= FindNearestNode( Seeker->Body->Location, false, 32.f );
	if( iStart == -1 )
	{
		// Failed to attach Seeker to navi network.
		return false;
	}
	TPathNode&	Start	= Nodes[iStart];

	// Get destination nearest node.
	Int32 iDest	= FindNearestNode( InDest, true, 32.f );
	if( iDest == -1 )
	{
		// Failed to attach Destination to navi network.
		return false;
	}
	TPathNode&	Dest	= Nodes[iDest];

	// Fast access seeker's stuff.
	TVector	Size		= Seeker->Base->Size,
			Location	= Seeker->Base->Location,
			Size2		= TVector( Size.X*0.5f, Size.Y*0.5f ),
			Bottom		= TVector( Location.X, Location.Y-Size2.Y ),
			Top			= TVector( Location.X, Location.Y+Size2.Y );

	// Hit detection.
	FBrushComponent*	Brush	= nullptr;
	TVector				Hit, HitNormal;

	if( iDest == iStart )
	{	
		//
		// A: The seeker and destination are at the same node.
		//

		// Figure out is AI can reach dest, if it over surface?
		Brush	= Level->TestLineGeom
		(
			InDest,
			TVector( InDest.X, InDest.Y - Seeker->JumpHeight - Seeker->Base->Size.Y*0.6f ),
			true, 
			Hit,
			HitNormal
		);

		if( Brush )
		{
			// A1: Destination should be reachable for seeker.
			if( Seeker->Body->Floor )
			{
ToDestDirectly:
				if( InRange( InDest.X, Location.X-Size2.X, Location.X+Size2.X ) )
				{
					// Jump to destination.
					Float	Height	= Max( 0.f, InDest.Y-Top.Y );
					if( Height < Seeker->JumpHeight )
					{
						Seeker->GoalHint	= FPuppetComponent::SuggestJumpSpeed( Height, Seeker->GravityScale );
						Seeker->iHoldenNode	= iStart;
						Seeker->iGoalNode	= -1;
						Seeker->Goal		= InDest;
						Seeker->GoalReach	= PATH_Jump;
						return true;
					}
					else
						goto NoPath;
				}
				else
				{
					// Walk to destination.
					Seeker->iHoldenNode	= iStart;
					Seeker->iGoalNode	= -1;
					Seeker->Goal		= TVector( InDest.X, Hit.Y+Size2.Y );
					Seeker->GoalHint	= 0.f;
					Seeker->GoalReach	= PATH_Walk;
					return true;
				}
			}
			else
			{
				// The seeker has no valid floor, wait until
				// seeker landed.
				goto NoPath;
			}
		}
		else
		{
			// A2: The destination are too high from seeker, and seeker can't
			// jump so high.
			goto NoPath;
		}
	}
	else
	{
		//
		// B: The seeker and destination are at different nodes.
		//
		
		// Figure out, does Seeker actually holds the nearest node?
		if( Seeker->Body->GetAABB().IsInside(Start.Location) )
		{
			// B1: The node actually holded by Seeker, so try to make
			//	path to the next node.
			ClearPaths();
			Int32	iPath	= BreadthFirstSearch( Seeker, iStart, iDest );
			if( iPath == -1 )
			{
				// The path doesn't exists or impassiable for Seeker.
				goto NoPath;
			}

			TPathEdge&	Path	= Edges[iPath];
			TPathNode&	Next	= Nodes[Path.iFinish];

			// Go to the next node, or just move to the destination 
			// directly without holding a next node.
			if( Path.iFinish==iDest && Path.PathType == PATH_Walk )
			{
				// To the goal, without holding a node!
				goto ToDestDirectly;
			}
			else
			{
				// Let's hold the next node.
				Seeker->iHoldenNode	= iStart;
				Seeker->iGoalNode	= Path.iFinish;
				Seeker->Goal		= Next.Location;
				Seeker->GoalReach	= Path.PathType;
				if( Path.PathType == PATH_Jump )
					Seeker->CanJumpTo( Bottom, Next.Location, Seeker->GoalHint ); 
				else
					Seeker->GoalHint	= 0.f;
				return true;
			}
		}
		else
		{
			//
			// B2: The seeker doesn't holds any node, so try to attach to
			//		the nearest node. AI will try to walk to the node.
			//
			if( Seeker->Body->Floor )
			{
				// The seeker has valid floor, so walk to node.
				ClearPaths();
				Int32 iPath = BreadthFirstSearch( Seeker, iStart, iDest );

				if( iPath != -1 && Edges[iPath].PathType==PATH_Walk )
				{
					// Make some shortcut for walking.
					Int32		iShort	= Edges[iPath].iFinish;
					TPathNode&	Short	= Nodes[iShort];

					if( iDest == iShort )
					{
						// Walk to the goal from the next node, but makes
						// sure it reachable, so let it jump.
						goto ToDestDirectly;
					}
					else
					{
						// Walk to the short node.
						Seeker->iHoldenNode	= -1;
						Seeker->iGoalNode	= iShort;
						Seeker->Goal		= Short.Location;
						Seeker->GoalHint	= 0.f;
						Seeker->GoalReach	= PATH_Walk;
						return true;
					}
				}
				else
				{
					// Walk to the nearest node first.
					Seeker->iHoldenNode	= -1;
					Seeker->iGoalNode	= iStart;
					Seeker->Goal		= Start.Location;
					Seeker->GoalHint	= 0.f;
					Seeker->GoalReach	= PATH_Walk;
					return true;
				}
			}
			else
			{
				// Seeker has no valid floor, so we cannot attach.
NoPath:
				Seeker->GoalStart	= 
				Seeker->Goal		= Seeker->Body->Location;
				Seeker->GoalReach	= PATH_None;
				Seeker->GoalHint	= 0.f;
				Seeker->iGoalNode	= 
				Seeker->iHoldenNode	= -1;
				return false;
			}
		}
	}
}


//
// Function to make a random path. Return true, if path was successfully
// created, otherwise return false.
//
Bool CNavigator::MakeRandomPath( FPuppetComponent* Seeker )
{
	// Get seeker's nearest node.
	Int32	iStart	= FindNearestNode( Seeker->Body->Location, false, 32.f );
	if( iStart == -1 )
	{
		// Failed to attach Seeker to navi network.
		return false;
	}
	TPathNode&	Start	= Nodes[iStart];
	TVector		Bottom	= Seeker->Base->Location - TVector( 0.f, Seeker->Base->Size.Y*0.5f );

	if( Seeker->Body->GetAABB().IsInside(Start.Location) )
	{
		// 
		// A: Seeker actually hold node, make path to next in chain.
		//
		if	( 
				(Seeker->iGoalNode == iStart) ||
				(Seeker->iGoalNode == -1)
			)
		{
			// End of chain reached, so make a new random path.
			Int32 iPath = -1, iFinish = -1; 
			enum{ NUM_RANDOM_SAMPLES = 8 };
			for( Int32 i=0; i<NUM_RANDOM_SAMPLES; i++ )
			{
				Int32 iTestPath, iTestFinish;
				iTestFinish	= Random(Nodes.Num());
				ClearPaths();
				iTestPath	= BreadthFirstSearch( Seeker, iStart, iTestFinish );
				if( iTestPath != -1 )
				{
					iPath	= iTestPath;
					iFinish	= iTestFinish;
					break;
				}
			}
			if( iPath == -1 )
				goto NoPath;

			// Initialize new walking.
			TPathEdge& Path		= Edges[iPath];
			Seeker->GoalStart	= Seeker->Body->Location;
			Seeker->Goal		= Nodes[Path.iFinish].Location;
			Seeker->GoalReach	= Path.PathType;
			Seeker->iHoldenNode	= iStart;
			Seeker->iGoalNode	= iFinish;
			if( Path.PathType == PATH_Jump )
				Seeker->CanJumpTo( Bottom, Seeker->Goal, Seeker->GoalHint );
			else
				Seeker->GoalHint	= 0.f;	
			return true;
		}
		else
		{
			// Follow the path next.
			assert(Seeker->iGoalNode != -1);
			ClearPaths();
			Int32 iPath = BreadthFirstSearch( Seeker, iStart, Seeker->iGoalNode );

			if( iPath != -1 )
			{
				// Goto next node.
				TPathEdge& Path		= Edges[iPath];
				Seeker->GoalStart	= Seeker->Body->Location;
				Seeker->Goal		= Nodes[Path.iFinish].Location;
				Seeker->GoalReach	= Path.PathType;
				Seeker->iHoldenNode	= iStart;
				if( Path.PathType == PATH_Jump )
					Seeker->CanJumpTo( Bottom, Seeker->Goal, Seeker->GoalHint );
				else
					Seeker->GoalHint	= 0.f;	
				return true;
			}
			else
				goto NoPath;
		}
	}
	else
	{
		//
		// B: Seeker don't actually hold any node, so attach to the nearest
		// node and then pick a random node.
		//
		if( Seeker->Body->Floor )
		{
			// Seeker has a floor, so it ok.
			Seeker->GoalStart	= Seeker->Body->Location;
			Seeker->Goal		= Start.Location;
			Seeker->GoalReach	= PATH_Walk;
			Seeker->GoalHint	= 0.f;
			Seeker->iGoalNode	= iStart;
			Seeker->iHoldenNode	= -1;
			return true;
		}
		else
		{
			// Seeker has no floor, so give up.
NoPath:
			Seeker->GoalStart	= 
			Seeker->Goal		= Seeker->Body->Location;
			Seeker->GoalReach	= PATH_None;
			Seeker->GoalHint	= 0.f;
			Seeker->iGoalNode	= 
			Seeker->iHoldenNode	= -1;
			return false;
		}
	}
}


/*-----------------------------------------------------------------------------
    CNavigator implementation.
-----------------------------------------------------------------------------*/

//
// Navigator constructor.
//
CNavigator::CNavigator( FLevel* InLevel )
	:	Nodes(),
		Edges()
{
	Level	= InLevel;
}


//
// Navigator destructor.
//
CNavigator::~CNavigator()
{
	Edges.Empty();
	Nodes.Empty();
}


//
// Navigator object serialization.
//
void Serialize( CSerializer& S, CNavigator*& V )
{
	if( S.GetMode() == SM_Load )
	{
		// Load navigator.
		freeandnil(V);
		Bool	bHasNavigator;
		Serialize( S, bHasNavigator );
		if( bHasNavigator )
		{
			FLevel*	Level;
			Serialize( S, Level );
			V	= new CNavigator( Level );
			Serialize( S, V->Nodes );
			Serialize( S, V->Edges );
		}
	}
	else
	{
		// Save navigator.
		Bool	bHasNavigator	= V != nullptr;
		Serialize( S, bHasNavigator );
		if( bHasNavigator )
		{
			Serialize( S, V->Level );
			Serialize( S, V->Nodes );
			Serialize( S, V->Edges );
		}
	}
}


/*-----------------------------------------------------------------------------
	Breadth first search.
-----------------------------------------------------------------------------*/

//
// Clear a temporal marks in the each path-node
// to prepare to path-finding algorithm.
//
void CNavigator::ClearPaths()
{
	for( Int32 iNode=0; iNode<Nodes.Num(); iNode++ )
	{
		TPathNode& Node = Nodes[iNode];

		// Mark node as unreachable.
		Node.iParent	= -1;
		Node.Weight		= 10000000;
	}
}


//
// Breadth first search through the graph network and make path from iStart to iFinish
// node. The path will be valid(according to Seeker's size, jump height
// and etc. ). Return the global index of TPathEdge from the iStart to the
// next node in the path's list. If no path found return -1.
//
Int32 CNavigator::BreadthFirstSearch( FPuppetComponent* Seeker, Int32 iStart, Int32 iFinish )
{
	// An internal queue used only in bfs purposes.
	// It's not good brilliant, but it's fast enough.
	enum{ BFS_QUEUE_SIZE	= 2048 };
	enum{ BFS_QUEUE_SIZE1	= BFS_QUEUE_SIZE-1 };
	static Int32	BFSQueue[BFS_QUEUE_SIZE];
	static Int32	BFSHead, BFSTail;

	// Initialize queue.
	BFSHead	= 0;
	BFSTail	= 0;

	Int32 iFrom, iNext, iEdge;

	// Add iStart node to the queue.
	Nodes[iStart].Weight	= 0;
	BFSQueue[BFSTail]		= iStart;
	BFSTail					= BFS_QUEUE_SIZE1 & (BFSTail+1);

	while( BFSHead != BFSTail )
	{
		iFrom			= BFSQueue[BFSHead];
		BFSHead			= BFS_QUEUE_SIZE1 & (BFSHead+1);
		TPathNode& Node	= Nodes[iFrom];

		iEdge = 0;
		while	(
					Node.iEdges[iEdge] != -1 &&
					iEdge < TPathNode::NUM_EDGES
				)
		{
			TPathEdge& Edge	= Edges[Node.iEdges[iEdge]];
			iNext			= Edge.iFinish;
			TPathNode& Next	= Nodes[iNext];

			if	(
					Next.Weight > Node.Weight + Edge.Cost &&
					CanPassThrough( Seeker, Edge )
				)
			{
				BFSQueue[BFSTail]		= iNext;
				BFSTail					= BFS_QUEUE_SIZE1 & (BFSTail+1);

				Next.iParent		= iFrom;
				Next.Weight			= Node.Weight + Edge.Cost;

				// If goal has been reached.
				if( iNext == iFinish )
					goto Found;
			}

			iEdge++;
		}
	}

	// No path found.
	return -1;

Found:
	// Path found, traverse the path and return the edge.
	Int32	iNode;
	for ( 
			iNode = iFinish; 
			Nodes[iNode].iParent != iStart; 
			iNode = Nodes[iNode].iParent 
		);

	// Return the edge that links iStart to iNode.
	TPathNode& Start	= Nodes[iStart];
	iEdge	= 0;
	while	( 
				Start.iEdges[iEdge] != -1 && 
				iEdge < TPathNode::NUM_EDGES 
			)
	{
		TPathEdge& Edge	= Edges[Start.iEdges[iEdge]];

		// Edge found.
		if( Edge.iFinish == iNode )
			return Start.iEdges[iEdge];

		iEdge++;
	}

	// No edge found, how it's possible?
	assert(false);
}


/*-----------------------------------------------------------------------------
    Navigation utility.
-----------------------------------------------------------------------------*/

//
// Tries to find a nearest node to point. Nodes will checked only
// in radius, if no node found return -1.
//
Int32 CNavigator::FindNearestNode( TVector P, Bool bTraceLine, Float Radius )
{
	Int32	Result		= -1;
	Float	RadiusSq	= Radius * Radius;
	Float	BestDist	= 100000.0f;

	for( Int32 iNode=0; iNode<Nodes.Num(); iNode++ )
	{
		TPathNode& Node		= Nodes[iNode];
		Float	TestDist	= (Node.Location - P).SizeSquared();

		if( TestDist<RadiusSq && TestDist<BestDist )
		{
			TVector Hit, Normal;
			if( !bTraceLine || !Level->TestLineGeom( Node.Location, P, true, Hit, Normal ) )
			{
				Result		= iNode;
				BestDist	= TestDist;
			}
		}
	}

	return Result;
}


//
// Return true, if Seeker can pass through this edge, due to walk speed
// jump height and others AI movement characteristics, otherwise return false.
// Basically result depend on reach type and hull size only.
//
Bool CNavigator::CanPassThrough( FPuppetComponent* Seeker, const TPathEdge& Edge )
{
	switch( Edge.PathType )
	{
		case PATH_Walk:
		{
			// Walking, if AI can walk and hull height
			// allows to pass.
			return	Seeker->MoveSpeed > 0.f && 
					Seeker->Base->Size.Y < Edge.Height;
		}
		case PATH_Jump:
		{
			// Complex jump path.
			Float UnusedSpeed;
			TPathNode& A = Nodes[Edge.iStart];
			TPathNode& B = Nodes[Edge.iFinish];

			return Seeker->CanJumpTo
			( 
				A.Location,
				B.Location,
				UnusedSpeed
			);
		}
		case PATH_Other:
		case PATH_Ladder:
		{
			// Controversial path, Ok, I'll let AI
			// pass.
			return true;
		}
		case PATH_Teleport:
		{
			// Teleportation. Even moron with rudiment brain
			// can teleport.
			return true;
		}
		default:
		{
			// What the hell is it?
			return false;
		}
	}
}


/*-----------------------------------------------------------------------------
    TPathEdge implementation.
-----------------------------------------------------------------------------*/

//
// Path edge constructor.
//
TPathEdge::TPathEdge()
{
	iStart			= -1;
	iFinish			= -1;
	PathType		= PATH_None;
	Cost			= 0;
	Height			= 0.f;

}


//
// Return color of edge for rendering.
// Depend on path type only.
//
TColor TPathEdge::GetDrawColor() const
{
	static const TColor PathColors[PATH_MAX]	=
	{

		TColor( 0x4e, 0x4e, 0x4e, 0xff ),		// PATH_None.
		TColor( 0x4e, 0x6c, 0x31, 0xff ),		// PATH_Walk.
		TColor( 0x31, 0x40, 0x6c, 0xff ),		// PATH_Jump.
		TColor( 0x6c, 0x5d, 0x31, 0xff ),		// PATH_Ladder.
		TColor( 0x5d, 0x31, 0x6c, 0xff ),		// PATH_Teleport.
		TColor( 0x6c, 0x31, 0x31, 0xff )		// PATH_Other.
	};

	return PathColors[PathType];
}


//
// Path edge serialization.
//
void Serialize( CSerializer& S, TPathEdge& V )
{
	Serialize( S, V.iStart );
	Serialize( S, V.iFinish );
	SerializeEnum( S, V.PathType );
	Serialize( S, V.Cost );
	Serialize( S, V.Height );
}


/*-----------------------------------------------------------------------------
    TPathNode implementation.
-----------------------------------------------------------------------------*/

//
// Pathnode initialization.
//
TPathNode::TPathNode()
{
	Location	= TVector( 0.f, 0.f );
	Marker		= nullptr;
	iParent		= -1;
	Weight		= 0xffff;

	for( Int32 i=0; i<NUM_EDGES; i++ )
		iEdges[i] = -1;
}


//
// Return color for path node rendering,
// also useful in debugging.
//
TColor TPathNode::GetDrawColor() const
{
	return TColor( 0x10, 0x10, 0x77, 0xff );
}


//
// Pathnode serialization.
//
void Serialize( CSerializer& S, TPathNode& V )
{
	Serialize( S, V.Location );
	Serialize( S, V.Marker );
	S.SerializeData( V.iEdges, sizeof(V.iEdges) );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/