//-----------------------------------------------------------------------------
//	Effect.cpp: A FFX effect implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "FFX.h"

namespace flu
{
namespace ffx
{
	Effect::Effect( String name, const rend::VertexDeclaration& vertexDeclaration, rend::Device* device )
		:	m_name( name ),
			m_vertexDeclaration( vertexDeclaration ),
			m_device( device )
	{
		assert( device );
		assert( vertexDeclaration.isValid() );

		for( auto& it : m_buffers )
		{
			it.bufferHandle = m_device->createConstantBuffer( CONSTANT_BUFFER_SIZE, rend::EUsage::Dynamic, nullptr, *wide2AnsiString( m_name ) );
			mem::zero( &it.data[0], CONSTANT_BUFFER_SIZE );
		}

		for( auto& it : m_samplerStates )
		{
			it = -1;
		}

		for( auto& it : m_srvs )
		{
			it.srv = nullptr;
		}

		m_blendState = -1;
	}

	Effect::~Effect()
	{
		cleanup();

		for( auto& it : m_buffers )
		{
			m_device->destroyConstantBuffer( it.bufferHandle );
			it.bufferHandle = INVALID_HANDLE<rend::ConstantBufferHandle>();
		}
	}

	void Effect::setData( const void* data, SizeT size, SizeT offset )
	{
		assert( data && size );
		assert( offset + size <= CONSTANT_BUFFER_SIZE );

		mem::copy( &m_buffers[0].data[offset], data, size );
		m_buffers[0].dirty = true;
	}

	void Effect::setSRV( Int32 slot, rend::ShaderResourceView srv )
	{
		m_srvs[slot] = srv;
	}

	void Effect::setSamplerState( Int32 slot, rend::SamplerStateId samplerId )
	{
		m_samplerStates[slot] = samplerId;
	}

	void Effect::setBlendState( rend::BlendStateId blendState )
	{
		m_blendState = blendState;
	}

	Bool Effect::reload( EffectLoadingContext& context )
	{
		assert( context.includeProvider );
		assert( context.relativeFileName );

		debug( L"Reloading of \"%s\"", *m_name );

		String errorMsg;
		String warnMsg;

		PreprocessorInput input;
		input.relativeFileName = context.relativeFileName;
		input.includeProvider = context.includeProvider;
		input.emitLines = true;

		PreprocessorOutput output;

		if( !preprocess( input, output, &errorMsg ) )
		{
			error( L"Unable to preprocess effect \"%s\" with error: \n%s", *context.relativeFileName, *errorMsg );
			return false;
		}

		rend::ShaderCompiler::UPtr compiler = m_device->createCompiler();

		auto compiledPS = compiler->compile( rend::EShaderType::Pixel, output.source, PS_ENTRY, &warnMsg, &errorMsg );
		if( !compiledPS.isValid() )
		{
			error( L"Unable to compile hlsl pixel shader with error: \n%s", *errorMsg );
			return false;
		}

		auto compiledVS = compiler->compile( rend::EShaderType::Vertex, output.source, VS_ENTRY, &warnMsg, &errorMsg );
		if( !compiledVS.isValid() )
		{
			error( L"Unable to compile hlsl vertex shader with error: \n%s", *errorMsg );
			return false;
		}

		cleanup();

		m_shader.ps = m_device->createPixelShader( compiledPS, *wide2AnsiString( m_name ) );
		m_shader.vs = m_device->createVertexShader( compiledVS, m_vertexDeclaration, *wide2AnsiString( m_name ) );

		context.dependencies = output.dependencies;
		return true;
	}

	void Effect::cleanup()
	{
		if( m_shader.vs != INVALID_HANDLE<rend::ShaderHandle>() )
		{
			m_device->destroyVertexShader( m_shader.vs );
			m_shader.vs = INVALID_HANDLE<rend::ShaderHandle>();
		}
	
		if( m_shader.ps != INVALID_HANDLE<rend::ShaderHandle>() )
		{
			m_device->destroyPixelShader( m_shader.ps );
			m_shader.ps = INVALID_HANDLE<rend::ShaderHandle>();
		}
	}

	void Effect::apply()
	{
		for( auto& it : m_buffers )
		{
			if( it.dirty )
			{
				m_device->updateConstantBuffer( it.bufferHandle, &it.data[0], CONSTANT_BUFFER_SIZE );
				it.dirty = false;
			}
		}
		
		m_device->setSamplerStates( rend::EShaderType::Vertex, 0, MAX_TEXTURES, &m_samplerStates[0] );
		m_device->setSamplerStates( rend::EShaderType::Pixel, 0, MAX_TEXTURES, &m_samplerStates[0] );

		m_device->setSRVs( rend::EShaderType::Vertex, 0, MAX_TEXTURES, &m_srvs[0] );
		m_device->setSRVs( rend::EShaderType::Pixel, 0, MAX_TEXTURES, &m_srvs[0] );

		m_device->setConstantBuffers( rend::EShaderType::Vertex, 0, 1, &m_buffers[0].bufferHandle );
		m_device->setConstantBuffers( rend::EShaderType::Pixel, 0, 1, &m_buffers[0].bufferHandle );

		if( m_blendState != -1 )
		{
			m_device->enableBlendState( m_blendState );
		}
		else
		{
			m_device->disableBlendState();
		}

		m_device->setVertexShader( m_shader.vs );
		m_device->setPixelShader( m_shader.ps );
	}
}
}