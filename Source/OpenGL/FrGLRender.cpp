/*=============================================================================
    FrGLRender.cpp: OpenGL render class.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

#include "OpenGLRend.h"
#include "FrGLExt.h"
#include "FrGLFbo.h"

/*-----------------------------------------------------------------------------
    COpenGLRender implementation.
-----------------------------------------------------------------------------*/

//
// OpenGL rendering constructor.
//
COpenGLRender::COpenGLRender( HWND InhWnd )
{
	// Prepare.
	hWnd	= InhWnd;
	hDc		= GetDC( hWnd );

	// Initialize OpenGL.
	PIXELFORMATDESCRIPTOR Pfd;
	Int32 nPixelFormat;
	mem::zero( &Pfd, sizeof(PIXELFORMATDESCRIPTOR) );
	Pfd.dwFlags		= PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	nPixelFormat	= ChoosePixelFormat( hDc, &Pfd );
	SetPixelFormat( hDc, nPixelFormat, &Pfd );
	hRc				= wglCreateContext( hDc );
	wglMakeCurrent( hDc, hRc );

	// Initialize extensions.
	InitOpenGLext();

	// Allocate canvas.
	Canvas			= new COpenGLCanvas( this );

	// Notify.
	info( L"OpenGL: OpenGL render initialized" );
}


//
// Change the viewport resolution.
//
void COpenGLRender::Resize( Int32 NewWidth, Int32 NewHeight )
{
	// Clamp.
	NewWidth	= clamp( NewWidth,  1, MAX_X_RES );
	NewHeight	= clamp( NewHeight, 1, MAX_Y_RES );

	// Store resolution.
	WinWidth	= NewWidth;
	WinHeight	= NewHeight;

	// Update the OpenGL.
	glViewport( 0, 0, math::trunc(WinWidth), math::trunc(WinHeight) );
}


//
// Lock the render and start drawing.
//
CCanvas* COpenGLRender::Lock()
{
	// not supported yet
	profile_counter( EProfilerGroup::Memory, GPU_Allocated_Kb, 0.0 );

	// Setup OpenGL.
	glClear( GL_COLOR_BUFFER_BIT );	
	
	// Copy info to canvas.
	Canvas->ScreenWidth		= WinWidth;
	Canvas->ScreenHeight	= WinHeight;
	Canvas->LockTime		= fmod( GPlat->Now(), 1000.f*2.f*math::PI );

	mem::zero( &Canvas->m_stats, sizeof(COpenGLCanvas::DrawStats) );

	return Canvas;
}


//
// Blit an image into screen, and 
// unlock the render.
//
void COpenGLRender::Unlock()
{
	{
		profile_zone( EProfilerGroup::Render, SwapBuffers );
		SwapBuffers( hDc );
	}

	profile_counter( EProfilerGroup::DrawCalls, Points, Canvas->m_stats.points );
	profile_counter( EProfilerGroup::DrawCalls, Lines, Canvas->m_stats.lines );
	profile_counter( EProfilerGroup::DrawCalls, Rects, Canvas->m_stats.rects );
	profile_counter( EProfilerGroup::DrawCalls, Polygons, Canvas->m_stats.polygons );
	profile_counter( EProfilerGroup::DrawCalls, Lists, Canvas->m_stats.lists );
}


//
// Unload all temporal OpenGL stuff.
//
void COpenGLRender::Flush()
{
	Array<GLuint>	List;

	// Kill all FBitmap's data.
	if( GProject )
		for( Int32 i=0; i<GProject->GObjects.size(); i++ )
			if( GProject->GObjects[i] && GProject->GObjects[i]->IsA(FBitmap::MetaClass) )
			{
				FBitmap* Bitmap	= As<FBitmap>(GProject->GObjects[i]);
				if( Bitmap->RenderInfo != -1 )
					List.push(Bitmap->RenderInfo);

				Bitmap->RenderInfo	= -1;
			}

	if( List.size() > 0 )
		glDeleteTextures( List.size(), &List[0] );
}


//
// OpenGL render destructor.
//
COpenGLRender::~COpenGLRender()
{
	// Release canvas.
	delete Canvas;

	// Release OpenGL context.
	wglMakeCurrent( 0, 0 );
	wglDeleteContext( hRc );

	// Release Window DC.
	ReleaseDC( hWnd, hDc );
}


/*-----------------------------------------------------------------------------
    COpenGLCanvas implementation.
-----------------------------------------------------------------------------*/

//
// GFX effects set.
//
static TPostEffect	GNullEffect;
static TPostEffect	GWarpEffect;
static TPostEffect	GMirrorEffect;


//
// OpenGL canvas constructor.
//
COpenGLCanvas::COpenGLCanvas( COpenGLRender* InRender )
{
	// Initialize fields.
	Render			= InRender;
	StackTop		= 0;
	ScreenWidth		= ScreenHeight = 0.f;
	OldBlend		= BLEND_MAX;
	OldBitmap		= nullptr;
	OldColor		= math::colors::WHITE;
	BitmapPan		= { 0.f, 0.f };
	OldStipple		= POLY_None;
	ActiveShader	= nullptr;

	// Load shaders.
	FluShader.Init( L"flu_shader" );
	FinalShader.Init( L"final" );
	HorizBlurShader.Init( L"horiz_blur" );
	VertBlurShader.Init( L"vert_blur" );

	// Set default OpenGL state.
	glActiveTexture( GL_TEXTURE0 );
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	// Initialize gfx.
	GNullEffect.Highlights[0]	= 1.f;
	GNullEffect.Highlights[1]	= 1.f;
	GNullEffect.Highlights[2]	= 1.f;
	GNullEffect.MidTones[0]		= 1.f;
	GNullEffect.MidTones[1]		= 1.f;
	GNullEffect.MidTones[2]		= 1.f;
	GNullEffect.Shadows[0]		= 0.f;
	GNullEffect.Shadows[1]		= 0.f;
	GNullEffect.Shadows[2]		= 0.f;
	GNullEffect.BWScale			= 0.f;

	GWarpEffect.Highlights[0]	= 1.2f;
	GWarpEffect.Highlights[1]	= 1.f;
	GWarpEffect.Highlights[2]	= 1.f;
	GWarpEffect.MidTones[0]		= 0.9f;
	GWarpEffect.MidTones[1]		= 1.f;
	GWarpEffect.MidTones[2]		= 1.f;
	GWarpEffect.Shadows[0]		= -0.4f;
	GWarpEffect.Shadows[1]		= 0.f;
	GWarpEffect.Shadows[2]		= 0.f;
	GWarpEffect.BWScale			= 0.f;

	GMirrorEffect.Highlights[0]	= 1.f;
	GMirrorEffect.Highlights[1]	= 1.f;
	GMirrorEffect.Highlights[2]	= 1.2f;
	GMirrorEffect.MidTones[0]	= 1.f;
	GMirrorEffect.MidTones[1]	= 1.f;
	GMirrorEffect.MidTones[2]	= 0.9f;
	GMirrorEffect.Shadows[0]	= 0.f;
	GMirrorEffect.Shadows[1]	= 0.f;
	GMirrorEffect.Shadows[2]	= -0.4f;
	GMirrorEffect.BWScale		= 0.f;

	// Set default effect.
	FinalShader.SetPostEffect( GNullEffect );

	// Initialize FBOs.
	MasterFBO = new CGLFbo;
	BlurFBO = new CGLFbo;

	// Notify.
	info( L"OpenGL: OpenGL canvas initialized" );
}


//
// Set OpenGL's projection matrix for TViewInfo, fill
// the entire matrix. Used all parameters such as
// FOV, Zoom, Coords, and now even screen bounds.
//
void COpenGLCanvas::SetTransform( const TViewInfo& Info )
{
	GLfloat M[4][4];
	Float XFOV2, YFOV2;

	// Reset old matrix.
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	// Precompute.
	XFOV2	= 2.f / (Info.FOV.x * Info.Zoom);
	YFOV2	= 2.f / (Info.FOV.y * Info.Zoom);

	// Screen computations.
	math::Vector SScale, SOffset;
	SScale.x	= (Info.Width / ScreenWidth);
	SScale.y	= (Info.Height / ScreenHeight);
	SOffset.x	= (2.f/ScreenWidth) * (Info.X + (Info.Width/2.f)) - 1.f;
	SOffset.y	= 1.f - (2.f/ScreenHeight) * (Info.Y + (Info.Height/2.f));

	// Compute cells.
    M[0][0]	= XFOV2 * +Info.Coords.xAxis.x * SScale.x;
    M[0][1] = YFOV2 * -Info.Coords.xAxis.y * SScale.y;
    M[0][2] = 0.f;
    M[0][3] = 0.f;

    M[1][0] = XFOV2 * -Info.Coords.yAxis.x * SScale.x;
    M[1][1] = YFOV2 * +Info.Coords.yAxis.y * SScale.y;
    M[1][2] = 0.f;
    M[1][3] = 0.f;

    M[2][0] = 0.f;
    M[2][1] = 0.f;
    M[2][2] = 1.f;
    M[2][3] = 0.f;

    M[3][0] = -(Info.Coords.origin.x*M[0][0] + Info.Coords.origin.y*M[1][0]) + SOffset.x;
    M[3][1] = -(Info.Coords.origin.x*M[0][1] + Info.Coords.origin.y*M[1][1]) + SOffset.y;
    M[3][2] = -1.f;
    M[3][3] = 1.f;

    // Send this matrix to OpenGL.
    glLoadMatrixf( (GLfloat*)M );

	// Store this coords system.
	View			= Info;
}


