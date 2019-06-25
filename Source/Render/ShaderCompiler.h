//-----------------------------------------------------------------------------
//	ShaderCompiler.h: An abstract platform-specific shaders compiler
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace rend
{
	/**
	 *	A shader type
	 */
	enum class EShaderType
	{
		Unknown,
		Vertex,
		Pixel,
		Compute,
		MAX
	};

	/**
	 *	An compiled shader
	 */
	class CompiledShader final
	{
	public:
		CompiledShader()
			:	m_type( EShaderType::Unknown ),
				m_checksum( -1 )
		{
		}

		CompiledShader( EShaderType type, const Array<UInt8>& data )
			:	m_type( type ),
				m_data( data )
		{
			if( m_data.size() > 0 )
			{
				m_checksum = hashing::murmur32( &data[0], sizeof( UInt8 ) * m_data.size() );			
			}
			else
			{
				m_checksum = 0;
			}
		}

		~CompiledShader()
		{
			m_data.empty();
		}

		Bool isValid() const
		{
			return m_type != EShaderType::Unknown && m_data.size() > 0;
		}

		EShaderType getType() const
		{
			return m_type;
		}

		UInt32 codeLength() const
		{
			return m_data.size();
		}

		const void* code() const
		{
			assert( isValid() );
			return &m_data[0];
		}

		UInt32 getChecksum() const
		{
			return m_checksum;
		}

	private:
		EShaderType m_type;
		Array<UInt8> m_data;
		UInt32 m_checksum;

	public:
		friend IOutputStream& operator<<( IOutputStream& stream, const CompiledShader& x )
		{
			stream.writeData( &x.m_type, sizeof( x.m_type ) );
			stream << x.m_data;
			stream << x.m_checksum;

			return stream;
		}

		friend IInputStream& operator>>( IInputStream& stream, CompiledShader& x )
		{
			stream.readData( &x.m_type, sizeof( x.m_type ) );
			stream >> x.m_data;
			stream >> x.m_checksum;

			return stream;
		}
	};

	// todo: add compilation flags here!!

	/**
	 *	An abstract shader compiler
	 */
	class ShaderCompiler
	{
	public:
		using UPtr = UniquePtr<ShaderCompiler>;

		~ShaderCompiler() = default;

		virtual CompiledShader compile( EShaderType shaderType, Text::Ptr shaderText, const Char* entryPoint,
			String* warnings = nullptr, String* errors = nullptr ) = 0;

		virtual String compilerMark() const = 0;
	};
}
}