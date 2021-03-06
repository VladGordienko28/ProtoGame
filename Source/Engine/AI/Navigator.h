//-----------------------------------------------------------------------------
//	Navigator.h: AI navigation system
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace navi
{
	/**
	 *	A path type, which is associated with a particular
	 *	edge of the navigator
	 */
	enum class EPathType
	{
		None,		// Bad path
		Walk,		// Straight path for walking without any abysses or obstacles
		Jump,		// Jump over abyss or obstacle
		Ladder,		// Ladder, requires climb ability
		Teleport,	// Immediate teleportation
		Other,		// Something special
		MAX
	};	

	/**
	 *	An information about someone or something who are looking for
	 *	the right way
	 */
	struct SeekerInfo
	{
		math::Vector location;
		math::Vector size;
		Float xSpeed;
		Float jumpHeight;
		Float gravity;
	};

	/**
	 *	An information about temporal target( part of the full path ) after
	 *	path computation
	 */
	struct TargetInfo
	{
		math::Vector location;
		EPathType moveType;
	};

	static const SizeT MAX_NODES = 4096;
	static const SizeT MAX_EDGES = 8192;
	static const SizeT MAX_EDGES_PER_NODE = 4;
	static const SizeT MAX_ROUTE_SIZE = 4;
	static const Int32 INVALID_NODE = -1;
	static const Int32 INVALID_EDGE = -1;

	/**
	 *	A node in the navigation graph
	 */
	struct PathNode
	{
		math::Vector location = { 0.f, 0.f };
		FBaseComponent* marker = nullptr;
		Int32 iEdges[MAX_EDGES_PER_NODE] = {};

		// legacy
		friend void Serialize( CSerializer& s, PathNode& v );
	};

	/**
	 *	An edge in the navigation graph
	 */
	struct PathEdge
	{
		EPathType type = EPathType::None;		
		Int32 iStartNode = INVALID_NODE;
		Int32 iEndNode = INVALID_NODE;
		Int32 cost = 0;
		Float breadth = 0.f;

		math::Color getDrawColor() const;

		// legacy
		friend void Serialize( CSerializer& s, PathEdge& v );
	};

	/**
	 *	A navigation system in the world
	 */
	class Navigator final
	{
	public:
		Navigator();
		~Navigator();

		/**
		 *	Return the index of the nearest edge. If no edge found 
		 *	return INVALID_EDGE
		 */
		Int32 findNearestEdge( FLevel* level, const math::Vector& location, Float searchRadius = 8.f, Bool walkOnly = true ) const;

		/**
		 *	Return the index of the nearest node. If no node found 
		 *	return INVALID_NODE
		 */
		Int32 findNearestNode( FLevel* level, const math::Vector& location, Float searchRadius = 8.f, Bool traceLine = false ) const;

		/**
		 *	Return a walking area along X-axis for the specified seeker. Depends on seeker location, size and
		 *	abilities.
		 */
		Bool getWalkArea( FLevel* level, const SeekerInfo& seeker, Float& minX, Float& maxX, Float& maxHeight ) const;

		/**
		 *	Tries to make path from the seeker's location to the destination. Returns true if path was 
		 *	successfully made and target contains some information of how to reach the destination. 
		 */
		Bool makePathTo( FLevel* level, const SeekerInfo& seeker, const math::Vector& destination, TargetInfo& target );

		/**
		 *	Tries to make a random path from the seeker's location to the random node. Returns true if path was 
		 *	successfully made and target contains some information of how to reach the destination. 
		 */
		Bool makeRandomPath( FLevel* level, const SeekerInfo& seeker, TargetInfo& target );

		/**
		 *	Return true if seeker could pass through the edge
		 *	Takes into account only physical characteristics of the seeker
		 */
		Bool canPassThrough( const SeekerInfo& seeker, const PathEdge& edge ) const;

		/**
		 *	Debug draw a navigation network
		 */
		void draw( gfx::PrimitiveDrawer& drawer, CCanvas* canvas );

		Int32 getNodesCount() const
		{
			return m_nodes.size();
		}

		Int32 getEdgesCount() const
		{
			return m_edges.size();
		}

		const PathNode& getNode( Int32 i ) const
		{
			return m_nodes[i];
		}

		const PathEdge& getEdge( Int32 i ) const
		{
			return m_edges[i];
		}

		Bool isValid() const
		{
			return m_nodes.size() > 0 && m_edges.size() > 0;
		}

		// legacy
		friend void Serialize( CSerializer& s, Navigator& v )
		{
			Serialize( s, v.m_nodes );
			Serialize( s, v.m_edges );
		}

	private:
		Array<PathNode> m_nodes;
		Array<PathEdge> m_edges;

		/**
		 *	Travers the graph and tryes to compute full path from the firstEdge to the goalEdge.
		 *	Returns index to the first node in the graph in the long path if it exists.
		 */
		Bool breadthFirstSearch( const SeekerInfo& seeker, Int32 firstEdgeIndex, Int32 goalEdgeIndex,
			StaticArray<Int32, MAX_ROUTE_SIZE>& route, Int32& routeSize );

		Bool isNodeOccupiedBy( const SeekerInfo& seeker, Int32 nodeIndex ) const;
		Bool isEdgeOccupiedBy( const SeekerInfo& seeker, Int32 edgeIndex ) const;

		// todo: eliminate friends
		friend class CPathBuilder;
	};
}
}