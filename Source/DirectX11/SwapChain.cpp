//-----------------------------------------------------------------------------
//	SwapChain.cpp: A Fluorine SwapChain implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "DirectX11.h"

namespace flu
{
namespace dx11
{
#if FLU_PLATFORM_XBOX
	SwapChain::SwapChain( ID3D11Device* device, IUnknown* coreWindow, UInt32 width, UInt32 height, Bool fullScreen )
#else
	SwapChain::SwapChain( ID3D11Device* device, HWND hwnd, UInt32 width, UInt32 height, Bool fullScreen )
#endif
		:	m_width( width ),
			m_height( height ),
			m_isFullScreen( fullScreen )
	{
		assert( device );

#if FLU_PLATFORM_XBOX
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
		mem::zero( &swapChainDesc, sizeof( swapChainDesc ) );

		swapChainDesc.Width = m_width;
		swapChainDesc.Height = m_height;
		swapChainDesc.Format = fluorineFormatToDirectX( SWAP_CHAIN_FORMAT );
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = SWAP_CHAIN_NUM_BUFFERS;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		swapChainDesc.Flags = 0;
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		IDXGIFactory4* dxgiFactory;
		HRESULT result = CreateDXGIFactory( __uuidof( IDXGIFactory4 ), (void**)&dxgiFactory );
		assert( SUCCEEDED( result ) );

		result = dxgiFactory->CreateSwapChainForCoreWindow( device, coreWindow, 
			&swapChainDesc, nullptr, &m_swapChain );
#else
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		mem::zero( &swapChainDesc, sizeof( DXGI_SWAP_CHAIN_DESC ) );

		swapChainDesc.BufferDesc.Width = width;
		swapChainDesc.BufferDesc.Height = height;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.Format = fluorineFormatToDirectX( SWAP_CHAIN_FORMAT );
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;

		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.OutputWindow = hwnd;
		swapChainDesc.Windowed = !fullScreen;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		IDXGIFactory* dxgiFactory;
		HRESULT result = CreateDXGIFactory( __uuidof( IDXGIFactory ), (void**)&dxgiFactory );
		assert( SUCCEEDED( result ) );

		result = dxgiFactory->CreateSwapChain( device, &swapChainDesc, &m_swapChain );
#endif

		if( FAILED( result ) )
		{
			fatal( L"Unable to create SwapChain with error %d", result );
		}

		result = m_swapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&m_backBuffer );
		assert( SUCCEEDED( result ) );

		result = device->CreateRenderTargetView( m_backBuffer, nullptr, &m_backBufferRTV );
		assert( SUCCEEDED( result ) );

		dxgiFactory->Release();
	}

	SwapChain::~SwapChain()
	{
		m_backBufferRTV = nullptr;
		m_backBuffer = nullptr;
		m_swapChain = nullptr;
	}

	void SwapChain::resize( ID3D11Device* device, UInt32 width, UInt32 height, Bool fullScreen )
	{
		m_backBufferRTV = nullptr;
		m_backBuffer = nullptr;

		HRESULT result = m_swapChain->ResizeBuffers( SWAP_CHAIN_NUM_BUFFERS, width, height, 
			fluorineFormatToDirectX( SWAP_CHAIN_FORMAT ), DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH );

		assert( SUCCEEDED( result ) );

		result = m_swapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&m_backBuffer );
		assert( SUCCEEDED( result ) );

		result = device->CreateRenderTargetView( m_backBuffer, nullptr, &m_backBufferRTV );
		assert( SUCCEEDED( result ) );

		m_width = width;
		m_height = height;
		m_isFullScreen = fullScreen;
	}

	void SwapChain::present( ID3D11Device* device, Bool lockToVSync )
	{
#if FLU_PLATFORM_XBOX
		DXGI_PRESENT_PARAMETERS parameters = { 0 };
		m_swapChain->Present1( lockToVSync ? 1 : 0, 0, &parameters );
#else
		m_swapChain->Present( lockToVSync ? 1 : 0, 0 );
#endif
	}
}
}