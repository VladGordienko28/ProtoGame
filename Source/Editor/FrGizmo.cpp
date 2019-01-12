/*=============================================================================
	FrGizmo.cpp: Gizmo tool.
	Created by Vlad Gordienko, Dec. 2017.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
	CGizmo implementation.
-----------------------------------------------------------------------------*/

//
// Gizmo constants.
//
#define GIZMO_RADIUS		4.f
#define GIZMO_ARROW_LEN		6.f
#define GIZMO_RECT_SIZE		2.0f


//
// Gizmo constructor.
//
CGizmo::CGizmo( Float InSize )
	:	Mode( GIZMO_Translate ),
		CurrentAxis( GIAX_None ),
		Size( InSize ),
		Location( 0.f, 0.f ),
		Rotation( 0 ),
		Scale( 1.f, 1.f )
{
}


//
// Gizmo destructor.
//
CGizmo::~CGizmo()
{
}


//
// Rest gizmo.
//
void CGizmo::Reset()
{
	Location	= TVector( 0.f, 0.f );
	Rotation	= 0;
	Scale		= TVector( 1.f, 1.f );
}


//
// Gizmo setters.
//
void CGizmo::SetLocation( const TVector& NewLoc )
{
	Location	= NewLoc;
}
void CGizmo::SetRotation( TAngle NewAng )
{
	Rotation	= NewAng;
}
void CGizmo::SetMode( EGizmoMode NewMode )
{
	Mode		= NewMode;
}
void CGizmo::SetAxis( EGizmoAxis Selected )
{
	CurrentAxis	= Selected;
}
void CGizmo::Move( const TVector& DeltaMove )
{
	Location	+= DeltaMove;
}
void CGizmo::Rotate( TAngle DeltaRot )
{
	Rotation	+= DeltaRot;
}


//
// Perform mouse movement to 2ds transformations.
//
void CGizmo::Perform
(
	const TViewInfo& View,
	const TVector& CursorPos,
	const TVector& MovementDelta,
	TVector* OutTranslation,
	TAngle* OutRotation,
	TVector* OutScale
)
{
	// Don't do anything if no axis.
	if( CurrentAxis == GIAX_None )
		return;

	TCoords ToLocal( Location, Rotation );

	// According to gizmo mode.
	switch( Mode )
	{
		case GIZMO_Translate:
		{
			//
			// Perform translation.
			//
			TVector StoredLocation = Location;

			if( CurrentAxis == GIAX_X )
			{
				Location += ToLocal.XAxis * (ToLocal.XAxis * MovementDelta);
			}
			else if( CurrentAxis == GIAX_Y )
			{
				Location += ToLocal.YAxis * (ToLocal.YAxis * MovementDelta);
			}
			else
			{
				Location += MovementDelta;
			}

			if( OutTranslation ) 
				*OutTranslation = Location - StoredLocation;
			break;
		}
		case GIZMO_Rotate:
		{
			//
			// Perform rotation.
			//
			TAngle StoredRotation = Rotation;
			TVector WorldCursor = View.Deproject(CursorPos.X, CursorPos.Y);
			TVector Normal = WorldCursor - Location;
			Normal.Normalize();
			TVector Tangent = Normal.Cross();

			Rotation += (Tangent * MovementDelta) * 0.1f;

			if( OutRotation )
				*OutRotation = Rotation - StoredRotation;
			break;
		}
		case GIZMO_Scale:
		{
			//
			// Perform scale.
			//
			TVector StoredScale = Scale;

			if( CurrentAxis == GIAX_X )
			{
				Float FactorX = (ToLocal.XAxis * MovementDelta)*0.15f / View.Zoom;
				Scale.X = Clamp( Scale.X+FactorX, 0.01f, 100.f );
			}
			else if( CurrentAxis == GIAX_Y )
			{
				Float FactorY = (ToLocal.YAxis * MovementDelta)*0.15f / View.Zoom;
				Scale.Y = Clamp( Scale.Y+FactorY, 0.01f, 100.f );
			}
			else
			{
				Float Factor = (MovementDelta * ((ToLocal.XAxis+ToLocal.YAxis)*0.5f))*0.15f / View.Zoom;
				Scale.X = Clamp( Scale.X+Factor, 0.01f, 100.f );
				Scale.Y = Clamp( Scale.Y+Factor, 0.01f, 100.f );
			}

			if( OutScale )
			{
				OutScale->X = Scale.X / StoredScale.X;
				OutScale->Y = Scale.Y / StoredScale.Y;
			}
			break;
		}
	}
}


