//-----------------------------------------------------------------------------
//	Device.cpp: A DirectX 11 rendering device implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "DirectX11.h"

#if FLU_PROFILE_GPU
	#define GPU_STAT(x) x
#else
	#define GPU_STAT(x)
#endif

namespace flu
{
namespace dx11
{
#if FLU_PLATFORM_XBOX
	Device::Device( IUnknown* coreWindow, UInt32 width, UInt32 height, Bool fullscreen )
#else
	Device::Device( HWND hwnd, UInt32 width, UInt32 height, Bool fullscreen )
#endif
		:	rend::Device()
	{
		const D3D_FEATURE_LEVEL allFeatureLevels[] =
		{
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};

		D3D_FEATURE_LEVEL selectedFeatureLevel = D3D_FEATURE_LEVEL_11_0;

		UINT deviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;

#if FLU_DEBUG
		deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT result = D3D11CreateDevice( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags, 
			allFeatureLevels, arraySize( allFeatureLevels ), D3D11_SDK_VERSION, &m_device, &selectedFeatureLevel, &m_immediateContext );

		if( FAILED( result ) )
		{
			fatal( L"Unable to create DirectX11 device and immediate context with error %d", result );
		}

		result = m_immediateContext->QueryInterface( __uuidof( ID3DUserDefinedAnnotation ), 
			reinterpret_cast<void**>( &m_userDefinedAnnotation ) );
		assert( SUCCEEDED( result ) );

		info( L"Device successfully created with feature level %s", *featureLevelToString( selectedFeatureLevel ) );

#if FLU_PLATFORM_XBOX
		m_swapChain = new SwapChain( m_device, coreWindow, width, height, fullscreen );
#else
		m_swapChain = new SwapChain( m_device, hwnd, width, height, fullscreen );
#endif

		// set initial render target and viewport
		rend::Viewport initialViewport = { 0.f, 0.f, Float( width ), Float( height ), 0.f, 1.f };

		this->setViewport( initialViewport );
		m_immediateContext->OMSetRenderTargets( 1, &m_swapChain->getBackBufferRTV(), nullptr );

		m_oldBlendStateId = rend::BlendState::INVALID;
		m_oldDepthStencilStateId = rend::DepthStencilState::INVALID;

		m_oldTopology = rend::EPrimitiveTopology::Unknown;

		// create render states
		D3D11_RASTERIZER_DESC rasterState;
		rasterState.FillMode = D3D11_FILL_SOLID;
		rasterState.CullMode = D3D11_CULL_NONE;
		rasterState.FrontCounterClockwise = FALSE;
		rasterState.DepthBias = 0;
		rasterState.DepthBiasClamp = 0.f;
		rasterState.SlopeScaledDepthBias = 0.f;
		rasterState.DepthClipEnable = FALSE;
		rasterState.MultisampleEnable = FALSE;
		rasterState.AntialiasedLineEnable = FALSE;

		rasterState.ScissorEnable = FALSE;
		m_device->CreateRasterizerState( &rasterState, &m_defaultRasterState );

		rasterState.ScissorEnable = TRUE;
		m_device->CreateRasterizerState( &rasterState, &m_scissorRasterState );

		m_immediateContext->RSSetState( m_defaultRasterState );
		m_scissorTestEnabled = false;

		// invalidate all slots caches
		for( auto& it : m_constantBuffersCache )
		{
			it.invalidate( INVALID_HANDLE<rend::ConstantBufferHandle>() );
		}
		for( auto& it : m_samplerStatesCache )
		{
			it.invalidate( rend::SamplerState::INVALID );
		}
		for( auto& it : m_srvsCache )
		{
			it.invalidate( rend::ShaderResourceView() );
		}
	}

	Device::~Device()
	{
		m_userDefinedAnnotation = nullptr;

		m_immediateContext->ClearState();
		m_immediateContext = nullptr;

		m_samplerStates.empty();
		m_vertexDeclarations.empty();
		m_blendStates.empty();
		m_depthStencilStates.empty();

		m_swapChain = nullptr;
		m_device = nullptr;
	}
	
	void Device::resize( UInt32 width, UInt32 height, Bool fullScreen )
	{
		assert( m_swapChain.hasObject() );

		m_swapChain->resize( m_device.get(), width, height, fullScreen );

		rend::Viewport newViewport = { 0.f, 0.f, Float(width), Float(height), 0.f, 1.f };

		this->setViewport( newViewport );
		m_immediateContext->OMSetRenderTargets( 1, &m_swapChain->getBackBufferRTV(), nullptr );
	}

	void Device::beginFrame()
	{
		GPU_STAT( mem::zero( &m_drawStats, sizeof( rend::DrawStats ) ) );

#if FLU_PLATFORM_XBOX
		// Set backbuffer render target view, only for XBox due to different SwapChain settings
		setRenderTarget( INVALID_HANDLE<rend::RenderTargetHandle>(), INVALID_HANDLE<rend::DepthBufferHandle>() );
#endif
	}

	void Device::endFrame( Bool lockToVSync )
	{
		m_swapChain->present( m_device.get(), lockToVSync );
	}

	rend::Texture1DHandle Device::createTexture1D( rend::EFormat format, Int32 width, Int32 mips, 
		rend::EUsage usage, const void* initialData, const AnsiChar* debugName )
	{
		rend::Texture1DHandle handle;

		m_textures1D.emplaceElement( handle )->create( m_device.get(), format, width, mips, usage, initialData, debugName );
		GPU_STAT( m_memoryStats.m_texureBytes += m_textures1D.get( handle ).memoryUsage() );

		return handle;
	}

