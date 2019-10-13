//-----------------------------------------------------------------------------
//	Api.cpp: Global rendering api
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Engine/Engine.h"

namespace flu
{
namespace gfx
{
namespace api
{
	static rend::Device* g_device = nullptr;

	void initialize( rend::Device* device )
	{
		assert( g_device == nullptr );
		assert( device );
		
		g_device = device;
	}

	Bool isInitialized()
	{
		return g_device != nullptr;
	}

	void finalize()
	{
		assert( g_device );
		g_device = nullptr;
	}

	rend::VertexBufferHandle createVertexBuffer( UInt32 vertexSize, UInt32 numVerts, 
		rend::EUsage usage, const void* initialData, const AnsiChar* debugName )
	{
		return g_device->createVertexBuffer( vertexSize, numVerts, usage, initialData, debugName );
	}

	void updateVertexBuffer( rend::VertexBufferHandle handle, const void* newData, UInt32 dataSize )
	{
		g_device->updateVertexBuffer( handle, newData, dataSize );
	}

	void destroyVertexBuffer( rend::VertexBufferHandle handle )
	{
		g_device->destroyVertexBuffer( handle );
	}

	rend::IndexBufferHandle createIndexBuffer( rend::EFormat indexFormat, UInt32 numIndexes, 
		rend::EUsage usage, const void* initialData, const AnsiChar* debugName )
	{
		return g_device->createIndexBuffer( indexFormat, numIndexes, usage, initialData, debugName );
	}

	void updateIndexBuffer( rend::IndexBufferHandle handle, const void* newData, UInt32 dataSize )
	{
		g_device->updateIndexBuffer( handle, newData, dataSize );
	}

	void destroyIndexBuffer( rend::IndexBufferHandle handle )
	{
		g_device->destroyIndexBuffer( handle );
	}

	rend::SamplerStateId getSamplerState( const rend::SamplerState& samplerState )
	{
		return g_device->getSamplerState( samplerState );
	}

	rend::BlendStateId getBlendState( const rend::BlendState& blendState )
	{
		return g_device->getBlendState( blendState );
	}

	void setVertexBuffer( rend::VertexBufferHandle handle )
	{
		g_device->setVertexBuffer( handle );
	}

	void setIndexBuffer( rend::IndexBufferHandle handle )
	{
		g_device->setIndexBuffer( handle );
	}

	void setTopology( rend::EPrimitiveTopology topology )
	{
		g_device->setTopology( topology );
	}

	void drawIndexed( UInt32 indexCount, UInt32 startIndexLocation, Int32 baseVertexOffset )
	{
		g_device->drawIndexed( indexCount, startIndexLocation, baseVertexOffset );
	}

	void draw( UInt32 vertexCount, UInt32 startVertexLocation )
	{
		g_device->draw( vertexCount, startVertexLocation );
	}

	void enterZone( const Char* zoneName )
	{
		g_device->enterZone( zoneName );
	}

	void leaveZone()
	{
		g_device->leaveZone();
	}

	rend::Texture1DHandle createTexture1D( rend::EFormat format, Int32 width, Int32 mips, 
		rend::EUsage usage, const void* initialData, const AnsiChar* debugName )
	{
		return g_device->createTexture1D( format, width, mips, usage, initialData, debugName );
	}

	void updateTexture1D( rend::Texture1DHandle handle, const void* newData )
	{
		g_device->updateTexture1D( handle, newData );
	}

	void destroyTexture1D( rend::Texture1DHandle handle )
	{
		g_device->destroyTexture1D( handle );
	}

	rend::Texture2DHandle createTexture2D( rend::EFormat format, Int32 width, Int32 height, Int32 mips, 
		rend::EUsage usage, const void* initialData, const AnsiChar* debugName )
	{
		return g_device->createTexture2D( format, width, height, mips, usage, initialData, debugName );
	}

	void updateTexture2D( rend::Texture2DHandle handle, const void* newData )
	{
		g_device->updateTexture2D( handle, newData );
	}

	void destroyTexture2D( rend::Texture2DHandle handle )
	{
		g_device->destroyTexture2D( handle );
	}

	rend::ShaderResourceView getShaderResourceView( rend::Texture1DHandle handle )
	{
		return g_device->getShaderResourceView( handle );
	}

	rend::ShaderResourceView getShaderResourceView( rend::Texture2DHandle handle )
	{
		return g_device->getShaderResourceView( handle );
	}

	rend::ShaderResourceView getShaderResourceView( rend::RenderTargetHandle handle )
	{
		return g_device->getShaderResourceView( handle );
	}
	rend::ShaderResourceView getShaderResourceView( rend::DepthBufferHandle handle )
	{
		return g_device->getShaderResourceView( handle );
	}
}
}
}