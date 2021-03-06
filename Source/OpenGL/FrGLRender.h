/*=============================================================================
    FrGLRender.h: OpenGL render class.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    Declarations.
-----------------------------------------------------------------------------*/

// Maximum resolution.
#define MAX_X_RES	1920
#define MAX_Y_RES	1080

#define OPENGL_ENABLED 0

#if OPENGL_ENABLED
/*-----------------------------------------------------------------------------
    COpenGLCanvas.
-----------------------------------------------------------------------------*/

//
// An OpenGL canvas.
//
class COpenGLCanvas: public CCanvas
{
public:
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
	COpenGLCanvas( COpenGLRender* InRender );
	~COpenGLCanvas();

	// CCanvas interface.
	void SetTransform( const gfx::ViewInfo& Info );
	void SetClip( const TClipArea& Area );
	void DrawPoint( const math::Vector& P, Float Size, math::Color Color );
	void DrawLine( const math::Vector& A, const math::Vector& B, math::Color Color, Bool bStipple );
	void DrawPoly( const TRenderPoly& Poly );
	void DrawRect( const TRenderRect& Rect );
	void DrawList( const TRenderList& List );

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
	Float				LockTime;
	CGLShaderBase*		ActiveShader;
	DrawStats			m_stats;
};


/*-----------------------------------------------------------------------------
    COpenGLRender.
-----------------------------------------------------------------------------*/

//
// An OpenGL render.
//
class COpenGLRender: public CRenderBase
{
public:
	// Window handles.
	HWND				hWnd;
	HDC					hDc;
	HGLRC				hRc;

	// Render variables.
	COpenGLCanvas*		Canvas;
	Int32				WinWidth;
	Int32				WinHeight;

	// COpenGLRender interface.
	COpenGLRender( HWND InhWnd );
	~COpenGLRender();

	// CRenderBase interface.
	void Resize( Int32 NewWidth, Int32 NewHeight );
	void Flush();
	void RenderLevel( CCanvas* InCanvas, FLevel* Level, Int32 X, Int32 Y, Int32 W, Int32 H );
	CCanvas* Lock();
	void Unlock();	
};

#endif

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/