/*=============================================================================
    FrThings.cpp: Various components implementation.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    FLightComponent implementation.
-----------------------------------------------------------------------------*/

//
// Light constructor.
//
FLightComponent::FLightComponent()
	:	FExtraComponent(),
		NextLight( nullptr )
{
	bRenderable	= true;

	bEnabled	= true;
	LightType	= LIGHT_Steady;
	LightFunc	= LF_Multiplicative;
	Radius		= 8.f;
	Brightness	= 1.f;
	Color		= math::colors::WHITE;
}


//
// Light destructor.
//
FLightComponent::~FLightComponent()
{
	// Remove from lights db.
	com_remove(Light);
}


//
// Initialize light for level.
//
void FLightComponent::InitForEntity( FEntity* InEntity )
{
	FExtraComponent::InitForEntity(InEntity);
	com_add(Light);
}


//
// When some field changed.
//
void FLightComponent::EditChange()
{
	FExtraComponent::EditChange();

	Radius		= clamp( Radius, 1.f, MAX_LIGHT_RADIUS );
	Brightness	= clamp( Brightness, 0.f, 10.f );
}


//
// Light serialization.
//
void FLightComponent::SerializeThis( CSerializer& S )
{
	FExtraComponent::SerializeThis( S );
	Serialize( S, Color );	
	Serialize( S, bEnabled );
	SerializeEnum( S, LightType );
	SerializeEnum( S, LightFunc );
	Serialize( S, Radius );
	Serialize( S, Brightness );
}


//
// Render light source.
//
void FLightComponent::Render( CCanvas* Canvas )
{
	if( Base->bSelected )
		Canvas->DrawCircle( Base->Location, Radius, math::colors::YELLOW, false, 48 );
}


//
// Return light map bounds.
//
math::Rect FLightComponent::GetLightRect()
{
	return math::Rect( Base->Location, Radius*2.f );
}


/*-----------------------------------------------------------------------------
    FSkyComponent implementation.
-----------------------------------------------------------------------------*/

//
// Sky constructor.
//
FSkyComponent::FSkyComponent()
	:	Parallax( 0.05f, 0.05f ),
		Extent( 8.f ),
		Offset( 0.f, 0.f ),
		RollSpeed( 0.f )
{
	Size			= math::Vector( 24.f, 16.f );
	bFixedAngle		= true;
	bHashable		= false;
}


//
// Sky initialization.
//
void FSkyComponent::InitForEntity( FEntity* InEntity )
{
	FZoneComponent::InitForEntity(InEntity);

	// If no sky, set self.
	if( !Level->Sky )
		Level->Sky	= this;
}


//
// Sky serialization.
//
void FSkyComponent::SerializeThis( CSerializer& S )
{
	FZoneComponent::SerializeThis(S);

	Serialize( S, Parallax );
	Serialize( S, Extent );
	Serialize( S, Offset );
	Serialize( S, RollSpeed );
}


//
// Validate sky parameters after edit in Object Inspector.
//
void FSkyComponent::EditChange()
{
	FZoneComponent::EditChange();
	Extent	= clamp( Extent, 1.f, Size.x * 0.5f );
}


//
// Render sky zone.
//
void FSkyComponent::Render( CCanvas* Canvas )
{
	// Never render self in self.
	if( Canvas->View.bMirage )
		return;

	// Draw border.
	FZoneComponent::Render( Canvas );

	// Draw view area.
	if( bSelected )
	{
		math::Vector ViewArea, Eye;
		math::Rect Skydome;
		Float Side, Side2;

		ViewArea.x	= Extent;
		ViewArea.y	= (Extent * Canvas->View.FOV.y) / Canvas->View.FOV.x;

		Skydome		= math::Rect( Location, Size.x, Size.y );
		Side		= math::sqrt( ViewArea.x*ViewArea.x + ViewArea.y*ViewArea.y );
		Side2		= Side * 0.5f;

		// Transform observer location and apply parallax.
		Eye.x	= Canvas->View.Coords.origin.x * Parallax.x + Offset.x;
		Eye.y	= Canvas->View.Coords.origin.y * Parallax.y + Offset.y;

		// Azimuth of sky should be wrapped.
		Eye.x	= Wrap( Eye.x, Skydome.min.x, Skydome.max.x );

		// Height of sky should be clamped.
		Eye.y	= clamp( Eye.y, Skydome.min.y+Side2, Skydome.max.y-Side2 );

		math::Angle Roll = math::Angle(fmodf(RollSpeed*(Float)GPlat->Now(), 2.f*math::PI ));		
		Canvas->DrawLineRect( Eye, ViewArea, Roll, math::colors::RED, false );
		Canvas->DrawLineStar( Eye, Roll, 1.f * Canvas->View.Zoom, math::colors::RED, false );
	}
}	


