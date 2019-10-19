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
	CCanvas( gfx::DrawContext& drawContext )
		:	m_drawContext( drawContext )
	{
	}

	// Global memory pool, for temporal rendering
	// objects.
	static CStaticPool<4*1024*1024>		GPool;

	// CCanvas interface.
	virtual void DrawPoly( const TRenderPoly& Poly ) = 0;
	virtual void DrawRect( const TRenderRect& Rect ) = 0;
	virtual void DrawList( const TRenderList& List ) = 0;

	// Transformations stack.
	void PushTransform( const gfx::ViewInfo& Info )
	{
		m_drawContext.pushViewInfo( Info );
	}

	void PopTransform()
	{
		m_drawContext.popViewInfo();
	}

	const gfx::ViewInfo& viewInfo() const
	{
		return m_drawContext.getViewInfo();
	}

protected:
	// Canvas internal.
	gfx::DrawContext& m_drawContext;
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/