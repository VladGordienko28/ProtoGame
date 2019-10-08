//-----------------------------------------------------------------------------
//	SwapChain.h: A Fluorine SwapChain
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace dx11
{
	/**
	 *	A DirectX 11 render swap chain
	 */
	class SwapChain final
	{
	public:
		using UPtr = UniquePtr<SwapChain>;

		SwapChain( ID3D11Device* device, HWND hwnd, UInt32 width, UInt32 height, Bool fullScreen = false );
		~SwapChain();

		void resize( ID3D11Device* device, UInt32 width, UInt32 height, Bool fullScreen = false );

		void present( ID3D11Device* device, Bool lockToVSync );

		ID3D11Texture2D* const& getBackBuffer()
		{
			return m_backBuffer.get();
		}

		ID3D11RenderTargetView* const& getBackBufferRTV()
		{
			return m_backBufferRTV.get();
		}

		HWND getHWND() const
		{
			return m_hwnd;
		}

		UInt32 getWidth() const
		{
			return m_width;
		}

		UInt32 getHeight() const
		{
			return m_height;
		}

		rend::EFormat getFormat() const
		{
			return SWAP_CHAIN_FORMAT;
		}

		Bool isFullscreen() const
		{
			return m_isFullScreen;
		}

	private:
		static const rend::EFormat SWAP_CHAIN_FORMAT = rend::EFormat::RGBA8_UNORM;

		DxRef<IDXGISwapChain> m_swapChain;
		DxRef<ID3D11Texture2D> m_backBuffer;
		DxRef<ID3D11RenderTargetView> m_backBufferRTV;

		HWND m_hwnd;

		UInt32 m_width;
		UInt32 m_height;
		Bool m_isFullScreen;

		SwapChain() = delete;
		SwapChain( const SwapChain& ) = delete;
		SwapChain( SwapChain&& ) = delete;
	};
}
}