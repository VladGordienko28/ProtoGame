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

	Int32 Navigator::findNearestNode( FLevel* level, const math::Vector& location, Float searchRadius, Bool traceLine ) const
	{
		Int32 resultNode = INVALID_NODE;
		Float bestDistanceSq = math::WORLD_SIZE * math::WORLD_SIZE;
		const Float radiusSq = searchRadius * searchRadius;

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

	Int32 Navigator::findNearestEdge( FLevel* level, const math::Vector& location, Float searchRadius, Bool walkOnly ) const
	{
		Int32 resultEdge = INVALID_EDGE;
		const math::Rect seeker = math::Rect( location, searchRadius );
		Float bestPriority = -math::WORLD_HALF;

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
				const Float testPriority = min( seeker.max.x, bounds.max.x ) - max( seeker.min.x, bounds.min.x );

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
		const Float maxSeekerSize = max( seeker.size.x, seeker.size.y );
		Int32 firstEdgeIndex = findNearestEdge( level, seeker.location, maxSeekerSize, true );

		if( firstEdgeIndex != -1 )
		{
			const PathEdge& firstEdge = m_edges[firstEdgeIndex];
			const PathNode& a = m_nodes[firstEdge.iStartNode];
			const PathNode& b = m_nodes[firstEdge.iEndNode];

			if( firstEdge.breadth < seeker.size.y )
			{
				// too low ceiling
				return false;
			}

			minX = min( a.location.x, b.location.x );
			maxX = max( a.location.x, b.location.x );
			maxHeight = firstEdge.breadth;

			StaticArray<Bool, MAX_EDGES> markedEdges;
			markedEdges[firstEdgeIndex] = true;
			
			RingQueue<Int32, MAX_NODES> nodesQueue;
			StaticArray<Bool, MAX_NODES> markedNodes;

			nodesQueue.enqueue( firstEdge.iStartNode );
			nodesQueue.enqueue( firstEdge.iEndNode );
			markedNodes[firstEdge.iStartNode] = true;
			markedNodes[firstEdge.iEndNode] = true;

			while( !nodesQueue.isEmpty() )
			{
				Int32 nodeIndex = nodesQueue.dequeue();
				const PathNode& node = m_nodes[nodeIndex];

				for( Int32 i = 0; i < MAX_EDGES_PER_NODE; ++i )
				{
					if( node.iEdges[i] != INVALID_EDGE )
					{
						Int32 edgeIndex = node.iEdges[i];
						const PathEdge& edge = m_edges[edgeIndex];

						if( edge.type == EPathType::Walk && markedEdges[edgeIndex] == false && 
							edge.breadth > seeker.size.y )
						{
							// good edge, add to area
							Int32 nodeNewIndex = edge.iStartNode;
							Int32 nodeOldIndex = edge.iEndNode;
							assert( nodeIndex == nodeNewIndex || nodeIndex == nodeOldIndex );

							if( nodeNewIndex == nodeIndex )
							{
								exchange( nodeNewIndex, nodeOldIndex );
							}

							const PathNode& newNode = m_nodes[nodeNewIndex];

							minX = min( minX, newNode.location.x );
							maxX = max( maxX, newNode.location.x );
							maxHeight = min( maxHeight, edge.breadth );

							markedEdges[edgeIndex] = true;

							if( markedNodes[nodeNewIndex] == false )
							{
								nodesQueue.enqueue( nodeNewIndex );
								markedNodes[nodeNewIndex] = true;
							}
						}
					}
				}
			}

			return true;
		}
		else
		{
			return false;
		}
	}

	Bool Navigator::makePathTo( FLevel* level, const SeekerInfo& seeker, const math::Vector& destination, TargetInfo& target )
	{
		// figure out seeker's edge
		math::Vector projectedDestination = destination;
		const Float seekerRadius = max( seeker.size.x, seeker.size.y );
		Int32 firstEdgeIndex = findNearestEdge( level, seeker.location, seekerRadius, true );

		if( firstEdgeIndex == INVALID_EDGE )
		{
			// unable to bound to network
			goto NoPath;
		}

		// figure our destination's edge
		Int32 destinationEdgeIndex = findNearestEdge( level, destination, seekerRadius, true );
		if( destinationEdgeIndex == INVALID_EDGE )
		{
			// destination is too high, so try to drop it
			math::Vector destinationFloor;
			if( level->TestLineGeom( destination, 
				{ destination.x, destination.y - seeker.jumpHeight - seeker.size.y * 0.5f }, false, &destinationFloor ) )
			{
				destinationEdgeIndex = findNearestEdge( level, destinationFloor, seekerRadius, true );

				if( destinationEdgeIndex == INVALID_EDGE )
				{
					goto NoPath;
				}
				else
				{
					projectedDestination = destinationFloor;
				}
			}
		}

		if( destinationEdgeIndex == firstEdgeIndex )
		{
			// Seeker and the destination at the same edge
			if( abs( seeker.location.x - destination.x ) > seeker.size.x * 0.5f )
			{
				// walk to destination 
				// todo: add support of early jumps, before seeker reached floorProjection of destination
				target.moveType = EPathType::Walk;
				target.location = projectedDestination;
				return true;
			}
			else
			{
				// seeker is below destination, so make a jump
				target.moveType = EPathType::Jump;
				target.location = destination;
				return true;
			}
		}
		else
		{
			// Seeker and the destination at the different edges
			StaticArray<Int32, MAX_ROUTE_SIZE> route;
			Int32 routeSize;

			if( breadthFirstSearch( seeker, firstEdgeIndex, destinationEdgeIndex, route, routeSize ) )
			{
				// follow the route
				assert( routeSize > 0 );
				
				if( isNodeOccupiedBy( seeker, route[0] ) )
				{
					// find the edge from the route[0] to route[1]
					assert( routeSize > 1 );
					const PathNode& occupiedNode = m_nodes[route[0]];
					const PathNode& nextNode = m_nodes[route[1]];

					Int32 resultEdgeIndex = INVALID_EDGE;
					for( Int32 i = 0; i < MAX_EDGES_PER_NODE; ++i )
					{
						Int32 tempEdge = occupiedNode.iEdges[i];
						if( tempEdge != INVALID_EDGE && m_edges[tempEdge].iEndNode == route[1] )
						{
							resultEdgeIndex = tempEdge;
							break;
						}
					}
					assert( resultEdgeIndex != INVALID_EDGE );

					const PathEdge& nextEdge = m_edges[resultEdgeIndex];

					target.moveType = nextEdge.type;
					target.location = nextNode.location;
					return true;
				}
				else
				{
					// go to the first node at the path, which is belong to seeker's edge
					target.moveType = m_edges[firstEdgeIndex].type;
					target.location = m_nodes[route[0]].location;
					return true;
				}
			}
			else
			{
				// there is no path to destination
				goto NoPath;
			}
		}


	NoPath:
		target.moveType = EPathType::None;
		target.location = seeker.location;
		return false;
	}

	Bool Navigator::makeRandomPath( FLevel* level, const SeekerInfo& seeker, TargetInfo& target )
	{
		// figure out seeker's edge
		const Float seekerRadius = max( seeker.size.x, seeker.size.y );
		Int32 seekerEdgeIndex = findNearestEdge( level, seeker.location, seekerRadius, true );

		if( seekerEdgeIndex == INVALID_EDGE )
		{
			// unable to bound to network
			goto NoPath;
		}

		const PathEdge& seekerEdge = m_edges[seekerEdgeIndex];
		
		Int32 firstNodeIndex = seekerEdge.iStartNode;
		Int32 secondNodeIndex = seekerEdge.iEndNode;
		assert( firstNodeIndex != INVALID_NODE && secondNodeIndex != INVALID_NODE );

		Bool isFirstOccupied = isNodeOccupiedBy( seeker, firstNodeIndex );
		Bool isSecondOccupied = isNodeOccupiedBy( seeker, secondNodeIndex );

		if( isFirstOccupied || isSecondOccupied )
		{
			// select random edge for next movement
			const PathNode& firstNode = m_nodes[isFirstOccupied ? firstNodeIndex : secondNodeIndex];

			// seeker's edge is low priority
			if( RandomF() < 0.75f * ( 1.f / MAX_EDGES_PER_NODE ) )
			{
				// ok, let's walk on seeker's edge to the other side
			UseSeekerEdge:;
				const PathNode& targetNode = m_nodes[!isFirstOccupied ? firstNodeIndex : secondNodeIndex];

				target.location = targetNode.location;
				target.moveType = seekerEdge.type;
				return true;
			}
			else
			{
				// select random one
				StaticArray<Int32, MAX_EDGES_PER_NODE> candidateEdges;
				Int32 numCandidates = 0;

				for( Int32 i = 0; i < MAX_EDGES_PER_NODE; ++i )
				{
					if( firstNode.iEdges[i] != INVALID_NODE && firstNode.iEdges[i] != seekerEdgeIndex &&
						canPassThrough( seeker, m_edges[firstNode.iEdges[i]] ) )
					{
						candidateEdges[numCandidates++] = firstNode.iEdges[i];
					}
				}

				if( numCandidates > 0 )
				{
					const PathEdge& bestEdge = m_edges[ candidateEdges[ Random( numCandidates ) ] ];

					target.location = m_nodes[bestEdge.iEndNode].location;
					target.moveType = bestEdge.type;
					return true;
				}
				else
				{
					// no edges except seeker's one
					goto UseSeekerEdge;
				}
			}
		}
		else
		{
			// tries to occupie first or second node
			Int32 nodeToOccupyIndex = RandomBool() ? firstNodeIndex : secondNodeIndex;
			const PathNode& node = m_nodes[nodeToOccupyIndex];

			target.location = node.location;
			target.moveType = seekerEdge.type;
			return true;
		}

	NoPath:
		target.moveType = EPathType::None;
		target.location = seeker.location;
		return false;
	}

	Bool Navigator::breadthFirstSearch( const SeekerInfo& seeker, Int32 firstEdgeIndex, Int32 goalEdgeIndex,
		StaticArray<Int32, MAX_ROUTE_SIZE>& route, Int32& routeSize )
	{
		assert( firstEdgeIndex != INVALID_EDGE );
		assert( goalEdgeIndex != INVALID_EDGE );

		RingQueue<Int32, MAX_NODES> nodesQueue;
		StaticArray<Float, MAX_NODES> nodesWeight;
		StaticArray<Int32, MAX_NODES> parentLinks;
		Int32 lastNodeInRoute = INVALID_NODE;
		routeSize = 0;

		// mark all nodes as far-far away
		for( Int32 i = 0; i < m_nodes.size(); ++i )
		{
			nodesWeight[i] = sqr( math::WORLD_SIZE );
			parentLinks[i] = INVALID_NODE;
		}

		// enqueue starting nodes
		const PathEdge& firstEdge = m_edges[firstEdgeIndex];

		nodesQueue.enqueue( firstEdge.iStartNode );
		nodesQueue.enqueue( firstEdge.iEndNode );
		nodesWeight[firstEdge.iStartNode] = 0.f;
		nodesWeight[firstEdge.iEndNode] = 0.f;

		while( !nodesQueue.isEmpty() )
		{
			Int32 currentNodeIndex = nodesQueue.dequeue();
			const PathNode& currentNode = m_nodes[currentNodeIndex];

			Int32 nodeEdgeIndex = 0;
			while( currentNode.iEdges[nodeEdgeIndex] != -1 && nodeEdgeIndex < MAX_EDGES_PER_NODE )
			{
				Int32 currentEdgeIndex = currentNode.iEdges[nodeEdgeIndex];

				if( currentEdgeIndex == goalEdgeIndex )
				{
					// found goal edge
					lastNodeInRoute = currentNodeIndex;
					goto PathFound;
				}

				const PathEdge& currentEdge = m_edges[currentEdgeIndex];
				Int32 nextNodeIndex = currentEdge.iEndNode;
				const PathNode& nextNode = m_nodes[nextNodeIndex];

				if( nodesWeight[nextNodeIndex] > nodesWeight[currentNodeIndex] + currentEdge.cost &&
					canPassThrough( seeker, currentEdge ) )
				{
					nodesQueue.enqueue( nextNodeIndex );

					nodesWeight[nextNodeIndex] = nodesWeight[currentNodeIndex] + currentEdge.cost;
					parentLinks[nextNodeIndex] = currentNodeIndex;
				}

				++nodeEdgeIndex;
			}
		}

		return false;

	PathFound:
		assert( lastNodeInRoute != INVALID_NODE );

		for( Int32 i = lastNodeInRoute; i != INVALID_NODE; i = parentLinks[i] )
		{
			// shift route if it doesn't fit
			if( routeSize >= MAX_ROUTE_SIZE )
			{
				for( Int32 j = 1; j < MAX_ROUTE_SIZE; ++j )
				{
					route[j-1] = route[j];
				}

				routeSize -= 1;
			}

			route[routeSize++] = i;
		}

		// flip nodes order
		assert( routeSize > 0 );
		for( Int32 i = 0; i < routeSize / 2; ++i )
		{
			route.swap( i, routeSize - 1 - i );
		}

		return true;
	}

	Bool Navigator::isEdgeOccupiedBy( const SeekerInfo& seeker, Int32 edgeIndex ) const
	{
		assert( edgeIndex != INVALID_EDGE );
		const PathEdge& edge = m_edges[edgeIndex];

		assert( edge.iStartNode != INVALID_NODE && edge.iEndNode != INVALID_NODE );
		const math::Vector verts[] =
		{
			m_nodes[edge.iStartNode].location,
			m_nodes[edge.iEndNode].location
		};

		const math::Rect seekerRect = math::Rect( seeker.location, seeker.size.x, seeker.size.y );
		const math::Rect edgeRect = math::Rect( verts, arraySize( verts ) );

		return seekerRect.isOverlap( edgeRect );
	}

	Bool Navigator::isNodeOccupiedBy( const SeekerInfo& seeker, Int32 nodeIndex ) const
	{
		assert( nodeIndex != INVALID_NODE );
		const PathNode& node = m_nodes[nodeIndex];

		const math::Rect seekerRect = math::Rect( seeker.location, seeker.size.x, seeker.size.y );
		return seekerRect.isInside( node.location );
	}

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