//
// Canvas destructor.
//
COpenGLCanvas::~COpenGLCanvas()
{
	// Destroy FBOs.
	freeandnil(MasterFBO);
	freeandnil(BlurFBO);
}


/*-----------------------------------------------------------------------------
    Primitives drawing routines.
-----------------------------------------------------------------------------*/

//
// Draw a simple colored point.
//
void COpenGLCanvas::DrawPoint( const math::Vector& P, Float Size, math::Color Color )
{
	glPointSize( Size );
	SetColor( Color );
	SetBitmap( nullptr );

	m_stats.points++;
	glBegin( GL_POINTS );
	{
		glVertex2fv( (GLfloat*)&P );
	}
	glEnd();
}


//
// Draw a simple colored line.
//
void COpenGLCanvas::DrawLine( const math::Vector& A, const math::Vector& B, math::Color Color, Bool bStipple )
{
	SetColor( Color );
	SetBitmap( nullptr );

	if( bStipple )
	{
		// Stippled line.
		glEnable( GL_LINE_STIPPLE );
		glLineStipple( 1, 0x9292 );
	}
	else
	{
		// Solid line.
		glDisable( GL_LINE_STIPPLE );
	}

	m_stats.lines++;
	glBegin( GL_LINES );
	{
		glVertex2fv( (GLfloat*)&A );
		glVertex2fv( (GLfloat*)&B );
	}
	glEnd();
}


//
// Draw rectangle.
//
void COpenGLCanvas::DrawRect( const TRenderRect& Rect )
{
	math::Vector Verts[4];

	// Compute sprite vertices.
	if( !Rect.Rotation )
	{
		// No rotation.
		Verts[0] = Rect.Bounds.min;
		Verts[1] = math::Vector( Rect.Bounds.min.x, Rect.Bounds.max.y );
		Verts[2] = Rect.Bounds.max;
		Verts[3] = math::Vector( Rect.Bounds.max.x, Rect.Bounds.min.y );
	}
	else
	{
		// Rotation.
		math::Vector Center	= Rect.Bounds.center();
		math::Vector Size2	= Rect.Bounds.size() * 0.5f;
		math::Coords Coords	= math::Coords( Center, Rect.Rotation );

		math::Vector XAxis = Coords.xAxis * Size2.x,
				YAxis = Coords.yAxis * Size2.y;

		// World coords.
		Verts[0] = Center - YAxis - XAxis;
		Verts[1] = Center + YAxis - XAxis;
		Verts[2] = Center + YAxis + XAxis;
		Verts[3] = Center - YAxis + XAxis;
	}


	if( Rect.Flags & POLY_FlatShade )
	{
		// Draw colored rectangle.
		SetBitmap( nullptr );
		SetColor( Rect.Color );
		SetStipple( Rect.Flags );
		if( Rect.Flags & POLY_Ghost )
			SetBlend( BLEND_Translucent );

		if( Rect.Flags & POLY_AlphaGhost )
			SetBlend( BLEND_Alpha );

		m_stats.rects++;
		glBegin( GL_POLYGON );
		{
			for( Int32 i=0; i<4; i++ )
				glVertex2fv( (GLfloat*)&Verts[i] );
		}
		glEnd();
		SetStipple( POLY_None );
	}
	else
	{
		FMaterial* Material = As<FMaterial>(Rect.Texture);	
		if( Material )
		{
			//
			// Render as material.
			//
			if( Material->Layers.size() == 0 )
			{
				// No render layers.
				SetBitmap(FBitmap::NullBitmap(), true);
				goto RenderNull;	
			}

			// Material texture coords.
			math::Vector T1	= Rect.TexCoords.min;
			math::Vector T2	= Rect.TexCoords.max;
			math::Vector RawTexVerts[4];
			RawTexVerts[0]	= math::Vector( T1.x, T1.y );
			RawTexVerts[1]	= math::Vector( T1.x, T2.y );
			RawTexVerts[2]	= math::Vector( T2.x, T2.y );
			RawTexVerts[3]	= math::Vector( T2.x, T1.y );

			// Foreach material layer.
			for( Int32 iLayer=Material->Layers.size()-1; iLayer>=0; iLayer-- )
			{
				FDiffuseLayer* Layer = As<FDiffuseLayer>(Material->Layers[iLayer]);
				if( !Layer->Bitmap )
					continue;

				SetBitmap(Layer->Bitmap, Layer->bUnlit || (Rect.Flags & POLY_Unlit));
				SetBlend(Layer->BlendMode);
				SetColor(Layer->OverlayColor * Rect.Color);

				math::Vector FinalTexCoords[4];
				Layer->ApplyTransform( View, RawTexVerts, FinalTexCoords, 4 );

				m_stats.rects++;
				glBegin( GL_POLYGON );
				{
					for( Int32 i=0; i<4; i++ )
					{
						glTexCoord2fv( (GLfloat*)&FinalTexCoords[i] );
						glVertex2fv( (GLfloat*)&Verts[i] );
					}
				}
				glEnd();
			}
		}
		else
		{
			//
			// Draw textured rectangle.
			//
			SetBitmap( Rect.Texture ? As<FBitmap>(Rect.Texture) : FBitmap::NullBitmap(), Rect.Flags & POLY_Unlit );

		RenderNull:;
			SetColor( Rect.Color );
			if( Rect.Flags & POLY_Ghost )
				SetBlend( BLEND_Brighten );

			// Texture coords.
			math::Vector T1	= Rect.TexCoords.min + BitmapPan;
			math::Vector T2	= Rect.TexCoords.max + BitmapPan;
			math::Vector TexVerts[4];
			TexVerts[0]	= math::Vector( T1.x, T1.y );
			TexVerts[1]	= math::Vector( T1.x, T2.y );
			TexVerts[2]	= math::Vector( T2.x, T2.y );
			TexVerts[3]	= math::Vector( T2.x, T1.y );

			m_stats.rects++;
			glBegin( GL_POLYGON );
			{
				for( Int32 i=0; i<4; i++ )
				{
					glTexCoord2fv( (GLfloat*)&TexVerts[i] );
					glVertex2fv( (GLfloat*)&Verts[i] );
				}
			}
			glEnd();
		}
	}
}


//
// Draw a list of rectangles.
//
void COpenGLCanvas::DrawList( const TRenderList& List )
{
	if( List.Flags & POLY_FlatShade )
	{
		// Draw a colored rectangles.
		SetBitmap( nullptr );
		if( List.Flags & POLY_Ghost )
			SetBlend( BLEND_Translucent );
		if( List.Flags & POLY_AlphaGhost )
			SetBlend( BLEND_Alpha );

		// Set color.
		if( List.Colors )
		{
			glEnableClientState( GL_COLOR_ARRAY );
			glColorPointer( 4, GL_UNSIGNED_BYTE, 0, List.Colors );
		}
		else
			SetColor( List.DrawColor );

		// Render it.
		m_stats.lists++;
		glVertexPointer( 2, GL_FLOAT, 0, List.Vertices );
		glDrawArrays( GL_QUADS, 0, List.NumRects*4 );

		// Unset color.
		if( List.Colors )
			glDisableClientState( GL_COLOR_ARRAY );
	}
	else
	{
		// Draw textured rectangles.
		SetBitmap( List.Texture ? As<FBitmap>(List.Texture) : FBitmap::NullBitmap(), List.Flags & POLY_Unlit );
		if( List.Flags & POLY_Ghost )
			SetBlend( BLEND_Brighten );
		if( List.Flags & POLY_AlphaGhost )
			SetBlend( BLEND_Alpha );

		// Set color.
		if( List.Colors )
		{
			glEnableClientState( GL_COLOR_ARRAY );
			glColorPointer( 4, GL_UNSIGNED_BYTE, 0, List.Colors );
		}
		else
			SetColor( List.DrawColor );

		// Apply panning if any.
		if( BitmapPan.sizeSquared() > 0.1f )
			for( Int32 i=0; i<List.NumRects*4; i++ )
				List.TexCoords[i]	+= BitmapPan;

		// Render it.
		m_stats.lists++;
		glVertexPointer( 2, GL_FLOAT, 0, List.Vertices );
		glTexCoordPointer( 2, GL_FLOAT, 0, List.TexCoords ); 
		glDrawArrays( GL_QUADS, 0, List.NumRects*4 );

		// Unset color.
		if( List.Colors )
			glDisableClientState( GL_COLOR_ARRAY );
	}
}


