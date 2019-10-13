/*=============================================================================
    FrEmit.cpp: Particles emitters.
    Copyright Aug.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    FEmitterComponent implementation.
-----------------------------------------------------------------------------*/

//
// Emitter constructor.
//
FEmitterComponent::FEmitterComponent()
	:	MaxParticles( 100 ),
		SpawnArea( 0.f, 0.f ),
		SpawnOffset( 0.f, 0.f ),
		EmitPerSec( 10 ),
		SizeParam( PPT_Random ),
		Texture( nullptr ),
		bUnlit( true ),
		NumUTiles( 1 ),
		NumVTiles( 1 ),
		Particles(),
		NumPrts( 0 ),
		Accumulator( 0.f )
{
	bRenderable			= true;

	LifeRange[0]		= 3.f;
	LifeRange[1]		= 5.f;

	SizeRange[0]		= 0.5f;
	SizeRange[1]		= 1.5f;

	Colors[0]			= math::Color( 0xff, 0xff, 0xff, 0xff );
	Colors[1]			= math::Color( 0xff, 0xff, 0xff, 0xff );
	Colors[2]			= math::Color( 0xff, 0xff, 0xff, 0xff );

	SpinRange[0]		= 0.f;
	SpinRange[1]		= 0.f;

	// Force to be on top.
	DrawOrder			= 0.03f;

	bTickable	= true;
}


//
// Emitter destructor.
//
FEmitterComponent::~FEmitterComponent()
{
}

//
// Initialize emitter for entity.
//
void FEmitterComponent::InitForEntity( FEntity* InEntity )
{
	FExtraComponent::InitForEntity( InEntity );
}


//
// On some field changed in emitter.
//
void FEmitterComponent::EditChange()
{
	FExtraComponent::EditChange();

	// Clamp tiles count.
	NumUTiles	= clamp<UInt8>( NumUTiles, 1, 4 );
	NumVTiles	= clamp<UInt8>( NumVTiles, 1, 4 );

	// Clamp particles limit.
	MaxParticles	= clamp<Int32>( MaxParticles, 1, MAX_PARTICLES );
}


//
// Initialize emitter after loading.
//
void FEmitterComponent::PostLoad()
{
	FExtraComponent::PostLoad();
}


//
// Update particles, should be implemented in
// subclasses.
//
void FEmitterComponent::UpdateEmitter( Float Delta )
{
}


//
// Return particle system cloud bound.
//
math::Rect FEmitterComponent::GetCloudRect()
{
	math::Rect Result;
	Result.min		= Base->Location;
	Result.max		= Base->Location;

	for( Int32 i=0; i<NumPrts; i++ )
	{
		TParticle& P = Particles[i];

		// Update bounds.
		Result.min.x	= min( Result.min.x, P.Location.x );
		Result.min.y	= min( Result.min.y, P.Location.y );
		Result.max.x	= max( Result.max.x, P.Location.x );
		Result.max.y	= max( Result.max.y, P.Location.y );
	}

    // Cloud bound is compute, but necessary to consider
    // the particles size too.
	Result.min.x	-= SizeRange[1];
	Result.min.y	-= SizeRange[1];
	Result.max.x	+= SizeRange[1];
	Result.max.y	+= SizeRange[1];

	return Result;
}


//
// Tick in editor.
//
void FEmitterComponent::TickNonPlay( Float Delta )
{
	Tick( Delta );
}


//
// Tick in game.
//
void FEmitterComponent::Tick( Float Delta )
{
	// Reallocate list, if need.
	if( MaxParticles != Particles.size() )
	{
		MaxParticles	= clamp<Int32>( MaxParticles, 1, MAX_PARTICLES );
		NumPrts			= 0;
		Particles.setSize( MaxParticles );
	}

	// Not really good way to determinate
	// is should update particles.
	math::Rect Camera = math::Rect( Level->Camera.Location, Level->Camera.FOV.x, Level->Camera.FOV.y );
	math::Rect Cloud = GetCloudRect();
	if( !Camera.isOverlap( Cloud ) )
		return;

	// Sub-classes will process particles.
	UpdateEmitter( Delta );
}


//
// Serialize spark.
//
void Serialize( CSerializer& S, TParticle& V )
{
	Serialize( S, V.Location );
	Serialize( S, V.Rotation );
	Serialize( S, V.Speed );
	Serialize( S, V.SpinRate );
	Serialize( S, V.Size );
	Serialize( S, V.Life );
	Serialize( S, V.MaxLifeInv );
	Serialize( S, V.Phase );
	Serialize( S, V.iTile );
}


