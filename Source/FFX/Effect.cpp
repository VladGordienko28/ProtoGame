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
			it.bufferHandle = m_device->createConstantBuffer( CONSTANT_BUFFER_SIZE, 
				rend::EUsage::Dynamic, nullptr, *wide2AnsiString( m_name ) );

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
		destroyShaders();

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

	void Effect::setTexture( Int32 slot, rend::Texture2DHandle texture )
	{
		rend::ShaderResourceView srv = m_device->getShaderResourceView( texture );
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
		BufferReader stream( compiledResource.data );

		String ffxVersion, apiCompilerMark;
		stream >> ffxVersion;
		stream >> apiCompilerMark;

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
		stream >> vertexDeclaration;
		assert( vertexDeclaration.isValid() );

		// load api shaders
		destroyShaders();

		Array<rend::CompiledShader> compiledShaders;
		stream >> compiledShaders;

		for( const auto& it: compiledShaders )
		{
			if( !it.isValid() )
			{
				error( L"Effect checksum mismatched" );
				return false;
			}

			switch ( it.getType() )
			{
				case rend::EShaderType::ST_Vertex:
				{
					ApiShader vs;
					vs.type = rend::EShaderType::ST_Vertex;
					vs.handle =  m_device->createVertexShader( it, 
						vertexDeclaration, *wide2AnsiString( m_name + TXT("_VS") ) );

					m_apiShaders.push( vs );
					break;
				}
				case rend::EShaderType::ST_Pixel:
				{
					ApiShader ps;
					ps.type = rend::EShaderType::ST_Pixel;
					ps.handle =  m_device->createPixelShader( it, 
						*wide2AnsiString( m_name + TXT("_PS") ) );

					m_apiShaders.push( ps );
					break;
				}
				default:
				{
					fatal( TXT("Unknown shader type %d"), it.getType() );
					break;
				}
			}
		}

		// load techniques
		stream >> m_techniques;

		assert( m_techniques.size() > 0 );
		m_currentTechnique = 0;

		// final validation
		if( stream.hasError() )
		{
			error( L"Unexpected end of effect file" );
			return false;
		}

		return true;
	}

	void Effect::destroyShaders()
	{
		for( auto& it : m_apiShaders )
		{
			switch ( it.type )
			{
				case rend::EShaderType::ST_Vertex:
					m_device->destroyVertexShader( it.handle );
					break;

				case rend::EShaderType::ST_Pixel:
					m_device->destroyPixelShader( it.handle );
					break;

				default:
					fatal( TXT("Unknown shader type %d"), it.type );
					break;
			}
		}

		m_apiShaders.empty();
		m_techniques.empty();
		m_currentTechnique = INVALID_TECHNIQUE;
	}

	void Effect::apply()
	{
		assert( m_currentTechnique != INVALID_TECHNIQUE );

		// bind shaders
		const Technique& tech = m_techniques[m_currentTechnique];

		m_device->setVertexShader( m_apiShaders[tech.shaderIds[rend::EShaderType::ST_Vertex]].handle );
		m_device->setPixelShader(  m_apiShaders[tech.shaderIds[rend::EShaderType::ST_Pixel]].handle );

		// bind buffers
		for( auto& it : m_buffers )
		{
			if( it.dirty )
			{
				m_device->updateConstantBuffer( it.bufferHandle, &it.data[0], CONSTANT_BUFFER_SIZE );
				it.dirty = false;
			}
		}
		
		m_device->setSamplerStates( rend::EShaderType::ST_Vertex, 0, MAX_TEXTURES, &m_samplerStates[0] );
		m_device->setSamplerStates( rend::EShaderType::ST_Pixel, 0, MAX_TEXTURES, &m_samplerStates[0] );

		m_device->setSRVs( rend::EShaderType::ST_Vertex, 0, MAX_TEXTURES, &m_srvs[0] );
		m_device->setSRVs( rend::EShaderType::ST_Pixel, 0, MAX_TEXTURES, &m_srvs[0] );

		m_device->setConstantBuffers( rend::EShaderType::ST_Vertex, EConstantBufferType::CBT_PerEffect, 1, &m_buffers[0].bufferHandle );
		m_device->setConstantBuffers( rend::EShaderType::ST_Pixel, EConstantBufferType::CBT_PerEffect, 1, &m_buffers[0].bufferHandle );

		m_device->applyBlendState( m_blendState );
	}

	TechniqueId Effect::getTechnique( String name ) const
	{
		for( Int32 i = 0; i < m_techniques.size(); ++i )
		{
			if( name == m_techniques[i].name )
			{
				return i;
			}
		}

		return INVALID_TECHNIQUE;
	}

	void Effect::setTechnique( TechniqueId tech )
	{
		assert( tech != INVALID_TECHNIQUE );
		m_currentTechnique = tech;
	}
}
}