/*-----------------------------------------------------------------------------
    FZoneComponent implementation.
-----------------------------------------------------------------------------*/

//
// Zone constructor.
//
FZoneComponent::FZoneComponent()
	:	FRectComponent()
{
	bFixedAngle		= true;
	bHashable		= true;
	Size			= math::Vector( 10.f, 10.f );
}


//
// Zone rendering.
//
void FZoneComponent::Render( CCanvas* Canvas )
{
	// Is visible?
	math::Rect Bounds = GetAABB();
	if( !Canvas->View.Bounds.isOverlap( Bounds ) || !(Level->RndFlags & RND_Other) )
		return;

	// Choose colors.
	math::Color Color1 = math::colors::CRIMSON;
	math::Color Color2 = bSelected ? math::colors::CRIMSON + math::colors::HIGHLIGHT_1 : math::colors::CRIMSON;

	if( bFrozen )
		Color2	= math::colors::GRAY + Color2 * 0.5f;

	// Draw wire bounds.
	Canvas->DrawLineRect
					( 
						Location, 
						Size, 
						math::Angle(0), 
						Color2, 
						false
					);

	// If selected - draw stretch points and zone area.
	if( bSelected )
	{
		// Colored area.
		TRenderRect Rect;
		Rect.Flags		= POLY_Unlit | POLY_FlatShade | POLY_Ghost;
		Rect.Bounds		= Bounds;
		Rect.Rotation	= math::Angle(0);
		Rect.Texture	= nullptr;
		Rect.Color		= Color1;

		Canvas->DrawRect(Rect);

		// Draw handles.
		FRectComponent::Render( Canvas );
	}
}


/*-----------------------------------------------------------------------------
    FRectComponent implementation.
-----------------------------------------------------------------------------*/

//
// Rectangular object constructor.
//
FRectComponent::FRectComponent()
	:	FBaseComponent()
{
	bRenderable	= true;
	bFixedAngle	= false;
	Size		= math::Vector( 2.f, 2.f );
}


//
// Return rectangle AABB.
//
math::Rect FRectComponent::GetAABB()
{
	if( !Rotation )
		return math::Rect( Location, Size.x, Size.y );
	else
		return math::Rect( Location, math::sqrt(sqr(Size.x)+sqr(Size.y)) );
}


//
// Game started.
//
void FRectComponent::BeginPlay()
{
	FBaseComponent::BeginPlay();

	// Normally everything is hidden.
	bShown		= false;
	Entity->OnHide();
}


//
// Render rectangle border.
//
void FRectComponent::Render( CCanvas* Canvas )
{
	// Is visible?
	math::Rect Bounds = GetAABB();
	Bool bVisible	= Canvas->View.Bounds.isOverlap( Bounds );

	// Send in game OnHide/OnShow notifications.
	if( Level->bIsPlaying && !Canvas->View.bMirage )
	{
		if( bVisible )
		{
			// Rect is visible.
			if( !bShown )
			{
				Entity->OnShow();
				bShown	= true;
			}
		}
		else
		{
			// Rect is invisible.
			if( bShown )
			{
				Entity->OnHide();
				bShown	= false;
			}
		}
	}

	// Don't actually render, if invisible.
	if( !bVisible )
		return;

	// Draw stretch handles.
	if( bSelected )
	{
		math::Vector Size2 = Size * 0.5f;
		math::Coords Coords	= math::Coords( Location, Rotation );
		math::Vector XAxis = Coords.xAxis * Size2.x, 
				YAxis = Coords.yAxis * Size2.y;

		// Draw wire border.
		Canvas->DrawLineRect( Location, Size, Rotation, math::colors::WHITE, false );

		// Draw points.
		if( !bFixedSize )
		{
			math::Vector Points[8] = 
			{
				Coords.origin - XAxis + YAxis,
				Coords.origin + YAxis,
				Coords.origin + XAxis + YAxis,
				Coords.origin + XAxis,
				Coords.origin + XAxis - YAxis,
				Coords.origin - YAxis,
				Coords.origin - XAxis - YAxis,
				Coords.origin - XAxis,
			};

			for( Int32 i=0; i<8; i++ )
				Canvas->DrawCoolPoint( Points[i], 8.f, math::colors::WHITE );
		}
	}
}


//
// Return true, if rect appeared on screen.
//
void FRectComponent::nativeIsShown( CFrame& Frame )
{
	*POPA_BOOL	= bShown;
}


/*-----------------------------------------------------------------------------
    FBrushComponent implementation.
-----------------------------------------------------------------------------*/