//
// Serialize emitter.
//
void FEmitterComponent::SerializeThis( CSerializer& S )
{
	FExtraComponent::SerializeThis( S );

	Serialize( S, MaxParticles );
	Serialize( S, LifeRange[0] );
	Serialize( S, LifeRange[1] );
	Serialize( S, SpawnArea );
	Serialize( S, EmitPerSec );
	SerializeEnum( S, SizeParam );
	Serialize( S, SizeRange[0] );
	Serialize( S, SizeRange[1] );
	Serialize( S, bUnlit );
	Serialize( S, Colors[0] );
	Serialize( S, Colors[1] );
	Serialize( S, Colors[2] );
	Serialize( S, SpinRange[0] );
	Serialize( S, SpinRange[1] );
	Serialize( S, Texture );
	Serialize( S, NumUTiles );
	Serialize( S, NumVTiles );
	Serialize( S, NumPrts );

	// Temporally shrink particles tables,
	// to store only "active" particles.
	if( S.GetMode() == SM_Save )
	{
		// Really reduce particles count, it's can crash
		// even CObjectDatabase!
		Int32 NumToSave = min( 100, NumPrts );
		if( NumToSave )
			S.SerializeData( &Particles[0], NumToSave*sizeof(TParticle) );
	}
	else if( S.GetMode() == SM_Load )
	{
		Int32 NumToLoad = min( 100, NumPrts );
		Particles.setSize(NumToLoad);
		if( NumToLoad )
			S.SerializeData( &Particles[0], NumToLoad*sizeof(TParticle) );
	}
}


/*-----------------------------------------------------------------------------
    FLissajousEmitterComponent implementation.
-----------------------------------------------------------------------------*/

//
// Lissajous curve constructor.
//
FLissajousEmitterComponent::FLissajousEmitterComponent()
	:	FEmitterComponent(),
		Alpha( 1.5f ),
		Beta( 1.f ),
		Delta( 20.f ),
		X( 5.f ),
		Y( 5.f )
{
	LifeRange[0]	= 5.f;
	LifeRange[1]	= 5.f;

	SizeRange[0]	= 1.f;
	SizeRange[1]	= 5.f;

	MaxParticles	= 30;
	EmitPerSec		= 4;
	SizeParam		= PPT_Random;
}


void FLissajousEmitterComponent::SerializeThis( CSerializer& S )
{
	FEmitterComponent::SerializeThis( S );
	Serialize( S, Alpha );
	Serialize( S, Beta );
	Serialize( S, Delta );
	Serialize( S, X );
	Serialize( S, Y );
}


//
// Update Lissajous trajectories.
//
void FLissajousEmitterComponent::UpdateEmitter( Float DeltaTime )
{
	math::Vector Basis	= Base->Location + SpawnOffset;

	// Accumulate time.
	Accumulator += DeltaTime;

	// Spawn new particles.
	Int32 NewPrts = math::trunc( Accumulator * EmitPerSec );
	if( NewPrts > 0 )
		Accumulator = 0.f;
	while( NewPrts>0 && NumPrts<MaxParticles )
	{
		TParticle P;
		P.Location.x	= RandomRange( Basis.x-SpawnArea.x, Basis.x+SpawnArea.x );
		P.Location.y	= RandomRange( Basis.y-SpawnArea.y, Basis.y+SpawnArea.y );

		P.Speed			= P.Location;		// Store spawn location.
		P.iTile			= Random(NumUTiles * NumVTiles);
		P.Phase			= RandomRange( 0.f, 2.f*math::PI );
		P.Life			= RandomRange( LifeRange[0], LifeRange[1] );
		P.MaxLifeInv	= 1.f / max( 0.001f, P.Life );
		P.Size			= SizeParam == PPT_Random ? RandomRange( SizeRange[0], SizeRange[1] ) : SizeRange[0];

		if( SpinRange[0] == SpinRange[1] && SpinRange[0] == 0.f )
		{
			// No rotation.
			P.Rotation	= 0;
			P.SpinRate	= 0.f;
		}
		else
		{
			// Rotate.
			P.Rotation	= Random(0xffff);
			P.SpinRate	= RandomRange( SpinRange[0], SpinRange[1] );
		}

		// Add to list.
		Particles[NumPrts]	= P;
		NumPrts++;
		NewPrts--;
	}
	
	// Process Lissajous physics.
	for( Int32 i=0; i<NumPrts;  )
	{
		TParticle& P = Particles[i];

		// Individual 'time' for each particle.
		Float T = P.Life * P.MaxLifeInv * (2.f*math::PI) + P.Phase;

		P.Location.x	= P.Speed.x + X * math::sin( Alpha * T + Delta );
		P.Location.y	= P.Speed.y + Y * math::sin( Beta * T );

		P.Rotation		+= DeltaTime * P.SpinRate;
		P.Life			-= DeltaTime;

		if( SizeParam == PPT_Linear )
		{
			Float Alpha = 1.f - P.Life * P.MaxLifeInv;
			P.Size		= SizeRange[0] + (SizeRange[1]-SizeRange[0]) * Alpha;
		}

		// Kill outlived.
		if( P.Life <= 0.f )
		{
			NumPrts--;
			Particles[i]	= Particles[NumPrts];
		}
		else
			i++;
	}
}


