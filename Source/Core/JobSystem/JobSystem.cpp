//-----------------------------------------------------------------------------
//	JobSystem.h: A job system
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Core/Core.h"

namespace flu
{
namespace job
{
	/**
	 *	Internal types
	 */
	using TaskId = UInt32;
	static const TaskId INVALID_TASK_ID = -1;

	/**
	 *	A task to execute
	 */
	struct Task
	{
		TaskFunc func = nullptr;
		void* userData = nullptr;

		TaskId parentTask = INVALID_TASK_ID;
		concurrency::Atomic openTasks;

		Task* nextAvailable = nullptr;
		TaskId thisId = INVALID_TASK_ID;
	};

	/**
	 *	An early and naive implementation yet
	 */
	class JobSystem final: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<JobSystem>;

		JobSystem( UInt32 numWorkerThreads );
		~JobSystem();

		void createTasks( UInt32 numTasks, Task** outTaskList );
		void submitTasks( UInt32 numTasks, Task** taskList );

		Bool isTaskFinished( TaskId taskId ) const;
		void helpWithTasks();

	private:
		static const SizeT TASK_POOL_SIZE = 512;
		
		StaticArray<Task, TASK_POOL_SIZE> m_taskPool;
		Task* m_firstAvailableTask;
		concurrency::CriticalSection::UPtr m_poolCS;

		Array<threading::Thread::UPtr> m_threads;
		concurrency::Atomic m_exit;

		RingQueue<Task*, TASK_POOL_SIZE> m_taskQueue; // todo: it will be cool to use some lockfree here
		concurrency::CriticalSection::UPtr m_taskQueueCS;
		concurrency::Semaphore::UPtr m_taskQueueSemaphore;

		JobSystem() = delete;

		Task* getTaskFromQueue();
		void processTask( Task* task );

		static void workerThreadEntry( void* context );
	};

	JobSystem::UPtr g_jobSystem;

	JobSystem::JobSystem( UInt32 numWorkerThreads )
		:	m_exit( 0 ),
			m_poolCS( concurrency::CriticalSection::create() ),
			m_taskQueueCS( concurrency::CriticalSection::create() ),
			m_taskQueueSemaphore( concurrency::Semaphore::create( 0 ) )
	{
		assert( numWorkerThreads > 0 );

		// create task pool
		for( UInt32 i = 0; i < TASK_POOL_SIZE; ++i )
		{
			m_taskPool[i].thisId = i;
			m_taskPool[i].nextAvailable = i < ( TASK_POOL_SIZE - 1 ) ? &m_taskPool[i + 1] : nullptr;
		}

		m_firstAvailableTask = &m_taskPool[0];

		// create working threads
		m_threads.setSize( numWorkerThreads );

		for( UInt32 i = 0; i < numWorkerThreads; ++i )
		{
			m_threads[i] = threading::Thread::create( &workerThreadEntry, this, 
				*AnsiString::format( "Worker Thread %d", i ) );
		}

		debug( L"Job System successfully created with %d worker threads", numWorkerThreads );
	}

	JobSystem::~JobSystem()
	{
		m_exit.setValue( 1 );

		// release semaphore for worker threads release
		m_taskQueueSemaphore->push( m_threads.size() );

		for( auto& it : m_threads )
		{
			it->wait();
		}

		m_threads.empty();

		m_poolCS = nullptr;
		m_taskQueueCS = nullptr;
		m_taskQueueSemaphore = nullptr;

		debug( L"Job System shutdown" );
	}

	void JobSystem::createTasks( UInt32 numTasks, Task** outTaskList )
	{
		assert( numTasks > 0 && outTaskList );

		concurrency::CriticalSection::Guard csg( m_poolCS );

		for( UInt32 i = 0; i < numTasks; ++i )
		{
			outTaskList[i] = m_firstAvailableTask;
			m_firstAvailableTask = m_firstAvailableTask->nextAvailable;
		}
	}

	void JobSystem::submitTasks( UInt32 numTasks, Task** taskList )
	{
		assert( numTasks > 0 && taskList );

		concurrency::CriticalSection::Guard csg( m_taskQueueCS );

		for( UInt32 i = 0; i < numTasks; ++i )
		{
			m_taskQueue.enqueue( taskList[i] );
		}

		m_taskQueueSemaphore->push( numTasks );
	}

