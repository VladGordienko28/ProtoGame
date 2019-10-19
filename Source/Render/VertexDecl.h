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

		friend IOutputStream& operator<<( IOutputStream& stream, const VertexElement& x )
		{
			stream << (UInt32)x.format << (UInt32)x.usage << x.usageIndex << x.offset;
			return stream;
		}

		friend IInputStream& operator>>( IInputStream& stream, VertexElement& x )
		{
			stream >> (UInt32&)x.format >> (UInt32&)x.usage >> x.usageIndex >> x.offset;
			return stream;
		}
	};

	/**
	 *	A vertex declaration
	 */
	class VertexDeclaration final
	{
	public:
		VertexDeclaration()
		{
		}

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

			return m_elements.size() > 0 ? 
				hashing::murmur32( &m_elements[0], m_elements.size() * sizeof( VertexElement ) ) : 0;
		}

		String getName() const
		{
			return m_name;
		}

		// todo: add validation for all elements
		Bool isValid() const
		{
			return m_name;
		}

		friend IOutputStream& operator<<( IOutputStream& stream, const VertexDeclaration& x )
		{
			stream << x.m_name << x.m_elements;
			return stream;
		}

		friend IInputStream& operator>>( IInputStream& stream, VertexDeclaration& x )
		{
			stream >> x.m_name >> x.m_elements;
			return stream;
		}

	private:
		String m_name;
		Array<VertexElement> m_elements;
	};

	inline EVertexElementUsage getVertexElementUsageByName( String name )
	{
		return name == TXT( "Position" ) ? EVertexElementUsage::Position :
			name == TXT( "TexCoord" ) ? EVertexElementUsage::TexCoord :
			name == TXT( "Color" ) ? EVertexElementUsage::Color :
				EVertexElementUsage::MAX;
	}
}
}