/*-----------------------------------------------------------------------------
	FWeatherEmitterComponent implementation.
-----------------------------------------------------------------------------*/

//
// Weather emitter constructor.
//
FWeatherEmitterComponent::FWeatherEmitterComponent()
	:	FEmitterComponent(),
		WeatherType( WEATHER_Snow )
{
	SpeedRange[0]	= 3.f;
	SpeedRange[1]	= 10.f;

	SpawnArea.x		= 32.f;
	SpawnArea.y		= 10.f;
}


//
// Return the particles cloud bounds. But
// snowflakes or raindrops are fall in the entire world!
//
math::Rect FWeatherEmitterComponent::GetCloudRect()
{
	return math::Rect( math::Vector( 0.f, 0.f ), math::WORLD_SIZE );
}


//
// Render the weather particles.
//
void FWeatherEmitterComponent::Render( CCanvas* Canvas )
{
    // Don't draw snowflakes or raindrops in
    // a mirage, especially in sky-zone.
	if( !Canvas->viewInfo().isMirage )
		FEmitterComponent::Render( Canvas );
}


void FWeatherEmitterComponent::SerializeThis( CSerializer& S )
{
	FEmitterComponent::SerializeThis( S );
	SerializeEnum( S, WeatherType );
	Serialize( S, SpeedRange[0] );
	Serialize( S, SpeedRange[1] );
}


//
// Update the weather particles.
//
void FWeatherEmitterComponent::UpdateEmitter( Float Delta )
{
	// Precompute.
	Float ViewTop	= Level->Camera.FOV.y*0.5f + Level->Camera.Location.y;

	// Accumulate time.
	Accumulator += Delta;

	// Spawn new particles.
	Int32 NewPrts = math::trunc( Accumulator * EmitPerSec );
	if( NewPrts > 0 )
		Accumulator = 0.f;
	while( NewPrts>0 && NumPrts<MaxParticles )
	{
		TParticle P;
		P.Location.x	= RandomRange( -SpawnArea.x, SpawnArea.x ) + Level->Camera.Location.x;
		P.Location.y	= ViewTop + SpawnArea.y;

		if( WeatherType == WEATHER_Snow )
		{
			// Emit new snowflake.
			P.Speed.x	= P.Location.x;		// Store origin X, to apply jitter effect.
			P.Speed.y	= RandomRange( SpeedRange[0], SpeedRange[1] );
		}
		else
		{
			// Emit new raindrop.
			P.Speed.x	= 0.f;		
			P.Speed.y	= RandomRange( SpeedRange[0], SpeedRange[1] );
		}

		P.iTile			= Random(NumUTiles * NumVTiles);
		P.Phase			= RandomRange( 0.f, 2.f*math::PI );
		P.Life			= RandomRange( LifeRange[0], LifeRange[1] );
		P.MaxLifeInv	= 1.f / max( 0.001f, P.Life );
		P.Size			= RandomRange( SizeRange[0], SizeRange[1] );

		if( SpinRange[0] == SpinRange[1] && SpinRange[0] == 0.f )
		{
			// No rotation.
			P.Rotation	= 0;
			P.SpinRate	= 0.f;
		}
		else
		{
			// Rotate.
			P.Rotation	= Random(0xffff);
			P.SpinRate	= RandomRange( SpinRange[0], SpinRange[1] );
		}
		
		// Add to list.
		Particles[NumPrts]	= P;
		NumPrts++;
		NewPrts--;
	}

	// Process weather physics.
	for( Int32 i=0; i<NumPrts;  )
	{
		TParticle& P = Particles[i];

		if( WeatherType == WEATHER_Snow )
		{
			// Snowflake physics.
			P.Location.x	= P.Speed.x + math::sin(P.Phase)*5.f;
			P.Location.y	-= P.Speed.y * Delta;
			P.Phase			+= Delta;
		}
		else
		{
			// Raindrop physics.
			P.Location.y -= P.Speed.y * Delta;
		}

		P.Rotation		+= Delta * P.SpinRate;
		P.Life			-= Delta;

		// Kill outlived.
		if( P.Life <= 0.f )
		{
			NumPrts--;
			Particles[i]	= Particles[NumPrts];
		}
		else
			i++;
	}
}


