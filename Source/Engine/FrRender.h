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
#define POLY_AlphaGhost			0x0020			// Alpha translucent poly.


//
// A simple rectangle to render.
//
struct TRenderRect
{
public:
	UInt32			Flags;
	math::Rect		Bounds;
	math::Angle		Rotation;
	rend::Texture2DHandle	Image;
	math::Color		Color;
	math::Rect		TexCoords;
};


//
// A polygon to render.
//
struct TRenderPoly
{
public:
	UInt32			Flags;
	math::Vector	Vertices[16];
	math::Vector	TexCoords[16];
	Int32			NumVerts;
	rend::Texture2DHandle	Image;
	math::Color		Color;
};


//
// A list of rectangles to render.
//
struct TRenderList
{
public:
	UInt32			Flags;
	rend::Texture2DHandle		Image;
	Int32			NumRects;
	math::Vector*	Vertices;
	math::Vector*	TexCoords;
	math::Color*	Colors;
	math::Color		DrawColor;

	// TRenderList interface.
	TRenderList( Int32 InNumRcts );
	TRenderList( Int32 InNumRcts, math::Color InColor );
	~TRenderList();
};


//
// An area to clip.
//
struct TClipArea
{
public:
	Int32 X, Y, Width, Height;

	TClipArea()
	{}
	TClipArea( Int32 InX, Int32 InY, Int32 InWidth, Int32 InHeight )
		:	X( InX ), 
			Y( InY ), 
			Width( InWidth ),
			Height( InHeight )
	{}
	Bool operator==( const TClipArea& other ) const
	{
		return X == other.X && Y == other.Y &&
					Width == other.Width && Height == other.Height;
	}
	Bool operator!=( const TClipArea& other ) const
	{
		return X != other.X || Y != other.Y ||
					Width != other.Width || Height != other.Height;
	}
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
	virtual ~CRenderBase() = default;

	virtual void Resize( Int32 NewWidth, Int32 NewHeight ) = 0;
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
	gfx::ViewInfo	View;
	TClipArea		Clip;

	// Global memory pool, for temporal rendering
	// objects.
	static CStaticPool<4*1024*1024>		GPool;

	// CCanvas interface.
	virtual void SetTransform( const gfx::ViewInfo& Info ) = 0;
	virtual void SetClip( const TClipArea& Area ) = 0;
	virtual void DrawPoint( const math::Vector& P, Float Size, math::Color Color ) = 0;
	virtual void DrawLine( const math::Vector& A, const math::Vector& B, math::Color Color, Bool bStipple ) = 0;
	virtual void DrawPoly( const TRenderPoly& Poly ) = 0;
	virtual void DrawRect( const TRenderRect& Rect ) = 0;
	virtual void DrawList( const TRenderList& List ) = 0;

	// Transformations stack.
	void PushTransform( const gfx::ViewInfo& Info );
	void PopTransform();

	// Drawing utilities.
	void DrawCircle
	( 
		const math::Vector& Center,
		Float Radius,
		math::Color Color,
		Bool bStipple,
		Int32 Detail = 32 
	);

	void DrawEllipse
	( 
		const math::Vector& Center,
		Float XSize,
		Float YSize,
		math::Color Color,
		Bool bStipple,
		Int32 Detail = 32 
	);

	void DrawCoolPoint
	( 
		const math::Vector& P,
		Float Size,
		math::Color Color 
	);

	void DrawSmoothLine
	( 
		const math::Vector& A,
		const math::Vector& B,
		math::Color Color,
		Bool bStipple,
		Int32 Detail = 16 
	);

	void DrawLineStar
	( 
		const math::Vector& Center,
		math::Angle Rotation,
		Float Size,
		math::Color Color,
		Bool bStipple 
	);

	void DrawLineRect
	( 
		const math::Vector& Center,
		const math::Vector& Size,
		math::Angle Rotation,
		math::Color Color,
		Bool bStipple 
	);

protected:
	// Canvas internal.
	enum{ VIEW_STACK_SIZE = 8 };
	gfx::ViewInfo	ViewStack[VIEW_STACK_SIZE];
	Int32			StackTop;
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/