	void Device::updateTexture1D( rend::Texture1DHandle handle, const void* newData )
	{
		DxTexture1D& texture = m_textures1D.get( handle );
		assert( texture.m_usage == rend::EUsage::Dynamic );
		assert( newData );

		// todo: add ability to pass mapping parameters here
		D3D11_MAPPED_SUBRESOURCE mappedData;
		m_immediateContext->Map( texture.m_texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData );
		{
			const rend::FormatInfo& format = rend::getFormatInfo( texture.m_format );
			const SizeT dataSize = ( texture.m_width * format.blockBytes ) / format.blockSizeX;

			mem::copy( mappedData.pData, newData, dataSize );
		}
		m_immediateContext->Unmap( texture.m_texture, 0 );
	}

	void Device::destroyTexture1D( rend::Texture1DHandle handle )
	{
		DxTexture1D& texture = m_textures1D.get( handle );

		GPU_STAT( m_memoryStats.m_texureBytes -= texture.memoryUsage() );
		texture.destroy( m_device.get() );

		m_textures1D.removeElement( handle );
	}

	rend::Texture2DHandle Device::createTexture2D( rend::EFormat format, Int32 width, Int32 height, Int32 mips, 
		rend::EUsage usage, const void* initialData, const AnsiChar* debugName )
	{
		rend::Texture2DHandle handle;

		m_textures2D.emplaceElement( handle )->create( m_device.get(), format, width, height, mips, usage, initialData, debugName );
		GPU_STAT( m_memoryStats.m_texureBytes += m_textures2D.get( handle ).memoryUsage() );

		return handle;
	}

	void Device::updateTexture2D( rend::Texture2DHandle handle, const void* newData )
	{
		DxTexture2D& texture = m_textures2D.get( handle );
		assert( texture.m_usage == rend::EUsage::Dynamic );
		assert( newData );

		// todo: add ability to pass mapping parameters here
		D3D11_MAPPED_SUBRESOURCE mappedData;
		m_immediateContext->Map( texture.m_texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData );
		{
			const rend::FormatInfo& format = rend::getFormatInfo( texture.m_format );
			const SizeT dataSize = ( texture.m_width * texture.m_height * format.blockBytes ) /
				( format.blockSizeX * format.blockSizeY );

			mem::copy( mappedData.pData, newData, dataSize );
		}
		m_immediateContext->Unmap( texture.m_texture, 0 );
	}

	void Device::destroyTexture2D( rend::Texture2DHandle handle )
	{
		DxTexture2D& texture = m_textures2D.get( handle );

		GPU_STAT( m_memoryStats.m_texureBytes -= texture.memoryUsage() );
		texture.destroy( m_device.get() );

		m_textures2D.removeElement( handle );
	}

	rend::RenderTargetHandle Device::createRenderTarget( rend::EFormat format, Int32 width, Int32 height, 
		const AnsiChar* debugName )
	{
		rend::RenderTargetHandle handle;

		m_renderTargets.emplaceElement( handle )->create( m_device.get(), format, width, height, debugName );
		GPU_STAT( m_memoryStats.m_texureBytes += m_renderTargets.get( handle ).memoryUsage() );

		return handle;
	}

	void Device::destroyRenderTarget( rend::RenderTargetHandle handle )
	{
		DxRenderTarget& renderTarget = m_renderTargets.get( handle );

		GPU_STAT( m_memoryStats.m_texureBytes -= renderTarget.memoryUsage() );
		renderTarget.destroy( m_device.get() );

		m_renderTargets.removeElement( handle );
	}

	rend::DepthBufferHandle Device::createDepthBuffer( rend::EFormat format, Int32 width, Int32 height,
		const AnsiChar* debugName )
	{
		rend::DepthBufferHandle handle;

		m_depthBuffers.emplaceElement( handle )->create( m_device.get(), format, width, height, debugName );
		GPU_STAT( m_memoryStats.m_texureBytes += m_depthBuffers.get( handle ).memoryUsage() );

		return handle;
	}

	void Device::destroyDepthBuffer( rend::DepthBufferHandle handle )
	{
		DxDepthBuffer& depthBuffer = m_depthBuffers.get( handle );

		GPU_STAT( m_memoryStats.m_texureBytes -= depthBuffer.memoryUsage() );
		depthBuffer.destroy( m_device.get() );

		m_depthBuffers.removeElement( handle );
	}

	rend::ShaderHandle Device::createPixelShader( const rend::CompiledShader& shader, const AnsiChar* debugName )
	{
		rend::ShaderHandle handle;

		m_pixelShaders.emplaceElement( handle )->create( m_device.get(), shader, debugName );
		return handle;
	}

	void Device::destroyPixelShader( rend::ShaderHandle handle )
	{
		DxPixelShader& pixelShader = m_pixelShaders.get( handle );
		pixelShader.destroy( m_device.get() );

		m_pixelShaders.removeElement( handle );
	}

	rend::ShaderHandle Device::createVertexShader( const rend::CompiledShader& shader, const rend::VertexDeclaration& vertexDecl,
		const AnsiChar* debugName )
	{
		UInt32 vertexDeclId = 0;

		if( auto cachedVertexDecl = getVertexDeclaration( vertexDecl.getHash() ) )
		{
			// found in cache
			vertexDeclId = vertexDecl.getHash();
		}
		else
		{
			// not in cache yet
			vertexDeclId = createVertexDeclaration( vertexDecl, shader );
		}

		rend::ShaderHandle handle;
		m_vertexShaders.emplaceElement( handle )->create( m_device.get(), shader, vertexDeclId, debugName );

		return handle;
	}

	void Device::destroyVertexShader( rend::ShaderHandle handle )
	{
		DxVertexShader& vertexShader = m_vertexShaders.get( handle );
		vertexShader.destroy( m_device.get() );

		m_vertexShaders.removeElement( handle );
	}