//
// Draw a convex polygon.
//
void COpenGLCanvas::DrawPoly( const TRenderPoly& Poly )
{
	if( Poly.Flags & POLY_FlatShade )
	{
		// Draw colored polygon.
		SetBitmap( nullptr );
		SetColor( Poly.Color );
		SetStipple( Poly.Flags );
		if( Poly.Flags & POLY_Ghost )
			SetBlend( BLEND_Translucent );

		if( Poly.Flags & POLY_AlphaGhost )
			SetBlend( BLEND_Alpha );

		m_stats.polygons++;
		glBegin( GL_POLYGON );
		{
			for( Int32 i=0; i<Poly.NumVerts; i++ )
				glVertex2fv( (GLfloat*)&Poly.Vertices[i] );
		}
		glEnd();
		SetStipple( POLY_None );
	}
	else
	{
		FMaterial* Material = As<FMaterial>(Poly.Texture);
		if( Material )
		{
			//
			// Render as material.
			//
			if( Material->Layers.size() == 0 )
			{
				// No render layers.
				SetBitmap(FBitmap::NullBitmap(), true);
				goto RenderNull;	
			}

			// Foreach material layer.
			for( Int32 iLayer=Material->Layers.size()-1; iLayer>=0; iLayer-- )
			{
				FDiffuseLayer* Layer = As<FDiffuseLayer>(Material->Layers[iLayer]);
				if( !Layer->Bitmap )
					continue;

				SetBitmap(Layer->Bitmap, Layer->bUnlit || (Poly.Flags & POLY_Unlit));
				SetBlend(Layer->BlendMode);
				SetColor(Layer->OverlayColor * Poly.Color);

				math::Vector FinalTexCoords[16];
				Layer->ApplyTransform( View, Poly.TexCoords, FinalTexCoords, Poly.NumVerts );

				m_stats.polygons++;
				glBegin( GL_POLYGON );
				{
					for( Int32 i=0; i<Poly.NumVerts; i++ )
					{
						glTexCoord2fv( (GLfloat*)&FinalTexCoords[i] );
						glVertex2fv( (GLfloat*)&Poly.Vertices[i] );
					}
				}
				glEnd();
			}
		}
		else
		{
			//
			// Render as bitmap.
			//
			SetBitmap( Poly.Texture ? As<FBitmap>(Poly.Texture) : FBitmap::NullBitmap(), Poly.Flags & POLY_Unlit );

		RenderNull:;
			SetColor( Poly.Color );
			if( Poly.Flags & POLY_Ghost )
				SetBlend( BLEND_Brighten );

			m_stats.polygons++;
			glBegin( GL_POLYGON );
			{
				for( Int32 i=0; i<Poly.NumVerts; i++ )
				{
					math::Vector T = Poly.TexCoords[i] + BitmapPan;
					glTexCoord2fv( (GLfloat*)&T );
					glVertex2fv( (GLfloat*)&Poly.Vertices[i] );
				}
			}
			glEnd();
		}
	}
}


/*-----------------------------------------------------------------------------
    Canvas support functions.
-----------------------------------------------------------------------------*/

//
// Convert a paletted image to 32-bit image.
//
static void* Palette8ToRGBA( UInt8* SourceData, math::Color* Palette, Int32 USize, Int32 VSize )
{
	static math::Color Buffer512[512*512];
	Int32 i, n;

    // Doesn't allow palette image with dimension > 512.
	if( USize*VSize > 512*512 )
		return nullptr;
	
// No asm required since VC++ generates well
// optimized code.
#if FLU_ASM && 0
	n = USize*VSize;
	__asm
	{
		mov	esi,	[Palette]
		mov ebx,	[SourceData]
		mov	ecx,	[n]

		align 16
	Reloop:
			movzx	eax,							byte ptr [ebx+ecx]
			mov		edx,							dword ptr [esi+eax*4]
			mov		dword ptr [Buffer512+ecx*4],	edx
			dec		ecx
			jnz Reloop
	}
#else	
	i	= USize * VSize;
	while( i-- > 0 )
	{
		Buffer512[i] = Palette[SourceData[i]];
	}
#endif

	return Buffer512;
}


//
// Set a bitmap for rendering.
// Here possible usage of function:
//  A. Bitmap!=nullptr && bUnlit  - Draw unlit texture.
//  B. Bitamp==nullptr && bUnlit  - Turn off shader, draw colored.
//  C. Bitmap!=nullptr && !bUnlit - Draw complex lit texture.
//  D. Bitamp==nullptr && !bUnlit - see case B.
//
void COpenGLCanvas::SetBitmap( FBitmap* Bitmap, Bool bUnlit )	
{
	if( Bitmap )
	{
		// Setup valid bitmap.
		if( Bitmap != OldBitmap )
		{
			// Load bitmap to OpenGL, if not loaded before.
			if( Bitmap->RenderInfo == -1 )
			{
				glGenTextures( 1, (GLuint*)&Bitmap->RenderInfo );
				glBindTexture( GL_TEXTURE_2D, Bitmap->RenderInfo );
				glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

				if( Bitmap->Format == BF_Palette8 )
				{             
					// Load a Palette image.
					glTexImage2D
							(
								GL_TEXTURE_2D,
								0,
								4,
								Bitmap->USize,
								Bitmap->VSize,
								0,
								GL_RGBA,
								GL_UNSIGNED_BYTE,
								Palette8ToRGBA
											( 
												(UInt8*)Bitmap->GetData(), 
												&Bitmap->Palette.Colors[0], 
												Bitmap->USize, 
												Bitmap->VSize 
											)
							);
				}
				else
				{
					// Load a RGBA image.
					glTexImage2D
							( 
								GL_TEXTURE_2D,
								0,
								4,
								Bitmap->USize,
								Bitmap->VSize,
								0,
								GL_RGBA,
								GL_UNSIGNED_BYTE,
								Bitmap->GetData() 
							);
				}
			}

			// Make current.
			glBindTexture( GL_TEXTURE_2D, Bitmap->RenderInfo );

			// Update the dynamic bitmap, if required.
			if( Bitmap->bDynamic && Bitmap->bRedrawn )
			{
				if( Bitmap->Format == BF_Palette8 )
				{             
					// Load a dynamic palette image.
					glTexSubImage2D
								(
									GL_TEXTURE_2D,
									0,
									0,
									0,
									Bitmap->USize,
									Bitmap->VSize,
									GL_RGBA,
									GL_UNSIGNED_BYTE,
									Palette8ToRGBA
												( 
													(UInt8*)Bitmap->GetData(), 
													&Bitmap->Palette.Colors[0], 
													Bitmap->USize, 
													Bitmap->VSize 
												)
								);
				}
				else
				{
					// Load a RGBA image.
					glTexSubImage2D
								( 
									GL_TEXTURE_2D,
									0,
									0,
									0,
									Bitmap->USize,
									Bitmap->VSize,
									GL_RGBA,
									GL_UNSIGNED_BYTE,
									Bitmap->GetData() 
								);
				}

				// Wait for the next redraw.
				Bitmap->bRedrawn = false;
			}

			// Set bitmap filter.
			if( Bitmap->Filter == BFILTER_Nearest )
			{
				// 8-bit style.
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			}
			else
			{
				// Smooth style.
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			}

			// Switch blending mode.
			SetBlend( Bitmap->BlendMode );

			// Update bitmap animation.
			Bitmap->Tick();

			// Bitmap panning.
			BitmapPan.x	= Bitmap->PanUSpeed * LockTime;
			BitmapPan.y	= Bitmap->PanVSpeed * LockTime;
		}

		// Update lit/unlit, even for same bitmap.
		if( bUnlit )
			FluShader.SetModeUnlit();
		else
			FluShader.SetModeComplex();

		// Saturation scale.
		EnableShader( &FluShader );
		FluShader.SetValue1f( FluShader.idSaturation, Bitmap->Saturation );
	}
	else
	{
		// Turn off bitmap.
		DisableShader();
		SetBlend( BLEND_Regular );
		OldBitmap	= nullptr;
	}
}