/*-----------------------------------------------------------------------------
	Gizmo hitting.
-----------------------------------------------------------------------------*/

//
// Test hit with translation gizmo.
//
static EGizmoAxis HitTranslationGizmo( const TViewInfo& View, Float Size, Int32 Cx, Int32 Cy, const TCoords& ToLocal, const TVector& Scale )
{
	// Project to screen space.
	TVector Center, XEnd, YEnd;
	View.Project( ToLocal.Origin, Center.X, Center.Y );
	View.Project( ToLocal.Origin + ToLocal.XAxis*((GIZMO_ARROW_LEN+0.93f)*View.Zoom*Size), XEnd.X, XEnd.Y );
	View.Project( ToLocal.Origin + ToLocal.YAxis*((GIZMO_ARROW_LEN+0.93f)*View.Zoom*Size), YEnd.X, YEnd.Y );

	if( PointOnSegment( TVector(Cx, Cy), Center, XEnd, 3.f ) )
	{
		// X hit.
		return GIAX_X;
	}
	else if( PointOnSegment( TVector(Cx, Cy), Center, YEnd, 3.f ) )
	{
		// Y hit.
		return GIAX_Y;
	}
	else
	{
		// Everything or nothing.
		TVector Tmp = XEnd - Center;
		Tmp.Normalize();
		TCoords RectLocal( Center, Tmp );

		TVector TestPoint = TransformPointBy( TVector(Cx, Cy), RectLocal );
		TestPoint.Y = -TestPoint.Y;
		Float RectSize = GIZMO_RECT_SIZE * (XEnd-Center).Size()/(GIZMO_ARROW_LEN+0.93f);

		TRect R;
		R.Min = TVector( 0.f, 0.f );
		R.Max = TVector( RectSize, RectSize );
		return R.IsInside(TestPoint) ? GIAX_Both : GIAX_None;
	}
}


//
// Test hit with rotation gizmo.
//
static EGizmoAxis HitRotationGizmo( const TViewInfo& View, Float Size, Int32 Cx, Int32 Cy, const TCoords& ToLocal, const TVector& Scale )
{
	TVector Center, CirclePoint;
	View.Project( ToLocal.Origin, Center.X, Center.Y );
	View.Project( ToLocal.Origin + (TVector(0.f, GIZMO_RADIUS)*View.Zoom*Size), CirclePoint.X, CirclePoint.Y );

	Float RealRadius = Distance( Center, CirclePoint );
	Float TestRadius = Distance( Center, TVector(Cx, Cy) );

	// 3px threshold.
	return (TestRadius>=RealRadius-1.5f && TestRadius<=RealRadius+1.5f) ? GIAX_Both : GIAX_None;
}


//
// Test hit with rotation gizmo.
//
static EGizmoAxis HitScaleGizmo( const TViewInfo& View, Float Size, Int32 Cx, Int32 Cy, const TCoords& ToLocal, const TVector& Scale )
{
	TVector Center, XEnd, YEnd;
	View.Project( ToLocal.Origin, Center.X, Center.Y );
	View.Project( ToLocal.Origin + ToLocal.XAxis*((GIZMO_ARROW_LEN+0.5f)*View.Zoom*Size), XEnd.X, XEnd.Y );
	View.Project( ToLocal.Origin + ToLocal.YAxis*((GIZMO_ARROW_LEN+0.5f)*View.Zoom*Size), YEnd.X, YEnd.Y );

	if( PointOnSegment(TVector(Cx, Cy), Center, XEnd, 3.f) )
	{
		// X stick hit.
		return GIAX_X;
	}
	else if( PointOnSegment(TVector(Cx, Cy), Center, YEnd, 3.f) )
	{
		// Y stick hit.
		return GIAX_Y;
	}
	else 
	{
		// Everything or nothing.
		TVector Tmp = XEnd - Center;
		Tmp.Normalize();
		TCoords TriLocal( Center, Tmp );

		TVector Point = TransformPointBy( TVector(Cx, Cy), TriLocal );
		Point.Y = -Point.Y;
		Float Side = GIZMO_RECT_SIZE * ((XEnd-Center).Size()/(GIZMO_ARROW_LEN+0.5f));

		return Point.X>=0 && Point.Y>=0 && Abs(Point.X+Point.Y)<=Side ? GIAX_Both : GIAX_None;
	}
}