//
// Brush constructor.
//
FBrushComponent::FBrushComponent()
	:	bUnlit( false ),
		bFlipH( false ),
		bFlipV( false ),
		Color( math::colors::WHITE ),
		Texture( nullptr ),
		Type( BRUSH_Solid ),
		Vertices(),
		NumVerts( 0 ),
		TexCoords( math::Vector( 0.f, 0.f ), math::Vector( 0.25f, 0.f ), math::Vector( 0.f, -0.25f ) ),
		Scroll( 0.f, 0.f )
{
	bHashable	= true;
	bRenderable	= true;
	mem::zero( Vertices, sizeof(Vertices) );
}


//
// Serialize brush.
//
void FBrushComponent::SerializeThis( CSerializer& S )
{
	FBaseComponent::SerializeThis( S );

	Serialize( S, bUnlit );
	Serialize( S, bFlipH );
	Serialize( S, bFlipV );
	Serialize( S, Color );
	Serialize( S, Texture );

	SerializeEnum( S, Type );
	Serialize( S, NumVerts );
	Serialize( S, TexCoords );
	Serialize( S, Scroll );

	for( Int32 i=0; i<NumVerts; i++ )
		Serialize( S, Vertices[i] );
}


//
// Return brush collision bounds.
//
math::Rect FBrushComponent::GetAABB()
{
	// Bounds in locals.
	math::Rect R( Vertices, NumVerts );

	// Simple transformation.
	R.min += Location;
	R.max += Location;

	return R;
}


//
// Render brush.
//
void FBrushComponent::Render( CCanvas* Canvas )
{
	// All brushes colors.
	static const math::Color BrushColors[BRUSH_MAX] =
	{			
  		math::Color( 0x40, 0x80, 0x00, 0xff ),	/* BRUSH_NotSolid */
  		math::Color( 0x80, 0x60, 0x20, 0xff ),	/* BRUSH_SemiSolid */
  		math::Color( 0x40, 0x40, 0x80, 0xff )	/* BRUSH_Solid */
	};

	// Is visible?
	math::Rect Bounds = GetAABB();
	if( !Canvas->View.Bounds.isOverlap(Bounds) )
		return;

	// Pick wire color.
	math::Color Color1, Color2;
	if( math::isConvexPoly( Vertices, NumVerts ) )
	{
		// A valid brush.
		Color1 = BrushColors[Type];
		Color2 = Color1 * 1.5f;
	}
	else
	{
		// An invalid brush.
		Color1 = math::colors::RED;
		Color2 = math::colors::RED;
	}

	// Transform and store vertices in TRenderPoly.
	TRenderPoly Poly;
	Poly.NumVerts	= NumVerts;
	for( Int32 i=0; i<NumVerts; i++ )
		Poly.Vertices[i]	= Vertices[i] + Location;

	// Draw textured surface.
	if( Texture )
	{
		Poly.Texture	= Texture;
		Poly.Flags		= POLY_Unlit*bUnlit | !(Level->RndFlags & RND_Lighting);
		Poly.Color		= Color;

		// Apply flips to matrix.
		math::Coords	Coords = TexCoords;
		if( bFlipH )	Coords.xAxis	= -Coords.xAxis;
		if( bFlipV )	Coords.yAxis	= -Coords.yAxis;

		Coords.origin	+= Scroll;

		// Compute texture coords.
		for( Int32 i=0; i<NumVerts; i++ )
			Poly.TexCoords[i]	= math::transformPointBy( Vertices[i], Coords );

		Canvas->DrawPoly( Poly );
	}

	// Draw a ghost highlight.
	if( !Texture && (Level->RndFlags & RND_Other) || bSelected )
	{
		Poly.Flags	 = POLY_FlatShade | POLY_Ghost;
		Poly.Texture = nullptr;
		Poly.Color	 = bSelected ? Color2 : Color1;

		if( bFrozen )
			Poly.Color	= math::colors::GRAY + Poly.Color*0.5f;

		Canvas->DrawPoly( Poly );
	}

	// Draw a wire.
	if( bSelected || (Level->RndFlags & RND_Other) )
	{
		math::Color WireColor;
		math::Vector V1, V2;

		if( !Texture )
			WireColor = bSelected ? Color1 : Color2;
		else
			WireColor = bSelected ? Color2 : Color1;

		if( bFrozen )
			WireColor	= math::colors::GRAY + WireColor*0.5f;

		V1 = Poly.Vertices[Poly.NumVerts-1];
		for( Int32 i=0; i<Poly.NumVerts; i++ )
		{
			V2 = Poly.Vertices[i];
			Canvas->DrawLine( V1, V2, WireColor, false );
			V1 = V2;
		}
	}

	// Draw a texture coords marker.
	if( bSelected )
	{
		Canvas->DrawLineStar
						( 
							Location + TexCoords.origin,
							math::vectorToAngle( TexCoords.xAxis ),
							2.5f * Canvas->View.Zoom,
							Color1,
							false
						);
		Canvas->DrawCoolPoint
							(
								Location + TexCoords.origin,
								5.f,
								Color2
							);
	}

	// Draw a stretch points.
	if( bSelected )
		for( Int32 i=0; i<NumVerts; i++ )
			Canvas->DrawCoolPoint( Poly.Vertices[i], 8.f, Color2 );
}


