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
	void DrawText( TPoint P, const Char* Text, Int32 Len, math::Color Color, FFont* Font );
	void SetClipArea( TPoint P, TSize S );
	void DrawPicture( TPoint P, TSize S, TPoint BP, TSize BS, FTexture* Texture );
	void SetBrightness( Float Brig );
};


/*-----------------------------------------------------------------------------
    CSG.
-----------------------------------------------------------------------------*/

// CSG functions.
extern void CSGUnion( FBrushComponent* Brush, FLevel* Level );
extern void CSGIntersection( FBrushComponent* Brush, FLevel* Level );
extern void CSGDifference( FBrushComponent* Brush, FLevel* Level );


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/