//-----------------------------------------------------------------------------
//	Effect.cpp: A FFX effect implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "FFX.h"

namespace flu
{
namespace ffx
{
	Effect::Effect( String name, rend::Device* device )
		:	m_name( name ),
			m_device( device )
	{
		assert( device );

		for( auto& it : m_buffers )
		{
			it.bufferHandle = m_device->createConstantBuffer( CONSTANT_BUFFER_SIZE, rend::EUsage::Dynamic, nullptr, *wide2AnsiString( m_name ) );
			mem::zero( &it.data[0], CONSTANT_BUFFER_SIZE );
		}

		for( auto& it : m_samplerStates )
		{
			it = rend::SamplerState::INVALID;
		}

		for( auto& it : m_srvs )
		{
			it.srv = nullptr;
		}

		m_blendState = rend::BlendState::INVALID;
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

	Bool Effect::reload( const res::CompiledResource& compiledResource )
	{
		IInputStream::Ptr stream = new BufferReader( compiledResource.data );
		debug( L"Reloading of \"%s\"", *m_name );

		String ffxVersion, apiCompilerMark;
		*stream >> ffxVersion;
		*stream >> apiCompilerMark;

		if( ffxVersion != FFX_VERSION )
		{
			error( L"Incompatible effect version \"%s\"", *ffxVersion );
			return false;
		}
		if( apiCompilerMark != m_device->compilerMark() )
		{
			error( L"Unsuitable compiled effect api \"%s\"", apiCompilerMark );
			return false;
		}

		// load vertex declaration
		rend::VertexDeclaration vertexDeclaration;
		*stream >> vertexDeclaration;
		assert( vertexDeclaration.isValid() );

		// load api shaders
		rend::CompiledShader compiledPS, compiledVS;
		*stream >> compiledPS;
		*stream >> compiledVS;

		if( !compiledPS.isValid() || !compiledVS.isValid() )
		{
			error( L"Effect checksum mismatched" );
			return false;
		}

		if( stream->hasError() )
		{
			error( L"Unexpected end of effect file" );
			return false;
		}

		cleanup();

		m_shader.ps = m_device->createPixelShader( compiledPS, *wide2AnsiString( m_name ) );
		m_shader.vs = m_device->createVertexShader( compiledVS, vertexDeclaration, *wide2AnsiString( m_name ) );

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

		m_device->setConstantBuffers( rend::EShaderType::Vertex, EConstantBufferType::CBT_PerEffect, 1, &m_buffers[0].bufferHandle );
		m_device->setConstantBuffers( rend::EShaderType::Pixel, EConstantBufferType::CBT_PerEffect, 1, &m_buffers[0].bufferHandle );

		m_device->applyBlendState( m_blendState );

		m_device->setVertexShader( m_shader.vs );
		m_device->setPixelShader( m_shader.ps );
	}
}
}