	rend::VertexBufferHandle Device::createVertexBuffer( UInt32 vertexSize, UInt32 numVerts, 
		rend::EUsage usage, const void* initialData, const AnsiChar* debugName  )
	{
		rend::VertexBufferHandle handle;

		m_vertexBuffers.emplaceElement( handle )->create( m_device.get(), vertexSize, numVerts, usage, initialData, debugName );
		GPU_STAT( m_memoryStats.m_vertexBufferBytes += m_vertexBuffers.get( handle ).memoryUsage() );

		return handle;
	}

	void Device::updateVertexBuffer( rend::VertexBufferHandle handle, const void* newData, UInt32 dataSize )
	{
		DxVertexBuffer& buffer = m_vertexBuffers.get( handle );
		assert( buffer.m_usage == rend::EUsage::Dynamic );
		assert( dataSize <= buffer.m_numVerts * buffer.m_vertexSize );
		assert( newData );

		// todo: add ability to pass mapping parameters here
		D3D11_MAPPED_SUBRESOURCE mappedData;
		m_immediateContext->Map( buffer.m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData );
		{
			mem::copy( mappedData.pData, newData, dataSize );
		}
		m_immediateContext->Unmap( buffer.m_buffer, 0 );
	}

	void Device::destroyVertexBuffer( rend::VertexBufferHandle handle )
	{
		DxVertexBuffer& vertexBuffer = m_vertexBuffers.get( handle );

		GPU_STAT( m_memoryStats.m_vertexBufferBytes -= vertexBuffer.memoryUsage() );
		vertexBuffer.destroy( m_device.get() );

		m_vertexBuffers.removeElement( handle );
	}

	rend::IndexBufferHandle Device::createIndexBuffer( rend::EFormat indexFormat, UInt32 numIndexes, 
		rend::EUsage usage, const void* initialData, const AnsiChar* debugName )
	{
		rend::IndexBufferHandle handle;

		m_indexBuffers.emplaceElement( handle )->create( m_device.get(), indexFormat, numIndexes, usage, initialData, debugName );
		GPU_STAT( m_memoryStats.m_indexBufferBytes += m_indexBuffers.get( handle ).memoryUsage() );

		return handle;
	}

	void Device::updateIndexBuffer( rend::IndexBufferHandle handle, const void* newData, UInt32 dataSize )
	{
		DxIndexBuffer& buffer = m_indexBuffers.get( handle );
		assert( buffer.m_usage == rend::EUsage::Dynamic );
		assert( dataSize <= buffer.m_numIndexes * buffer.m_indexTypeSize );
		assert( newData );
	
		// todo: add ability to pass mapping parameters here
		D3D11_MAPPED_SUBRESOURCE mappedData;
		m_immediateContext->Map( buffer.m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData );
		{
			mem::copy( mappedData.pData, newData, dataSize );
		}
		m_immediateContext->Unmap( buffer.m_buffer, 0 );
	}

	void Device::destroyIndexBuffer( rend::IndexBufferHandle handle )
	{
		DxIndexBuffer& indexBuffer = m_indexBuffers.get( handle );

		GPU_STAT( m_memoryStats.m_indexBufferBytes -= indexBuffer.memoryUsage() );
		indexBuffer.destroy( m_device.get() );

		m_indexBuffers.removeElement( handle );
	}

	rend::ConstantBufferHandle Device::createConstantBuffer( UInt32 bufferSize, rend::EUsage usage, 
		const void* initialData, const AnsiChar* debugName )
	{
		rend::ConstantBufferHandle handle;

		m_constantBuffers.emplaceElement( handle )->create( m_device.get(), bufferSize, usage, initialData, debugName );
		GPU_STAT( m_memoryStats.m_constantBufferBytes += m_constantBuffers.get( handle ).memoryUsage() );

		return handle;
	}

	void Device::updateConstantBuffer( rend::ConstantBufferHandle handle, const void* newData, UInt32 dataSize )
	{
		DxConstantBuffer& buffer = m_constantBuffers.get( handle );
		assert( buffer.m_usage == rend::EUsage::Dynamic );
		assert( dataSize == buffer.m_size );
		assert( newData );

		D3D11_MAPPED_SUBRESOURCE mappedData;
		m_immediateContext->Map( buffer.m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData );
		{
			mem::copy( mappedData.pData, newData, dataSize );
		}
		m_immediateContext->Unmap( buffer.m_buffer, 0 );
	}

	void Device::destroyConstantBuffer( rend::ConstantBufferHandle handle )
	{
		DxConstantBuffer& constantBuffer = m_constantBuffers.get( handle );

		GPU_STAT( m_memoryStats.m_constantBufferBytes -= constantBuffer.memoryUsage() );
		constantBuffer.destroy( m_device.get() );

		m_constantBuffers.removeElement( handle );
	}

	D3D11_BLEND_OP fluorineBlendOpToDirectX( rend::EBlendOp blendOp )
	{
		switch ( blendOp )
		{
			case rend::EBlendOp::Add:			return D3D11_BLEND_OP_ADD;
			case rend::EBlendOp::Subtract:		return D3D11_BLEND_OP_SUBTRACT;
			case rend::EBlendOp::RevSubtract:	return D3D11_BLEND_OP_REV_SUBTRACT;
			case rend::EBlendOp::Min:			return D3D11_BLEND_OP_MIN;
			case rend::EBlendOp::Max:			return D3D11_BLEND_OP_MAX;

			default:
				fatal( L"Unknown blend op %d", blendOp );
				return D3D11_BLEND_OP_ADD;
		}
	}

