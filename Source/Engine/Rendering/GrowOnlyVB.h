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
	template<typename VERTEX_TYPE, Int32 INITIAL_SIZE = 128> class GrowOnlyVB: public NonCopyable
	{
	public:
		GrowOnlyVB( const AnsiChar* inName )
			:	m_name( inName ),
				m_cpuBuffer( INITIAL_SIZE ),
				m_position( 0 ),
				m_gpuBuffer( INVALID_HANDLE<rend::VertexBufferHandle>() ),
				m_gpuBufferSize( 0 )
		{
		}

		~GrowOnlyVB()
		{
			if( m_gpuBuffer != INVALID_HANDLE<rend::VertexBufferHandle>() )
			{
				api::destroyVertexBuffer( m_gpuBuffer );
				m_gpuBuffer = INVALID_HANDLE<rend::VertexBufferHandle>();
			}

			m_cpuBuffer.empty();
		}

		void add( VERTEX_TYPE vertex )
		{
			VERTEX_TYPE* ptr = reserve( 1, firstVertId );
			*ptr = vertex;
		}

		void insert( VERTEX_TYPE* buffer, UInt32 numVerts )
		{
			assert( buffer && numVerts > 0 );

			VERTEX_TYPE* ptr = reserve( numVerts, firstVertId );
			mem::copy( ptr, buffer, numVerts * sizeof( VERTEX_TYPE ) );
		}

		VERTEX_TYPE* reserve( Int32 numVerts )
		{
			if( m_position + numVerts > m_cpuBuffer.size() )
			{
				m_cpuBuffer.setSize( m_position + numVerts );
			}

			const Int32 from = m_position;
			m_position += numVerts;

			return &m_cpuBuffer[from];
		}

		void flushAndBind()
		{
			flush();
			bind();
		}

		void flush()
		{
			assert( m_cpuBuffer.size() > 0 );

			if( m_gpuBuffer == INVALID_HANDLE<rend::VertexBufferHandle>() 
				|| m_gpuBufferSize < static_cast<UInt32>( m_cpuBuffer.size() ) )
			{
				// recreate gpu buffer
				if( m_gpuBuffer != INVALID_HANDLE<rend::VertexBufferHandle>() )
				{
					api::destroyVertexBuffer( m_gpuBuffer );
				}

				m_gpuBuffer = api::createVertexBuffer( sizeof( VERTEX_TYPE ), m_cpuBuffer.size(), 
					rend::EUsage::Dynamic, &m_cpuBuffer[0], *m_name );

				m_gpuBufferSize = m_cpuBuffer.size();
			}
			else
			{
				// just update gpu buffer
				api::updateVertexBuffer( m_gpuBuffer, 
					&m_cpuBuffer[0], m_position * sizeof( VERTEX_TYPE ) );
			}

			m_position = 0;
		}

		void bind()
		{
			assert( m_gpuBuffer != INVALID_HANDLE<rend::VertexBufferHandle>() );
			api::setVertexBuffer( m_gpuBuffer );
		}

		Int32 getSize() const
		{
			return m_position;
		}

	private:
		AnsiString m_name;

		Array<VERTEX_TYPE> m_cpuBuffer;
		Int32 m_position;

		rend::VertexBufferHandle m_gpuBuffer;
		UInt32 m_gpuBufferSize;
	
		GrowOnlyVB() = delete;

		static_assert( INITIAL_SIZE > 0, "Initial size of vertex buffer must be > 0" );
	};
}
}