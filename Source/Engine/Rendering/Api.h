//-----------------------------------------------------------------------------
//	Api.h: Global rendering api
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace gfx
{
namespace api
{
	extern void initialize( rend::Device* device );
	extern Bool isInitialized();
	extern void finalize();

	extern rend::VertexBufferHandle createVertexBuffer( UInt32 vertexSize, UInt32 numVerts, 
		rend::EUsage usage, const void* initialData, const AnsiChar* debugName );
	extern void updateVertexBuffer( rend::VertexBufferHandle handle, const void* newData, UInt32 dataSize );
	extern void destroyVertexBuffer( rend::VertexBufferHandle handle );

	extern rend::IndexBufferHandle createIndexBuffer( rend::EFormat indexFormat, UInt32 numIndexes, 
		rend::EUsage usage, const void* initialData, const AnsiChar* debugName );
	extern void updateIndexBuffer( rend::IndexBufferHandle handle, const void* newData, UInt32 dataSize );
	extern void destroyIndexBuffer( rend::IndexBufferHandle handle );

	extern rend::SamplerStateId getSamplerState( const rend::SamplerState& samplerState );
	extern rend::BlendStateId getBlendState( const rend::BlendState& blendState );

	extern void setVertexBuffer( rend::VertexBufferHandle handle );
	extern void setIndexBuffer( rend::IndexBufferHandle handle );

	extern void setTopology( rend::EPrimitiveTopology topology );

	extern void drawIndexed( UInt32 indexCount, UInt32 startIndexLocation, Int32 baseVertexOffset );
	extern void draw( UInt32 vertexCount, UInt32 startVertexLocation );

	extern void enterZone( const Char* zoneName );
	extern void leaveZone();

	extern rend::Texture1DHandle createTexture1D( rend::EFormat format, Int32 width, Int32 mips, 
		rend::EUsage usage, const void* initialData, const AnsiChar* debugName );
	extern void updateTexture1D( rend::Texture1DHandle handle, const void* newData );
	extern void destroyTexture1D( rend::Texture1DHandle handle );

	extern rend::Texture2DHandle createTexture2D( rend::EFormat format, Int32 width, Int32 height, Int32 mips, 
		rend::EUsage usage, const void* initialData, const AnsiChar* debugName );
	extern void updateTexture2D( rend::Texture2DHandle handle, const void* newData );
	extern void destroyTexture2D( rend::Texture2DHandle handle );

	extern rend::ShaderResourceView getShaderResourceView( rend::Texture1DHandle handle );
	extern rend::ShaderResourceView getShaderResourceView( rend::Texture2DHandle handle );
	extern rend::ShaderResourceView getShaderResourceView( rend::RenderTargetHandle handle );
	extern rend::ShaderResourceView getShaderResourceView( rend::DepthBufferHandle handle );

	extern void enterGPUProfileZone( const Char* zoneName );
	extern void leaveGPUProfileZone();
}
}
}