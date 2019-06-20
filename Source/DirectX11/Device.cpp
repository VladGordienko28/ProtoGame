//-----------------------------------------------------------------------------
//	Device.cpp: A DirectX 11 rendering device implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "DirectX11.h"

namespace flu
{
namespace dx11
{
	Device::Device( HWND hwnd, UInt32 width, UInt32 height, Bool fullscreen )
		:	rend::Device()
	{
		assert( hwnd != INVALID_HANDLE_VALUE );
	
		// todo: smart features detection need
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
		D3D_FEATURE_LEVEL finalFeatureLevel = D3D_FEATURE_LEVEL_11_0;

		HRESULT result = D3D11CreateDevice( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_SINGLETHREADED, 
			&featureLevel, 1, D3D11_SDK_VERSION, &m_device, &finalFeatureLevel, &m_immediateContext );

		if( FAILED( result ) )
		{
			fatal( L"Unable to create DirectX11 device and immediate context with error %d", result );
		}

		result = m_immediateContext->QueryInterface( __uuidof( ID3DUserDefinedAnnotation ), 
			reinterpret_cast<void**>( &m_userDefinedAnnotation ) );
		assert( SUCCEEDED( result ) );

		m_swapChain = new SwapChain( m_device, hwnd, width, height, fullscreen );

		// set initial render target and viewport
		rend::Viewport initialViewport = { 0.f, 0.f, Float( width ), Float( height ), 0.f, 1.f };

		this->setViewport( initialViewport );
		m_immediateContext->OMSetRenderTargets( 1, &m_swapChain->getBackBufferRTV(), nullptr );

		m_oldTopology = rend::EPrimitiveTopology::Unknown;
	}

	Device::~Device()
	{
		m_userDefinedAnnotation = nullptr;

		m_immediateContext->ClearState();
		m_immediateContext = nullptr;

		m_samplerStates.empty();
		m_vertexDeclarations.empty();
		m_blendStates.empty();

		m_swapChain = nullptr;
		m_device = nullptr;
	}
	
	void Device::resize( UInt32 width, UInt32 height, Bool fullScreen )
	{
		assert( m_swapChain.hasObject() );

		m_swapChain->resize( m_device.get(), width, height, fullScreen );

		rend::Viewport newViewport = { 0.f, 0.f, width, height, 0.f, 1.f };

		this->setViewport( newViewport );
		m_immediateContext->OMSetRenderTargets( 1, &m_swapChain->getBackBufferRTV(), nullptr );
	}

	void Device::beginFrame()
	{
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
		return handle;
	}

	void Device::destroyTexture1D( rend::Texture1DHandle handle )
	{
		DxTexture1D& texture = m_textures1D.get( handle );
		texture.destroy( m_device.get() );

		m_textures1D.removeElement( handle );
	}

	rend::Texture2DHandle Device::createTexture2D( rend::EFormat format, Int32 width, Int32 height, Int32 mips, 
		rend::EUsage usage, const void* initialData, const AnsiChar* debugName )
	{
		rend::Texture2DHandle handle;

		m_textures2D.emplaceElement( handle )->create( m_device.get(), format, width, height, mips, usage, initialData, debugName );
		return handle;
	}

	void Device::destroyTexture2D( rend::Texture2DHandle handle )
	{
		DxTexture2D& texture = m_textures2D.get( handle );
		texture.destroy( m_device.get() );

		m_textures2D.removeElement( handle );
	}

	rend::RenderTargetHandle Device::createRenderTarget( rend::EFormat format, Int32 width, Int32 height, 
		const AnsiChar* debugName )
	{
		rend::RenderTargetHandle handle;

		m_renderTargets.emplaceElement( handle )->create( m_device.get(), format, width, height, debugName );
		return handle;
	}

	void Device::destroyRenderTarget( rend::RenderTargetHandle handle )
	{
		DxRenderTarget& renderTarget = m_renderTargets.get( handle );
		renderTarget.destroy( m_device.get() );

		m_renderTargets.removeElement( handle );
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
		vertexBuffer.destroy( m_device.get() );

		m_vertexBuffers.removeElement( handle );
	}

