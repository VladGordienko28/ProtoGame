/*=============================================================================
    FrEdUtil.h: Various editor stuff.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    CGUIRender.
-----------------------------------------------------------------------------*/

//
// GUI Render.
//
class CGUIRender: public CGUIRenderBase
{
public:
	// CGUIRender interface.
	CGUIRender();
	~CGUIRender();
	void BeginPaint( gfx::DrawContext& drawContext );
	void EndPaint( gfx::DrawContext& drawContext );

	// CGUIRenderBase interface.
	void DrawRegion( TPoint P, TSize S, math::Color Color, math::Color BorderColor, EBrushPattern Pattern );
	void DrawText( TPoint P, const Char* Text, Int32 Len, math::Color Color, fnt::Font::Ptr Font );
	void SetClipArea( TPoint P, TSize S );
	void DrawImage( TPoint P, TSize S, TPoint BP, TSize BS, img::Image::Ptr image );
	void DrawTexture( TPoint P, TSize S, TPoint BP, TSize BS, rend::Texture2DHandle image, UInt32 width, UInt32 height );
	void SetBrightness( Float Brig );

private:
	Float m_brightness;

	gfx::TextDrawer m_textDrawer;

	ffx::Effect::Ptr m_coloredEffect;
	ffx::Effect::Ptr m_texturedEffect;

	ffx::TechniqueId m_solidTech;
	ffx::TechniqueId m_stippleTech;

	rend::VertexBufferHandle m_coloredVB;
	rend::VertexBufferHandle m_texturedVB;
};


/*-----------------------------------------------------------------------------
    CSG.
-----------------------------------------------------------------------------*/

// CSG functions.
extern void CSGUnion( FBrushComponent* Brush, FLevel* Level );
extern void CSGIntersection( FBrushComponent* Brush, FLevel* Level );
extern void CSGDifference( FBrushComponent* Brush, FLevel* Level );



//
// Experimental stuff
//
JSon::Ptr exportLevel( FLevel* level );


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/