//
// Set a blend mode for rendering.
//
void COpenGLCanvas::SetBlend( EBitmapBlend Blend )
{
	// Don't use OpenGL too often.
	if( Blend == OldBlend )
		return;

	switch( Blend )
	{
		case BLEND_Regular:
			// No blending.
			glDisable( GL_BLEND );
			glDisable( GL_ALPHA_TEST );
			break;

		case BLEND_Masked:
			// Masked ( with rough edges ).
			glDisable( GL_BLEND );
			glEnable( GL_ALPHA_TEST );
			glAlphaFunc( GL_GREATER, 0.95f );
			break;

		case BLEND_Translucent:
			// Additive blending.
			glEnable( GL_BLEND );
			glDisable( GL_ALPHA_TEST );
			glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_COLOR );
			break;

		case BLEND_Modulated:
			// Modulation.
			glEnable( GL_BLEND );
			glDisable( GL_ALPHA_TEST );
			glBlendFunc( GL_DST_COLOR, GL_SRC_COLOR );
			break;

		case BLEND_Alpha:
			// Alpha mask.
			glEnable( GL_BLEND );
			glDisable( GL_ALPHA_TEST );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			break;

		case BLEND_Darken:
			// Dark modulation.
			glEnable( GL_BLEND );
			glDisable( GL_ALPHA_TEST );
			glBlendFunc( GL_ZERO, GL_ONE_MINUS_SRC_COLOR );
			break;

		case BLEND_Brighten:
			// Extra bright addition.
			glEnable( GL_BLEND );
			glDisable( GL_ALPHA_TEST );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE );
			break;

		case BLEND_FastOpaque:
			// Software rendering only.
			glEnable( GL_BLEND );
			glDisable( GL_ALPHA_TEST );
			glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_COLOR );
			break;

		default:
			error( L"OpenGL: Bad blend type %d", Blend );
			break;
	}
	OldBlend = Blend;
}


//
// Set color to draw objects.
//
void COpenGLCanvas::SetColor( math::Color Color )
{
	// Don't overuse OpenGL.
	if( OldColor == Color )
		return;

	// Set it.
	glColor4ubv( (GLubyte*)&Color );
	OldColor = Color;
}


//
// Set a stipple pattern for poly render.
//
void COpenGLCanvas::SetStipple( UInt32 Stipple )
{
	Stipple	&= POLY_StippleI | POLY_StippleII;

	if( Stipple == OldStipple )
		return;

	if( Stipple & POLY_StippleI )
	{
		// Polka dot pattern.
		static const GLubyte PolkaDot[128] =
		{
			0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00, 0x44, 0x44, 0x44, 0x44,
			0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00, 0x44, 0x44, 0x44, 0x44,
			0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00, 0x44, 0x44, 0x44, 0x44,
			0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00, 0x44, 0x44, 0x44, 0x44,
			0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};	

		glEnable( GL_POLYGON_STIPPLE );
		glPolygonStipple( PolkaDot );
	}
	else if( Stipple & POLY_StippleII )
	{
		// Pin stripes pattern.
		static const GLubyte PinStripes[128] =
		{
			0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
			0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 
			0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 
			0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 
			0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
			0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 
			0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 
			0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 
			0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
			0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 
			0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 
			0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 
			0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
			0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 
			0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 
			0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88
		};

		glEnable( GL_POLYGON_STIPPLE );
		glPolygonStipple( PinStripes );
	}
	else
	{
		// No pattern.
		glDisable( GL_POLYGON_STIPPLE );
	}

	OldStipple = Stipple;
}


//
// Set a clipping area.
//
void COpenGLCanvas::SetClip( const TClipArea& Area )
{
	if	( 
			( Area.X == -1 ) && 
			( Area.Y == -1 ) && 
			( Area.Width == -1 ) && 
			( Area.Height == -1 )
		)
	{
		// Turn off clipping.
		glDisable( GL_SCISSOR_TEST );
	}
	else
	{
		// Turn on clipping.
		glEnable( GL_SCISSOR_TEST );
		glScissor
			( 
				Area.X,
				(Int32)ScreenHeight - Area.Y - Area.Height,
				Area.Width,
				Area.Height
			);
	}

	// Store clipping area.
	Clip	= Area;
}


//
// Turn on shader.
//
void COpenGLCanvas::EnableShader( CGLShaderBase* Shader )
{
	if( ActiveShader == Shader )
		return;

	assert(Shader);

	if( ActiveShader )
		ActiveShader->bEnabled = false;

	ActiveShader = Shader;
	glUseProgram( ActiveShader->iglProgram );

	ActiveShader->bEnabled = true;
	ActiveShader->CommitValues();
}


//
// Disable any shader.
//
void COpenGLCanvas::DisableShader()
{
	if( ActiveShader )
	{
		glUseProgram( 0 );
		ActiveShader->bEnabled = false;
		ActiveShader = nullptr;
	}
}


/*-----------------------------------------------------------------------------
	CGLFluShader implementation.
-----------------------------------------------------------------------------*/

//
// FluShader constructor.
//
CGLFluShader::CGLFluShader()
	:	CGLShaderBase()
{
}


//
// FluShader destructor.
//
CGLFluShader::~CGLFluShader()
{
}


//
// FluShader initialization.
//
bool CGLFluShader::Init( String ShaderName )
{
	if( !CGLShaderBase::Init(ShaderName) )
		return false;

	// Bind to uniform variables.
	idUnlit				= RegisterUniform( "bUnlit" );
	idRenderLightmap	= RegisterUniform( "bRenderLightmap" );
	idBitmap			= RegisterUniform( "Bitmap" );
	idSaturation		= RegisterUniform( "Saturation" );
	idMNum				= RegisterUniform( "MNum" );
	idANum				= RegisterUniform( "ANum" );
	idGameTime			= RegisterUniform( "Time" );
	idAmbientLight		= RegisterUniform( "AmbientLight" );

	// Bind all lights.
	for( Int32 iLight=0; iLight<MAX_LIGHTS; iLight++ )
	{
		AnsiChar Buffer[64];

		sprintf_s( Buffer, "ALights[%d].Effect", iLight );
		idALights[iLight].Effect		= RegisterUniform( Buffer );
		sprintf_s( Buffer, "ALights[%d].Color", iLight );
		idALights[iLight].Color			= RegisterUniform( Buffer );
		sprintf_s( Buffer, "ALights[%d].Brightness", iLight );
		idALights[iLight].Brightness	= RegisterUniform( Buffer );
		sprintf_s( Buffer, "ALights[%d].Radius", iLight );
		idALights[iLight].Radius		= RegisterUniform( Buffer );
		sprintf_s( Buffer, "ALights[%d].Location", iLight );
		idALights[iLight].Location		= RegisterUniform( Buffer );
		sprintf_s( Buffer, "ALights[%d].Rotation", iLight );
		idALights[iLight].Rotation		= RegisterUniform( Buffer );

		sprintf_s( Buffer, "MLights[%d].Effect", iLight );
		idMLights[iLight].Effect		= RegisterUniform( Buffer );
		sprintf_s( Buffer, "MLights[%d].Color", iLight );
		idMLights[iLight].Color			= RegisterUniform( Buffer );
		sprintf_s( Buffer, "MLights[%d].Brightness", iLight );
		idMLights[iLight].Brightness	= RegisterUniform( Buffer );
		sprintf_s( Buffer, "MLights[%d].Radius", iLight );
		idMLights[iLight].Radius		= RegisterUniform( Buffer );
		sprintf_s( Buffer, "MLights[%d].Location", iLight );
		idMLights[iLight].Location		= RegisterUniform( Buffer );
		sprintf_s( Buffer, "MLights[%d].Rotation", iLight );
		idMLights[iLight].Rotation		= RegisterUniform( Buffer );
	}

	// Only one bitmap supported.
	SetValue1i( idBitmap, 0 );

	glUseProgram( 0 );
	bEnabled = false;

	// Notify.
	info( L"Rend: GLFluShader initialized" );
	return true;
}


//
// Set an scene ambient color.
//
void CGLFluShader::SetAmbientLight( const math::Color& InAmbient )
{
	// Really need to update?
	static math::Color	OldAmbient = math::colors::ALICE_BLUE;
	if( InAmbient == OldAmbient )
		return;
	OldAmbient	= InAmbient;

	// Send data to shader.
	GLfloat Ambient[3] = { InAmbient.r/255.f, InAmbient.g/255.f, InAmbient.b/255.f };
	SetValue3f( idAmbientLight, Ambient );
}


//
// Reset a lights lists. Prepare
// manager for lights collection.
//
void CGLFluShader::ResetLights()
{
	SetValue1i( idMNum, 0 );
	SetValue1i( idANum, 0 );

	MNum = ANum = 0;
}


