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
	CCanvas*	Canvas;
	Float		Brightness;

	// CGUIRender interface.
	CGUIRender();
	~CGUIRender();
	void BeginPaint( CCanvas* InCanvas );
	void EndPaint();

	// CGUIRenderBase interface.
	void DrawRegion( TPoint P, TSize S, math::Color Color, math::Color BorderColor, EBrushPattern Pattern );
	void DrawText( TPoint P, const Char* Text, Int32 Len, math::Color Color, fnt::Font::Ptr Font );
	void SetClipArea( TPoint P, TSize S );
	void DrawImage( TPoint P, TSize S, TPoint BP, TSize BS, img::Image::Ptr image );
	void DrawTexture( TPoint P, TSize S, TPoint BP, TSize BS, rend::Texture2DHandle image, UInt32 width, UInt32 height );
	void SetBrightness( Float Brig );

	void FlushText() override
	{
		m_textDrawer.flush();
	}

private:
	gfx::TextDrawer m_textDrawer;
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