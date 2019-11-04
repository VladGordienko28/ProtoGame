//-----------------------------------------------------------------------------
//	Effect.h: A FFX effect
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ffx
{
	/**
	 *	An effect
	 */
	class Effect: public res::Resource
	{
	public:
		DECLARE_RESOURCE( Effect, System, Compiler );

		~Effect();

		void apply();

		TechniqueId getTechnique( String name ) const;
		void setTechnique( TechniqueId tech );

		void setSRV( Int32 slot, rend::ShaderResourceView srv );
		void setTexture( Int32 slot, rend::Texture2DHandle texture );
		void setSamplerState( Int32 slot, rend::SamplerStateId samplerId );
		void setBlendState( rend::BlendStateId blendState );
		void setData( const void* data, SizeT size, SizeT offset );

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

		void setBool( SizeT offset, Bool value )
		{
			setData( &value, sizeof( Bool ), offset );
		}

	private:
		////////////////////////////////////////////
		static const SizeT CONSTANT_BUFFER_SIZE = 1024;
		static const SizeT MAX_TEXTURES = 2;

		struct Buffer
		{
			rend::ConstantBufferHandle bufferHandle;
			UInt8 data[CONSTANT_BUFFER_SIZE];
			Bool dirty = false;
		};





		StaticArray<Buffer, 1> m_buffers;
		StaticArray<rend::SamplerStateId, MAX_TEXTURES> m_samplerStates;
		StaticArray<rend::ShaderResourceView, MAX_TEXTURES> m_srvs;
		rend::BlendStateId m_blendState;
		////////////////////////////////////////////////


		struct ApiShader
		{
			rend::EShaderType type = rend::EShaderType::ST_MAX;
			rend::ShaderHandle handle = INVALID_HANDLE<rend::ShaderHandle>();
		};

		// permanent tables
		Array<ApiShader> m_apiShaders;
		Array<Technique> m_techniques;

		TechniqueId m_currentTechnique;

		rend::Device* m_device;

		Effect() = delete;
		Effect( String name, rend::Device* device );

		Bool reload( const res::CompiledResource& compiledResource );
		void destroyShaders();

		friend class System;
	};
}
}