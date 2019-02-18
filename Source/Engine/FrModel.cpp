/*=============================================================================
    FrModel.cpp: Tile graphics model.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    FModelComponent implementation.
-----------------------------------------------------------------------------*/

//
// Model constructor, need to initialize map
// due defaults.
//
FModelComponent::FModelComponent()
	:	bUnlit( false ),
		Color( COLOR_White ),
		Texture( nullptr ),
		MapXSize( 30 ),
		MapYSize( 15 ),
		TileSize( 2.f, 2.f ),
		TilesPerU( 16 ),
		TilesPerV( 16 ),
		PenIndex( -1 )
{
	Size		= math::Vector( 0.f, 0.f );
	bFixedAngle	= true;
	bRenderable	= true;

	// Compute initial tables.
	ReallocMap();
	SetAtlasTable();
}


//
// Setup atlas table for tile model, it computes
// texture coords for each tile.
//
void FModelComponent::SetAtlasTable()
{
	// Clamp parameters.
	TilesPerU	= Clamp<UInt8>( TilesPerU, 1, 200 );
	TilesPerV	= Clamp<UInt8>( TilesPerV, 1, 200 );

	Float TileSizeU	= 1.f / TilesPerU;
	Float TileSizeV	= 1.f / TilesPerV;
	Int32 iTile = 0, U, V;

	for( V=0; V<TilesPerV; V++ )
		for( U=0; U<TilesPerU; U++ )
		{
			TRect Tile;
			Tile.Min		= math::Vector( (Float)TileSizeU*(U+0.f), (Float)TileSizeV*(V+1.f) );
			Tile.Max		= math::Vector( (Float)TileSizeU*(U+1.f), (Float)TileSizeV*(V+0.f) );

			AtlasTable[iTile++]	= Tile;

			if( iTile > 255 )
				goto Enough;
		}

Enough:;
}


//
// On field changed in object inspector.
//
void FModelComponent::EditChange()
{
	FBaseComponent::EditChange();

	// If map size changed - realloc the map,
	// honestly not really good check.
	if( Map.size() != MapXSize*MapYSize )
		ReallocMap();

	// Anyway refresh atlas table.
	SetAtlasTable();

	// Clamp tiles dim.
	while( TilesPerU * TilesPerV > 256 )
	{
		if( TilesPerU > TilesPerV )
			TilesPerU--;
		else
			TilesPerV--;
	}
}


//
// Reallocate tiles map, its cleanup entire
// model, so use a little trick to store
// tiles, see TTileEditor.
//
void FModelComponent::ReallocMap()
{
	// Clamp map size.
	MapXSize	= Clamp<Int32>( MapXSize, 1, MAX_TILES_SIDE );
	MapYSize	= Clamp<Int32>( MapYSize, 1, MAX_TILES_SIDE );

	// Reallocate data, and zero map, since
	// reallocation cause tiles shuffle.
	Map.setSize( MapXSize * MapYSize );
	mem::zero( &Map[0], MapXSize*MapYSize*sizeof(UInt16) );
}


//
// Discretize world's location into index of tile of the
// tile model. If given point is outside mode, return -1.
//
Int32 FModelComponent::WorldToMapIndex( Float vX, Float vY )
{
	// Transform to model local coords.
	vX	-= Location.x;
	vY	-= Location.y;

	// Rescale.
	vX	/= TileSize.x;
	vY	/= TileSize.y;

	// Discretize.
	Int32	X	= math::floor( vX );
	Int32	Y	= math::floor( vY );

	// Check bounds.
	if( ( X < 0 )||( Y < 0 )||( X >= MapXSize )||( Y >= MapYSize ) )
		return -1;
	else
		return X + Y * MapXSize;
}


//
// After loading model initialization.
//
void FModelComponent::PostLoad()
{
	FBaseComponent::PostLoad();
	SetAtlasTable();
}


//
// Return the model collision AABB bounds.
// Probably unused, since model is even not
// hashable entity.
//
TRect FModelComponent::GetAABB()
{
	TRect Result;

	Result.Min.x	= Location.x;
	Result.Min.y	= Location.y;

	Result.Max.x	= Location.x + MapXSize * TileSize.x;
	Result.Max.y	= Location.y + MapYSize * TileSize.y;

	return Result;
}


