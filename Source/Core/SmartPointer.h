//-----------------------------------------------------------------------------
//	SmartPointer.h: Cool smart pointers
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 *	An unique object wrapper
	 */
	template<typename T> class UniquePtr
	{
	public:
		UniquePtr()
			:	m_object( nullptr )
		{
		}

		UniquePtr( T* object )
			:	m_object( object )
		{
		}

		~UniquePtr()
		{
			reset( nullptr );
		}

		/**
		 *	Release from the uniqueness and return
		 *	it as a raw pointer
		 */
		T* release()
		{
			T* result = m_object;
			m_object = nullptr;
			return result;
		}

		/**
		 *	Reset unique pointer
		 */
		void reset( T* newObject = nullptr )
		{
			if( newObject != m_object )
			{
				if( m_object != nullptr )
					delete m_object;

				m_object = newObject;
			}
		}

		T* get() const
		{
			return m_object;
		}

		template<class D> D* getAs() const
		{
			return static_cast<D*>( m_object );
		}

		Bool hasObject() const
		{
			return m_object != nullptr;
		}

		operator Bool() const
		{
			return hasObject();
		}

		T& operator*() const
		{
			assert( m_object != nullptr );
			return *m_object;
		}

		T* operator->() const
		{
			assert( m_object != nullptr );
			return m_object;
		}

		Bool operator==( const UniquePtr<T>& other ) const
		{
			return m_object == other.m_object;
		}

		Bool operator==( const T* other ) const
		{
			return m_object == other;
		}

		Bool operator!=( const UniquePtr<T>& other ) const
		{
			return m_object != other.m_object;
		}

		Bool operator!=( const T* other ) const
		{
			return m_object != other;
		}

		UniquePtr<T>& operator=( T* inObject )
		{
			reset( inObject );
			return *this;
		}

	private:
		T* m_object;

		UniquePtr( const UniquePtr& other ) = delete;
		UniquePtr( UniquePtr&& other ) = delete;
		UniquePtr<T>& operator=( const UniquePtr<T>& other ) = delete;
	};


	/**
	 *	An abstract object, which can not be copied.
	 */
	class NonCopyable
	{
	public:
		NonCopyable() = default;
		virtual ~NonCopyable() = default;

	private:
		NonCopyable( const NonCopyable& other ) = delete;
		NonCopyable& operator=( const NonCopyable& other ) = delete;
	};

	/**
	 *	An abstract object, which able to count references.
	 *	Only heirs of this class are able to be shared ptr'ed :)
	 */
	class ReferenceCount
	{
	public:
		ReferenceCount()
			:	m_refCount( 0 )
		{
		}

		~ReferenceCount()
		{
			assert( m_refCount == 0 );
		}

		UInt32 refCount() const
		{
			return m_refCount;
		}

	private:
		UInt32 m_refCount;

		void incRef()
		{
			++m_refCount;
		}

		void decRef()
		{
			--m_refCount;
		}

		template<class T> friend class SharedPtr;
	};

#if 0
	/**
	 *	An abstract object, which able to safe count references.
	 *	Only heirs of this class are able to be shared ptr'ed
	 *	across the threads :)
	 */
	class SafeReferenceCount
	{
	public:
		SafeReferenceCount()
			:	m_refCount( 0 )
		{
		}

		~SafeReferenceCount()
		{
			assert( m_refCount.getValue() == 0 );
		}

		UInt32 refCount() const
		{
			return m_refCount.getValue();
		}

	private:
		concurrency::Atomic m_refCount;		

		void incRef()
		{
			m_refCount.increment();
		}

		void decRef()
		{
			m_refCount.decrement();
		}

		template<class T> friend class SharedPtr;
	};
#endif

	/**
	 *	A shared object wrapper
	 */
	template<typename T> class SharedPtr
	{
	public:
		SharedPtr()
			:	m_object( nullptr )
		{
		}

		SharedPtr( const SharedPtr<T>& other )
			:	m_object( nullptr )
		{
			reset( other.m_object );
		}

		SharedPtr( T* inObject )
			:	m_object( nullptr )
		{
			reset( inObject );
		}

		SharedPtr( SharedPtr<T>&& other )
		{
			m_object = other.m_object;
			other.m_object = nullptr;
		}

		~SharedPtr()
		{
			reset();
		}

		Bool isUnique() const
		{
			return m_object ? m_object->refCount() == 1 : false;
		}

		UInt32 refCount() const
		{
			return m_object ? m_object->refCount() : 0;
		}

		Bool hasObject() const
		{
			return m_object != nullptr;
		}

		void reset( T* newObject = nullptr )
		{
			if( newObject != m_object )
			{
				if( m_object )
				{
					m_object->decRef();
					if( m_object->refCount() == 0 )
					{
						delete m_object;
					}

					m_object = nullptr;
				}

				if( newObject )
				{
					m_object = newObject;
					m_object->incRef();
				}
			}
		}

		T* get() const
		{
			return m_object;
		}

		template<class D> D* getAs() const
		{
			return static_cast<D*>( m_object );
		}

		operator Bool() const
		{
			return hasObject();
		}

		T& operator*() const
		{
			assert( m_object );
			return *m_object;
		}

		T* operator->() const
		{
			assert( m_object );
			return m_object;
		}

		Bool operator==( const SharedPtr<T>& other ) const
		{
			return m_object == other.m_object;
		}

		Bool operator!=( const SharedPtr<T>& other ) const
		{
			return m_object != other.m_object;
		}

		SharedPtr<T>& operator=( T* inObject )
		{
			reset( inObject );
			return *this;
		}

		SharedPtr<T>& operator=( const SharedPtr<T>& other )
		{
			reset( other.m_object );
			return *this;
		}

	private:
		T* m_object;
	};
}