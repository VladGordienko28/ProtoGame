//-----------------------------------------------------------------------------
//	Concurrency.h: A multithreading synchronization objects
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu
{
namespace concurrency
{
	/**
	 *	An abstract synchronization object
	 */
	template<typename THIS_TYPE> class ISyncObject: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<THIS_TYPE>;

		class Guard
		{
		public:
			Guard( const UPtr& syncObject )
				:	m_syncObject( syncObject.get() )
			{
				assert( m_syncObject );
				m_syncObject->lock();
			}

			~Guard()
			{
				m_syncObject->unlock();
			}

		private:
			THIS_TYPE* m_syncObject;

			Guard() = delete;
		};
	};

	/**
	 *	A dummy synchronization object
	 */
	class Dummy: public ISyncObject<Dummy>
	{
	public:
		void lock()
		{
		}

		void unlock()
		{
		}

		static Dummy* create();

	protected:
		Dummy() = default;
	};

	/**
	 *	A critical section.
	 */
	class CriticalSection: public ISyncObject<CriticalSection>
	{
	public:
		virtual void lock() = 0;
		virtual Bool tryLock() = 0;
		virtual void unlock() = 0;

		static CriticalSection* create();

	protected:
		CriticalSection() = default;
	};

	/**
	 *	A mutex
	 */
	class Mutex: public ISyncObject<Mutex>
	{
	public:
		virtual void lock() = 0;
		virtual void unlock() = 0;

		static Mutex* create();

	protected:
		Mutex() = default;
	};

	/**
	 *	A lightweight spinlock
	 */
	class SpinLock: public ISyncObject<SpinLock>
	{
	public:
		virtual void lock() = 0;
		virtual void unlock() = 0;

		static SpinLock* create();

	protected:
		SpinLock() = default;
	};

} // namespace concurrency
} // namespace flu