	D3D11_BLEND fluorineBlendFactorToDirectX( rend::EBlendFactor blendFactor )
	{
		switch( blendFactor )
		{
			case rend::EBlendFactor::Zero:			return D3D11_BLEND_ZERO;
			case rend::EBlendFactor::One:			return D3D11_BLEND_ONE;
			case rend::EBlendFactor::SrcColor:		return D3D11_BLEND_SRC_COLOR;
			case rend::EBlendFactor::InvSrcColor:	return D3D11_BLEND_INV_SRC_COLOR;
			case rend::EBlendFactor::SrcAlpha:		return D3D11_BLEND_SRC_ALPHA;
			case rend::EBlendFactor::InvSrcAlpha:	return D3D11_BLEND_INV_SRC_ALPHA;
			case rend::EBlendFactor::DestAlpha:		return D3D11_BLEND_DEST_ALPHA;
			case rend::EBlendFactor::InvDestAlpha:	return D3D11_BLEND_INV_DEST_ALPHA;
			case rend::EBlendFactor::DestColor:		return D3D11_BLEND_DEST_COLOR;
			case rend::EBlendFactor::InvDestColor:	return D3D11_BLEND_INV_DEST_COLOR;
			case rend::EBlendFactor::SrcAlphaSat:	return D3D11_BLEND_SRC_ALPHA_SAT;

			default:
				fatal( L"Unknown blend factor %d", blendFactor );
				return D3D11_BLEND_ONE;
		}
	}

	rend::BlendStateId Device::getBlendState( const rend::BlendState& blendState )
	{
		rend::BlendStateId requiredBlendHash = blendState.getHash();

		if( m_blendStates.hasKey( requiredBlendHash ) )
		{
			// reuse blend state from cache
			return requiredBlendHash;
		}
		else
		{
			// create new blend state
			D3D11_BLEND_DESC blendDescription;
			mem::zero( &blendDescription, sizeof( D3D11_BLEND_DESC ) );

			blendDescription.AlphaToCoverageEnable = false;
			blendDescription.IndependentBlendEnable = false;

			auto& firstRT = blendDescription.RenderTarget[0];
			firstRT.BlendEnable = true;
			firstRT.SrcBlend = fluorineBlendFactorToDirectX( blendState.getSrcFactor() );
			firstRT.DestBlend = fluorineBlendFactorToDirectX( blendState.getDestFactor() );
			firstRT.BlendOp = fluorineBlendOpToDirectX( blendState.getOp() );
			firstRT.SrcBlendAlpha = fluorineBlendFactorToDirectX( blendState.getSrcAlphaFactor() );
			firstRT.DestBlendAlpha = fluorineBlendFactorToDirectX( blendState.getDestAlphaFactor() );
			firstRT.BlendOpAlpha = fluorineBlendOpToDirectX( blendState.getAlphaOp() );
			firstRT.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			DxRef<ID3D11BlendState> dxBlendState;
			HRESULT result = m_device->CreateBlendState( &blendDescription, &dxBlendState );
			assert( SUCCEEDED( result ) );

			m_blendStates.put( requiredBlendHash, dxBlendState );
			return requiredBlendHash;
		}
	}

	void Device::applyBlendState( rend::BlendStateId blendStateId )
	{
		if( blendStateId != m_oldBlendStateId )
		{
			if( blendStateId != rend::BlendState::INVALID )
			{
				DxRef<ID3D11BlendState>& blendState = m_blendStates.getRef( blendStateId );
				assert( blendState.hasObject() );

				m_immediateContext->OMSetBlendState( blendState.get(), nullptr, MAX_UINT32 );
			}
			else
			{
				m_immediateContext->OMSetBlendState( nullptr, nullptr, MAX_UINT32 );
			}

			GPU_STAT( m_drawStats.m_blendStateSwitches++ );
			m_oldBlendStateId = blendStateId;
		}
	}

	D3D11_COMPARISON_FUNC fluorineComparsionFuncToDirectX( rend::EComparsionFunc func )
	{
		switch( func )
		{
			case rend::EComparsionFunc::Never:			return D3D11_COMPARISON_NEVER;
			case rend::EComparsionFunc::Less:			return D3D11_COMPARISON_LESS;
			case rend::EComparsionFunc::Equal:			return D3D11_COMPARISON_EQUAL;
			case rend::EComparsionFunc::LessEqual:		return D3D11_COMPARISON_LESS_EQUAL;
			case rend::EComparsionFunc::Greater:		return D3D11_COMPARISON_GREATER;
			case rend::EComparsionFunc::NotEqual:		return D3D11_COMPARISON_NOT_EQUAL;
			case rend::EComparsionFunc::GreaterEqual:	return D3D11_COMPARISON_GREATER_EQUAL;
			case rend::EComparsionFunc::Always:			return D3D11_COMPARISON_ALWAYS;

			default:
				fatal( L"Unknown comparsion function %d", func );
				return D3D11_COMPARISON_ALWAYS;
		}
	}

	D3D11_STENCIL_OP fluorineStencilOpToDirectX( rend::EStencilOp op )
	{
		switch( op )
		{
			case rend::EStencilOp::Keep:	return D3D11_STENCIL_OP_KEEP;
			case rend::EStencilOp::Zero:	return D3D11_STENCIL_OP_ZERO;
			case rend::EStencilOp::Replace:	return D3D11_STENCIL_OP_REPLACE;
			case rend::EStencilOp::IncSat:	return D3D11_STENCIL_OP_INCR_SAT;
			case rend::EStencilOp::DecSat:	return D3D11_STENCIL_OP_DECR_SAT;
			case rend::EStencilOp::Invert:	return D3D11_STENCIL_OP_INVERT;
			case rend::EStencilOp::Inc:		return D3D11_STENCIL_OP_INCR;
			case rend::EStencilOp::Dec:		return D3D11_STENCIL_OP_DECR;

			default:
				fatal( L"Unknown stencil op %d", op );
				return D3D11_STENCIL_OP_KEEP;
		}
	}

