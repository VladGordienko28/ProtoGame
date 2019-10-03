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
		m_font( nullptr )
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
/*
	// General info.
	Int32 NumBitmaps = Bitmaps.size();
	EXPORT_INTEGER( Height );
	EXPORT_INTEGER( NumBitmaps );
	for( Int32 i=0; i<NumBitmaps; i++ )
		Ex.ExportObject( *String::format(L"Bitmaps[%i]", i), Bitmaps[i] );

	// Remap table.
	Int32 NumRemap = Remap.size();
	EXPORT_INTEGER( NumRemap );
	for( Int32 i=0; i<NumRemap; i++ )
		Ex.ExportByte( *String::format(L"Remap[%i]", i), Remap[i] );

	// All glyphs.
	Int32 NumGlyphs = Glyphs.size();
	EXPORT_INTEGER( NumGlyphs );
	for( Int32 i=0; i<NumGlyphs; i++ )
	{
		Ex.ExportInteger( *String::format( L"Glyphs[%d].A", i ), *(Int32*)(&Glyphs[i].iBitmap) );
		Ex.ExportInteger( *String::format( L"Glyphs[%d].B", i ), *(Int32*)(&Glyphs[i].X) );
	}*/
}	


//
// Import resource object.
//
void FFont::Import( CImporterBase& Im )
{
	FResource::Import(Im);

	String fontName;
	IMPORT_STRING( fontName );

	m_font = res::ResourceManager::get<fnt::Font>( fontName, res::EFailPolicy::FATAL );

/*
	// General info.
	Int32 NumBitmaps;
	IMPORT_INTEGER( NumBitmaps );
	IMPORT_INTEGER( Height );
	Bitmaps.setSize( NumBitmaps );
	for( Int32 i=0; i<NumBitmaps; i++ )
		Bitmaps[i] = (FBitmap*)Im.ImportObject( *String::format(L"Bitmaps[%i]", i) );

	// Remap table.
	Int32 NumRemap;
	IMPORT_INTEGER( NumRemap );
	Remap.setSize( NumRemap );
	for( Int32 i=0; i<NumRemap; i++ )
		Remap[i] = Im.ImportByte( *String::format(L"Remap[%i]", i) );

	// All glyphs.
	Int32 NumGlyphs;
	IMPORT_INTEGER( NumGlyphs );
	Glyphs.setSize( NumGlyphs );
	for( Int32 i=0; i<NumGlyphs; i++ )
	{
		*(Int32*)(&Glyphs[i].iBitmap)	=	Im.ImportInteger( *String::format( L"Glyphs[%d].A", i ) );
		*(Int32*)(&Glyphs[i].X)		=	Im.ImportInteger( *String::format( L"Glyphs[%d].B", i ) );
	}*/
}

//
// Serialize font.
//
void FFont::SerializeThis( CSerializer& S )
{ 
	// Call parent.
	FResource::SerializeThis(S); 
/*
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
		Int32 MapSize;
		Serialize( S, MapSize );
		Remap.setSize( MapSize );

		if( MapSize > 0 )
		{
			// Mark each glyph as blank.
			mem::set
				(
					&Remap[0],
					MapSize*sizeof(UInt8),
					0xff
				);

			Int32 NumGlyphs;
			Serialize( S, NumGlyphs );

			// Restore each index.
			for( Int32 i=0; i<NumGlyphs; i++ )
			{
				Int32 iSlot;
				UInt8 b;
				Serialize( S, iSlot );
				Serialize( S, b );

				Remap[iSlot]	= b;
			}
		}
	}
	else if( S.GetMode() == SM_Save )
	{
		// Store remap table.
		Int32 MapSize	= Remap.size();
		Serialize( S, MapSize );

		// How many glyphs.
		Int32 NumGlyphs = 0;
		for( Int32 i=0; i<MapSize; i++ )
			if( Remap[i] != 0xff )
				NumGlyphs++;

		assert(NumGlyphs <= Glyphs.size());
		Serialize( S, NumGlyphs );

		// Store each used glyph.
		for( Int32 i=0; i<MapSize; i++ )
			if( Remap[i] != 0xff )
			{
				UInt8 b = Remap[i];

				// Store pair of value and it index.
				Serialize( S, i );
				Serialize( S, b );
			}
	}*/
}


//
// Return the width of given text in C-style, in
// pixels of course.
//
Int32 FFont::TextWidth( const Char* Text )
{
	Float width = 0.f;

	for( const Char* C=Text; *C; C++ )
	{
		const auto& glyph = GetGlyph(*C);
		width += glyph.width;
	}

	return width * m_font->getImage()->getUSize();
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
