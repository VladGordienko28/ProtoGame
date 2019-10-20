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

#if FLU_PLATFORM_XBOX
		SwapChain( ID3D11Device* device, IUnknown* coreWindow, UInt32 width, UInt32 height, Bool fullScreen = false );
#else
		SwapChain( ID3D11Device* device, HWND hwnd, UInt32 width, UInt32 height, Bool fullScreen = false );
#endif

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

#if FLU_PLATFORM_XBOX
		static const UInt32 SWAP_CHAIN_NUM_BUFFERS = 2;
#else
		static const UInt32 SWAP_CHAIN_NUM_BUFFERS = 1;
#endif

#if FLU_PLATFORM_XBOX
		DxRef<IDXGISwapChain1> m_swapChain;
#else
		DxRef<IDXGISwapChain> m_swapChain;
#endif

		DxRef<ID3D11Texture2D> m_backBuffer;
		DxRef<ID3D11RenderTargetView> m_backBufferRTV;

		UInt32 m_width;
		UInt32 m_height;
		Bool m_isFullScreen;

		SwapChain() = delete;
		SwapChain( const SwapChain& ) = delete;
		SwapChain( SwapChain&& ) = delete;
	};
}
}