	rend::DepthStencilStateId Device::getDepthStencilState( const rend::DepthStencilState& depthStencilState )
	{
		rend::DepthStencilStateId requiredDepthStencilHash = depthStencilState.getHash();

		if( m_depthStencilStates.hasKey( requiredDepthStencilHash ) )
		{
			// reuse depth-stencil state from cache
			return requiredDepthStencilHash;
		}
		else
		{
			// create new depth-stencil state
			D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
			mem::zero( &depthStencilDesc, sizeof( D3D11_DEPTH_STENCIL_DESC ) );

			depthStencilDesc.DepthEnable = depthStencilState.isDepthEnabled();
			depthStencilDesc.DepthWriteMask = depthStencilState.isDepthWriteEnabled() ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
			depthStencilDesc.DepthFunc = fluorineComparsionFuncToDirectX( depthStencilState.getDepthFunc() );

			depthStencilDesc.StencilEnable = depthStencilState.isStencilEnabled();
			depthStencilDesc.StencilReadMask = 0xff;
			depthStencilDesc.StencilWriteMask = 0xff;

			depthStencilDesc.FrontFace.StencilFunc = fluorineComparsionFuncToDirectX( depthStencilState.getStencilFunc() );
			depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			depthStencilDesc.FrontFace.StencilPassOp = fluorineStencilOpToDirectX( depthStencilState.getStencilPassOp() );
			depthStencilDesc.FrontFace.StencilFailOp = fluorineStencilOpToDirectX( depthStencilState.getStencilFailOp() );

			depthStencilDesc.BackFace = depthStencilDesc.FrontFace;

			DxRef<ID3D11DepthStencilState> dxDepthStencilState;
			HRESULT result = m_device->CreateDepthStencilState( &depthStencilDesc, &dxDepthStencilState );
			assert( SUCCEEDED( result ));

			m_depthStencilStates.put( requiredDepthStencilHash, dxDepthStencilState );
			return requiredDepthStencilHash;	
		}
	}

	void Device::applyDepthStencilState( rend::DepthStencilStateId depthStencilStateId )
	{
		if( depthStencilStateId != m_oldDepthStencilStateId )
		{
			if( depthStencilStateId != rend::DepthStencilState::INVALID )
			{
				DxRef<ID3D11DepthStencilState>& depthStencilState = m_depthStencilStates.getRef( depthStencilStateId );
				assert( depthStencilState.hasObject() );

				m_immediateContext->OMSetDepthStencilState( depthStencilState.get(), 0 );
			}
			else
			{
				m_immediateContext->OMSetDepthStencilState( nullptr, 0 );			
			}
		
			m_oldDepthStencilStateId = depthStencilStateId;
		}
	}

	rend::SamplerStateId Device::getSamplerState( const rend::SamplerState& samplerState )
	{
		rend::SamplerStateId requiredSamplerHash = samplerState.getHash();
		
		if( m_samplerStates.hasKey( requiredSamplerHash ) )
		{
			// reuse sampler state from cache
			return requiredSamplerHash;	
		}
		else
		{
			// create new sampler state
			D3D11_SAMPLER_DESC samplerDescription;
			mem::zero( &samplerDescription, sizeof( D3D11_SAMPLER_DESC ) );

			switch( samplerState.getFilter() )
			{
				case rend::ESamplerFilter::Point:
					samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
					break;

				case rend::ESamplerFilter::Linear:
					samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
					break;

				default:
					fatal( L"Unknown sampler filter %d", samplerState.getFilter() );
			}

			switch( samplerState.getAddressMode() )
			{
				case rend::ESamplerAddressMode::Clamp:
					samplerDescription.AddressU = samplerDescription.AddressV =
						samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
					break;

				case rend::ESamplerAddressMode::Mirror:
					samplerDescription.AddressU = samplerDescription.AddressV =
						samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
					break;

				case rend::ESamplerAddressMode::Wrap:
					samplerDescription.AddressU = samplerDescription.AddressV =
						samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
					break;

				default:
					fatal( L"Unknown sampler address mode %d", samplerState.getAddressMode() );
			}

			samplerDescription.MipLODBias = 0.f;
			samplerDescription.MaxAnisotropy = 1;
			samplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
			samplerDescription.MinLOD = 0.f;
			samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

			DxRef<ID3D11SamplerState> dxSamplerState;
			HRESULT result = m_device->CreateSamplerState( &samplerDescription, &dxSamplerState );
			assert( SUCCEEDED( result ) );

			m_samplerStates.put( requiredSamplerHash, dxSamplerState );
			return requiredSamplerHash;
		}
	}

	void Device::clearRenderTarget( rend::RenderTargetHandle handle, const math::FloatColor& clearColor )
	{
		ID3D11RenderTargetView* rtv = nullptr;

		if( handle != INVALID_HANDLE<rend::RenderTargetHandle>() )
		{
			DxRenderTarget& renderTarget = m_renderTargets.get( handle );
			rtv = renderTarget.m_rtv.get();
		}
		else
		{
			rtv = m_swapChain->getBackBufferRTV();
		}

		m_immediateContext->ClearRenderTargetView( rtv, reinterpret_cast<const Float*>( &clearColor ) );
	}

	void Device::clearDepthBuffer( rend::DepthBufferHandle handle, Float depth, UInt8 stencil )
	{
		assert( handle != INVALID_HANDLE<rend::DepthBufferHandle>() );

		DxDepthBuffer& depthBuffer = m_depthBuffers.get( handle );
		assert( depthBuffer.m_dsv.hasObject() );
		assert( depthBuffer.m_format == rend::EFormat::D24S8 );

		m_immediateContext->ClearDepthStencilView( depthBuffer.m_dsv.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 
			depth, stencil );
	}

	void Device::setVertexBuffer( rend::VertexBufferHandle handle )
	{
		if( m_oldVertexBuffer != handle )
		{
			if( handle != INVALID_HANDLE<rend::VertexBufferHandle>() )
			{
				DxVertexBuffer& vertexBuffer = m_vertexBuffers.get( handle );

				UINT stride = vertexBuffer.m_vertexSize;
				UINT offset = 0;
				m_immediateContext->IASetVertexBuffers( 0, 1, &vertexBuffer.m_buffer, &stride, &offset );			
			}
			else
			{
				DxRef<ID3D11Buffer> nullBuffer;

				UINT stride = 0;
				UINT offset = 0;
				m_immediateContext->IASetVertexBuffers( 0, 1, &nullBuffer, &stride, &offset );	
			}

			m_oldVertexBuffer = handle;
		}
	}

