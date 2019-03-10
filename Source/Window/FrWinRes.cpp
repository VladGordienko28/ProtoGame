/*=============================================================================
	FrGUIRes.cpp: GUI resources loading.
	Copyright Jan.2017 Vlad Gordienko.
=============================================================================*/

#include "Window.h"

/*-----------------------------------------------------------------------------
	CResourceStream.
-----------------------------------------------------------------------------*/

//
// Stream to load resource.
//
class CResourceStream: public CSerializer
{
public:
	// CResourceStream interface.
	CResourceStream( HINSTANCE hInstance, LPCTSTR ResID, LPCTSTR ResType )
	{
		hResInfo	= FindResource( hInstance, ResID, ResType );
		assert(hResInfo != 0);
		hGlobal		= LoadResource( hInstance, hResInfo );
		assert(hGlobal != 0);

		Size		= SizeofResource( hInstance, hResInfo );
		Memory		= LockResource(hGlobal);
		Mode		= SM_Load;
		Pos			= 0;
	}
	~CResourceStream()
	{
		UnlockResource(hGlobal);
		FreeResource(hGlobal);
	}

	// CSerializer interface.
	void SerializeData( void* Mem, SizeT Count )
	{
		mem::copy( Mem, (UInt8*)Memory + Pos, Count );
		Pos	+= Count;
	}
	void SerializeRef( FObject*& Obj )
	{
		fatal(L"CResourceStream::SerializeRef");
	}
	SizeT TotalSize()
	{
		return Size;
	}
	void Seek( SizeT NewPos )
	{
		Pos	= NewPos;
	}
	SizeT Tell()
	{
		return Pos;
	}

private:
	// Internal.
	HRSRC				hResInfo;
	HGLOBAL				hGlobal;
	void*				Memory;
	SizeT				Size, Pos;
};


/*-----------------------------------------------------------------------------
    Bitmap loading.
-----------------------------------------------------------------------------*/

// 
// Load bitmap.
//
TStaticBitmap* LoadBitmapFromResource( HINSTANCE hInstance, LPCTSTR ResID )
{
	CResourceStream		Stream( hInstance, ResID, RT_BITMAP );

	BITMAPINFOHEADER	Info;
	Stream.SerializeData( &Info, sizeof(BITMAPINFOHEADER) );
	assert(Info.biBitCount==8);

	// Preinitialize bitmap.
	TStaticBitmap* Bitmap = new TStaticBitmap();
	Bitmap->Format		= BF_Palette8;
	Bitmap->USize		= Info.biWidth;
	Bitmap->VSize		= Info.biHeight;
	Bitmap->UBits		= IntLog2(Bitmap->USize);
	Bitmap->VBits		= IntLog2(Bitmap->VSize);
	Bitmap->Filter		= BFILTER_Nearest;
	Bitmap->BlendMode	= BLEND_Regular;
	Bitmap->RenderInfo	= -1;
	Bitmap->PanUSpeed	= 0.f;
	Bitmap->PanVSpeed	= 0.f;
	Bitmap->Saturation	= 1.f;
	Bitmap->AnimSpeed	= 0.f;
	Bitmap->bDynamic	= false;
	Bitmap->bRedrawn	= false;
	Bitmap->Data.setSize( Bitmap->USize*Bitmap->VSize*sizeof(UInt8) );

	// Load palette.
	RGBQUAD RawPalette[256];
	Stream.SerializeData( RawPalette, sizeof(RGBQUAD)*Info.biClrUsed );

	Bitmap->Palette.Allocate(Info.biClrUsed);
	for( Int32 i=0; i<Info.biClrUsed; i++ )
	{
		math::Color Col( RawPalette[i].rgbRed, RawPalette[i].rgbGreen, RawPalette[i].rgbBlue, 0xff );
		if( Col == MASK_COLOR )	Col.a	= 0x00;
		Bitmap->Palette.Colors[i]	= Col;
	}

	// Load data, don't forget V-flip.
	UInt8* Data = (UInt8*)Bitmap->GetData();
	for( Int32 V=0; V<Bitmap->VSize; V++ )
		Stream.SerializeData
		(
			&Data[(Bitmap->VSize-1-V) * Bitmap->USize],
			Bitmap->USize * sizeof(UInt8)
		);

	// Return it.
	return Bitmap;
}


/*-----------------------------------------------------------------------------
    Font loading.
-----------------------------------------------------------------------------*/

//
// Load font and it bitmap.
//
TStaticFont* LoadFontFromResource( HINSTANCE hInstance, LPCTSTR FontID, LPCTSTR BitmapID )
{
	CResourceStream		Stream( hInstance, FontID, L"FLUFONT" );

	AnsiChar* Text = (AnsiChar*)mem::alloc(Stream.TotalSize()+sizeof(AnsiChar));
	AnsiChar* Walk = Text, *End = Text + Stream.TotalSize(); 
	Stream.SerializeData( Text, Stream.TotalSize() );
	// Temporal workaround for FLU-50. Need to be redesigned.
	#define to_next { while( Walk<End && *Walk != '\n') Walk++; Walk++; }

	TStaticFont* Font = new TStaticFont();

	// Main line.
	AnsiChar Name[64];
	Int32 Height;
	sscanf( Walk, "%d %s\n", &Height, Name );	
	to_next;

	// Read characters.
	Font->Height = -1; 
	Int32 NumPages = 0;
	while( Walk < End )
	{
		Char C[2] = { 0, 0 };
		Int32 X, Y, W, H, iBitmap;
		C[0] = *Walk++;

		if( (UInt16)C[0]+1 > Font->Remap.size() )
			Font->Remap.setSize( (UInt16)C[0]+1 );

		sscanf( Walk, "%d %d %d %d %d\n", &iBitmap, &X, &Y, &W, &H );
		to_next;

		NumPages = max( NumPages, iBitmap+1 );

		TGlyph Glyph;
		Glyph.iBitmap	= iBitmap;
		Glyph.X			= X;
		Glyph.Y			= Y;
		Glyph.W			= W;
		Glyph.H			= H;	

		Font->Height			= max( Font->Height, H );
		Font->Remap[(UInt16)C[0]] = Font->Glyphs.push(Glyph);

#if 0
		log( L"Import Char: %s", C );
#endif
	}

	// Load page.
	assert(NumPages == 1);
	FBitmap* Page = LoadBitmapFromResource(hInstance, BitmapID);
	Page->BlendMode = BLEND_Translucent;
	Font->Bitmaps.push( Page );

	mem::free(Text);
	#undef to_next

	// Convert palette from RGB to RGBA format with solid white color and alpha mask.
	for( Int32 iPage=0; iPage<Font->Bitmaps.size(); iPage++ )
	{
		FBitmap* Page = Font->Bitmaps[iPage];
		assert(Page->Format == BF_Palette8);

		for( Int32 i=0; i<Page->Palette.Colors.size(); i++ )
		{
			math::Color Ent = Page->Palette.Colors[i];
			Page->Palette.Colors[i] = math::Color( 0xff, 0xff, 0xff, Int32(Ent.r + Ent.g + Ent.b)/3 );
		}
	
		Page->BlendMode = BLEND_Alpha;
	}

	return Font;
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/