//
// Add a new light to the shader. Return true, if added
// successfully, return false if list overflow, and light not
// added.
//
Bool CGLFluShader::AddLight( FLightComponent* Light, const math::Vector& Location, math::Angle Rotation )
{
	// Don't add disabled light source.
	if( !Light->bEnabled )
		return true;

	TLightSource* idSource	= nullptr;

	// Add lightsource to appropriate lights list.
	if( Light->LightFunc == LF_Additive )
	{
		if( ANum >= MAX_LIGHTS )
			return false;

		ANum++;
		idSource	= &idALights[ANum-1];
		SetValue1i( idANum, ANum );
	}
	else
	{
		if( MNum >= MAX_LIGHTS )
			return false;

		MNum++;
		idSource	= &idMLights[MNum-1];
		SetValue1i( idMNum, MNum );
	}

	// Setup lightsource params.
	GLfloat	LightColor[4]	=
	{
		Light->Color.r / 255.f,
		Light->Color.g / 255.f,
		Light->Color.b / 255.f,
		1.f
	};

	// Compute brightness according to light type.
	GLfloat Scale;
	switch( Light->LightType )
	{
		case LIGHT_Flicker:		
			Scale	= RandomF();								
			break;

		case LIGHT_Pulse:		
			Scale	= 0.6f + 0.39f*math::sin( GPlat->Now()*(2.f*math::PI)*35.f/60.f );								
			break;

		case LIGHT_SoftPulse:		
			Scale	= 0.9f + 0.09f*math::sin( GPlat->Now()*(2.f*math::PI)*35.f/50.f );							
			break;

		default:
			Scale	= 1.f;
			break;
	}

	SetValue1i( idSource->Effect, (Int32)Light->LightType );
	SetValue4f( idSource->Color, LightColor );
	SetValue1f( idSource->Brightness, Light->Brightness * Scale );
	SetValue1f( idSource->Radius, Light->Radius );
	SetValue2f( idSource->Location, Location );
	SetValue1f( idSource->Rotation, Rotation.toRads() );

	return true;
}


//
// Set a complex lit mode.
//
void CGLFluShader::SetModeComplex()
{
	SetValue1i( idUnlit, 0 );
	SetValue1i( idRenderLightmap, 0 );
}


//
// Set unlit render mode. 
//
void CGLFluShader::SetModeUnlit()
{
	SetValue1i( idUnlit, 1 );
	SetValue1i( idRenderLightmap, 0 );
}


//
// Set lightmap rendering mode.
//
void CGLFluShader::SetModeLightmap()
{
	SetValue1i( idRenderLightmap, 1 );
}


/*-----------------------------------------------------------------------------
    Level rendering.
-----------------------------------------------------------------------------*/

//
// Draw an editor grid with cell size 1x1.
//
void drawGrid( COpenGLCanvas* Canvas )
{
	// Compute bounds.
	Int32 CMinX = math::trunc(max<Float>( Canvas->View.Bounds.min.x, -math::WORLD_HALF ));
	Int32 CMinY = math::trunc(max<Float>( Canvas->View.Bounds.min.y, -math::WORLD_HALF ));
	Int32 CMaxX = math::trunc(min<Float>( Canvas->View.Bounds.max.x, +math::WORLD_HALF ));
	Int32 CMaxY = math::trunc(min<Float>( Canvas->View.Bounds.max.y, +math::WORLD_HALF ));

	// Pick colors.
	math::Color GridColor0 = math::Color( 0x40, 0x40, 0x40, 0xff );
	math::Color GridColor1 = math::Color( 0x80, 0x80, 0x80, 0xff );
	
	// Vertical lines.
	for( Int32 i=CMinX; i<=CMaxX; i++ )
	{
		math::Vector V1( i, -math::WORLD_HALF );
		math::Vector V2( i, +math::WORLD_HALF );
	
		if( !(i & 7) )
			Canvas->DrawLine( V1, V2, GridColor1, false );
		else if( !(i & 3) )
			Canvas->DrawLine( V1, V2, GridColor0, false );
		else
			Canvas->DrawLine( V1, V2, GridColor0, true );
	}

	// Horizontal lines.
	for( Int32 i=CMinY; i<=CMaxY; i++ )
	{
		math::Vector V1( -math::WORLD_HALF, i );
		math::Vector V2( +math::WORLD_HALF, i );
	
		if( !(i & 7) )
			Canvas->DrawLine( V1, V2, GridColor1, false );
		else if( !(i & 3) )
			Canvas->DrawLine( V1, V2, GridColor0, false );
		else
			Canvas->DrawLine( V1, V2, GridColor0, true );
	}
}


//
// Render overlay lightmap, in the current 
// view info.
//
void COpenGLCanvas::RenderLightmap()
{
	EnableShader( &FluShader );
	FluShader.SetModeLightmap();
	SetBlend( BLEND_Translucent );

	m_stats.rects++;
	glBegin( GL_QUADS );
	{
		glVertex2f( View.Bounds.min.x, View.Bounds.min.y );
		glVertex2f( View.Bounds.min.x, View.Bounds.max.y );
		glVertex2f( View.Bounds.max.x, View.Bounds.max.y );
		glVertex2f( View.Bounds.max.x, View.Bounds.min.y );
	}
	glEnd();
}


//
// Draw safe frame for observer.
//
void drawSafeFrame( COpenGLCanvas* Canvas, TCamera& Observer )
{
	// Draw simple editor bounds.	
	Canvas->DrawLineRect
	( 
		Observer.Location, 
		Observer.FOV, 
		Observer.Rotation, 
		math::colors::PERU, 
		false 
	);
}


//
// Render level's sky zone, if any.
//
void drawSkyZone( COpenGLCanvas* Canvas, FLevel* Level, const TViewInfo& Parent )
{
	profile_zone( EProfilerGroup::Render, RenderSky );

	// Prepare.
	FSkyComponent* Sky = Level->Sky;
	if( !Sky )
		return;

	// Compute sky frustum.
	math::Vector ViewArea, Eye;
	ViewArea.x	= Sky->Extent;
	ViewArea.y	= Sky->Extent * Parent.FOV.y / Parent.FOV.x;

	math::Rect SkyDome	= math::Rect( Sky->Location, Sky->Size.x, Sky->Size.y );
	Float	Side	= math::sqrt(sqr(ViewArea.x) + sqr(ViewArea.y)),
			Side2	= Side * 0.5f;

	// Transform observer location and apply parallax.
	Eye.x	= Canvas->View.Coords.origin.x * Sky->Parallax.x + Sky->Offset.x;
	Eye.y	= Canvas->View.Coords.origin.y * Sky->Parallax.y + Sky->Offset.y;

	// Azimuth of sky should be wrapped.
	Eye.x	= Wrap( Eye.x, SkyDome.min.x, SkyDome.max.x );

	// Height of sky should be clamped.
	Eye.y	= clamp( Eye.y, SkyDome.min.y+Side2, SkyDome.max.y-Side2 );

	// Sky roll angle.
	math::Angle Roll = math::Angle(fmodf( Sky->RollSpeed*(Float)GPlat->Now(), 2.f*math::PI ));	

	// Flags of sides renderings.
	Bool	bDrawWest	= false;
	Bool	bDrawEast	= false;

	// Setup main sky view.
	TViewInfo SkyView	= TViewInfo
	(
		Eye,
		Roll,
		ViewArea,
		/*1.f /*/ Parent.Zoom,
		true,
		Parent.X,
		Parent.Y,
		Parent.Width,
		Parent.Height			
	);
	TViewInfo WestView, EastView;

	// Compute wrapped pieces of sky zone.
	if( Eye.x-Side2 < SkyDome.min.x )
	{
		// Draw also west piece.
		math::Vector WestEye = math::Vector( SkyDome.max.x+(Eye.x-SkyDome.min.x), Eye.y );
		WestView		= TViewInfo
		(
			WestEye,
			Roll,
			ViewArea,
			SkyView.Zoom,
			true,
			SkyView.X,
			SkyView.Y,
			SkyView.Width,
			SkyView.Height	
		);
		bDrawWest	= true;
	}
	if( Eye.x+Side2 > SkyDome.max.x )
	{
		// Draw also east piece.
		math::Vector EastEye = math::Vector( SkyDome.min.x-(SkyDome.max.x-Eye.x), Eye.y );
		EastView		= TViewInfo
		(
			EastEye,
			Roll,
			ViewArea,
			SkyView.Zoom,
			true,
			SkyView.X,
			SkyView.Y,
			SkyView.Width,
			SkyView.Height	
		);
		bDrawEast	= true;
	}

	// Collect lightsource.
	if( Level->RndFlags & RND_Lighting )
	{
		// Clear lights list.
		Canvas->FluShader.ResetLights();

		// Collect it.
		for( FLightComponent* Light=Level->FirstLight; Light; Light=Light->NextLight )
		{
			FBaseComponent*		Base	= Light->Base;

			// From master view.
			math::Rect LightRect = math::Rect( Base->Location, Light->Radius*2.f );
			if( SkyDome.isOverlap(LightRect) )
			{
				if( !Canvas->FluShader.AddLight( Light, Base->Location, Base->Rotation ) )
					break;
			}
			else
				continue;

			// Fake west side.
			LightRect	= math::Rect( Base->Location-math::Vector( Sky->Size.x, 0.f ), Light->Radius*2.f );
			if( SkyDome.isOverlap(LightRect) )
				if( !Canvas->FluShader.AddLight( Light, LightRect.center(), Base->Rotation ) )
					break;

			// Fake east side.
			LightRect	= math::Rect( Base->Location+math::Vector( Sky->Size.x, 0.f ), Light->Radius*2.f );
			if( SkyDome.isOverlap(LightRect) )
				if( !Canvas->FluShader.AddLight( Light, LightRect.center(), Base->Rotation ) )
					break;
		}
	}

	// Render west sky piece.
	if( bDrawWest )
	{
		Canvas->PushTransform( WestView );
		{
			for( Int32 i=0; i<Level->RenderObjects.size(); i++ )
				Level->RenderObjects[i]->Render( Canvas );
		}
		Canvas->PopTransform();
	}

	// Render east sky piece.
	if( bDrawEast )
	{
		Canvas->PushTransform( EastView );
		{
			for( Int32 i=0; i<Level->RenderObjects.size(); i++ )
				Level->RenderObjects[i]->Render( Canvas );
		}
		Canvas->PopTransform();
	}

	// Render central sky piece.
	Canvas->PushTransform( SkyView );
	{
		for( Int32 i=0; i<Level->RenderObjects.size(); i++ )
			Level->RenderObjects[i]->Render( Canvas );

		// Draw overlay lightmap, in sky coords!
		Canvas->RenderLightmap();
	}
	Canvas->PopTransform();		
}