	void Device::setIndexBuffer( rend::IndexBufferHandle handle )
	{
		if( m_oldIndexBuffer != handle )
		{
			DxIndexBuffer& indexBuffer = m_indexBuffers.get( handle );
			m_immediateContext->IASetIndexBuffer( indexBuffer.m_buffer, indexBuffer.m_dxFormat, 0 );	

			m_oldIndexBuffer = handle;
		}
	}

	void Device::setVertexShader( rend::ShaderHandle vertexShaderHandle )
	{
		if( vertexShaderHandle != m_oldVertexShader )
		{
			DxVertexShader& vertexShader = m_vertexShaders.get( vertexShaderHandle );

			CachedVertexDeclaration* declaration = m_vertexDeclarations.get( vertexShader.m_vertexDeclaration );
			assert( declaration );

			m_immediateContext->IASetInputLayout( declaration->m_inputLayout );
			m_immediateContext->VSSetShader( vertexShader.m_shader, nullptr, 0 );

			m_oldVertexShader = vertexShaderHandle;
		}
	}

	void Device::setPixelShader( rend::ShaderHandle pixelShaderHandle )
	{
		if( pixelShaderHandle != m_oldPixelShader )
		{
			DxPixelShader& pixelShader = m_pixelShaders.get( pixelShaderHandle );
			m_immediateContext->PSSetShader( pixelShader.m_shader, nullptr, 0 );

			m_oldPixelShader = pixelShaderHandle;
		}
	}

	void Device::setSRVs( rend::EShaderType shader, UInt32 firstSlot, UInt32 numSlots, rend::ShaderResourceView* resourceViews )
	{
		assert( numSlots > 0 );
		assert( resourceViews != nullptr );

		UInt32 firstValueSlot = 0;
		m_srvsCache[static_cast<SizeT>( shader )].applyFilter( firstSlot, numSlots, firstValueSlot, resourceViews );

		if( numSlots > 0 )
		{
			StaticArray<ID3D11ShaderResourceView*, MAX_SRVS_SLOTS> srvs;

			for( UInt32 i = 0; i < numSlots; ++i )
			{
				srvs[i] = reinterpret_cast<ID3D11ShaderResourceView*>( resourceViews[firstValueSlot + i].srv );
			}

			switch( shader )
			{
				case rend::EShaderType::ST_Vertex:
					m_immediateContext->VSSetShaderResources( firstSlot, numSlots, &srvs[0] );
					break;

				case rend::EShaderType::ST_Pixel:
					m_immediateContext->PSSetShaderResources( firstSlot, numSlots, &srvs[0] );
					break;

				case rend::EShaderType::ST_Compute:
					m_immediateContext->CSSetShaderResources( firstSlot, numSlots, &srvs[0] );
					break;

				default:
					fatal( L"Unable to set shader resources to unknown shader %d", shader );
					break;
			}		
		}
	}

	void Device::setSamplerStates( rend::EShaderType shader, UInt32 firstSlot, UInt32 numSlots, rend::SamplerStateId* ids )
	{
		assert( numSlots > 0 && numSlots < MAX_SAMPLER_STATES_SLOTS );
		assert( ids != nullptr );

		UInt32 firstValueSlot = 0;
		m_samplerStatesCache[static_cast<SizeT>( shader )].applyFilter( firstSlot, numSlots, firstValueSlot, ids );

		if( numSlots > 0 )
		{
			StaticArray<ID3D11SamplerState*, MAX_SAMPLER_STATES_SLOTS> samplerStates;

			for( UInt32 i = 0; i < numSlots; ++i )
			{
				if( ids[firstValueSlot + i] != rend::SamplerState::INVALID )
				{
					DxRef<ID3D11SamplerState>& sampler = m_samplerStates.getRef( ids[firstValueSlot + i] );
					samplerStates[i] = sampler.get();			
				}
				else
				{
					samplerStates[i] = nullptr;
				}
			}
	
			switch( shader )
			{
				case rend::EShaderType::ST_Vertex:
					m_immediateContext->VSSetSamplers( firstSlot, numSlots, &samplerStates[0] );
					break;

				case rend::EShaderType::ST_Pixel:
					m_immediateContext->PSSetSamplers( firstSlot, numSlots, &samplerStates[0] );
					break;

				case rend::EShaderType::ST_Compute:
					m_immediateContext->CSSetSamplers( firstSlot, numSlots, &samplerStates[0] );
					break;

				default:
					fatal( L"Unable to set sampler states to unknown shader %d", shader );
					break;
			}
		}
	}

	void Device::setConstantBuffers( rend::EShaderType shader, UInt32 firstSlot, UInt32 numSlots, 
		rend::ConstantBufferHandle* buffers )
	{
		assert( numSlots > 0 );
		assert( buffers != nullptr );

		UInt32 firstValueSlot = 0;
		m_constantBuffersCache[static_cast<SizeT>( shader )].applyFilter( firstSlot, numSlots, firstValueSlot, buffers );

		if( numSlots > 0 )
		{
			StaticArray<ID3D11Buffer*, MAX_CONSTANT_BUFFERS_SLOTS> constantBuffers;

			for( UInt32 i = 0; i < numSlots; i++ )
			{
				DxConstantBuffer& buffer = m_constantBuffers.get( buffers[firstValueSlot + i] );
				constantBuffers[i] = buffer.m_buffer.get();
			}

			switch( shader )
			{
				case rend::EShaderType::ST_Vertex:
					m_immediateContext->VSSetConstantBuffers( firstSlot, numSlots, &constantBuffers[0] );
					break;

				case rend::EShaderType::ST_Pixel:
					m_immediateContext->PSSetConstantBuffers( firstSlot, numSlots, &constantBuffers[0] );
					break;

				case rend::EShaderType::ST_Compute:
					m_immediateContext->CSSetConstantBuffers( firstSlot, numSlots, &constantBuffers[0] );
					break;

				default:
					fatal( L"Unable to set constant buffers to unknown shader %d", shader );
					break;
			}		
		}
	}