/*-----------------------------------------------------------------------------
	FPhysEmitterComponent implementation.
-----------------------------------------------------------------------------*/

//
// Physics emitter constructor.
//
FPhysEmitterComponent::FPhysEmitterComponent()
	:	FEmitterComponent()
{
	SpeedRange[0]	= math::Vector( -5.f, -5.f );
	SpeedRange[1]	= math::Vector( +5.f, +5.f );
	Acceleration	= math::Vector( 0.f, 0.f );
}


void FPhysEmitterComponent::SerializeThis( CSerializer& S )
{
	FEmitterComponent::SerializeThis( S );
	Serialize( S, SpeedRange[0] );
	Serialize( S, SpeedRange[1] );
	Serialize( S, Acceleration );
}


//
// Update physics.
//
void FPhysEmitterComponent::UpdateEmitter( Float Delta )
{
	// Let's velocity and acceleration in local coords.
	math::Coords LocalToWorld = Base->ToWorld();
	math::Vector WorldAcc = math::transformVectorBy( Acceleration, LocalToWorld );
	math::Vector Basis	= Base->Location + SpawnOffset;

	// Accumulate time.
	Accumulator += Delta;

	// Spawn new particles.
	Int32 NewPrts = math::trunc( Accumulator * EmitPerSec );
	if( NewPrts > 0 )
		Accumulator = 0.f;
	while( NewPrts>0 && NumPrts<MaxParticles )
	{
		TParticle P;
		P.Location.x	= RandomRange( Basis.x-SpawnArea.x, Basis.x+SpawnArea.x );
		P.Location.y	= RandomRange( Basis.y-SpawnArea.y, Basis.y+SpawnArea.y );

		P.Speed.x		= RandomRange( SpeedRange[0].x, SpeedRange[1].x );
		P.Speed.y		= RandomRange( SpeedRange[0].y, SpeedRange[1].y );
		P.Speed			= math::transformVectorBy( P.Speed, LocalToWorld );

		P.Life			= RandomRange( LifeRange[0], LifeRange[1] );
		P.MaxLifeInv	= 1.f / max( 0.001f, P.Life );
		P.iTile			= Random(NumUTiles * NumVTiles);
		P.Size			= SizeParam == PPT_Random ? RandomRange( SizeRange[0], SizeRange[1] ) : SizeRange[0];

		if( SpinRange[0] == SpinRange[1] && SpinRange[0] == 0.f )
		{
			// No rotation.
			P.Rotation	= 0;
			P.SpinRate	= 0.f;
		}
		else
		{
			// Rotate.
			P.Rotation	= Random(0xffff);
			P.SpinRate	= RandomRange( SpinRange[0], SpinRange[1] );
		}

		// Add to list.
		Particles[NumPrts]	= P;
		NumPrts++;
		NewPrts--;
	}

	// Process physics.
	for( Int32 i=0; i<NumPrts;  )
	{
		TParticle& P = Particles[i];

		P.Location		+= P.Speed * Delta;
		P.Speed			+= WorldAcc * Delta;
		P.Life			-= Delta;
		P.Rotation		+= Delta * P.SpinRate;

		if( SizeParam == PPT_Linear )
		{
			Float Alpha = 1.f - P.Life * P.MaxLifeInv;
			P.Size		= SizeRange[0] + (SizeRange[1]-SizeRange[0]) * Alpha;
		}

		// Kill outlived.
		if( P.Life <= 0.f )
		{
			NumPrts--;
			Particles[i]	= Particles[NumPrts];
		}
		else
			i++;
	}
}


/*-----------------------------------------------------------------------------
	Particles rendering.
-----------------------------------------------------------------------------*/

