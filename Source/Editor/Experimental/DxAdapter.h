//-----------------------------------------------------------------------------
//	DxAdapter.h: An experimental DirectX render integration into legacy interface
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 *	An experimental DirectX11 backend integrated
	 *	into legacy fluorine rendering system
	 */
	class CDirectX11Canvas: public CCanvas
	{
	public:
		// COpenGLCanvas interface.
		CDirectX11Canvas( class CDirectX11Render* render, rend::Device* device, gfx::DrawContext& drawContext );
		~CDirectX11Canvas();

		// CCanvas interface.
		void SetClip( const TClipArea& area ) override;
		void DrawPoly( const TRenderPoly& poly ) override;
		void DrawRect( const TRenderRect& rect ) override;
		void DrawList( const TRenderList& list ) override;


		ffx::Effect::Ptr m_coloredEffect;
		ffx::Effect::Ptr m_texturedEffect;

		ffx::TechniqueId m_solidTech;
		ffx::TechniqueId m_stippleTech;

	private:
		friend class CDirectX11Render;

		// not good, but well for now
		CDirectX11Render* m_render;
		rend::Device* m_device;





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
		CDirectX11Render( rend::Device* inDevice, gfx::DrawContext& drawContext );
		~CDirectX11Render();

		// CRenderBase interface.
		CCanvas* Lock() override;
		void Unlock() override;

	private:

		rend::Device* m_device;
		UniquePtr<CDirectX11Canvas> m_canvas;
	};
}