//-----------------------------------------------------------------------------
//	DxRef.h: IUnknown smart pointer
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu {
namespace dx11 {
	template<typename T> class DxRef final
	{
	public:
		DxRef()
			:	m_object( nullptr )
		{
		}

		DxRef( T* inObject )
			:	m_object( inObject )
		{
			if( m_object )
			{
				m_object->AddRef();
			}
		}

		DxRef( const DxRef<T>& other )
			:	m_object( other.m_object )
		{
			if( m_object )
			{
				m_object->AddRef();
			}
		}
		
		DxRef( DxRef<T>&& other )
		{
			m_object = other.m_object;
			other.m_object = nullptr;
		}

		~DxRef()
		{
			if( m_object )
			{
				m_object->Release();
			}
		}

		inline Bool hasObject() const
		{
			return m_object != nullptr;
		}

		inline T*& get()
		{
			return m_object;
		}

		inline const T*& get() const
		{
			return m_object;
		}

		inline operator T*() const
		{
			return m_object;
		}

		inline T* operator->() const
		{
			assert( m_object != nullptr );
			return m_object;
		}

		inline T** operator&()
		{
			return &m_object;
		}

		inline T* const* operator&() const
		{
			return &m_object;
		}

		inline void operator=( T* other )
		{
			if( m_object )
			{
				m_object->Release();
			}

			m_object = other;

			if( m_object )
			{
				m_object->AddRef();
			}
		}

		inline void operator=( const DxRef<T>& other )
		{
			if( m_object )
			{
				m_object->Release();
			}

			m_object = other.m_object;

			if( m_object )
			{
				m_object->AddRef();
			}
		}

	private:
		T* m_object;
	};
} // namespace dx11
} // namespace flu