//
// Color linear interpolation.
// Try to optimize. Probably I'm mad, just
// 3 multiplications it's not very expensive.
// I think via MMX it's will be super fast.
// Keep calm, this take about 8 cycles for 256 palette!!
// even table usage cost about 16 cycles.
//
inline math::Color ColorLerp( math::Color Color1, math::Color Color2, UInt8 Alpha )
{
	math::Color Result;
	Result.r = (Int32)(Color1.r) + (((Int32)(Color2.r)-(Int32)(Color1.r))*Alpha >> 8); 
	Result.g = (Int32)(Color1.g) + (((Int32)(Color2.g)-(Int32)(Color1.g))*Alpha >> 8); 
	Result.b = (Int32)(Color1.b) + (((Int32)(Color2.b)-(Int32)(Color1.b))*Alpha >> 8); 
	Result.a = (Int32)(Color1.a) + (((Int32)(Color2.a)-(Int32)(Color1.a))*Alpha >> 8); 
	return Result;
}


//
// Render particles.
//
void FEmitterComponent::Render( CCanvas* Canvas )
{
	// Particles render turn on?
	if( !(Level->RndFlags & RND_Particles) )
		return;

	// Render particles if they actually visible.
	math::Rect Cloud = GetCloudRect();
	if( !Canvas->viewInfo().bounds.isOverlap( Cloud ) )
		return;

#if 0
	// Compute texture coords for each tile, will
	// be cool to cache it.
	TRect CoordTable[16];
	for( Integer V=0; V<NumVTiles; V++ )
		for( Integer U=0; U<NumUTiles; U++ )
		{
			CoordTable[V*NumUTiles+U].Min.X	= (Float)(U+0.f) / (Float)NumUTiles;
			CoordTable[V*NumUTiles+U].Max.X	= (Float)(U+1.f) / (Float)NumUTiles;

			CoordTable[V*NumUTiles+U].Min.Y	= 1.f - (Float)(V+0.f) / (Float)NumVTiles;
			CoordTable[V*NumUTiles+U].Max.Y	= 1.f - (Float)(V+1.f) / (Float)NumVTiles;
		}

	// Initialize shared fields of render rect.
	TRenderRect Rect;
	Rect.Flags			= POLY_Unlit*bUnlit | !(Level->RndFlags & RND_Lighting);
	Rect.Rotation		= 0;
	Rect.Bitmap			= Bitmap;
	
	// Render particles.
	for( Integer i=0; i<NumPrts; i++ )
	{
		TParticle&	P = Particles[i];
		Float		Side = P.Size * 0.5f;
		Float		Alpha = 1.f - P.Life * P.MaxLifeInv;

		// Interpolate color.
		if( Alpha < 0.5f )
			Rect.Color	= ColorLerp( Colors[0], Colors[1], (Byte)(Alpha*512) );
		else
			Rect.Color	= ColorLerp( Colors[1], Colors[2], (Byte)((Alpha-0.5f)*512) );

		// Draw.
		Rect.Bounds		= TRect( P.Location, Side );
		Rect.TexCoords	= CoordTable[P.iTile];
		Rect.Rotation	= P.Rotation;
		Canvas->DrawRect( Rect );
	}
#else
	// Compute texture coords for each tile, will
	// be cool to cache it.
	math::Vector CoordTable[16][4];
	for( Int32 V=0; V<NumVTiles; V++ )
	for( Int32 U=0; U<NumUTiles; U++ )
	{
		Int32 iSlot	= V*NumUTiles+U;

		Float	X1		= (Float)(U+0.f) / (Float)NumUTiles;
		Float	X2		= (Float)(U+1.f) / (Float)NumUTiles;

		Float	Y1		= 1.f - (Float)(V+0.f) / (Float)NumVTiles;
		Float	Y2		= 1.f - (Float)(V+1.f) / (Float)NumVTiles;

		CoordTable[iSlot][0]	= math::Vector( X1, Y1 );
		CoordTable[iSlot][1]	= math::Vector( X1, Y2 );
		CoordTable[iSlot][2]	= math::Vector( X2, Y2 );
		CoordTable[iSlot][3]	= math::Vector( X2, Y1 );
	}

	// Initialize list.
	TRenderList List( NumPrts );
	List.Image			= Texture ? As<FBitmap>(Texture)->m_image->getHandle() : INVALID_HANDLE<rend::Texture2DHandle>();
	List.Flags			= POLY_Unlit*bUnlit | !(Level->RndFlags & RND_Lighting);

	// Initialize list.
	for( Int32 i=0; i<NumPrts; i++ )
	{
		TParticle&	P = Particles[i];
		Float		Side = P.Size * 0.5f;
		Float		Alpha = 1.f - P.Life * P.MaxLifeInv;
		math::Color	Color;

		// Interpolate color.
		if( Alpha < 0.5f )
			Color	= ColorLerp( Colors[0], Colors[1], (UInt8)(Alpha*512) );
		else
			Color	= ColorLerp( Colors[1], Colors[2], (UInt8)((Alpha-0.5f)*512) );

		// Particle color.
		List.Colors[i*4+0]	= Color;
		List.Colors[i*4+1]	= Color;
		List.Colors[i*4+2]	= Color;
		List.Colors[i*4+3]	= Color;

		// Texture coords.
		List.TexCoords[i*4+0]	= CoordTable[P.iTile][0];
		List.TexCoords[i*4+1]	= CoordTable[P.iTile][1];
		List.TexCoords[i*4+2]	= CoordTable[P.iTile][2];
		List.TexCoords[i*4+3]	= CoordTable[P.iTile][3];

		// Vertices.
		math::Coords Coords		= math::Coords( P.Location, P.Rotation );
		math::Vector XAxis = Coords.xAxis * Side,
				YAxis = Coords.yAxis * Side;

		List.Vertices[i*4+0]	= P.Location - YAxis - XAxis;
		List.Vertices[i*4+1]	= P.Location + YAxis - XAxis;
		List.Vertices[i*4+2]	= P.Location + YAxis + XAxis;
		List.Vertices[i*4+3]	= P.Location - YAxis + XAxis;
	}

	// Render particles.
	Canvas->DrawList( List );
#endif

	// Render cloud boundS if emitter are selected.
	if( Base->bSelected )
		Level->m_primitiveDrawer.batchLineRect
						( 
							Cloud.center(), 
							Cloud.sizeX(), Cloud.sizeY(), 
							0, 
							1.f,
							math::colors::ORANGE
						);
}