	Bool JobSystem::isTaskFinished( TaskId taskId ) const
	{
		return m_taskPool[taskId].openTasks.getValue() == 0;
	}

	void JobSystem::helpWithTasks()
	{
		if( m_taskQueueSemaphore->tryPop() )
		{
			Task* task = getTaskFromQueue();

			if( task )
			{
				processTask( task );
			}		
		}
	}

	Task* JobSystem::getTaskFromQueue()
	{
		concurrency::CriticalSection::Guard csg( m_taskQueueCS );

		if( !m_taskQueue.isEmpty() )
		{
			return m_taskQueue.dequeue();
		}
		else
		{
			return nullptr;
		}
	}

	void JobSystem::processTask( Task* task )
	{
		while( task->openTasks.getValue() > 1 )
		{
			threading::yield();
		}

		task->func( task->userData );
		task->openTasks.decrement();

		if( task->parentTask != INVALID_TASK_ID )
		{
			m_taskPool[task->parentTask].openTasks.decrement();
		}

		// return to pool
		concurrency::CriticalSection::Guard csg( m_poolCS );
		task->nextAvailable = m_firstAvailableTask;
		m_firstAvailableTask = task;
	}

	void JobSystem::workerThreadEntry( void* context )
	{
		JobSystem* system = reinterpret_cast<JobSystem*>( context );		
		
		while( system->m_exit.getValue() == 0 )
		{
			system->m_taskQueueSemaphore->pop();

			if( Task* task = system->getTaskFromQueue() )
			{
				system->processTask( task );
			}
			else
			{
				threading::yield();
			}
		}
	}

	void initialize( UInt32 numWorkerThreads )
	{
		assert( !g_jobSystem.hasObject() );
		g_jobSystem = new JobSystem( numWorkerThreads );
	}

	void shutdown()
	{
		g_jobSystem = nullptr;
	}

	Bool isInitialized()
	{
		return g_jobSystem.hasObject();
	}

	TaskGraph::TaskGraph( UInt32 maxTasks )
		:	m_numNodes( 0 )
	{
	}

	TaskGraph::~TaskGraph()
	{
		assert( m_numNodes == 0 );
	}

	NodeId TaskGraph::addTask( TaskFunc func, void* userData )
	{
		Node node;
		node.parent = INVALID_NODE_ID;
		node.openTasks = 1;
		node.func = func;
		node.userData = userData;

		m_nodes[m_numNodes++] = node;
		return m_numNodes - 1;
	}

	NodeId TaskGraph::addChildTask( NodeId parent, TaskFunc func, void* userData )
	{
		assert( false && L"Not implemented yet" );
		return INVALID_NODE_ID;
	}

	void TaskGraph::async()
	{
		submit( false );
	}

	void TaskGraph::wait()
	{
		submit( true );
	}

	void TaskGraph::submit( Bool waitForComplete )
	{
		assert( m_numNodes > 0 );

		// Rearrange nodes for traverse
		for( auto& node : m_nodes )
		{
			node.parent = m_numNodes;
		}

		// add new fake root node
		Node root;
		root.parent = INVALID_NODE_ID;
		root.openTasks = m_numNodes + 1;
		root.func = []( void* ){};
		root.userData = nullptr;
		m_nodes[m_numNodes++] = root;

		// create and fill tasks
		Task* tasks[MAX_NODES];
		g_jobSystem->createTasks( m_numNodes, tasks );

		for( UInt32 i = 0; i < m_numNodes; ++i )
		{
			Node& node = m_nodes[i];
			Task* task = tasks[i];

			task->func = node.func;
			task->userData = node.userData;
			task->parentTask = node.parent != INVALID_NODE_ID ? tasks[node.parent]->thisId : INVALID_TASK_ID;
			task->openTasks.setValue( node.openTasks );
		}

		// submit
		g_jobSystem->submitTasks( m_numNodes, tasks );

		// wait and help with tasks
		if( waitForComplete )
		{
			TaskId rootTaskId = tasks[m_numNodes - 1]->thisId;

			while( !g_jobSystem->isTaskFinished( rootTaskId ) )
			{
				g_jobSystem->helpWithTasks();
			}
		}

		m_numNodes = 0;
	}
}
}