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
	Location	= { 0.f, 0.f };
	Rotation	= 0;
	Scale		= { 1.f, 1.f };
}


//
// Gizmo setters.
//
void CGizmo::SetLocation( const math::Vector& NewLoc )
{
	Location	= NewLoc;
}
void CGizmo::SetRotation( math::Angle NewAng )
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
void CGizmo::Move( const math::Vector& DeltaMove )
{
	Location	+= DeltaMove;
}
void CGizmo::Rotate( math::Angle DeltaRot )
{
	Rotation	+= DeltaRot;
}


//
// Perform mouse movement to 2ds transformations.
//
void CGizmo::Perform
(
	const gfx::ViewInfo& View,
	const math::Vector& CursorPos,
	const math::Vector& MovementDelta,
	math::Vector* OutTranslation,
	math::Angle* OutRotation,
	math::Vector* OutScale
)
{
	// Don't do anything if no axis.
	if( CurrentAxis == GIAX_None )
		return;

	math::Coords ToLocal( Location, Rotation );

	// According to gizmo mode.
	switch( Mode )
	{
		case GIZMO_Translate:
		{
			//
			// Perform translation.
			//
			math::Vector StoredLocation = Location;

			if( CurrentAxis == GIAX_X )
			{
				Location += ToLocal.xAxis * (ToLocal.xAxis * MovementDelta);
			}
			else if( CurrentAxis == GIAX_Y )
			{
				Location += ToLocal.yAxis * (ToLocal.yAxis * MovementDelta);
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
			math::Angle StoredRotation = Rotation;
			math::Vector WorldCursor = View.deproject(CursorPos.x, CursorPos.y);
			math::Vector Normal = WorldCursor - Location;
			Normal.normalize();
			math::Vector Tangent = Normal.cross();

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
			math::Vector StoredScale = Scale;

			if( CurrentAxis == GIAX_X )
			{
				Float FactorX = (ToLocal.xAxis * MovementDelta)*0.15f / View.zoom;
				Scale.x = clamp( Scale.x+FactorX, 0.01f, 100.f );
			}
			else if( CurrentAxis == GIAX_Y )
			{
				Float FactorY = (ToLocal.yAxis * MovementDelta)*0.15f / View.zoom;
				Scale.y = clamp( Scale.y+FactorY, 0.01f, 100.f );
			}
			else
			{
				Float Factor = (MovementDelta * ((ToLocal.xAxis+ToLocal.yAxis)*0.5f))*0.15f / View.zoom;
				Scale.x = clamp( Scale.x+Factor, 0.01f, 100.f );
				Scale.y = clamp( Scale.y+Factor, 0.01f, 100.f );
			}

			if( OutScale )
			{
				OutScale->x = Scale.x / StoredScale.x;
				OutScale->y = Scale.y / StoredScale.y;
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
static EGizmoAxis HitTranslationGizmo( const gfx::ViewInfo& View, Float Size, Int32 Cx, Int32 Cy, const math::Coords& ToLocal, const math::Vector& Scale )
{
	// Project to screen space.
	math::Vector Center, XEnd, YEnd;
	View.project( ToLocal.origin, Center.x, Center.y );
	View.project( ToLocal.origin + ToLocal.xAxis*((GIZMO_ARROW_LEN+0.93f)*View.zoom*Size), XEnd.x, XEnd.y );
	View.project( ToLocal.origin + ToLocal.yAxis*((GIZMO_ARROW_LEN+0.93f)*View.zoom*Size), YEnd.x, YEnd.y );

	if( math::isPointOnSegment( math::Vector(Cx, Cy), Center, XEnd, 3.f ) )
	{
		// X hit.
		return GIAX_X;
	}
	else if( math::isPointOnSegment( math::Vector(Cx, Cy), Center, YEnd, 3.f ) )
	{
		// Y hit.
		return GIAX_Y;
	}
	else
	{
		// Everything or nothing.
		math::Vector Tmp = XEnd - Center;
		Tmp.normalize();
		math::Coords RectLocal( Center, Tmp );

		math::Vector TestPoint = math::transformPointBy( math::Vector(Cx, Cy), RectLocal );
		TestPoint.y = -TestPoint.y;
		Float RectSize = GIZMO_RECT_SIZE * (XEnd-Center).size()/(GIZMO_ARROW_LEN+0.93f);

		math::Rect R;
		R.min = math::Vector( 0.f, 0.f );
		R.max = math::Vector( RectSize, RectSize );
		return R.isInside(TestPoint) ? GIAX_Both : GIAX_None;
	}
}


//
// Test hit with rotation gizmo.
//
static EGizmoAxis HitRotationGizmo( const gfx::ViewInfo& View, Float Size, Int32 Cx, Int32 Cy, const math::Coords& ToLocal, const math::Vector& Scale )
{
	math::Vector Center, CirclePoint;
	View.project( ToLocal.origin, Center.x, Center.y );
	View.project( ToLocal.origin + (math::Vector(0.f, GIZMO_RADIUS)*View.zoom*Size), CirclePoint.x, CirclePoint.y );

	Float RealRadius = math::distance( Center, CirclePoint );
	Float TestRadius = math::distance( Center, math::Vector(Cx, Cy) );

	// 3px threshold.
	return (TestRadius>=RealRadius-1.5f && TestRadius<=RealRadius+1.5f) ? GIAX_Both : GIAX_None;
}


//
// Test hit with rotation gizmo.
//
static EGizmoAxis HitScaleGizmo( const gfx::ViewInfo& View, Float Size, Int32 Cx, Int32 Cy, const math::Coords& ToLocal, const math::Vector& Scale )
{
	math::Vector Center, XEnd, YEnd;
	View.project( ToLocal.origin, Center.x, Center.y );
	View.project( ToLocal.origin + ToLocal.xAxis*((GIZMO_ARROW_LEN+0.5f)*View.zoom*Size), XEnd.x, XEnd.y );
	View.project( ToLocal.origin + ToLocal.yAxis*((GIZMO_ARROW_LEN+0.5f)*View.zoom*Size), YEnd.x, YEnd.y );

	if( math::isPointOnSegment(math::Vector(Cx, Cy), Center, XEnd, 3.f) )
	{
		// X stick hit.
		return GIAX_X;
	}
	else if( math::isPointOnSegment(math::Vector(Cx, Cy), Center, YEnd, 3.f) )
	{
		// Y stick hit.
		return GIAX_Y;
	}
	else 
	{
		// Everything or nothing.
		math::Vector Tmp = XEnd - Center;
		Tmp.normalize();
		math::Coords TriLocal( Center, Tmp );

		math::Vector Point = math::transformPointBy( math::Vector(Cx, Cy), TriLocal );
		Point.y = -Point.y;
		Float Side = GIZMO_RECT_SIZE * ((XEnd-Center).size()/(GIZMO_ARROW_LEN+0.5f));

		return Point.x>=0 && Point.y>=0 && abs(Point.x+Point.y)<=Side ? GIAX_Both : GIAX_None;
	}
}


//
// Return gizmo's axis at specified cursor location.
//
EGizmoAxis CGizmo::AxisAt( const gfx::ViewInfo& ViewInfo, Int32 Cx, Int32 Cy )
{
	static EGizmoAxis(*GizmoHitTable[GIZMO_MAX])( const gfx::ViewInfo&, Float, Int32, Int32, const math::Coords&, const math::Vector& ) = 
	{
		HitTranslationGizmo,
		HitRotationGizmo,
		HitScaleGizmo
	};

	// Test hit.
	return GizmoHitTable[Mode](ViewInfo, Size, Cx, Cy, math::Coords(Location, Rotation), Scale );
}


/*-----------------------------------------------------------------------------
	Gizmo rendering.
-----------------------------------------------------------------------------*/

//
// Gizmo colors.
//
#define GIZMO_X_COLOR		math::colors::BLUE
#define GIZMO_Y_COLOR		math::colors::RED
#define GIZMO_BOTH_COLOR	math::colors::GOLD


//
// Draw a gizmo arrow.
//
static void DrawArrow( CCanvas* Canvas, const math::Vector& From, const math::Vector& Dir, Float Length, Float Size, math::Color Color )
{
/*
	math::Vector End = From + Dir * (Length * Canvas->View.zoom);
	math::Coords ToWorld = math::Coords( End, Dir ).transpose();

	// Draw arrow tail.
	Canvas->DrawLine( From, End, Color, false );

	// Draw arrowhead.
	TRenderPoly Poly;
	Poly.Image			= INVALID_HANDLE<rend::Texture2DHandle>();
	Poly.Color			= Color;
	Poly.Flags			= POLY_FlatShade;
	Poly.NumVerts		= 3;

	Poly.Vertices[0]	= math::transformPointBy( math::Vector( +0.0000f, -0.3125f )*Canvas->View.zoom*Size, ToWorld );
	Poly.Vertices[1]	= math::transformPointBy( math::Vector( +0.9375f, -0.0000f )*Canvas->View.zoom*Size, ToWorld );
	Poly.Vertices[2]	= math::transformPointBy( math::Vector( +0.0000f, +0.3125f )*Canvas->View.zoom*Size, ToWorld );

	Canvas->DrawPoly(Poly);
*/
}


//
// Draw a gizmo stick.
//
static void DrawStick( CCanvas* Canvas, const math::Vector& From, const math::Vector& Dir, Float Length, math::Color Color )
{
/*
	// Line.
	math::Vector End = From + Dir * (Length * Canvas->View.zoom);
	Canvas->DrawLine( From, End, Color, false );

	// Head.
	Canvas->DrawPoint( End, 6.f, Color );
*/
}


//
// Draw translation gizmo.
//
static void DrawTranslationGizmo( CCanvas* Canvas, Float Size, EGizmoAxis Axis, const math::Coords& ToLocal, const math::Vector& Scale )
{
/*
	Float RectSize = GIZMO_RECT_SIZE * Canvas->View.zoom * Size;

	// Draw 'both' semi-solid rect if selected.
	if( Axis == GIAX_Both )
	{
		TRenderRect R;
		R.Image		= INVALID_HANDLE<rend::Texture2DHandle>();
		R.Color		= GIZMO_BOTH_COLOR * 0.5f;
		R.Flags		= POLY_FlatShade | POLY_Ghost;
		R.Rotation	= math::vectorToAngle(ToLocal.xAxis);
		R.Bounds	= math::Rect( ToLocal.origin + (ToLocal.xAxis+ToLocal.yAxis)*(RectSize*0.5f), RectSize );
		Canvas->DrawRect( R );
	}

	// Draw 'both' corner.
	math::Vector Opposite = ToLocal.origin + (ToLocal.xAxis+ToLocal.yAxis)*RectSize;
	Canvas->DrawLine( ToLocal.origin + ToLocal.xAxis*RectSize, Opposite, GIZMO_BOTH_COLOR, false );
	Canvas->DrawLine( ToLocal.origin + ToLocal.yAxis*RectSize, Opposite, GIZMO_BOTH_COLOR, false );

	// And finally draw arrows.
	DrawArrow( Canvas, ToLocal.origin, ToLocal.xAxis, GIZMO_ARROW_LEN*Size, Size, (Axis==GIAX_X || Axis==GIAX_Both) ? GIZMO_BOTH_COLOR : GIZMO_X_COLOR );
	DrawArrow( Canvas, ToLocal.origin, ToLocal.yAxis, GIZMO_ARROW_LEN*Size, Size, (Axis==GIAX_Y || Axis==GIAX_Both) ? GIZMO_BOTH_COLOR : GIZMO_Y_COLOR );
*/
}


//
// Draw rotation gizmo.
//
static void DrawRotationGizmo( CCanvas* Canvas, Float Size, EGizmoAxis Axis, const math::Coords& ToLocal, const math::Vector& Scale )
{
/*
	Canvas->DrawCircle
	(
		ToLocal.origin,
		GIZMO_RADIUS * Canvas->View.zoom * Size,
		Axis == AXIS_None ? GIZMO_X_COLOR : GIZMO_BOTH_COLOR,
		false
	);
	Canvas->DrawLineStar
	(
		ToLocal.origin,
		math::vectorToAngle(ToLocal.xAxis),
		GIZMO_RADIUS * Canvas->View.zoom * 0.75f * Size,
		GIZMO_BOTH_COLOR,
		false
	);
*/
}


//
// Draw scale gizmo.
//
static void DrawScaleGizmo( CCanvas* Canvas, Float Size, EGizmoAxis Axis, const math::Coords& ToLocal, const math::Vector& Scale )
{
/*
	Float RectSizeX = GIZMO_RECT_SIZE * Canvas->View.zoom * Scale.x * Size;
	Float RectSizeY = GIZMO_RECT_SIZE * Canvas->View.zoom * Scale.y * Size;

	// Draw semi-solid rect.
	if( Axis == GIAX_Both )
	{
		TRenderPoly P;
		P.Image			= INVALID_HANDLE<rend::Texture2DHandle>();
		P.Color			= GIZMO_BOTH_COLOR * 0.5f;
		P.Flags			= POLY_FlatShade | POLY_Ghost;
		P.NumVerts		= 3;
		P.Vertices[0]	= ToLocal.origin;
		P.Vertices[1]	= ToLocal.origin + ToLocal.xAxis*RectSizeX;
		P.Vertices[2]	= ToLocal.origin + ToLocal.yAxis*RectSizeY;
		Canvas->DrawPoly(P);
	}

	// Draw triangle edge.
	Canvas->DrawLine
	(
		ToLocal.origin + ToLocal.xAxis*RectSizeX,
		ToLocal.origin + ToLocal.yAxis*RectSizeY,
		GIZMO_BOTH_COLOR,
		false
	);

	// Draw sticks-handles.
	DrawStick( Canvas, ToLocal.origin, ToLocal.xAxis, GIZMO_ARROW_LEN*Scale.x*Size, (Axis==GIAX_X || Axis==GIAX_Both) ? GIZMO_BOTH_COLOR : GIZMO_X_COLOR );
	DrawStick( Canvas, ToLocal.origin, ToLocal.yAxis, GIZMO_ARROW_LEN*Scale.y*Size, (Axis==GIAX_Y || Axis==GIAX_Both) ? GIZMO_BOTH_COLOR : GIZMO_Y_COLOR );
*/
}


//
// Render gizmp.
//
void CGizmo::Render( CCanvas* Canvas )
{
	static void(*GizmoRenderTable[GIZMO_MAX])( CCanvas*, Float, EGizmoAxis, const math::Coords&, const math::Vector& ) = 
	{
		DrawTranslationGizmo,
		DrawRotationGizmo,
		DrawScaleGizmo
	};

	// Render it.
	GizmoRenderTable[Mode](Canvas, Size, CurrentAxis, math::Coords(Location, Rotation), Scale );
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/