//
// Model rendering.
//
void FModelComponent::Render( CCanvas* Canvas )
{
	// Check for validness.
	assert(Map.size() == MapXSize*MapYSize);

	if( !( Level->RndFlags & RND_Model ) )
		return;

	TRect	View		= Canvas->View.Bounds;
	TRect	ModelBounds	= GetAABB();

	// Is visible?
	if( !View.IsOverlap(ModelBounds) )
		return;

	// Draw model pad.
	if( bSelected )
	{
		TRenderRect Pad;
		Pad.Flags			= POLY_Unlit | POLY_FlatShade | POLY_Ghost;
		Pad.Bounds			= ModelBounds;
		Pad.Color			= TColor( 0x20, 0x20, 0x30, 0xff );
		Pad.Rotation		= 0;
		Pad.Texture			= nullptr;
		Canvas->DrawRect( Pad );
	}

	// Compute screen bound indexes for drawing tiles.
	Int32 XMin = Max( math::trunc((View.Min.x - Location.x) / TileSize.x), 0 );
	Int32 YMin = Max( math::trunc((View.Min.y - Location.y) / TileSize.y), 0 );

	Int32 XMax = Min( math::ceil((View.Max.x - Location.x) / TileSize.x), MapXSize );
	Int32 YMax = Min( math::ceil((View.Max.y - Location.y) / TileSize.y), MapYSize );

	// Setup shared tile info.
#if 0
	TRenderRect Tile;
	Tile.Flags		= POLY_Unlit*bUnlit | !(Level->RndFlags & RND_Lighting);
	Tile.Rotation	= 0;
	Tile.Bitmap		= Bitmap;
	Tile.Color		= Color;

	for( Integer Y=YMin; Y<YMax; Y++ )
		for( Integer X=XMin; X<XMax; X++ )
		{
			Integer iTile = Map[X + Y * MapXSize];

			if( iTile )
			{
				Tile.TexCoords		= AtlasTable[iTile];

				Tile.Bounds.Min.X	= (X + 0.f)*TileSize.X + Location.X;
				Tile.Bounds.Min.Y	= (Y + 0.f)*TileSize.Y + Location.Y;
				Tile.Bounds.Max.X	= (X + 1.f)*TileSize.X + Location.X;
				Tile.Bounds.Max.Y	= (Y + 1.f)*TileSize.Y + Location.Y;

				Canvas->DrawRect(Tile);
			}
		}
#else
	// Count real visible tiles.
	Int32 NumVis = 0;
	for( Int32 Y=YMin; Y<YMax; Y++ )
	for( Int32 X=XMin; X<XMax; X++ )
	{
		if( Map[X + Y * MapXSize] & 0x00ff )
			NumVis++;
		if( Map[X + Y * MapXSize] & 0xff00 )
			NumVis++;
	}

	// Setup list.
	Int32	NumTiles = 0;
	TRenderList List( NumVis, Color );
	List.Texture	= Texture;
	List.DrawColor	= bFrozen ? Color*0.75f : Color;
	List.Flags		= POLY_Unlit*bUnlit | !(Level->RndFlags & RND_Lighting);

	// Collect all tiles.
	for( Int32 Y=YMin; Y<YMax; Y++ )
	for( Int32 X=XMin; X<XMax; X++ )
	{
		Int32 iTile		= Map[X + Y * MapXSize];

		Float	MinX	= (X + 0.f)*TileSize.x + Location.x;
		Float	MinY	= (Y + 0.f)*TileSize.y + Location.y;
		Float	MaxX	= (X + 1.f)*TileSize.x + Location.x;
		Float	MaxY	= (Y + 1.f)*TileSize.y + Location.y;

#define DRAW_TILE(itile)\
	if( itile )\
	{\
		TRect	T						= AtlasTable[itile];\
		List.Vertices[NumTiles*4+0]		= math::Vector( MinX, MinY );\
		List.Vertices[NumTiles*4+1]		= math::Vector( MinX, MaxY );\
		List.Vertices[NumTiles*4+2]		= math::Vector( MaxX, MaxY );\
		List.Vertices[NumTiles*4+3]		= math::Vector( MaxX, MinY );\
		List.TexCoords[NumTiles*4+0]	= math::Vector( T.Min.x, T.Min.y );\
		List.TexCoords[NumTiles*4+1]	= math::Vector( T.Min.x, T.Max.y );\
		List.TexCoords[NumTiles*4+2]	= math::Vector( T.Max.x, T.Max.y );\
		List.TexCoords[NumTiles*4+3]	= math::Vector( T.Max.x, T.Min.y );\
		NumTiles++;\
	};\

		// Draw two layers.
		DRAW_TILE(iTile & 0x00ff);
		DRAW_TILE((iTile & 0xff00) >> 8);

#undef DRAW_TILE
	}

	// And render it.
	List.NumRects	= NumTiles;
	Canvas->DrawList( List );
#endif

	// Draw grid and pen tiles.
	if( !( Level->RndFlags & RND_Other ) )
		return;

	TColor GridColor = bSelected ? COLOR_LightBlue : COLOR_CadetBlue;
	if( bFrozen )
		GridColor	= COLOR_Gray;

	// Vertical lines.
	for( Int32 X=XMin; X<=XMax; X++ )
	{
		math::Vector V1 = math::Vector( Location.x + X*TileSize.x, Location.y );
		math::Vector V2 = math::Vector( Location.x + X*TileSize.x, Location.y + MapYSize * TileSize.y );
		Canvas->DrawLine( V1, V2, GridColor, false );
	}

	// Horizontal lines.
	for( Int32 Y=YMin; Y<=YMax; Y++ )
	{
		math::Vector V1 = math::Vector( Location.x, Location.y + Y*TileSize.y );
		math::Vector V2 = math::Vector( Location.x + MapXSize*TileSize.x, Location.y + Y*TileSize.y );
		Canvas->DrawLine( V1, V2, GridColor, false );
	}

	// Draw temporal tiles.
	// Editor only.
	if( bSelected && PenIndex != -1 )
	{
		if( Selected.size() == 1 && Selected[0] == 0 )
		{
			// Draw erase tile.
			Int32 X = PenIndex % MapXSize;
			Int32 Y = PenIndex / MapXSize;

			TRenderRect Tile;
			Tile.Flags				= POLY_Unlit | POLY_FlatShade | POLY_Ghost;
			Tile.Rotation			= 0;
			Tile.Color				= COLOR_FireBrick;
			Tile.Texture			= nullptr;
			
			Tile.Bounds.Min.x		= (X + 0.f)*TileSize.x + Location.x;
			Tile.Bounds.Min.y		= (Y + 0.f)*TileSize.y + Location.y;
			Tile.Bounds.Max.x		= (X + 1.f)*TileSize.x + Location.x;
			Tile.Bounds.Max.y		= (Y + 1.f)*TileSize.y + Location.y;

			Canvas->DrawRect( Tile );
		}
		else if( Selected.size() > 0 )
		{
			// Draw ghost tiles :3
			Int32	BaseX	= Selected[0] % TilesPerU;
			Int32 BaseY	= Selected[0] / TilesPerU;

			Int32 PenX	= PenIndex % MapXSize;
			Int32 PenY	= PenIndex / MapXSize;

			TRenderRect Tile;
			Tile.Flags				= POLY_Unlit | POLY_Ghost;
			Tile.Rotation			= 0;
			Tile.Texture			= Texture;
			Tile.Color				= TColor( 0xa0, 0xa0, 0xa0, 0xff );

			for( Int32 i=0; i<Selected.size(); i++ )
			{
				Int32 X = PenX + ((Selected[i] % TilesPerU) - BaseX);
				Int32 Y = PenY - ((Selected[i] / TilesPerU) - BaseY);

				// Test bounds.
				if( ( X>=0 )&&( Y>=0 )&&( X<MapXSize )&&( Y<MapYSize ) )
				{
					Int32 iTile = Selected[i];

					if( iTile )
					{
						Tile.TexCoords		= AtlasTable[iTile];
						Tile.Bounds.Min.x	= (X + 0.f)*TileSize.x + Location.x;
						Tile.Bounds.Min.y	= (Y + 0.f)*TileSize.y + Location.y;
						Tile.Bounds.Max.x	= (X + 1.f)*TileSize.x + Location.x;
						Tile.Bounds.Max.y	= (Y + 1.f)*TileSize.y + Location.y;
						Canvas->DrawRect(Tile);
					}
				}
			}
		}
	}
}