	void Device::setTopology( rend::EPrimitiveTopology topology )
	{
		assert( topology != rend::EPrimitiveTopology::Unknown );

		if( topology != m_oldTopology )
		{
			D3D11_PRIMITIVE_TOPOLOGY dxTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

			switch( topology )
			{
				case rend::EPrimitiveTopology::PointList:
					dxTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
					break;

				case rend::EPrimitiveTopology::LineList:
					dxTopology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
					break;

				case rend::EPrimitiveTopology::LineStrip:
					dxTopology = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
					break;

				case rend::EPrimitiveTopology::TriangleList:
					dxTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
					break;

				case rend::EPrimitiveTopology::TriangleStrip:
					dxTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
					break;

				default:
					fatal( L"Unknown primitive topology %d", topology );
					break;
			}

			m_immediateContext->IASetPrimitiveTopology( dxTopology );		
			m_oldTopology = topology;
		}
	}

	void Device::drawIndexed( UInt32 indexCount, UInt32 startIndexLocation, Int32 baseVertexOffset )
	{
		GPU_STAT( m_drawStats.m_drawCalls++ );
		m_immediateContext->DrawIndexed( indexCount, startIndexLocation, baseVertexOffset );
	}

	void Device::draw( UInt32 vertexCount, UInt32 startVertexLocation )
	{
		GPU_STAT( m_drawStats.m_drawCalls++ );
		m_immediateContext->Draw( vertexCount, startVertexLocation );
	}

	void Device::setScissorArea( const rend::ScissorArea& area )
	{
		if( area != rend::ScissorArea::NULL_AREA() )
		{
			if( !m_scissorTestEnabled )
			{
				m_immediateContext->RSSetState( m_scissorRasterState );
				m_scissorTestEnabled = true;
			}

			CD3D11_RECT rect( area.left, area.top, area.right, area.bottom );
			m_immediateContext->RSSetScissorRects( 1, &rect );
			GPU_STAT( m_drawStats.m_renderStateSwitches++ );
		}
		else
		{
			if( m_scissorTestEnabled )
			{
				m_immediateContext->RSSetState( m_defaultRasterState );
				GPU_STAT( m_drawStats.m_renderStateSwitches++ );

				m_scissorTestEnabled = false;
			}
		}
	}

	void Device::setRenderTarget( rend::RenderTargetHandle rtHandle, rend::DepthBufferHandle dbHandle )
	{
		ID3D11RenderTargetView* rtv = rtHandle != INVALID_HANDLE<rend::RenderTargetHandle>() ?
			m_renderTargets.get( rtHandle ).m_rtv.get() : m_swapChain->getBackBufferRTV();

		ID3D11DepthStencilView* dsv = dbHandle != INVALID_HANDLE<rend::DepthBufferHandle>() ? 
			m_depthBuffers.get( dbHandle ).m_dsv.get() : nullptr;

		m_immediateContext->OMSetRenderTargets( 1, &rtv, dsv );
	}

	void Device::setViewport( const rend::Viewport& viewport )
	{
		assert( viewport.x >= 0.f && viewport.y >= 0.f );
		assert( viewport.width >= 0.f && viewport.height >= 0.f );
		assert( viewport.minDepth >= 0.f && viewport.maxDepth >= 0.f );
	
		D3D11_VIEWPORT dxViewport;
		dxViewport.TopLeftX = viewport.x;
		dxViewport.TopLeftY = viewport.y;
		dxViewport.Width = viewport.width;
		dxViewport.Height = viewport.height;
		dxViewport.MinDepth = viewport.minDepth;
		dxViewport.MaxDepth = viewport.maxDepth;

		m_immediateContext->RSSetViewports( 1, &dxViewport );
	}

	Int32 Device::getBackbufferWidth() const
	{
		return m_swapChain->getWidth();
	}

	Int32 Device::getBackbufferHeight() const
	{
		return m_swapChain->getHeight();
	}

	void Device::enterZone( const Char* zoneName )
	{
		assert( zoneName );
		m_userDefinedAnnotation->BeginEvent( zoneName );
	}

	void Device::leaveZone()
	{
		m_userDefinedAnnotation->EndEvent();
	}

	Device::CachedVertexDeclaration* Device::getVertexDeclaration( UInt32 hash )
	{
		return m_vertexDeclarations.get( hash );
	}

	rend::ShaderResourceView Device::getShaderResourceView( rend::Texture1DHandle handle )
	{
		DxTexture1D& texture = m_textures1D.get( handle );
		assert( texture.m_srv.hasObject() );

		rend::ShaderResourceView shaderResourceView;
		shaderResourceView.srv = texture.m_srv.get();
		return shaderResourceView;
	}

	rend::ShaderResourceView Device::getShaderResourceView( rend::Texture2DHandle handle )
	{
		DxTexture2D& texture = m_textures2D.get( handle );
		assert( texture.m_srv.hasObject() );

		rend::ShaderResourceView shaderResourceView;
		shaderResourceView.srv = texture.m_srv.get();
		return shaderResourceView;
	}

	rend::ShaderResourceView Device::getShaderResourceView( rend::RenderTargetHandle handle )
	{
		DxRenderTarget& renderTarget = m_renderTargets.get( handle );
		assert( renderTarget.m_srv.hasObject() );

		rend::ShaderResourceView shaderResourceView;
		shaderResourceView.srv = renderTarget.m_srv.get();
		return shaderResourceView;
	}

