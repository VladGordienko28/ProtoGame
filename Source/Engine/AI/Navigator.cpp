//-----------------------------------------------------------------------------
//	Navigator.cpp: AI navigation system implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Engine/Engine.h"

namespace flu
{
namespace navi
{
	Navigator::Navigator()
	{
	}

	Navigator::~Navigator()
	{
		m_edges.empty();
		m_nodes.empty();
	}

	Int32 Navigator::findNearbyNode( FLevel* level, const math::Vector& location, Float nearbyRadius, Bool traceLine ) const
	{
		Int32 resultNode = INVALID_NODE;
		Float bestDistanceSq = math::WORLD_SIZE * math::WORLD_SIZE;
		const Float radiusSq = nearbyRadius * nearbyRadius;

		for( Int32 i = 0; i < m_nodes.size(); ++i )
		{
			const PathNode& node = m_nodes[i];
			const Float testDistanceSq = ( node.location - location ).sizeSquared();
		
			if( testDistanceSq < radiusSq && testDistanceSq < bestDistanceSq )
			{
				if( !traceLine || !level->TestLineGeom( node.location, location, true, nullptr, nullptr ) )
				{
					resultNode = i;
					bestDistanceSq = testDistanceSq;
				}
			}
		}

		return resultNode;
	}

	Int32 Navigator::findNearbyEdge( FLevel* level, const math::Vector& location, Float nearbyRadius, Bool walkOnly ) const
	{
		Int32 resultEdge = INVALID_EDGE;
		const math::Rect seeker = math::Rect( location, nearbyRadius );
		Float bestPriority = -math::WORLD_SIZE;

		for( Int32 i = 0; i < m_edges.size(); ++i )
		{
			const PathEdge& edge = m_edges[i];

			if( walkOnly && edge.type != EPathType::Walk )
			{
				continue;
			}

			const math::Vector verts[2] = 
			{
				m_nodes[edge.iStartNode].location,
				m_nodes[edge.iEndNode].location
			};

			const math::Rect bounds = math::Rect( verts, 2 );

			if( seeker.isOverlap( bounds ) )
			{
				const Float testPriority = bounds.sizeX() * 0.25f - abs( location.y - bounds.min.y );

				if( testPriority > bestPriority )
				{
					resultEdge = i;
					bestPriority = testPriority;
				}
			}
		}

		return resultEdge;
	}


	Bool Navigator::getWalkArea( FLevel* level, const SeekerInfo& seeker, Float& minX, Float& maxX, Float& maxHeight ) const
	{
		Int32 edgeIndex = findNearbyEdge( level, seeker.location, 32.f, true );////////////////////////

		if( edgeIndex != -1 )
		{
			// !! warning no traversal now

			const PathEdge& edge = m_edges[edgeIndex];
			const PathNode& a = m_nodes[edge.iStartNode];
			const PathNode& b = m_nodes[edge.iEndNode];

			minX = min( a.location.x, b.location.x );
			maxX = max( a.location.x, b.location.x );
			maxHeight = edge.breadth;

			return true;
		}
		else
		{
			return false;
		}
	}







	Bool Navigator::makeRandomPath( FLevel* level, const SeekerInfo& seeker, TargetInfo& target )
	{
		// find a nearby node
		Int32 iStartNode = findNearbyNode( level, seeker.location, 32.f );
		
		if( iStartNode == INVALID_NODE )
		{
			return false;
		}



/*
	// Get seeker's nearest node.
	Int32	iStart	= FindNearestNode( Seeker->Body->Location, false, 32.f );
	if( iStart == -1 )
	{
		// Failed to attach Seeker to navi network.
		return false;
	}
	TPathNode&	Start	= Nodes[iStart];
	math::Vector	Bottom	= Seeker->Base->Location - math::Vector( 0.f, Seeker->Base->Size.y*0.5f );

	if( Seeker->Body->GetAABB().isInside( Start.Location ) )
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
				iTestFinish	= Random(Nodes.size());
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
*/

	

	}









