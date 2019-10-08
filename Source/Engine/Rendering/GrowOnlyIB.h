//-----------------------------------------------------------------------------
//	GrowOnlyIB.h: Grow Only index buffer wrapper
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace gfx
{
	/**
	 *	Grow only index buffer
	 */
	template<typename INDEX_TYPE, Int32 INITIAL_SIZE = 128> class GrowOnlyIB: public NonCopyable
	{
	public:
		GrowOnlyIB( const AnsiChar* inName )
			:	m_name( inName ),
				m_cpuBuffer( INITIAL_SIZE ),
				m_position( 0 ),
				m_gpuBuffer( INVALID_HANDLE<rend::IndexBufferHandle>() ),
				m_gpuBufferSize( 0 )
		{
		}

		~GrowOnlyIB()
		{
			if( m_gpuBuffer != INVALID_HANDLE<rend::IndexBufferHandle>() )
			{
				api::destroyIndexBuffer( m_gpuBuffer );
				m_gpuBuffer = INVALID_HANDLE<rend::IndexBufferHandle>();
			}

			m_cpuBuffer.empty();
		}

		void add( INDEX_TYPE index )
		{
			INDEX_TYPE* ptr = reserve( 1 );
			*ptr = index;
		}

		void insert( INDEX_TYPE* buffer, UInt32 numIndexes )
		{
			assert( buffer && numIndexes > 0 );

			INDEX_TYPE* ptr = reserve( numIndexes );
			mem::copy( ptr, buffer, numIndexes * sizeof( INDEX_TYPE ) );
		}

		INDEX_TYPE* reserve( Int32 numIndexes )
		{
			if( m_position + numIndexes > m_cpuBuffer.size() )
			{
				m_cpuBuffer.setSize( m_position + numIndexes );
			}

			const Int32 from = m_position;
			m_position += numIndexes;

			return &m_cpuBuffer[from];
		}

		UInt32 flushAndBind()
		{
			UInt32 numIndices = flush();
			bind();

			return numIndices;
		}

		UInt32 flush()
		{
			assert( m_cpuBuffer.size() > 0 );

			if( m_gpuBuffer == INVALID_HANDLE<rend::IndexBufferHandle>() 
				|| m_gpuBufferSize < m_cpuBuffer.size() )
			{
				// recreate gpu buffer
				if( m_gpuBuffer != INVALID_HANDLE<rend::IndexBufferHandle>() )
				{
					api::destroyIndexBuffer( m_gpuBuffer );
				}

				const constexpr rend::EFormat INDEX_FORMAT = sizeof( INDEX_TYPE ) == sizeof( UInt32 ) ?
					rend::EFormat::R32_U : rend::EFormat::R16_U;

				m_gpuBuffer = api::createIndexBuffer( INDEX_FORMAT, m_cpuBuffer.size(), 
					rend::EUsage::Dynamic, &m_cpuBuffer[0], *m_name );

				m_gpuBufferSize = m_cpuBuffer.size();
			}
			else
			{
				// just update gpu buffer
				api::updateIndexBuffer( m_gpuBuffer, 
					&m_cpuBuffer[0], m_position * sizeof( INDEX_TYPE ) );
			}

			UInt32 numIndices = m_position;
			m_position = 0;

			return numIndices;
		}

		void bind()
		{
			assert( m_gpuBuffer != INVALID_HANDLE<rend::IndexBufferHandle>() );
			api::setIndexBuffer( m_gpuBuffer );
		}

	private:
		AnsiString m_name;

		Array<INDEX_TYPE> m_cpuBuffer;
		Int32 m_position;

		rend::IndexBufferHandle m_gpuBuffer;
		UInt32 m_gpuBufferSize;

		GrowOnlyIB() = delete;

		static_assert( sizeof( INITIAL_SIZE > 0 ), "Bad initial IndexBuffer size" );
		static_assert( sizeof( INDEX_TYPE ) == sizeof( UInt16 ) || 
			sizeof( INDEX_TYPE ) == sizeof( UInt32 ), "Invalid IndexBuffer index type" );
	};
}
}