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
		MemCopy( Mem, (Byte*)Memory + Pos, Count );
		Pos	+= Count;
	}
	void SerializeRef( FObject*& Obj )
	{
		error(L"CResourceStream::SerializeRef");
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
	Bitmap->Data.SetNum( Bitmap->USize*Bitmap->VSize*sizeof(Byte) );

	// Load palette.
	RGBQUAD RawPalette[256];
	Stream.SerializeData( RawPalette, sizeof(RGBQUAD)*Info.biClrUsed );

	Bitmap->Palette.Allocate(Info.biClrUsed);
	for( Integer i=0; i<Info.biClrUsed; i++ )
	{
		TColor Col( RawPalette[i].rgbRed, RawPalette[i].rgbGreen, RawPalette[i].rgbBlue, 0xff );
		if( Col == MASK_COLOR )	Col.A	= 0x00;
		Bitmap->Palette.Colors[i]	= Col;
	}

	// Load data, don't forget V-flip.
	Byte* Data = (Byte*)Bitmap->GetData();
	for( Integer V=0; V<Bitmap->VSize; V++ )
		Stream.SerializeData
		(
			&Data[(Bitmap->VSize-1-V) * Bitmap->USize],
			Bitmap->USize * sizeof(Byte)
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

	AnsiChar* Text = (AnsiChar*)MemAlloc(Stream.TotalSize()+sizeof(AnsiChar));
	AnsiChar* Walk = Text, *End = Text + Stream.TotalSize(); 
	Stream.SerializeData( Text, Stream.TotalSize() );
	// Temporal workaround for FLU-50. Need to be redesigned.
	#define to_next { while( Walk<End && *Walk != '\n') Walk++; Walk++; }

	TStaticFont* Font = new TStaticFont();

	// Main line.
	AnsiChar Name[64];
	Integer Height;
	sscanf( Walk, "%d %s\n", &Height, Name );	
	to_next;

	// Read characters.
	Font->Height = -1; 
	Integer NumPages = 0;
	while( Walk < End )
	{
		Char C[2] = { 0, 0 };
		Integer X, Y, W, H, iBitmap;
		C[0] = *Walk++;

		if( (Word)C[0]+1 > Font->Remap.Num() )
			Font->Remap.SetNum( (Word)C[0]+1 );

		sscanf( Walk, "%d %d %d %d %d\n", &iBitmap, &X, &Y, &W, &H );
		to_next;

		NumPages = Max( NumPages, iBitmap+1 );

		TGlyph Glyph;
		Glyph.iBitmap	= iBitmap;
		Glyph.X			= X;
		Glyph.Y			= Y;
		Glyph.W			= W;
		Glyph.H			= H;	

		Font->Height			= Max( Font->Height, H );
		Font->Remap[(Word)C[0]] = Font->Glyphs.Push(Glyph);

#if 0
		log( L"Import Char: %s", C );
#endif
	}

	// Load page.
	assert(NumPages == 1);
	FBitmap* Page = LoadBitmapFromResource(hInstance, BitmapID);
	Page->BlendMode = BLEND_Translucent;
	Font->Bitmaps.Push( Page );

	MemFree(Text);
	#undef to_next

	// Convert palette from RGB to RGBA format with solid white color and alpha mask.
	for( Integer iPage=0; iPage<Font->Bitmaps.Num(); iPage++ )
	{
		FBitmap* Page = Font->Bitmaps[iPage];
		assert(Page->Format == BF_Palette8);

		for( Integer i=0; i<Page->Palette.Colors.Num(); i++ )
		{
			TColor Ent = Page->Palette.Colors[i];
			Page->Palette.Colors[i] = TColor( 0xff, 0xff, 0xff, Integer(Ent.R + Ent.G + Ent.B)/3 );
		}
	
		Page->BlendMode = BLEND_Alpha;
	}

	return Font;
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/