//-----------------------------------------------------------------------------
//	VertexDecl.h: A vertex declaration description
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace rend
{
	/**
	 *	An usage of vertex element
	 */
	enum class EVertexElementUsage : UInt32
	{
		Position,
		Color,
		TexCoord,
		MAX
	};

	/**
	 *	A vertex element
	 */
	struct VertexElement
	{
		EFormat format = EFormat::Unknown;
		EVertexElementUsage usage = EVertexElementUsage::Position;
		UInt32 usageIndex = 0;
		UInt32 offset = 0;
	};

	/**
	 *	A vertex declaration
	 */
	class VertexDeclaration final
	{
	public:
		VertexDeclaration( String name )
			:	m_name( name ),
				m_elements()
		{
		}

		~VertexDeclaration()
		{
			m_elements.empty();
		}

		Int32 addElement( const VertexElement& element )
		{
			assert( element.format != EFormat::Unknown );
			
			m_elements.push( element );
			return m_elements.size();
		}

		const Array<VertexElement>& getElements() const
		{
			return m_elements;
		}

		UInt32 getHash() const
		{
			assert( isValid() );
			return hashing::murmur32( &m_elements[0], m_elements.size() * sizeof( VertexElement ) );
		}

		String getName() const
		{
			return m_name;
		}

		Bool isValid() const
		{
			return m_name && m_elements.size() > 0;
		}

	private:
		String m_name;
		Array<VertexElement> m_elements;
	};
}
}