//
// Draw all half-planes from all visible 
// mirrors.
//
void drawHalfPlaneMirror( COpenGLCanvas* Canvas, FLevel* Level, const TViewInfo& Parent )
{
	math::Rect Observer	= Parent.Bounds;

	for( FPortalComponent* Portal=Level->FirstPortal; Portal; Portal=Portal->NextPortal )
	{
		if( !Portal->IsA(FMirrorComponent::MetaClass) )
			continue;

		Float HalfWidth = Portal->Width * 0.5f;

		// Fast X reject.
		if	( 
				Portal->Location.x < Observer.min.x ||
				Portal->Location.x > Observer.max.x	
			)
			continue;

		// Fast Y reject.
		if	(
				Portal->Location.y+HalfWidth < Observer.min.y ||
				Portal->Location.y-HalfWidth > Observer.max.y
			)
			continue;

		// Yes, mirror visible, render objects.
		TViewInfo MirrorView;
		FMirrorComponent* Mirror = (FMirrorComponent*)Portal;
		Mirror->ComputeViewInfo( Parent, MirrorView );
		Canvas->PushTransform( MirrorView );
		{
			// Collect mirror's light sources.
			if( Level->RndFlags & RND_Lighting )
			{
				Canvas->FluShader.ResetLights();

				for( FLightComponent* Light=Level->FirstLight; Light; Light=Light->NextLight )
				{
					if( Canvas->View.Bounds.isOverlap( Light->GetLightRect() ) )
						Canvas->FluShader.AddLight
						( 
							Light, 
							Light->Base->Location, 
							Light->Base->Rotation 
						);
				}
			}

			/*
			// Highlight portal.
			if( !Level->bIsPlaying )
				Canvas->FluShader.SetPostEffect(GMirrorEffect);
			*/

			// Render level.
			for( Int32 i=0; i<Level->RenderObjects.size(); i++ )			
				Level->RenderObjects[i]->Render( Canvas );

			// Render overlay lightmap.
			if( Level->RndFlags & RND_Lighting )
				Canvas->RenderLightmap();
		}
		Canvas->PopTransform();
	}
}


//
// Draw all half-planes from all visible 
// warps.
//
void drawHalfPlaneWarp( COpenGLCanvas* Canvas, FLevel* Level, const TViewInfo& Parent )
{
	math::Rect Observer	= Parent.Bounds;

	for( FPortalComponent* Portal=Level->FirstPortal; Portal; Portal=Portal->NextPortal )
	{
		if( !Portal->IsA(FWarpComponent::MetaClass) )
			continue;

		FWarpComponent* Warp	= (FWarpComponent*)Portal;
		if( !Warp->Other )
			continue;

		Float HalfWidth = Portal->Width * 0.5f;

		// Fast X reject.
		if	( 
				Portal->Location.x+HalfWidth < Observer.min.x ||
				Portal->Location.x-HalfWidth > Observer.max.x	
			)
			continue;

		// Fast Y reject.
		if	(
				Portal->Location.y+HalfWidth < Observer.min.y ||
				Portal->Location.y-HalfWidth > Observer.max.y
			)
			continue;

		// Compute warp bounding volume.
		math::Coords TestToWorld = Warp->ToWorld();
		math::Vector V[2];
		V[0] = math::transformPointBy( math::Vector(0.f, +HalfWidth), TestToWorld );
		V[1] = math::transformPointBy( math::Vector(0.f, -HalfWidth), TestToWorld );
		if( !Observer.isOverlap( math::Rect( V, 2.f ) ) )
			continue;

		// Yes, warp is visible, render objects.
		TViewInfo WarpView;
		Warp->ComputeViewInfo( Parent, WarpView );

		Canvas->PushTransform( WarpView );
		{
			// Collect mirror's light sources.
			if( Level->RndFlags & RND_Lighting )
			{
				Canvas->FluShader.ResetLights();
				
				for( FLightComponent* Light=Level->FirstLight; Light; Light=Light->NextLight )
				{
					//TVector				LightPos	= Warp->TransferPoint(Light->Base->Location);

					if( Canvas->View.Bounds.isOverlap( Light->GetLightRect() ) )
						Canvas->FluShader.AddLight
						( 
							Light, 
							Light->Base->Location, 
							Light->Base->Rotation 
						);
				}
			}

			/*
			// Highlight portal.
			if( !Level->bIsPlaying )
				Canvas->FluShader.SetPostEffect(GWarpEffect);
			*/

			// Render level.
			for( Int32 i=0; i<Level->RenderObjects.size(); i++ )			
				Level->RenderObjects[i]->Render( Canvas );

			// Render overlay lightmap.
			if( Level->RndFlags & RND_Lighting )
				Canvas->RenderLightmap();
		}
		Canvas->PopTransform();
	}
}


//
// Render objects comparison.
//
Bool RenderObjectCmp( FComponent*const &A, FComponent*const &B )
{
	return A->GetLayer() < B->GetLayer();
}


//
// Render a full-screen quad, for post effect.
//
void drawFullScreenRect()
{
	glBegin( GL_QUADS );
	{
		glTexCoord2f( 0.f, 0.f ); glVertex2f( -1.f, -1.f );
		glTexCoord2f( 0.f, 1.f ); glVertex2f( -1.f, +1.f );
		glTexCoord2f( 1.f, 1.f ); glVertex2f( +1.f, +1.f );
		glTexCoord2f( 1.f, 0.f ); glVertex2f( +1.f, -1.f );
	}
	glEnd();
}


