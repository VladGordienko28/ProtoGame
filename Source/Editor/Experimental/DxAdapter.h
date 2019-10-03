//-----------------------------------------------------------------------------
//	DxAdapter.h: An experimental DirectX render integration into legacy interface
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
	extern img::Image::Ptr g_coolImage;

	extern rend::Device* g_device;
	extern gfx::GridDrawer g_grid;

	extern void renderLoadEffects();

	/**
	 *	An experimental DirectX11 backend integrated
	 *	into legacy fluorine rendering system
	 */
	class CDirectX11Canvas: public CCanvas
	{
	public:
		// COpenGLCanvas interface.
		CDirectX11Canvas( class CDirectX11Render* render, rend::Device* device );
		~CDirectX11Canvas();

		// CCanvas interface.
		void SetTransform( const gfx::ViewInfo& info ) override;
		void SetClip( const TClipArea& area ) override;
		void DrawPoint( const math::Vector& p, Float size, math::Color color ) override;
		void DrawLine( const math::Vector& a, const math::Vector& b, math::Color color, Bool stipple ) override;
		void DrawPoly( const TRenderPoly& poly ) override;
		void DrawRect( const TRenderRect& rect ) override;
		void DrawList( const TRenderList& list ) override;




	/*

	// Variables.
	COpenGLRender*		Render;

	// Shaders.
	CGLFluShader		FluShader;
	CGLFinalShader		FinalShader;
	CGLHorizBlurShader	HorizBlurShader;
	CGLVertBlurShader	VertBlurShader;

	// FBOs.
	class CGLFbo*		MasterFBO;
	class CGLFbo*		BlurFBO;



	// COpenGLCanvas interface.
	void SetBlend( EBitmapBlend Blend );
	void SetColor( math::Color Color );
	void SetBitmap( FBitmap* Bitmap, Bool bUnlit=true );
	void SetStipple( UInt32 Stipple );
	void RenderLightmap();

	void EnableShader( CGLShaderBase* Shader );
	void DisableShader();

	// Friends.
	friend COpenGLRender;

private:
	// todo: move to the other file
	struct DrawStats
	{
		Int32 points;
		Int32 lines;
		Int32 rects;
		Int32 polygons;
		Int32 lists;
	};

	// Internal used.
	math::Vector		BitmapPan;
	EBitmapBlend		OldBlend;
	FBitmap*			OldBitmap;
	math::Color			OldColor;
	UInt32				OldStipple;
	CGLShaderBase*		ActiveShader;
	DrawStats			m_stats;

*/

		ffx::SharedConstants::UPtr m_sharedConstants;  /// not here!!!!!!!!!!!!!!!!!!

	private:
		friend class CDirectX11Render;

		// not good, but well for now
		CDirectX11Render* m_render;
		rend::Device* m_device;

		Float m_lockTime;





		rend::VertexBufferHandle m_quadVB_XY;
		rend::VertexBufferHandle m_quadVB_XYUV;

		rend::VertexBufferHandle m_polyVB_XY;
		rend::VertexBufferHandle m_polyVB_XYUV;

		rend::IndexBufferHandle m_polyIB;

		rend::SamplerStateId m_samplerNearest;
		rend::SamplerStateId m_samplerLinear;

		rend::BlendStateId m_normalBlendState;
		rend::BlendStateId m_alphaBlendState;
		rend::BlendStateId m_translucentBlendState;

		//rend::BlendStateId m_blendStates[EBitmapBlend::BLEND_MAX];

		struct PerViewData
		{
			Float viewProjectionMatrix[4][4];
			math::Vector4 worldCamera;
		};


		friend class CDirectX11Render;
	};

	/**
	 *	An experimental DirectX11 backend integrated
	 *	into legacy fluorine rendering system
	 */
	class CDirectX11Render: public CRenderBase
	{
	public:
		// CDirectX11Render interface.
		CDirectX11Render( HWND hwnd );
		~CDirectX11Render();

		// CRenderBase interface.
		void Resize( Int32 newWidth, Int32 newHeight ) override;
		void Flush() override;
		void RenderLevel( CCanvas* canvas, FLevel* level, Int32 x, Int32 y, Int32 w, Int32 h ) override;
		CCanvas* Lock() override;
		void Unlock() override;

	private:
		HWND m_hwnd;

		rend::Device::UPtr m_device;
		UniquePtr<CDirectX11Canvas> m_canvas;
	};
}