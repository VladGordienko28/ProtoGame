//-----------------------------------------------------------------------------
//	GrowOnlyVB.h: Grow Only vertex buffer wrapper
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace gfx
{
	/**
	 *	Grow only vertex buffer
	 */
	template<typename VERTEX_TYPE, Int32 INITIAL_SIZE> class GrowOnlyVB: public NonCopyable
	{
	public:
		GrowOnlyVB()
		{
			m_data.setSize( INITIAL_SIZE );
			m_handle = api::createVertexBuffer( sizeof( VERTEX_TYPE ), INITIAL_SIZE, 
				rend::EUsage::Dynamic, nullptr, nullptr );
		}

		~GrowOnlyVB()
		{
			if( m_handle != INVALID_HANDLE<rend::VertexBufferHandle>() )
			{
				api::destroyVertexBuffer( m_handle );
			}
		}

		VERTEX_TYPE* prepare( Int32 requiredSize )
		{
			assert( requiredSize > 0 );

			if( requiredSize > m_data.size() )
			{
				m_data.setSize( requiredSize );

				api::destroyVertexBuffer( m_handle );

				m_handle = api::createVertexBuffer( sizeof( VERTEX_TYPE ), requiredSize, 
					rend::EUsage::Dynamic, nullptr, nullptr );
			}

			return &m_data[0];
		}

		void bind()
		{
			assert( m_data.size() > 0 );
			assert( m_handle != INVALID_HANDLE<rend::VertexBufferHandle>() );

			api::updateVertexBuffer( m_handle, &m_data[0], m_data.size() * sizeof( VERTEX_TYPE ) );
			api::setVertexBuffer( m_handle );
		}

	private:
		rend::VertexBufferHandle m_handle;
		Array<VERTEX_TYPE> m_data;
	
		static_assert( INITIAL_SIZE > 0, "Initial size of vertex buffer must be > 0" );
	};
}
}