//
// Render entire level!
//
void COpenGLRender::RenderLevel( CCanvas* InCanvas, FLevel* Level, Int32 X, Int32 Y, Int32 W, Int32 H )
{
	Canvas->MasterFBO->Bind( Canvas->ScreenWidth, Canvas->ScreenHeight );
	glClear(GL_COLOR_BUFFER_BIT);

	// Check pointers.
	assert(Level);
	assert(InCanvas == this->Canvas);

	// Sort render objects according to it layer.
	if( GFrameStamp & 31 )
		Level->RenderObjects.sort(RenderObjectCmp);
	
	// Update shader time.
	Canvas->EnableShader( &Canvas->FluShader );
	Canvas->FluShader.SetModeComplex();
	Canvas->FluShader.SetValue1f( Canvas->FluShader.idGameTime, Canvas->LockTime );
	Canvas->FluShader.SetAmbientLight( math::colors::BLACK );

	// Clamp level scrolling when we play.
	if( Level->bIsPlaying )
	{
		TCamera& Camera = Level->Camera;

		Camera.Location.x	= clamp
		(
			Camera.Location.x,
			Camera.ScrollBound.min.x + Camera.FOV.x*0.5f,
			Camera.ScrollBound.max.x - Camera.FOV.x*0.5f
		);

		Camera.Location.y	= clamp
		(
			Camera.Location.y,
			Camera.ScrollBound.min.y + Camera.FOV.y*0.5f,
			Camera.ScrollBound.max.y - Camera.FOV.y*0.5f
		);
	}

	// Compute master view.
	TViewInfo MasterView	= TViewInfo
	(
		Level->Camera.Location,
		Level->Camera.Rotation,
		Level->Camera.GetFitFOV( W, H ),
		Level->Camera.Zoom,
		false,
		X,
		Y, 
		W, 
		H
	);

	// Setup clipping area. In game only.
	if( Level->bIsPlaying )
	{
		math::Vector RealFOV	= MasterView.FOV;
		math::Vector CamFOV		= Level->Camera.FOV;

		Canvas->SetClip
		(
			TClipArea
			(
				X,
				Y+H*((RealFOV.y-CamFOV.y)/2.f)/RealFOV.y,
				W,
				H*(CamFOV.y/RealFOV.y)
			)
		);
	}

	// Draw scene.
	Canvas->PushTransform( MasterView );
	{

		// Set ambient light in level.
		if( Level->RndFlags & RND_Lighting )
			//Canvas->FluShader.SetAmbientLight(Level->AmbientLight);
			Canvas->FluShader.SetAmbientLight( Level->m_ambientColors.SampleLinearAt( Level->m_timeOfDay.toPercent(), math::colors::BLACK ) );

		/*
		// Render sky zone if any.
		if( Level->Sky && (Level->RndFlags & RND_Backdrop) )
			drawSkyZone( Canvas, Level, MasterView );
			*/

		// Temporary render sky
		if( Level->RndFlags & RND_Backdrop )
		{
			Canvas->PushTransform( TViewInfo( math::Vector(0, 0), 0, math::Vector(1, 1), 1, true, X,
				Y, 
				W, 
				H ) );

			TRenderRect rect;
			rect.Bounds = math::Rect( math::Vector(0.f, 0.f), 1.f );

			rect.Flags = POLY_Unlit | POLY_AlphaGhost;
			rect.TexCoords.min = math::Vector( 1, 1 );
			rect.TexCoords.max = math::Vector( 0, 0 );
			rect.Texture = Level->m_duskBitmap;


			Float progress = Level->m_timeOfDay.toPercent();
			FBitmap* first = nullptr;
			FBitmap* second = nullptr;
			Float alpha = 0.5f;

			Float midnightTime = envi::TimeOfDay( 0, 0 ).toPercent();
			Float dawnTime = envi::TimeOfDay( 8, 0 ).toPercent();
			Float noonTime = envi::TimeOfDay( 15, 0 ).toPercent();
			Float duskTime = envi::TimeOfDay( 21, 0 ).toPercent();

			if( inRange( progress, midnightTime, dawnTime ) )
			{
				first = Level->m_midnightBitmap;
				second = Level->m_dawnBitmap;
				alpha = (dawnTime - progress) / (dawnTime - midnightTime);
			}
			else if ( inRange( progress, dawnTime, noonTime ) )
			{
				first = Level->m_dawnBitmap;
				second = Level->m_noonBitmap;
				alpha = (noonTime - progress) / (noonTime - dawnTime);
			}
			else if( inRange( progress, noonTime, duskTime ) )
			{
				first = Level->m_noonBitmap;
				second = Level->m_duskBitmap;
				alpha = (duskTime - progress) / (duskTime - noonTime);
			}
			else 
			{
				first = Level->m_duskBitmap;
				second = Level->m_midnightBitmap;
				alpha = (1.f - progress) / (1.f - duskTime);
			}


			rect.Texture = first;
			rect.Color = math::Color( 255, 255, 255, 255 );
			Canvas->DrawRect( rect );

			rect.Texture = second;
			rect.Color = math::Color( 255, 255, 255, 255 * (1-alpha) );
			Canvas->DrawRect( rect );



			


			Canvas->PopTransform();
		}


		// Draw editor grid.
		if( Level->RndFlags & RND_Grid )
			drawGrid( Canvas );	



		// Handle all portals.
		if( Level->RndFlags & RND_Portals )
		{
			drawHalfPlaneMirror( Canvas, Level, MasterView );
			drawHalfPlaneWarp( Canvas, Level, MasterView );
		}

		// Reset lights. And prepare for their collection.
		if( Level->RndFlags & RND_Lighting )
			Canvas->FluShader.ResetLights();

		// Collect all light sources from master view.
		if( Level->RndFlags & RND_Lighting )
			for( FLightComponent* Light=Level->FirstLight; Light; Light=Light->NextLight )
			{
				if( Canvas->View.Bounds.isOverlap( Light->GetLightRect() ) )
					Canvas->FluShader.AddLight
					(
						Light,
						Light->Base->Location,
						Light->Base->Rotation
					);
			}

		// Render all objects in master view.
		for( Int32 i=0; i<Level->RenderObjects.size(); i++ )
			Level->RenderObjects[i]->Render( Canvas );

		// Draw addition-lighting lightmap.
		if( Level->RndFlags & RND_Lighting )
			Canvas->RenderLightmap();

		// Draw debug stuff.
		// !!todo: add special flag for level.
		CDebugDrawHelper::Instance().Render(Canvas);

		// Draw safe frame area.
		if( !Level->bIsPlaying )
			drawSafeFrame( Canvas, Level->Camera );
	}
	Canvas->PopTransform();

	// Render HUD in-game only.
	if( Level->bIsPlaying && (Level->RndFlags & RND_HUD) )
	{
		// Set screen coords.
		Canvas->PushTransform
		(
			TViewInfo
			( 
				Canvas->Clip.X, 
				Canvas->Clip.Y, 
				Canvas->Clip.Width, 
				Canvas->Clip.Height 
			)
		); 
		{
			// Draw each element.
			for( FPainterComponent* P=Level->FirstPainter; P; P=P->NextPainter )
				P->RenderHUD( Canvas );
		}
		Canvas->PopTransform();
	}

	// post-processing
	{
		profile_zone( EProfilerGroup::Render, PostFX );

		// Turn off scene rendering stuff.
		Canvas->SetClip( CLIP_NONE );
		Canvas->SetBlend( BLEND_Regular );
		glPushMatrix();
		glLoadIdentity();

		//Canvas->MasterFBO->Unbind();

		// blur stage
		if( Level->BlurIntensity > math::EPSILON && Level->RndFlags & RND_Effects )
		{
			// apply horiz blur
			Canvas->BlurFBO->Bind( Canvas->ScreenWidth, Canvas->ScreenHeight );
			Canvas->HorizBlurShader.SetTargetWidth( Canvas->ScreenWidth );
			Canvas->HorizBlurShader.SetIntensity( Level->BlurIntensity );

			glBindTexture( GL_TEXTURE_2D, Canvas->MasterFBO->GetTextureId() );

			Canvas->EnableShader(&Canvas->HorizBlurShader);
			{
				drawFullScreenRect();
			}
			Canvas->DisableShader();

			// apply vert blur
			Canvas->MasterFBO->Bind( Canvas->ScreenWidth, Canvas->ScreenHeight );
			Canvas->VertBlurShader.SetTargetHeight( Canvas->ScreenHeight );
			Canvas->VertBlurShader.SetIntensity( Level->BlurIntensity );

			glBindTexture( GL_TEXTURE_2D, Canvas->BlurFBO->GetTextureId() );

			Canvas->EnableShader(&Canvas->VertBlurShader);
			{
				drawFullScreenRect();
			}
			Canvas->DisableShader();
		}

		// color correction stage (final)
		Canvas->MasterFBO->Unbind();

		// Select master scene effect.
		if( !(Level->RndFlags & RND_Effects) )
		{
			// Post-effects are turned off.
			Canvas->FinalShader.SetPostEffect(GNullEffect);
		}
		else if( Level->bIsPlaying )
		{
			// Set effect in game, used interpolator.
			Canvas->FinalShader.SetPostEffect(Level->GFXManager->GetResult());
		}
		else
		{
			// Default for editor
			Canvas->FinalShader.SetPostEffect(Level->Effect);
		}

		Canvas->FinalShader.setAberrationIntensity( Level->AberrationIntensity );
		Canvas->FinalShader.setVignette( Level->m_vignette );
		Canvas->FinalShader.setEnableFXAA( Level->m_enableFXAA );
		Canvas->FinalShader.setRTSize( Canvas->ScreenWidth, Canvas->ScreenHeight );

		// Render FBO to screen.
		glBindTexture( GL_TEXTURE_2D, Canvas->MasterFBO->GetTextureId() );
		Canvas->OldBitmap = nullptr;

		Canvas->EnableShader(&Canvas->FinalShader);
		{
			drawFullScreenRect();
		}
		Canvas->DisableShader();

		glPopMatrix();
	}
}


/*-----------------------------------------------------------------------------
	CGLShaderBase implementation.
-----------------------------------------------------------------------------*/

//
// GL shader constructor.
//
CGLShaderBase::CGLShaderBase()
	:	iglProgram( 0 ),
		iglVertShader( 0 ),
		iglFragShader( 0 ),
		Uniforms(),
		iCommitFirst( -1 ),
		bEnabled( false ),
		Name( L"" )
{
}


//
// GL shader destructor.
//
CGLShaderBase::~CGLShaderBase()
{
	glUseProgram( 0 );
	if( iglVertShader )	glDeleteShader( iglVertShader );
	if( iglFragShader )	glDeleteShader( iglFragShader );
	if( iglProgram )	glDeleteProgram( iglProgram );
}


//
// Load shader source from the file. Please 'free'
// returned value after use. I'll use C style here.
//
GLchar* LoadShaderCode( String InFileName )
{
	FILE* file;
	_wfopen_s( &file, *InFileName, L"r" );

	if( !file )
		return 0;

	// Figure out file size.
	fseek( file, 0, SEEK_END );
	int file_size = ftell(file);	
	rewind(file);

	// Allocate buffer and load file.
	GLchar* buffer = (GLchar*)calloc( file_size+1, 1 );
	fread( buffer, 1, file_size, file );

	// Return it.
	return buffer;
}