//
// Return gizmo's axis at specified cursor location.
//
EGizmoAxis CGizmo::AxisAt( const TViewInfo& ViewInfo, Int32 Cx, Int32 Cy )
{
	static EGizmoAxis(*GizmoHitTable[GIZMO_MAX])( const TViewInfo&, Float, Int32, Int32, const TCoords&, const TVector& ) = 
	{
		HitTranslationGizmo,
		HitRotationGizmo,
		HitScaleGizmo
	};

	// Test hit.
	return GizmoHitTable[Mode](ViewInfo, Size, Cx, Cy, TCoords(Location, Rotation), Scale );
}


/*-----------------------------------------------------------------------------
	Gizmo rendering.
-----------------------------------------------------------------------------*/

//
// Gizmo colors.
//
#define GIZMO_X_COLOR		COLOR_Blue
#define GIZMO_Y_COLOR		COLOR_Red
#define GIZMO_BOTH_COLOR	COLOR_Gold


//
// Draw a gizmo arrow.
//
static void DrawArrow( CCanvas* Canvas, const TVector& From, const TVector& Dir, Float Length, Float Size, TColor Color )
{
	TVector End		= From + Dir * (Length * Canvas->View.Zoom);
	TCoords ToWorld	= TCoords( End, Dir ).Transpose();

	// Draw arrow tail.
	Canvas->DrawLine( From, End, Color, false );

	// Draw arrowhead.
	TRenderPoly Poly;
	Poly.Texture		= nullptr;
	Poly.Color			= Color;
	Poly.Flags			= POLY_FlatShade;
	Poly.NumVerts		= 3;

	Poly.Vertices[0]	= TransformPointBy( TVector( +0.0000f, -0.3125f )*Canvas->View.Zoom*Size, ToWorld );
	Poly.Vertices[1]	= TransformPointBy( TVector( +0.9375f, -0.0000f )*Canvas->View.Zoom*Size, ToWorld );
	Poly.Vertices[2]	= TransformPointBy( TVector( +0.0000f, +0.3125f )*Canvas->View.Zoom*Size, ToWorld );

	Canvas->DrawPoly(Poly);
}


//
// Draw a gizmo stick.
//
static void DrawStick( CCanvas* Canvas, const TVector& From, const TVector& Dir, Float Length, TColor Color )
{
	// Line.
	TVector End		= From + Dir * (Length * Canvas->View.Zoom);
	Canvas->DrawLine( From, End, Color, false );

	// Head.
	Canvas->DrawPoint( End, 6.f, Color );
}


//
// Draw translation gizmo.
//
static void DrawTranslationGizmo( CCanvas* Canvas, Float Size, EGizmoAxis Axis, const TCoords& ToLocal, const TVector& Scale )
{
	Float RectSize = GIZMO_RECT_SIZE * Canvas->View.Zoom * Size;

	// Draw 'both' semi-solid rect if selected.
	if( Axis == GIAX_Both )
	{
		TRenderRect R;
		R.Texture	= nullptr;
		R.Color		= GIZMO_BOTH_COLOR * 0.5f;
		R.Flags		= POLY_FlatShade | POLY_Ghost;
		R.Rotation	= VectorToAngle(ToLocal.XAxis);
		R.Bounds	= TRect( ToLocal.Origin + (ToLocal.XAxis+ToLocal.YAxis)*(RectSize*0.5f), RectSize );
		Canvas->DrawRect( R );
	}

	// Draw 'both' corner.
	TVector Opposite = ToLocal.Origin + (ToLocal.XAxis+ToLocal.YAxis)*RectSize;
	Canvas->DrawLine( ToLocal.Origin + ToLocal.XAxis*RectSize, Opposite, GIZMO_BOTH_COLOR, false );
	Canvas->DrawLine( ToLocal.Origin + ToLocal.YAxis*RectSize, Opposite, GIZMO_BOTH_COLOR, false );

	// And finally draw arrows.
	DrawArrow( Canvas, ToLocal.Origin, ToLocal.XAxis, GIZMO_ARROW_LEN*Size, Size, (Axis==GIAX_X || Axis==GIAX_Both) ? GIZMO_BOTH_COLOR : GIZMO_X_COLOR );
	DrawArrow( Canvas, ToLocal.Origin, ToLocal.YAxis, GIZMO_ARROW_LEN*Size, Size, (Axis==GIAX_Y || Axis==GIAX_Both) ? GIZMO_BOTH_COLOR : GIZMO_Y_COLOR );
}


