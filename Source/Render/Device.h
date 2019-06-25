//-----------------------------------------------------------------------------
//	Device.h: An abstract render device
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace rend
{
	/**
	 *	An abstract rendering device
	 */
	class Device
	{
	public:
		using UPtr = UniquePtr<Device>;

		Device() = default;
		virtual ~Device() = default;

		virtual void resize( UInt32 width, UInt32 height, Bool fullScreen = false ) = 0;

		virtual void beginFrame() = 0;
		virtual void endFrame( Bool lockToVSync = false ) = 0;

		virtual Texture1DHandle createTexture1D( EFormat format, Int32 width, Int32 mips = 1, 
			EUsage usage = EUsage::Immutable, const void* initialData = nullptr, const AnsiChar* debugName = nullptr ) = 0;
		virtual void updateTexture1D( Texture1DHandle handle, const void* newData ) = 0;
		virtual void destroyTexture1D( Texture1DHandle handle ) = 0;

		virtual Texture2DHandle createTexture2D( EFormat format, Int32 width, Int32 height, Int32 mips = 1, 
			EUsage usage = EUsage::Immutable, const void* initialData = nullptr, const AnsiChar* debugName = nullptr ) = 0;
		virtual void updateTexture2D( Texture2DHandle handle, const void* newData ) = 0;
		virtual void destroyTexture2D( Texture2DHandle handle ) = 0;

		virtual RenderTargetHandle createRenderTarget( EFormat format, Int32 width, Int32 height, 
			const AnsiChar* debugName = nullptr ) = 0;
		virtual void destroyRenderTarget( RenderTargetHandle handle ) = 0;

		virtual ShaderHandle createPixelShader( const CompiledShader& shader, const AnsiChar* debugName = nullptr ) = 0;
		virtual void destroyPixelShader( ShaderHandle handle ) = 0;

		virtual ShaderHandle createVertexShader( const CompiledShader& shader, const VertexDeclaration& vertexDecl,
			const AnsiChar* debugName = nullptr ) = 0;
		virtual void destroyVertexShader( ShaderHandle handle ) = 0;

		virtual VertexBufferHandle createVertexBuffer( UInt32 vertexSize, UInt32 numVerts, 
			EUsage usage = EUsage::Dynamic, const void* initialData = nullptr, const AnsiChar* debugName = nullptr  ) = 0;
		virtual void updateVertexBuffer( VertexBufferHandle handle, const void* newData, UInt32 dataSize ) = 0;
		virtual void destroyVertexBuffer( VertexBufferHandle handle ) = 0;

		virtual IndexBufferHandle createIndexBuffer( EFormat indexFormat, UInt32 numIndexes, 
			EUsage usage = EUsage::Dynamic, const void* initialData = nullptr, const AnsiChar* debugName = nullptr  ) = 0;
		virtual void destroyIndexBuffer( IndexBufferHandle handle ) = 0;

		virtual ConstantBufferHandle createConstantBuffer( UInt32 bufferSize, EUsage usage = EUsage::Dynamic, 
			const void* initialData = nullptr, const AnsiChar* debugName = nullptr ) = 0;
		virtual void updateConstantBuffer( ConstantBufferHandle handle, const void* newData, UInt32 dataSize ) = 0;
		virtual void destroyConstantBuffer( ConstantBufferHandle handle ) = 0;

		virtual BlendStateId getBlendState( const BlendState& blendState ) = 0;
		virtual void enableBlendState( BlendStateId blendStateId ) = 0;
		virtual void disableBlendState() = 0;

		virtual SamplerStateId getSamplerState( const SamplerState& samplerState ) = 0;

		virtual void clearRenderTarget( RenderTargetHandle handle, const math::FloatColor& clearColor ) = 0;

		virtual void setVertexBuffer( VertexBufferHandle handle ) = 0;
		virtual void setIndexBuffer( IndexBufferHandle handle ) = 0;

		virtual void setVertexShader( ShaderHandle vertexShaderHandle ) = 0;
		virtual void setPixelShader( ShaderHandle pixelShaderHandle ) = 0;

		virtual void setSRVs( EShaderType shader, UInt32 firstSlot, UInt32 numSlots, ShaderResourceView* resourceViews ) = 0;
		virtual void setSamplerStates( EShaderType shader, UInt32 firstSlot, UInt32 numSlots, SamplerStateId* ids ) = 0;
		virtual void setConstantBuffers( EShaderType shader, UInt32 firstSlot, UInt32 numSlots, ConstantBufferHandle* buffers ) = 0;

		virtual void setTopology( EPrimitiveTopology topology ) = 0;

		virtual void drawIndexed( UInt32 indexCount, UInt32 startIndexLocation = 0, Int32 baseVertexOffset = 0 ) = 0;
		virtual void draw( UInt32 vertexCount, UInt32 startVertexLocation = 0 ) = 0;

		virtual void setViewport( const Viewport& viewport ) = 0;

		virtual Int32 getBackbufferWidth() const = 0;
		virtual Int32 getBackbufferHeight() const = 0;

		virtual void enterZone( const Char* zoneName ) = 0;
		virtual void leaveZone() = 0;

		virtual ShaderResourceView getShaderResourceView( Texture1DHandle handle ) = 0;
		virtual ShaderResourceView getShaderResourceView( Texture2DHandle handle ) = 0;
		virtual ShaderResourceView getShaderResourceView( RenderTargetHandle handle ) = 0;

		virtual ShaderCompiler* createCompiler() const = 0;
		virtual String compilerMark() const = 0;

	private:
		Device( const Device& ) = delete;
		Device( Device&& ) = delete;
		Device& operator=( const Device& ) = delete;
		Device& operator=( Device&& ) = delete;
	};
}
}