/*-----------------------------------------------------------------------------
    FInputComponent implementation.
-----------------------------------------------------------------------------*/

//
// Input component constructor.
//
FInputComponent::FInputComponent()
	:	FExtraComponent(),
		NextInput(nullptr)
{
}


//
// Input component destructor.
//
FInputComponent::~FInputComponent()
{
	com_remove(Input);
}


//
// Initialize input for level.
// 
void FInputComponent::InitForEntity( FEntity* InEntity )
{
	FExtraComponent::InitForEntity(InEntity);
	com_add(Input);
}


/*-----------------------------------------------------------------------------
    Registration.
-----------------------------------------------------------------------------*/

REGISTER_CLASS_CPP( FLightComponent, FExtraComponent, CLASS_Sterile )
{
	BEGIN_ENUM(ELightType)
		ENUM_ELEM(LIGHT_Steady);
		ENUM_ELEM(LIGHT_Flicker);
		ENUM_ELEM(LIGHT_Pulse);
		ENUM_ELEM(LIGHT_SoftPulse);
		ENUM_ELEM(LIGHT_SlowWave);
		ENUM_ELEM(LIGHT_FastWave);
		ENUM_ELEM(LIGHT_SpotLight);
		ENUM_ELEM(LIGHT_Searchlight);
		ENUM_ELEM(LIGHT_Fan);
		ENUM_ELEM(LIGHT_Disco);
		ENUM_ELEM(LIGHT_Flower);
		ENUM_ELEM(LIGHT_Hypnosis);
		ENUM_ELEM(LIGHT_Whirligig);
	END_ENUM;

	BEGIN_ENUM(ELightFunc)
		ENUM_ELEM(LF_Additive);
		ENUM_ELEM(LF_Multiplicative);
	END_ENUM;

	ADD_PROPERTY( bEnabled, PROP_Editable );

	ADD_PROPERTY( LightType, PROP_Editable );
	ADD_PROPERTY( LightFunc, PROP_Editable );
	ADD_PROPERTY( Color, PROP_Editable );
	ADD_PROPERTY( Radius, PROP_Editable );
	ADD_PROPERTY( Brightness, PROP_Editable );
}


REGISTER_CLASS_CPP( FSkyComponent, FZoneComponent, CLASS_None )
{
	ADD_PROPERTY( Parallax, PROP_Editable );
	ADD_PROPERTY( Offset, PROP_Editable );
	ADD_PROPERTY( RollSpeed, PROP_Editable );
	ADD_PROPERTY( Extent, PROP_Editable );
}


REGISTER_CLASS_CPP( FZoneComponent, FRectComponent, CLASS_None )
{
}


REGISTER_CLASS_CPP( FRectComponent, FBaseComponent, CLASS_None )
{
	DECLARE_METHOD( IsShown, TYPE_Bool, END );
}


REGISTER_CLASS_CPP( FBrushComponent, FBaseComponent, CLASS_None )
{
	BEGIN_ENUM(EBrushType)
		ENUM_ELEM(BRUSH_NotSolid);
		ENUM_ELEM(BRUSH_SemiSolid);
		ENUM_ELEM(BRUSH_Solid);
	END_ENUM;

	ADD_PROPERTY( Type, PROP_Editable );
	ADD_PROPERTY( bUnlit, PROP_Editable );
	ADD_PROPERTY( bFlipH, PROP_Editable );
	ADD_PROPERTY( bFlipV, PROP_Editable );
	ADD_PROPERTY( Color, PROP_Editable );
	ADD_PROPERTY( Texture, PROP_Editable );
	ADD_PROPERTY( Scroll, PROP_None );
	ADD_PROPERTY( NumVerts, PROP_None );
	ADD_PROPERTY( Vertices, PROP_None );
	ADD_PROPERTY( TexCoords, PROP_None );
}


REGISTER_CLASS_CPP( FInputComponent, FExtraComponent, CLASS_SingleComp )
{
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/