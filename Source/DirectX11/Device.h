//-----------------------------------------------------------------------------
//	Device.h: A DirectX 11 rendering device
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace dx11
{
	/**
	 *	A DirectX 11 rendering device
	 */
	class Device: public rend::Device
	{
	public:
		Device( HWND hwnd, UInt32 width, UInt32 height, Bool fullscreen = false );
		~Device();

		void resize( UInt32 width, UInt32 height, Bool fullScreen = false ) override;

		void beginFrame() override;
		void endFrame( Bool lockToVSync ) override;

		rend::Texture1DHandle createTexture1D( rend::EFormat format, Int32 width, Int32 mips, 
			rend::EUsage usage, const void* initialData, const AnsiChar* debugName ) override;
		void updateTexture1D( rend::Texture1DHandle handle, const void* newData ) override;
		void destroyTexture1D( rend::Texture1DHandle handle ) override;

		rend::Texture2DHandle createTexture2D( rend::EFormat format, Int32 width, Int32 height, Int32 mips, 
			rend::EUsage usage, const void* initialData, const AnsiChar* debugName ) override;
		void updateTexture2D( rend::Texture2DHandle handle, const void* newData ) override;
		void destroyTexture2D( rend::Texture2DHandle handle ) override;

		rend::RenderTargetHandle createRenderTarget( rend::EFormat format, Int32 width, Int32 height, 
			const AnsiChar* debugName ) override;
		void destroyRenderTarget( rend::RenderTargetHandle handle ) override;

		rend::ShaderHandle createPixelShader( const rend::CompiledShader& shader, const AnsiChar* debugName ) override;
		void destroyPixelShader( rend::ShaderHandle handle ) override;

		rend::ShaderHandle createVertexShader( const rend::CompiledShader& shader, const rend::VertexDeclaration& vertexDecl,
			const AnsiChar* debugName ) override;
		void destroyVertexShader( rend::ShaderHandle handle ) override;

		rend::VertexBufferHandle createVertexBuffer( UInt32 vertexSize, UInt32 numVerts, 
			rend::EUsage usage, const void* initialData, const AnsiChar* debugName  ) override;
		void updateVertexBuffer( rend::VertexBufferHandle handle, const void* newData, UInt32 dataSize ) override;
		void destroyVertexBuffer( rend::VertexBufferHandle handle ) override;

		rend::IndexBufferHandle createIndexBuffer( rend::EFormat indexFormat, UInt32 numIndexes, 
			rend::EUsage usage, const void* initialData, const AnsiChar* debugName ) override;
		void destroyIndexBuffer( rend::IndexBufferHandle handle ) override;

		rend::ConstantBufferHandle createConstantBuffer( UInt32 bufferSize, rend::EUsage usage, 
			const void* initialData, const AnsiChar* debugName ) override;
		void updateConstantBuffer( rend::ConstantBufferHandle handle, const void* newData, UInt32 dataSize ) override;
		void destroyConstantBuffer( rend::ConstantBufferHandle handle ) override;

		rend::BlendStateId getBlendState( const rend::BlendState& blendState ) override;
		void enableBlendState( rend::BlendStateId blendStateId ) override;
		void disableBlendState() override;

		rend::SamplerStateId getSamplerState( const rend::SamplerState& samplerState ) override;

		void clearRenderTarget( rend::RenderTargetHandle handle, const math::FloatColor& clearColor ) override;

		void setVertexBuffer( rend::VertexBufferHandle handle ) override;
		void setIndexBuffer( rend::IndexBufferHandle handle ) override;

		void setVertexShader( rend::ShaderHandle vertexShaderHandle ) override;
		void setPixelShader( rend::ShaderHandle pixelShaderHandle ) override;

		void setSRVs( rend::EShaderType shader, UInt32 firstSlot, UInt32 numSlots, rend::ShaderResourceView* resourceViews ) override;
		void setSamplerStates( rend::EShaderType shader, UInt32 firstSlot, UInt32 numSlots, rend::SamplerStateId* ids ) override;
		void setConstantBuffers( rend::EShaderType shader, UInt32 firstSlot, UInt32 numSlots, 
			rend::ConstantBufferHandle* buffers ) override;

		void setTopology( rend::EPrimitiveTopology topology ) override;

		void drawIndexed( UInt32 indexCount, UInt32 startIndexLocation, Int32 baseVertexOffset ) override;
		void draw( UInt32 vertexCount, UInt32 startVertexLocation ) override;

		void setViewport( const rend::Viewport& viewport ) override;

		Int32 getBackbufferWidth() const override;
		Int32 getBackbufferHeight() const override;

		void enterZone( const Char* zoneName ) override;
		void leaveZone() override;

		rend::ShaderResourceView getShaderResourceView( rend::Texture1DHandle handle ) override;
		rend::ShaderResourceView getShaderResourceView( rend::Texture2DHandle handle ) override;
		rend::ShaderResourceView getShaderResourceView( rend::RenderTargetHandle handle ) override;

		rend::ShaderCompiler* createCompiler() const override;

	private:
		SwapChain::UPtr m_swapChain;

		DxRef<ID3D11Device> m_device;
		DxRef<ID3D11DeviceContext> m_immediateContext;

		DxRef<ID3DUserDefinedAnnotation> m_userDefinedAnnotation;

		static const SizeT MAX_TEXTURES_1D = 64;
		static const SizeT MAX_TEXTURES_2D = 2048;
		static const SizeT MAX_RENDER_TARGETS = 32;
		static const SizeT MAX_PIXEL_SHADERS = 128;
		static const SizeT MAX_VERTEX_SHADERS = 128;
		static const SizeT MAX_VERTEX_BUFFERS = 256;
		static const SizeT MAX_INDEX_BUFFERS = 128;
		static const SizeT MAX_CONSTANT_BUFFERS = 512;

		// all resources
		HandleArray<rend::Texture1DHandle, DxTexture1D, MAX_TEXTURES_1D> m_textures1D;
		HandleArray<rend::Texture2DHandle, DxTexture2D, MAX_TEXTURES_2D> m_textures2D;
		HandleArray<rend::RenderTargetHandle, DxRenderTarget, MAX_RENDER_TARGETS> m_renderTargets;
		HandleArray<rend::ShaderHandle, DxPixelShader, MAX_PIXEL_SHADERS> m_pixelShaders;
		HandleArray<rend::ShaderHandle, DxVertexShader, MAX_VERTEX_SHADERS> m_vertexShaders;
		HandleArray<rend::VertexBufferHandle, DxVertexBuffer, MAX_VERTEX_BUFFERS> m_vertexBuffers;
		HandleArray<rend::IndexBufferHandle, DxIndexBuffer, MAX_INDEX_BUFFERS> m_indexBuffers;
		HandleArray<rend::ConstantBufferHandle, DxConstantBuffer, MAX_CONSTANT_BUFFERS> m_constantBuffers;

	private:
		struct CachedVertexDeclaration
		{
		public:
			rend::VertexDeclaration m_vertexDecl;
			DxRef<ID3D11InputLayout> m_inputLayout;

			CachedVertexDeclaration( ID3D11Device* device, const rend::VertexDeclaration& declaration, 
				const rend::CompiledShader& vertexShader );
			~CachedVertexDeclaration();

		private:
			CachedVertexDeclaration() = delete;
		};

		Map<UInt32, CachedVertexDeclaration> m_vertexDeclarations;

		CachedVertexDeclaration* getVertexDeclaration( UInt32 hash );
		UInt32 createVertexDeclaration( const rend::VertexDeclaration& declaration, const rend::CompiledShader& vertexShader );
	
	private:
		Map<rend::SamplerStateId, DxRef<ID3D11SamplerState>> m_samplerStates;
		Map<rend::BlendStateId, DxRef<ID3D11BlendState>> m_blendStates;

		rend::EPrimitiveTopology m_oldTopology;
	};
}
}