//
// Draw rotation gizmo.
//
static void DrawRotationGizmo( CCanvas* Canvas, Float Size, EGizmoAxis Axis, const TCoords& ToLocal, const TVector& Scale )
{
	Canvas->DrawCircle
	(
		ToLocal.Origin,
		GIZMO_RADIUS * Canvas->View.Zoom * Size,
		Axis == AXIS_None ? GIZMO_X_COLOR : GIZMO_BOTH_COLOR,
		false
	);
	Canvas->DrawLineStar
	(
		ToLocal.Origin,
		VectorToAngle(ToLocal.XAxis),
		GIZMO_RADIUS * Canvas->View.Zoom * 0.75f * Size,
		GIZMO_BOTH_COLOR,
		false
	);
}


//
// Draw scale gizmo.
//
static void DrawScaleGizmo( CCanvas* Canvas, Float Size, EGizmoAxis Axis, const TCoords& ToLocal, const TVector& Scale )
{
	Float RectSizeX = GIZMO_RECT_SIZE * Canvas->View.Zoom * Scale.X * Size;
	Float RectSizeY = GIZMO_RECT_SIZE * Canvas->View.Zoom * Scale.Y * Size;

	// Draw semi-solid rect.
	if( Axis == GIAX_Both )
	{
		TRenderPoly P;
		P.Texture		= nullptr;
		P.Color			= GIZMO_BOTH_COLOR * 0.5f;
		P.Flags			= POLY_FlatShade | POLY_Ghost;
		P.NumVerts		= 3;
		P.Vertices[0]	= ToLocal.Origin;
		P.Vertices[1]	= ToLocal.Origin + ToLocal.XAxis*RectSizeX;
		P.Vertices[2]	= ToLocal.Origin + ToLocal.YAxis*RectSizeY;
		Canvas->DrawPoly(P);
	}

	// Draw triangle edge.
	Canvas->DrawLine
	(
		ToLocal.Origin + ToLocal.XAxis*RectSizeX,
		ToLocal.Origin + ToLocal.YAxis*RectSizeY,
		GIZMO_BOTH_COLOR,
		false
	);

	// Draw sticks-handles.
	DrawStick( Canvas, ToLocal.Origin, ToLocal.XAxis, GIZMO_ARROW_LEN*Scale.X*Size, (Axis==GIAX_X || Axis==GIAX_Both) ? GIZMO_BOTH_COLOR : GIZMO_X_COLOR );
	DrawStick( Canvas, ToLocal.Origin, ToLocal.YAxis, GIZMO_ARROW_LEN*Scale.Y*Size, (Axis==GIAX_Y || Axis==GIAX_Both) ? GIZMO_BOTH_COLOR : GIZMO_Y_COLOR );
}


//
// Render gizmp.
//
void CGizmo::Render( CCanvas* Canvas )
{
	static void(*GizmoRenderTable[GIZMO_MAX])( CCanvas*, Float, EGizmoAxis, const TCoords&, const TVector& ) = 
	{
		DrawTranslationGizmo,
		DrawRotationGizmo,
		DrawScaleGizmo
	};

	// Render it.
	GizmoRenderTable[Mode](Canvas, Size, CurrentAxis, TCoords(Location, Rotation), Scale );
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/