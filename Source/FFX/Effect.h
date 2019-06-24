//-----------------------------------------------------------------------------
//	Effect.h: A FFX effect
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ffx
{
	/**
	 *	An effect loading context
	 */
	struct EffectLoadingContext
	{
	public:
		class IIncludeProvider* includeProvider = nullptr;
		String relativeFileName;
		Array<String> dependencies;
	};

	/**
	 *	An effect
	 */
	class Effect: public ReferenceCount
	{
	public:
		using Ptr = SharedPtr<Effect>;

		~Effect();

		void setFloat( SizeT offset, Float value )
		{
			setData( &value, sizeof( Float ), offset );
		}

		void setVector( SizeT offset, const math::Vector& value )
		{
			setData( &value, sizeof( math::Vector ), offset );
		}

		void setColor( SizeT offset, const math::FloatColor& value )
		{
			setData( &value, sizeof( math::FloatColor ), offset );
		}

		void setData( const void* data, SizeT size, SizeT offset );
		void setSRV( Int32 slot, rend::ShaderResourceView srv );
		void setSamplerState( Int32 slot, rend::SamplerStateId samplerId );

		void setBlendState( rend::BlendStateId blendState );

		void apply();

		String name() const
		{
			return m_name;
		}

	private:
		static const constexpr Char* PS_ENTRY = TEXT( "psMain" );
		static const constexpr Char* VS_ENTRY = TEXT( "vsMain" );

		static const SizeT CONSTANT_BUFFER_SIZE = 1024;
		static const SizeT MAX_TEXTURES = 2;

		struct ShaderSet
		{
			rend::ShaderHandle vs = INVALID_HANDLE<rend::ShaderHandle>();
			rend::ShaderHandle ps = INVALID_HANDLE<rend::ShaderHandle>();
		};

		struct Buffer
		{
			rend::ConstantBufferHandle bufferHandle;
			UInt8 data[CONSTANT_BUFFER_SIZE];
			Bool dirty = false;
		};

		String m_name;

		rend::Device* m_device;

		ShaderSet m_shader;
		StaticArray<Buffer, 1> m_buffers;
		StaticArray<rend::SamplerStateId, MAX_TEXTURES> m_samplerStates;
		StaticArray<rend::ShaderResourceView, MAX_TEXTURES> m_srvs;
		rend::BlendStateId m_blendState;

		rend::VertexDeclaration m_vertexDeclaration;

		Effect() = delete;
		Effect( String name, const rend::VertexDeclaration& vertexDeclaration, rend::Device* device );

		Bool reload( EffectLoadingContext& context );
		void cleanup();

		friend class System;
	};
}
}