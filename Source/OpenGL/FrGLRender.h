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

	// FBOs.
	class CGLFbo*		MasterFBO;

	// COpenGLCanvas interface.
	COpenGLCanvas( COpenGLRender* InRender );
	~COpenGLCanvas();

	// CCanvas interface.
	void SetTransform( const TViewInfo& Info );
	void SetClip( const TClipArea& Area );
	void DrawPoint( const TVector& P, Float Size, TColor Color );
	void DrawLine( const TVector& A, const TVector& B, TColor Color, Bool bStipple );
	void DrawPoly( const TRenderPoly& Poly );
	void DrawRect( const TRenderRect& Rect );
	void DrawList( const TRenderList& List );

	// COpenGLCanvas interface.
	void SetBlend( EBitmapBlend Blend );
	void SetColor( TColor Color );
	void SetBitmap( FBitmap* Bitmap, Bool bUnlit=true );
	void SetStipple( DWord Stipple );
	void RenderLightmap();

	void EnableShader( CGLShaderBase* Shader );
	void DisableShader();

	// Friends.
	friend COpenGLRender;

private:
	// Internal used.
	TVector				BitmapPan;
	EBitmapBlend		OldBlend;
	FBitmap*			OldBitmap;
	TColor				OldColor;
	DWord				OldStipple;
	Float				LockTime;
	CGLShaderBase*		ActiveShader;
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
	Integer				WinWidth;
	Integer				WinHeight;

	// COpenGLRender interface.
	COpenGLRender( HWND InhWnd );
	~COpenGLRender();

	// CRenderBase interface.
	void Resize( Integer NewWidth, Integer NewHeight );
	void Flush();
	void RenderLevel( CCanvas* InCanvas, FLevel* Level, Integer X, Integer Y, Integer W, Integer H );
	CCanvas* Lock();
	void Unlock();	
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/