	rend::ShaderResourceView Device::getShaderResourceView( rend::DepthBufferHandle handle )
	{
		DxDepthBuffer& depthBuffer = m_depthBuffers.get( handle );
		assert( depthBuffer.m_srv.hasObject() );

		rend::ShaderResourceView shaderResourceView;
		shaderResourceView.srv = depthBuffer.m_srv.get();
		return shaderResourceView;	
	}

	UInt32 Device::createVertexDeclaration( const rend::VertexDeclaration& declaration, const rend::CompiledShader& vertexShader )
	{
		UInt32 hash = declaration.getHash();
		assert( !m_vertexDeclarations.hasKey( hash ) );

		m_vertexDeclarations.put( hash, CachedVertexDeclaration( m_device.get(), declaration, vertexShader ) );
		return hash;
	}

	const rend::MemoryStats& Device::getMemoryStats() const
	{
		return m_memoryStats;
	}

	const rend::DrawStats& Device::getDrawStats() const
	{
		return m_drawStats;
	}

	Bool Device::copyTextureToCPU( rend::Texture2DHandle handle, rend::EFormat& outFormat,
		UInt32& outWidth, UInt32& outHeight, Array<UInt8>& outData )
	{
		ID3D11Texture2D* sourceTexture = nullptr;

		if( handle != INVALID_HANDLE<rend::Texture2DHandle>() )
		{
			// normal texture
			DxTexture2D& texture = m_textures2D.get( handle );
			assert( texture.m_texture.hasObject() );

			outFormat = texture.m_format;
			outWidth = texture.m_width;
			outHeight = texture.m_height;

			sourceTexture = texture.m_texture;
		}
		else
		{
			// swap chain's texture
			outFormat = m_swapChain->getFormat();
			outWidth = m_swapChain->getWidth();
			outHeight = m_swapChain->getHeight();

			sourceTexture = m_swapChain->getBackBuffer();
		}

        D3D11_TEXTURE2D_DESC stagingTexDesc;
		mem::zero( &stagingTexDesc, sizeof( D3D11_TEXTURE2D_DESC ) );

        stagingTexDesc.Width = outWidth;
        stagingTexDesc.Height = outHeight;
        stagingTexDesc.MipLevels = 1;
        stagingTexDesc.ArraySize = 1;
        stagingTexDesc.Format = fluorineFormatToDirectX( outFormat );
        stagingTexDesc.SampleDesc.Count = 1;
		stagingTexDesc.SampleDesc.Quality = 0;
        stagingTexDesc.Usage = D3D11_USAGE_STAGING;
        stagingTexDesc.BindFlags = 0;
        stagingTexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        stagingTexDesc.MiscFlags = 0;

		DxRef<ID3D11Texture2D> stagingTexture;
		HRESULT result = m_device->CreateTexture2D( &stagingTexDesc, nullptr, &stagingTexture );
		assert( SUCCEEDED( result ) );	

		// copy to staging
		m_immediateContext->CopyResource( stagingTexture, sourceTexture );

		// map to cpu
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		result = m_immediateContext->Map( stagingTexture, 0, D3D11_MAP_READ, 0, &mappedResource );
		assert( SUCCEEDED( result ) );
		{
			auto formatInfo = rend::getFormatInfo( outFormat );
		
			// todo: formula is not correct
			SizeT lineSize = outWidth * formatInfo.blockBytes;
			SizeT dataSize = outHeight * lineSize;
			outData.setSize( dataSize );

			// copy line by line according to row pitch
			for( UInt32 y = 0; y < outHeight; ++y )
			{
				const UInt8* sourceLine = &reinterpret_cast<UInt8*>( mappedResource.pData )[y * mappedResource.RowPitch];
				mem::copy( &outData[y * lineSize], sourceLine, lineSize );	
			}
		}
		m_immediateContext->Unmap( stagingTexture, 0 );

		return true;
	}

	Device::CachedVertexDeclaration::CachedVertexDeclaration( ID3D11Device* device, const rend::VertexDeclaration& declaration, 
		const rend::CompiledShader& vertexShader )
		:	m_vertexDecl( declaration )
	{
		assert( declaration.isValid() );
		assert( vertexShader.isValid() );

		static const AnsiChar* DX_SEMANTICS[(SizeT)rend::EVertexElementUsage::MAX] = 
		{
			"POSITION",
			"COLOR",
			"TEXCOORD"
		};

		Array<D3D11_INPUT_ELEMENT_DESC> inputElementsDesc;

		for( const auto& it : declaration.getElements() )
		{
			D3D11_INPUT_ELEMENT_DESC element;

			element.SemanticName = DX_SEMANTICS[ (SizeT)it.usage ];
			element.SemanticIndex = it.usageIndex;
			element.Format = fluorineFormatToDirectX( it.format );
			element.InputSlot = 0;
			element.AlignedByteOffset = it.offset;
			element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			element.InstanceDataStepRate = 0;

			inputElementsDesc.push( element );
		}

		if( inputElementsDesc.size() > 0 )
		{
			HRESULT result = device->CreateInputLayout( &inputElementsDesc[0], inputElementsDesc.size(), 
				vertexShader.code(), vertexShader.codeLength(), &m_inputLayout );

			assert( SUCCEEDED( result ) );

			if( declaration.getName() )
			{
				AnsiString ansiName = wide2AnsiString( declaration.getName() );
				m_inputLayout->SetPrivateData( WKPDID_D3DDebugObjectName, ansiName.len(), *ansiName );
			}		
		}
		else
		{
			m_inputLayout = nullptr;
		}
	}

	Device::CachedVertexDeclaration::~CachedVertexDeclaration()
	{
		m_inputLayout = nullptr;
	}
}
}