//
// Model import.
//
void FModelComponent::Import( CImporterBase& Im )
{
	FBaseComponent::Import(Im);

	Map.setSize( MapXSize*MapYSize );
	for( Int32 iTile=0; iTile<Map.size(); iTile++ )
		Map[iTile] = Im.ImportInteger( *String::Format( L"Map[%i]", iTile ) );
}


//
// Model export.
//
void FModelComponent::Export( CExporterBase& Ex )
{
	FBaseComponent::Export(Ex);

	// Store all tiles, without blank(0 index),
	// it's safe about 70% of memory.
	for( Int32 iTile=0; iTile<Map.size(); iTile++ )
		if( Map[iTile] )
			Ex.ExportInteger( *String::Format( L"Map[%i]", iTile ), Map[iTile] );
}


//
// Model serialization.
//
void FModelComponent::SerializeThis( CSerializer& S )
{
	FBaseComponent::SerializeThis( S );
	
	Serialize( S, bUnlit );
	Serialize( S, Color );
	Serialize( S, Texture );

	Serialize( S, Map );
	Serialize( S, MapXSize );
	Serialize( S, MapYSize );
	Serialize( S, TileSize );
	Serialize( S, TilesPerU );
	Serialize( S, TilesPerV );
}


//
// Initialize model.
//
void FModelComponent::InitForEntity( FEntity* InEntity )
{
	FBaseComponent::InitForEntity( InEntity );
}