/*-----------------------------------------------------------------------------
    Registration.
-----------------------------------------------------------------------------*/

REGISTER_CLASS_CPP( FEmitterComponent, FExtraComponent, CLASS_Abstract )
{
	BEGIN_ENUM(EParticleParam)
		ENUM_ELEM(PPT_Random)
		ENUM_ELEM(PPT_Linear)
	END_ENUM;
	
	ADD_PROPERTY( MaxParticles, PROP_Editable );
	ADD_PROPERTY( LifeRange, PROP_Editable );
	ADD_PROPERTY( SpawnArea, PROP_Editable );
	ADD_PROPERTY( SpawnOffset, PROP_Editable );
	ADD_PROPERTY( EmitPerSec, PROP_Editable );
	ADD_PROPERTY( SizeParam, PROP_Editable );
	ADD_PROPERTY( SizeRange, PROP_Editable );
	ADD_PROPERTY( bUnlit, PROP_Editable );
	ADD_PROPERTY( Colors, PROP_Editable );
	ADD_PROPERTY( Texture, PROP_Editable );
	ADD_PROPERTY( SpinRange, PROP_Editable );
	ADD_PROPERTY( NumUTiles, PROP_Editable );
	ADD_PROPERTY( NumVTiles, PROP_Editable );
}


REGISTER_CLASS_CPP( FPhysEmitterComponent, FEmitterComponent, CLASS_None )
{
	ADD_PROPERTY( SpeedRange, PROP_Editable );
	ADD_PROPERTY( Acceleration, PROP_Editable );
}


REGISTER_CLASS_CPP( FLissajousEmitterComponent, FEmitterComponent, CLASS_None )
{
	ADD_PROPERTY( Alpha, PROP_Editable );
	ADD_PROPERTY( Beta, PROP_Editable );
	ADD_PROPERTY( Delta, PROP_Editable );
	ADD_PROPERTY( X, PROP_Editable );
	ADD_PROPERTY( Y, PROP_Editable );
}


REGISTER_CLASS_CPP( FWeatherEmitterComponent, FEmitterComponent, CLASS_SingleComp )
{
	BEGIN_ENUM(EWeather)
		ENUM_ELEM(WEATHER_Snow);
		ENUM_ELEM(WEATHER_Rain);
	END_ENUM;

	ADD_PROPERTY( WeatherType, PROP_Editable );
	ADD_PROPERTY( SpeedRange, PROP_Editable );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/