//
// Compile vertex or fragment shader.
//
bool CompileShader( String FileName, GLenum ShaderType, GLuint& iShader )
{
	// Load file.
	GLchar* Text	= LoadShaderCode( FileName );
	assert(Text);

	// Compile shader.
	iShader = glCreateShader( ShaderType );
	glShaderSource( iShader, 1, (const GLchar**)&Text, nullptr );
	glCompileShader( iShader );
	free(Text);

	// Test it.
	GLint	Success;
	glGetShaderiv( iShader, GL_COMPILE_STATUS, &Success );

	if( Success != GL_TRUE )
	{
		GLchar	cError[2048];

		glGetShaderInfoLog
		(
			iShader,
			arraySize(cError),
			nullptr,
			cError
		);

		fatal( L"Failed compile shader '%s' with message: %hs", *FileName, cError );
		return false;
	}

	return true;
}


//
// Shader loading and initialization.
//
bool CGLShaderBase::Init( String ShaderName )
{
	// Real shader filename.
	String VertShaderFile	= fm::getCurrentDirectory() + SHADER_DIR + ShaderName + VERT_SHADER_EXT;
	String FragShaderFile	= fm::getCurrentDirectory() + SHADER_DIR + ShaderName + FRAG_SHADER_EXT;

	// Test files.
	if( !fm::fileExists( *VertShaderFile ) )
		fatal( L"Vertex shader '%s' not found", *VertShaderFile );
	if( !fm::fileExists( *FragShaderFile ) )
		fatal( L"Fragment shader '%s' not found", *FragShaderFile );

	// Compile shaders.
	if( !CompileShader( VertShaderFile, GL_VERTEX_SHADER, iglVertShader ) )
		return false;

	if( !CompileShader( FragShaderFile, GL_FRAGMENT_SHADER, iglFragShader ) )
		return false;

	// Link program.
	iglProgram	= glCreateProgram();
	
	if( iglProgram == 0 )
	{
		fatal( L"Failed link shader '%s'", *ShaderName );
		return false;
	}

	glAttachShader( iglProgram, iglVertShader );
	glAttachShader( iglProgram, iglFragShader );
	glLinkProgram( iglProgram );
	glUseProgram( iglProgram );

	bEnabled = true;
	Name = ShaderName;
	info( L"Shader '%s' successfully loaded", *ShaderName );

	return true;
}


//
// Register a new uniform variable.
//
Int32 CGLShaderBase::RegisterUniform( AnsiChar* Name )
{
	assert(bEnabled);

	TUniform Uniform;

	Uniform.bDirty = false;
	Uniform.Dimension = 1;
	Uniform.iCommitNext = -1;
	Uniform.iUniform = glGetUniformLocation( iglProgram, Name );

	if( Uniform.iUniform == -1 )
		fatal( L"Uniform variable '%hs' not found in shader '%s'", Name, *this->Name );
	
	return Uniforms.push(Uniform);
}


//
// Update Shader uniform variable.
//
void CGLShaderBase::SetValue( Int32 iUniform, UInt32 Dimension, const void* Value )
{
	// Check if value changed.
	if( Dimension != 0 )
	{
		if( mem::cmp( Value, Uniforms[iUniform].FloatValue, Dimension*sizeof(Float) ) )
			return;
	}
	else
	{
		if( *(Int32*)Value == Uniforms[iUniform].IntValue[0] )
			return;
	}

	if( bEnabled )
	{
		// Shader enabled, so change value immediately.
		TUniform& Uniform = Uniforms[iUniform];

		switch( Dimension )
		{
			case 0:
				glUniform1i( Uniform.iUniform, *(Int32*)Value );
				break;

			case 1:
				glUniform1f( Uniform.iUniform, *(Float*)Value );
				break;

			case 2:
				glUniform2fv( Uniform.iUniform, 1, (Float*)Value );
				break;

			case 3:
				glUniform3fv( Uniform.iUniform, 1, (Float*)Value );
				break;

			case 4:
				glUniform4fv( Uniform.iUniform, 1, (Float*)Value );
				break;

			default:
				fatal( L"Unexpected uniform variable dimension in shader '%s'", *Name );
				break;
		}

		mem::copy( Uniform.FloatValue, Value, Dimension!=0 ? Dimension*sizeof(Float) : sizeof(Int32) );
	}
	else
	{
		// Shader disabled, so wait for activation.
		TUniform& Uniform = Uniforms[iUniform];

		mem::copy( Uniform.FloatValue, Value, Dimension!=0 ? Dimension*sizeof(Float) : sizeof(Int32) );
		Uniform.Dimension = Dimension;

		if( !Uniform.bDirty )
		{
			Uniform.iCommitNext = iCommitFirst;
			iCommitFirst = iUniform;
			Uniform.bDirty = true;
		}
	}
}


//
// Send all changed variables to shader.
//
void CGLShaderBase::CommitValues()
{
	assert(bEnabled);
	assert(iglProgram != 0);

	for( Int32 i = iCommitFirst; i != -1; )
	{
		TUniform& Uniform = Uniforms[i];

		switch( Uniform.Dimension )
		{
			case 0:
				glUniform1i( Uniform.iUniform, Uniform.IntValue[0] );
				break;

			case 1:
				glUniform1f( Uniform.iUniform, Uniform.FloatValue[0] );
				break;

			case 2:
				glUniform2fv( Uniform.iUniform, 1, Uniform.FloatValue );
				break;

			case 3:
				glUniform3fv( Uniform.iUniform, 1, Uniform.FloatValue );
				break;

			case 4:
				glUniform4fv( Uniform.iUniform, 1, Uniform.FloatValue );
				break;

			default:
				fatal( L"Unexpected uniform variable dimension in shader '%s'", *Name );
				break;
		}

		i = Uniform.iCommitNext;
		Uniform.iCommitNext = -1;
		Uniform.bDirty = false;
	}

	iCommitFirst = -1;
}


/*-----------------------------------------------------------------------------
	CGLFinalShader implementation.
-----------------------------------------------------------------------------*/

//
// Final shader constructor.
//
CGLFinalShader::CGLFinalShader()
	:	CGLShaderBase()
{
}


//
// Final shader destructor.
//
CGLFinalShader::~CGLFinalShader()
{
}


//
// Shader initialization.
//
bool CGLFinalShader::Init( String ShaderName ) 
{
	if( !CGLShaderBase::Init(ShaderName) )
		return false;

	idHighlights	= RegisterUniform( "highlights" );
	idMidTones		= RegisterUniform( "midTones" );
	idShadows		= RegisterUniform( "shadows" );
	idBWScale		= RegisterUniform( "bwScale" );

	idAberrationIntensity = RegisterUniform( "aberrationIntensity" );

	idVignette = RegisterUniform( "m_vignette" );
	idEnableFXAA = RegisterUniform( "m_enableFXAA" );
	idRTSize = RegisterUniform( "m_RTSize" );

	// Only one bitmap supported.
	idTexture = RegisterUniform( "texture" );
	SetValue1i( idTexture, 0 );

	glUseProgram( 0 );
	bEnabled = false;

	// Notify.
	info( L"Rend: GLFluShader initialized" );
	return true;
}


/*-----------------------------------------------------------------------------
	CGLHorizBlurShader implementation.
-----------------------------------------------------------------------------*/

//
// Shader constructor.
//
CGLHorizBlurShader::CGLHorizBlurShader()
	:	CGLShaderBase()
{
}


//
// Shader destructor.
//
CGLHorizBlurShader::~CGLHorizBlurShader()
{
}


//
// Shader initialization.
//
bool CGLHorizBlurShader::Init( String ShaderName )
{
	if( !CGLShaderBase::Init(ShaderName) )
		return false;

	// Only one bitmap supported.
	idTargetWidth = RegisterUniform( "targetWidth" );
	idIntensity = RegisterUniform( "intensity" );
	idTexture = RegisterUniform( "texture" );
	SetValue1i( idTexture, 0 );

	glUseProgram( 0 );
	bEnabled = false;

	// Notify.
	debug( L"Rend: CGLHorizBlurShader initialized" );
	return true;
}


/*-----------------------------------------------------------------------------
	CGLVertBlurShader implementation.
-----------------------------------------------------------------------------*/

//
// Shader constructor.
//
CGLVertBlurShader::CGLVertBlurShader()
	:	CGLShaderBase()
{
}


//
// Shader destructor.
//
CGLVertBlurShader::~CGLVertBlurShader()
{
}


//
// Shader initialization.
//
bool CGLVertBlurShader::Init( String ShaderName )
{
	if( !CGLShaderBase::Init(ShaderName) )
		return false;

	// Only one bitmap supported.
	idTargetHeight = RegisterUniform( "targetHeight" );
	idIntensity = RegisterUniform( "intensity" );
	idTexture = RegisterUniform( "texture" );
	SetValue1i( idTexture, 0 );

	glUseProgram( 0 );
	bEnabled = false;

	// Notify.
	debug( L"Rend: CGLVertBlurShader initialized" );
	return true;
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/