//
// Get tile index at specified matrix location.
//
void FModelComponent::nativeGetTile( CFrame& Frame )
{
	Int32	X	= POP_INTEGER,
			Y	= POP_INTEGER;

	if	( 
			X >= 0 && X < MapXSize &&
			Y >= 0 && Y < MapYSize
		)
	{
		*POPA_INTEGER	= Map[X+Y*MapXSize];
	}
	else
		*POPA_INTEGER	= -1;
}


//
// Set tile index at specified location.
//
void FModelComponent::nativeSetTile( CFrame& Frame )
{
	Int32	X		= POP_INTEGER,
			Y		= POP_INTEGER,
			iTile	= POP_INTEGER;

	if	( 
			X >= 0 && X < MapXSize &&
			Y >= 0 && Y < MapYSize
		)
	{
		Map[X+Y*MapXSize] = iTile;
	}
}


//
// Convert world point to index in tile map.
//
void FModelComponent::nativeWorldToMap( CFrame& Frame )
{
	math::Vector	V	= POP_VECTOR;
	*POPA_INTEGER	= WorldToMapIndex( V.x, V.y );
}


/*-----------------------------------------------------------------------------
    Registration.
-----------------------------------------------------------------------------*/

REGISTER_CLASS_CPP( FModelComponent, FBaseComponent, CLASS_None )
{
	ADD_PROPERTY( MapXSize, PROP_Editable );
	ADD_PROPERTY( MapYSize, PROP_Editable );
	ADD_PROPERTY( TileSize, PROP_Editable );
	ADD_PROPERTY( TilesPerU, PROP_Editable );
	ADD_PROPERTY( TilesPerV, PROP_Editable );
	ADD_PROPERTY( bUnlit, PROP_Editable );
	ADD_PROPERTY( Texture, PROP_Editable );
	ADD_PROPERTY( Color, PROP_Editable );

	DECLARE_METHOD( GetTile, TYPE_Integer, ARG(x, TYPE_Integer, ARG(y, TYPE_Integer, END)) );
	DECLARE_METHOD( SetTile, TYPE_None, ARG(x, TYPE_Integer, ARG(y, TYPE_Integer, ARG(iTile, TYPE_Integer, END))) );
	DECLARE_METHOD( WorldToMap, TYPE_Integer, ARG(worldPoint, TYPE_Vector, END) );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/