	rend::IndexBufferHandle Device::createIndexBuffer( rend::EFormat indexFormat, UInt32 numIndexes, 
		rend::EUsage usage, const void* initialData, const AnsiChar* debugName )
	{
		rend::IndexBufferHandle handle;
		m_indexBuffers.emplaceElement( handle )->create( m_device.get(), indexFormat, numIndexes, usage, initialData, debugName );

		return handle;
	}

	void Device::destroyIndexBuffer( rend::IndexBufferHandle handle )
	{
		DxIndexBuffer& indexBuffer = m_indexBuffers.get( handle );
		indexBuffer.destroy( m_device.get() );

		m_indexBuffers.removeElement( handle );
	}

	rend::ConstantBufferHandle Device::createConstantBuffer( UInt32 bufferSize, rend::EUsage usage, 
		const void* initialData, const AnsiChar* debugName )
	{
		rend::ConstantBufferHandle handle;
		m_constantBuffers.emplaceElement( handle )->create( m_device.get(), bufferSize, usage, initialData, debugName );

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

	void Device::enableBlendState( rend::BlendStateId blendStateId )
	{
		DxRef<ID3D11BlendState>& blendState = m_blendStates.getRef( blendStateId );
		assert( blendState.hasObject() );

		m_immediateContext->OMSetBlendState( blendState.get(), nullptr, MAX_UINT32 );
	}

	void Device::disableBlendState()
	{
		m_immediateContext->OMSetBlendState( nullptr, nullptr, MAX_UINT32 );
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

	void Device::setVertexBuffer( rend::VertexBufferHandle handle )
	{
		DxVertexBuffer& vertexBuffer = m_vertexBuffers.get( handle );

		UINT stride = vertexBuffer.m_vertexSize;
		UINT offset = 0;
		m_immediateContext->IASetVertexBuffers( 0, 1, &vertexBuffer.m_buffer, &stride, &offset );
	}

	void Device::setIndexBuffer( rend::IndexBufferHandle handle )
	{
		DxIndexBuffer& indexBuffer = m_indexBuffers.get( handle );
		m_immediateContext->IASetIndexBuffer( indexBuffer.m_buffer, indexBuffer.m_format, 0 );
	}

	void Device::setVertexShader( rend::ShaderHandle vertexShaderHandle )
	{
		DxVertexShader& vertexShader = m_vertexShaders.get( vertexShaderHandle );

		CachedVertexDeclaration* declaration = m_vertexDeclarations.get( vertexShader.m_vertexDeclaration );
		assert( declaration );

		m_immediateContext->IASetInputLayout( declaration->m_inputLayout );
		m_immediateContext->VSSetShader( vertexShader.m_shader, nullptr, 0 );
	}

	void Device::setPixelShader( rend::ShaderHandle pixelShaderHandle )
	{
		DxPixelShader& pixelShader = m_pixelShaders.get( pixelShaderHandle );
		m_immediateContext->PSSetShader( pixelShader.m_shader, nullptr, 0 );
	}

	void Device::setSRVs( rend::EShaderType shader, UInt32 firstSlot, UInt32 numSlots, rend::ShaderResourceView* resourceViews )
	{
		assert( numSlots > 0 );
		assert( resourceViews != nullptr );

		static const SizeT MAX_SRV_SLOTS = 16;
		assert( numSlots < MAX_SRV_SLOTS );

		StaticArray<ID3D11ShaderResourceView*, MAX_SRV_SLOTS> srvs;

		for( UInt32 i = 0; i < numSlots; ++i )
		{
			srvs[i] = reinterpret_cast<ID3D11ShaderResourceView*>( resourceViews[i].srv );
		}

		switch( shader )
		{
			case rend::EShaderType::Vertex:
				m_immediateContext->VSSetShaderResources( firstSlot, numSlots, &srvs[0] );
				break;

			case rend::EShaderType::Pixel:
				m_immediateContext->PSSetShaderResources( firstSlot, numSlots, &srvs[0] );
				break;

			case rend::EShaderType::Compute:
				m_immediateContext->CSSetShaderResources( firstSlot, numSlots, &srvs[0] );
				break;

			default:
				fatal( L"Unable to set shader resources to unknown shader %d", shader );
				break;
		}
	}

	void Device::setSamplerStates( rend::EShaderType shader, UInt32 firstSlot, UInt32 numSlots, rend::SamplerStateId* ids )
	{
		assert( numSlots > 0 );
		assert( ids != nullptr );

		static const SizeT MAX_SAMPLER_STATE_SLOTS = 16;
		assert( numSlots < MAX_SAMPLER_STATE_SLOTS );

		StaticArray<ID3D11SamplerState*, MAX_SAMPLER_STATE_SLOTS> samplerStates;

		for( UInt32 i = 0; i < numSlots; ++i )
		{
			DxRef<ID3D11SamplerState>& sampler = m_samplerStates.getRef( ids[i] );
			samplerStates[i] = sampler.get();
		}
	
		switch( shader )
		{
			case rend::EShaderType::Vertex:
				m_immediateContext->VSSetSamplers( firstSlot, numSlots, &samplerStates[0] );
				break;

			case rend::EShaderType::Pixel:
				m_immediateContext->PSSetSamplers( firstSlot, numSlots, &samplerStates[0] );
				break;

			case rend::EShaderType::Compute:
				m_immediateContext->CSSetSamplers( firstSlot, numSlots, &samplerStates[0] );
				break;

			default:
				fatal( L"Unable to set sampler states to unknown shader %d", shader );
				break;
		}
	}

	void Device::setConstantBuffers( rend::EShaderType shader, UInt32 firstSlot, UInt32 numSlots, 
		rend::ConstantBufferHandle* buffers )
	{
		assert( numSlots > 0 );
		assert( buffers != nullptr );

		static const SizeT MAX_CONSTANT_BUFFER_SLOTS = 16;
		assert( numSlots < MAX_CONSTANT_BUFFER_SLOTS );

		StaticArray<ID3D11Buffer*, MAX_CONSTANT_BUFFER_SLOTS> constantBuffers;

		for( UInt32 i = 0; i < numSlots; i++ )
		{
			DxConstantBuffer& buffer = m_constantBuffers.get( buffers[i] );
			constantBuffers[i] = buffer.m_buffer.get();
		}

		switch( shader )
		{
			case rend::EShaderType::Vertex:
				m_immediateContext->VSSetConstantBuffers( firstSlot, numSlots, &constantBuffers[0] );
				break;

			case rend::EShaderType::Pixel:
				m_immediateContext->PSSetConstantBuffers( firstSlot, numSlots, &constantBuffers[0] );
				break;

			case rend::EShaderType::Compute:
				m_immediateContext->CSSetConstantBuffers( firstSlot, numSlots, &constantBuffers[0] );
				break;

			default:
				fatal( L"Unable to set constant buffers to unknown shader %d", shader );
				break;
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
		m_immediateContext->DrawIndexed( indexCount, startIndexLocation, baseVertexOffset );
	}

	void Device::draw( UInt32 vertexCount, UInt32 startVertexLocation )
	{
		m_immediateContext->Draw( vertexCount, startVertexLocation );
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

	UInt32 Device::createVertexDeclaration( const rend::VertexDeclaration& declaration, const rend::CompiledShader& vertexShader )
	{
		UInt32 hash = declaration.getHash();
		assert( !m_vertexDeclarations.hasKey( hash ) );

		m_vertexDeclarations.put( hash, CachedVertexDeclaration( m_device.get(), declaration, vertexShader ) );
		return hash;
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

		HRESULT result = device->CreateInputLayout( &inputElementsDesc[0], inputElementsDesc.size(), vertexShader.code(), 
			vertexShader.codeLength(), &m_inputLayout );

		assert( SUCCEEDED( result ) );

		if( declaration.getName() )
		{
			AnsiString ansiName = wide2AnsiString( declaration.getName() );
			m_inputLayout->SetPrivateData( WKPDID_D3DDebugObjectName, ansiName.len(), *ansiName );
		}
	}

	Device::CachedVertexDeclaration::~CachedVertexDeclaration()
	{
		m_inputLayout = nullptr;
	}
}
}