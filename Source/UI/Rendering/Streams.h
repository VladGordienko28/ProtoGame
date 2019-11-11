//-----------------------------------------------------------------------------
//	Streams.h: An UI batch streams
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
namespace rendering
{
	/**
	 *	A base batches stream
	 */
	template<typename CHILDREN_TYPE, typename VERTEX_TYPE> class StreamBase: public NonCopyable
	{
	public:
		StreamBase( rend::Device* device )
			:	m_device( device ),
				m_gpuVB( INVALID_HANDLE<rend::VertexBufferHandle>() ),
				m_gpuIB( INVALID_HANDLE<rend::IndexBufferHandle>() )
		{
			assert( m_device );

			const Char* effectName = CHILDREN_TYPE::EFFECT_NAME;
			m_effect = res::ResourceManager::get<ffx::Effect>( effectName, res::EFailPolicy::FATAL );
		}

		virtual ~StreamBase()
		{
			if( m_gpuVB != INVALID_HANDLE<rend::VertexBufferHandle>() )
			{
				assert( m_cpuVB.capacity() > 0 );
				m_device->destroyVertexBuffer( m_gpuVB );
			}

			if( m_gpuIB != INVALID_HANDLE<rend::IndexBufferHandle>() )
			{
				assert( m_cpuIB.capacity() > 0 );
				m_device->destroyIndexBuffer( m_gpuIB );
			}

			m_effect = nullptr;
			m_device = nullptr;
		}

		void bindBuffers()
		{
			assert( m_gpuVB != INVALID_HANDLE<rend::VertexBufferHandle>() );
			assert( m_gpuIB != INVALID_HANDLE<rend::IndexBufferHandle>() );

			m_device->setVertexBuffer( m_gpuVB );
			m_device->setIndexBuffer( m_gpuIB );
		}

		void clearBuffers()
		{
			m_cpuVB.empty();
			m_cpuIB.empty();
		}

		void submitToGPU()
		{
			// submit vertex buffer to GPU
			if( m_cpuVB.size() > 0 )
			{
				if( m_device->getVertexBufferSize( m_gpuVB ) < static_cast<UInt32>( m_cpuVB.size() ) )
				{
					if( m_gpuVB != INVALID_HANDLE<rend::VertexBufferHandle>() )
					{
						m_device->destroyVertexBuffer( m_gpuVB );
					}

					m_gpuVB = m_device->createVertexBuffer( sizeof( VERTEX_TYPE ), 
						m_cpuVB.capacity(), rend::EUsage::Dynamic, &m_cpuVB[0], "UI_VB" );
				}
				else
				{
					m_device->updateVertexBuffer( m_gpuVB, 
						&m_cpuVB[0], m_cpuVB.size() * sizeof( VERTEX_TYPE ) );
				}
			}

			// submit index buffer to GPU
			if( m_cpuIB.size() > 0 )
			{
				if( m_device->getIndexBufferSize( m_gpuIB ) < static_cast<UInt32>( m_cpuIB.size() ) )
				{
					if( m_gpuIB != INVALID_HANDLE<rend::IndexBufferHandle>() )
					{
						m_device->destroyIndexBuffer( m_gpuIB );
					}

					m_gpuIB = m_device->createIndexBuffer( rend::EFormat::R16_U, 
						m_cpuIB.capacity(), rend::EUsage::Dynamic, &m_cpuIB[0], "UI_IB" );
				}
				else
				{
					m_device->updateIndexBuffer( m_gpuIB, 
						&m_cpuIB[0], m_cpuIB.size() * sizeof( UInt16 ) );
				}			
			}
		}

		ffx::Effect::Ptr getEffect() const
		{
			return m_effect;
		}

		VERTEX_TYPE* obtainVertices( UInt32 count )
		{
			return m_cpuVB.obtainRaw( count );
		}

		UInt32 getVertexCount() const
		{
			return m_cpuVB.size();
		}

		UInt16* obtainIndices( UInt32 count )
		{
			return m_cpuIB.obtainRaw( count );
		}

		UInt32 getIndexCount() const
		{
			return m_cpuIB.size();
		}

	protected:
		rend::Device* m_device;
		ffx::Effect::Ptr m_effect;

		GrowOnlyArray<VERTEX_TYPE> m_cpuVB;
		rend::VertexBufferHandle m_gpuVB;

		GrowOnlyArray<UInt16> m_cpuIB;
		rend::IndexBufferHandle m_gpuIB;

	private:
		StreamBase() = delete;
	};

	/**
	 *	A colored rects batches stream
	 */
	class FlatShadeStream: public StreamBase<FlatShadeStream, FlatShadeOp::Vertex>
	{
	public:
		static constexpr const Char EFFECT_NAME[] = TXT("System.Shaders.UI.FlatShade");

		FlatShadeStream( rend::Device* device );
		~FlatShadeStream();
	};







	/**
	 *	A text batches stream
	 */
	class TextStream: public StreamBase<TextStream, TextOp::Vertex>
	{
	public:
		static constexpr const Char EFFECT_NAME[] = TXT("System.Shaders.UI.Text");

		TextStream( rend::Device* device );
		~TextStream();
	};
}
}
}