	//////////////////////////////////////////////////////////////////////////////


	Bool Navigator::canPassThrough( const SeekerInfo& seeker, const PathEdge& edge ) const
	{
		switch( edge.type )
		{
			case EPathType::Walk:
			{
				// walk path requires only ability to walk and corresponding tall
				return seeker.size.y < edge.breadth;
			}
			case EPathType::Jump:
			{
				// jumping from one node to other
				const PathNode& from = m_nodes[edge.iStartNode];
				const PathNode& to = m_nodes[edge.iEndNode];

				// todo: add breadth logic: 'seeker.size.x < edge.breadth'
				return ai::canMakeJumpTo( from.location, to.location, 
					seeker.xSpeed, seeker.jumpHeight, seeker.gravity, nullptr );
			}
			case EPathType::Ladder:
			{
				// ladder climb
				// todo: add size cheking
				return true;
			}
			case EPathType::Teleport:
			{
				// everyone can use teleporters even morons
				// todo: add temporary disabled teleporters logic
				return true;
			}
			case EPathType::Other:
			{
				// special pass with marker's logic
				// todo: implement marker logic
				return false;
			}
			default:
			{
				// unknown or unsupported path
				return false;
			}
		}
	}

	void Navigator::draw( CCanvas* canvas )
	{
		if( isValid() )
		{
			static const Float NODE_DRAW_SIZE = 10.f;
			static const Float EDGE_DRAW_SIZE = 2.5f;

			TRenderRect lineRect;
			lineRect.Texture = nullptr;
			lineRect.Flags = POLY_Unlit | POLY_AlphaGhost;

			const Float lineWidth = ( EDGE_DRAW_SIZE * canvas->View.FOV.x * canvas->View.Zoom ) / canvas->View.Width;
			const Float time = GPlat->Now();

			for( const auto& it : m_edges )
			{
				const math::Vector verts[2] =
				{
					getNode( it.iStartNode ).location,
					getNode( it.iEndNode ).location
				};

				if( canvas->View.Bounds.isOverlap( math::Rect( verts, 2 ) ) )
				{
					const math::Vector delta = verts[1] - verts[0];
					const Float deltaSize = delta.size();
					const math::Color edgeColor = it.getDrawColor();

					lineRect.Rotation = math::vectorToAngle( delta );
					lineRect.Bounds = math::Rect( ( verts[0] + verts[1] ) * 0.5f, deltaSize, lineWidth );
					lineRect.Color = { edgeColor.r, edgeColor.g, edgeColor.b, 128 };

					lineRect.TexCoords.min = { time, 0.f };
					lineRect.TexCoords.max = { time + deltaSize * 0.125f, 1.f / 32.f };

					canvas->DrawRect( lineRect );	
				}
			}

			for( const auto& it : m_nodes )
			{
				canvas->DrawCoolPoint( it.location, NODE_DRAW_SIZE, math::colors::GHOST_WHITE );
			}
		}
	}

	math::Color PathEdge::getDrawColor() const
	{
		switch( type )
		{
			case EPathType::Walk:		return math::colors::LIME;
			case EPathType::Jump:		return math::colors::AQUA;
			case EPathType::Ladder:		return math::colors::ORANGE;
			case EPathType::Teleport:	return math::colors::MAGENTA;
			case EPathType::Other:		return math::colors::AQUA;
			default:					return math::colors::RED;
		}
	}

	void Serialize( CSerializer& s, PathNode& v )
	{
		Serialize( s, v.location );
		Serialize( s, v.marker );
		s.SerializeData( v.iEdges, sizeof( v.iEdges ) );
	}

	void Serialize( CSerializer& s, PathEdge& v )
	{
		SerializeEnum( s, v.type );
		Serialize( s, v.iStartNode );
		Serialize( s, v.iEndNode );
		Serialize( s, v.cost );
		Serialize( s, v.breadth );
	}
}
}