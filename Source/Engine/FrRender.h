/*=============================================================================
    FrRender.h: Abstract render and structs.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    Render structs.
-----------------------------------------------------------------------------*/

//
// A polygon render flags.
//
#define POLY_None				0x0000			// No special flags.
#define POLY_Unlit				0x0001			// No lighting.
#define POLY_FlatShade			0x0002			// Draw as colored poly.
#define POLY_Ghost				0x0004			// Half-Translucent poly.
#define POLY_StippleI			0x0008			// Draw with pattern.
#define POLY_StippleII			0x0010			// Draw with pattern.


//
// A simple rectangle to render.
//
struct TRenderRect
{
public:
	DWord			Flags;
	TRect			Bounds;
	TAngle			Rotation;
	FTexture*		Texture;
	TColor			Color;
	TRect			TexCoords;
};


//
// A polygon to render.
//
struct TRenderPoly
{
public:
	DWord			Flags;
	TVector			Vertices[16];
	TVector			TexCoords[16];
	Integer			NumVerts;
	FTexture*		Texture;
	TColor			Color;
};


//
// A list of rectangles to render.
//
struct TRenderList
{
public:
	DWord			Flags;
	FTexture*		Texture;
	Integer			NumRects;
	TVector*		Vertices;
	TVector*		TexCoords;
	TColor*			Colors;
	TColor			DrawColor;

	// TRenderList interface.
	TRenderList( Integer InNumRcts );
	TRenderList( Integer InNumRcts, TColor InColor );
	~TRenderList();
};


//
// An information about view.
//
struct TViewInfo
{
public:
	// To-screen transform.
	Float		X;
	Float		Y;
	Float		Width;
	Float		Height;

	// World transform.
	TCoords		Coords;
	TCoords		UnCoords;
	TVector		FOV;
	Float		Zoom;
	Bool		bMirage;
	TRect		Bounds;

	// Constructor.
	TViewInfo();
	TViewInfo( Float InX, Float InY, Float InWidth, Float InHeight );
	TViewInfo( TVector InLocation, TAngle InRotation, TVector InFOV, Float InZoom, Bool InbMirage, Float InX, Float InY, Float InWidth, Float InHeight  );

	// Projection functions.
	void Project( const TVector V, Float& OutX, Float& OutY ) const;
	TVector Deproject( Float InX, Float InY ) const;
};


//
// An area to clip.
//
struct TClipArea
{
public:
	Integer X, Y, Width, Height;

	TClipArea()
	{}
	TClipArea( Integer InX, Integer InY, Integer InWidth, Integer InHeight )
		:	X( InX ), 
			Y( InY ), 
			Width( InWidth ),
			Height( InHeight )
	{}
};

// Special value, mean no clip.
#define CLIP_NONE	TClipArea( -1, -1, -1, -1 )


/*-----------------------------------------------------------------------------
    CRenderBase.
-----------------------------------------------------------------------------*/

//
// An abstract render class.
//
class CRenderBase
{
public:
	// CRenderBase interface.
	virtual void Resize( Integer NewWidth, Integer NewHeight ) = 0;
	virtual void Flush() = 0;
	virtual void RenderLevel( CCanvas* Canvas, FLevel* Level, Integer X, Integer Y, Integer W, Integer H ) = 0;
	virtual CCanvas* Lock() = 0;
	virtual void Unlock() = 0;			
};


/*-----------------------------------------------------------------------------
    CCanvas.
-----------------------------------------------------------------------------*/

//
// An abstract canvas.
//
class CCanvas
{
public:
	// Window info.
	Float			ScreenWidth;
	Float			ScreenHeight;

	// View info.
	TViewInfo		View;
	TClipArea		Clip;

	// Global memory pool, for temporal rendering
	// objects.
	static CStaticPool<4*1024*1024>		GPool;

	// CCanvas interface.
	virtual void SetTransform( const TViewInfo& Info ) = 0;
	virtual void SetClip( const TClipArea& Area ) = 0;
	virtual void DrawPoint( const TVector& P, Float Size, TColor Color ) = 0;
	virtual void DrawLine( const TVector& A, const TVector& B, TColor Color, Bool bStipple ) = 0;
	virtual void DrawPoly( const TRenderPoly& Poly ) = 0;
	virtual void DrawRect( const TRenderRect& Rect ) = 0;
	virtual void DrawList( const TRenderList& List ) = 0;

	// Transformations stack.
	void PushTransform( const TViewInfo& Info );
	void PopTransform();

	// Drawing utilities.
	void DrawCircle
	( 
		const TVector& Center,
		Float Radius,
		TColor Color,
		Bool bStipple,
		Integer Detail = 32 
	);

	void DrawCoolPoint
	( 
		const TVector& P,
		Float Size,
		TColor Color 
	);

	void DrawSmoothLine
	( 
		const TVector& A,
		const TVector& B,
		TColor Color,
		Bool bStipple,
		Integer Detail = 16 
	);

	void DrawLineStar
	( 
		const TVector& Center,
		TAngle Rotation,
		Float Size,
		TColor Color,
		Bool bStipple 
	);

	void DrawLineRect
	( 
		const TVector& Center,
		const TVector& Size,
		TAngle Rotation,
		TColor Color,
		Bool bStipple 
	);

	void DrawText
	(	
		const Char* Text,
		Integer Len,
		FFont* Font, 
		TColor Color,
		const TVector& Start, 
		const TVector& Scale = TVector( 1.f, 1.f )  
	);

	void DrawText
	(	
		const String& S,
		FFont* Font, 
		TColor Color,
		const TVector& Start, 
		const TVector& Scale = TVector( 1.f, 1.f ) 
	)
	{
		DrawText( *S, S.Len(), Font, Color, Start, Scale );
	}

protected:
	// Canvas internal.
	enum{ VIEW_STACK_SIZE = 8 };
	TViewInfo	ViewStack[VIEW_STACK_SIZE];
	Integer		StackTop;
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/