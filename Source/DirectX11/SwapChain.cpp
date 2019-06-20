//-----------------------------------------------------------------------------
//	SwapChain.cpp: A Fluorine SwapChain implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "DirectX11.h"

namespace flu
{
namespace dx11
{
	SwapChain::SwapChain( ID3D11Device* device, HWND hwnd, UInt32 width, UInt32 height, Bool fullScreen )
		:	m_hwnd( hwnd ),
			m_width( width ),
			m_height( height ),
			m_isFullScreen( fullScreen )
	{
		assert( device );

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		mem::zero( &swapChainDesc, sizeof( DXGI_SWAP_CHAIN_DESC ) );

		swapChainDesc.BufferDesc.Width = width;
		swapChainDesc.BufferDesc.Height = height;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.Format = SWAP_CHAIN_FORMAT;
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

		HRESULT result = m_swapChain->ResizeBuffers( 1, width, height, SWAP_CHAIN_FORMAT, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH );
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
		m_swapChain->Present( lockToVSync ? 1 : 0, 0 );
	}
}
}