//-----------------------------------------------------------------------------
//	JobSystem.h: A job system
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace job
{
	/**
	 *	Base types
	 */
	using NodeId = UInt32;
	using TaskFunc = void(*)( void* userData ); 

	static const NodeId INVALID_NODE_ID = -1;

	/**
	 *	A graph of tasks to execute
	 */
	class TaskGraph: public NonCopyable
	{
	public:
		TaskGraph( UInt32 maxTasks );
		~TaskGraph();

		NodeId addTask( TaskFunc func, void* userData );
		NodeId addChildTask( NodeId parent, TaskFunc func, void* userData );

		void async();
		void wait();

	private:
		static const UInt32 MAX_NODES = 128;

		struct Node
		{
			TaskFunc func;
			void* userData;
			NodeId parent;
			UInt32 openTasks;
		};

		Node m_nodes[MAX_NODES];
		UInt32 m_numNodes;

		TaskGraph() = delete;

		void submit( Bool waitForComplete );
	};

	/**
	 *	Functions
	 */
	extern void initialize( UInt32 numWorkerThreads );
	extern void shutdown();
	extern Bool isInitialized();
}
}