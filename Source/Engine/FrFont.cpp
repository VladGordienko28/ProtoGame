/*=============================================================================
    FrFont.cpp: Font object.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    FFont implementation.
-----------------------------------------------------------------------------*/

//
// Font constructor.
//
FFont::FFont()
	:	FResource(),
		Bitmaps(),
		Height( 0 ),
		Glyphs(),
		Remap()
{
}

//
// Font destructor.
//
FFont::~FFont()
{
}


//
// Font after loading initialization.
//
void FFont::PostLoad()
{
	FResource::PostLoad();
}


//
// Export resource object.
//
void FFont::Export( CExporterBase& Ex )
{
	FResource::Export(Ex);

	// General info.
	Integer NumBitmaps = Bitmaps.Num();
	EXPORT_INTEGER( Height );
	EXPORT_INTEGER( NumBitmaps );
	for( Integer i=0; i<NumBitmaps; i++ )
		Ex.ExportObject( *String::Format(L"Bitmaps[%i]", i), Bitmaps[i] );

	// Remap table.
	Integer NumRemap = Remap.Num();
	EXPORT_INTEGER( NumRemap );
	for( Integer i=0; i<NumRemap; i++ )
		Ex.ExportByte( *String::Format(L"Remap[%i]", i), Remap[i] );

	// All glyphs.
	Integer NumGlyphs = Glyphs.Num();
	EXPORT_INTEGER( NumGlyphs );
	for( Integer i=0; i<NumGlyphs; i++ )
	{
		Ex.ExportInteger( *String::Format( L"Glyphs[%d].A", i ), *(Integer*)(&Glyphs[i].iBitmap) );
		Ex.ExportInteger( *String::Format( L"Glyphs[%d].B", i ), *(Integer*)(&Glyphs[i].X) );
	}
}	


//
// Import resource object.
//
void FFont::Import( CImporterBase& Im )
{
	FResource::Import(Im);

	// General info.
	Integer NumBitmaps;
	IMPORT_INTEGER( NumBitmaps );
	IMPORT_INTEGER( Height );
	Bitmaps.SetNum( NumBitmaps );
	for( Integer i=0; i<NumBitmaps; i++ )
		Bitmaps[i] = (FBitmap*)Im.ImportObject( *String::Format(L"Bitmaps[%i]", i) );

	// Remap table.
	Integer NumRemap;
	IMPORT_INTEGER( NumRemap );
	Remap.SetNum( NumRemap );
	for( Integer i=0; i<NumRemap; i++ )
		Remap[i] = Im.ImportByte( *String::Format(L"Remap[%i]", i) );

	// All glyphs.
	Integer NumGlyphs;
	IMPORT_INTEGER( NumGlyphs );
	Glyphs.SetNum( NumGlyphs );
	for( Integer i=0; i<NumGlyphs; i++ )
	{
		*(Integer*)(&Glyphs[i].iBitmap)	=	Im.ImportInteger( *String::Format( L"Glyphs[%d].A", i ) );
		*(Integer*)(&Glyphs[i].X)		=	Im.ImportInteger( *String::Format( L"Glyphs[%d].B", i ) );
	}
}


// Glyph serialization.
void Serialize( CSerializer& S, TGlyph& V )
{
	Serialize( S, V.iBitmap );
	Serialize( S, V.X );
	Serialize( S, V.Y );
	Serialize( S, V.W );
	Serialize( S, V.H );
}

//
// Serialize font.
//
void FFont::SerializeThis( CSerializer& S )
{ 
	// Call parent.
	FResource::SerializeThis(S); 

	// Serialize common variables.
	Serialize( S, Bitmaps );
	Serialize( S, Height );
	Serialize( S, Glyphs );

	// The remap table is often too huge for store/serialize,
	// so I'll apply non trivial method to store it, its uses 
	// about 4x less memory.
	if( S.GetMode() == SM_Load )
	{
		// Restore remap table.
		Integer MapSize;
		Serialize( S, MapSize );
		Remap.SetNum( MapSize );

		if( MapSize > 0 )
		{
			// Mark each glyph as blank.
			MemSet
				(
					&Remap[0],
					MapSize*sizeof(Byte),
					0xff
				);

			Integer NumGlyphs;
			Serialize( S, NumGlyphs );

			// Restore each index.
			for( Integer i=0; i<NumGlyphs; i++ )
			{
				Integer iSlot;
				Byte b;
				Serialize( S, iSlot );
				Serialize( S, b );

				Remap[iSlot]	= b;
			}
		}
	}
	else if( S.GetMode() == SM_Save )
	{
		// Store remap table.
		Integer MapSize	= Remap.Num();
		Serialize( S, MapSize );

		// How many glyphs.
		Integer NumGlyphs = 0;
		for( Integer i=0; i<MapSize; i++ )
			if( Remap[i] != 0xff )
				NumGlyphs++;

		assert(NumGlyphs <= Glyphs.Num());
		Serialize( S, NumGlyphs );

		// Store each used glyph.
		for( Integer i=0; i<MapSize; i++ )
			if( Remap[i] != 0xff )
			{
				Byte b = Remap[i];

				// Store pair of value and it index.
				Serialize( S, i );
				Serialize( S, b );
			}
	}
}


//
// Return the width of given text in C-style, in
// pixels of course.
//
Integer FFont::TextWidth( const Char* Text )
{
	Integer Width = 0;

	for( const Char* C=Text; *C; C++ )
	{
		TGlyph& Glyph = GetGlyph(*C);
		Width += Glyph.W;
	}

	return Width;
}


/*-----------------------------------------------------------------------------
    TStaticFont implementation.
-----------------------------------------------------------------------------*/

//
// GUI font constructor.
//
TStaticFont::TStaticFont()
	:	FFont()
{
}


//
// GUI font destructor.
//
TStaticFont::~TStaticFont()
{
	// Kill manually all pages.
	for( Integer i=0; i<Bitmaps.Num(); i++ )
		delete Bitmaps[i];
}


/*-----------------------------------------------------------------------------
    Registration.
-----------------------------------------------------------------------------*/

REGISTER_CLASS_CPP( FFont, FResource, CLASS_None )
{
	// No properties or methods here.
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/
