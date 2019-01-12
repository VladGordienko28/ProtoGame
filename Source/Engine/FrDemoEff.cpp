/*=============================================================================
    FrDemoEff.cpp: Demo effects implementation.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    Default palettes.
-----------------------------------------------------------------------------*/

//
// A simple ramp palette.
//
void paletteSimple( TColor* Palette )
{
	for( Integer i=0; i<256; i++ )
	{
		Palette[i].R	= i;
		Palette[i].G	= i;
		Palette[i].B	= i;
		Palette[i].A	= 0xff;
	}
}


//
// A plasma rainbow palette.
//
void palettePlasma( TColor* Palette )
{
	for( Integer i=0; i<256; i++ )
	{
		Palette[i]		= TColor::HSLToRGB( ~i, 0xff, 0x80 );
		Palette[i].A	= 0xff;
	}
}


//
// A nice fire palette.
//
void paletteFire( TColor* Palette )
{
	for( Integer i=0; i<256; i++ )
	{
		Palette[i]		= TColor::HSLToRGB( i/0x03, 0xff, Min( 0xff, i*0x02 ) );
		Palette[i].A	= 0xff;
	}
}


//
// A temporal tech palette.
//
void paletteTech( TColor* Palette )
{
	for( Integer i=0; i<256; i++ )
	{
		Palette[i].R	= i >> 1;
		Palette[i].G	= i;
		Palette[i].B	= i >> 1;
		Palette[i].A	= 0xff;
	}
}


/*-----------------------------------------------------------------------------
    Utility.
-----------------------------------------------------------------------------*/

//
// Effects shared variables.
//
DWord		RndSeed = 0x28282828;
Integer		ShiftTab[256];
Byte		SineTab[256];
Byte		WaveTab[1536];


//
// XORShift fast random generator.
// Its amazing!
//
inline Byte RandomByte()
{
	RndSeed ^= ( RndSeed << 13 );
	RndSeed	^= ( RndSeed >> 4 );
	RndSeed	^= ( RndSeed >> 7 );
	return RndSeed;
}


//
// Initialize tables.
//
void InitTables()
{
	// -1..1 shift table.
	// It's affect fire diffusion, make it
	// more stable, i.e. set more zeros than 1 or -1.
	for( Integer i=0; i<256; i++ )
		ShiftTab[i] = RandomByte() < 200 ? 0 : RandomByte() > 128 ? 1 : -1;

	// Sine table.
	for( Integer i=0; i<256; i++ )
		SineTab[i] = Sin(i/256.f*2.f*PI)*127.f+128;

	// Wave table.
	for( Integer i=0; i<1536; i++ )
	{
		Integer W = i / 2 - 256;
		if( (i-512) < 256 ) 
			W++;
		WaveTab[i] = Clamp( W, 0, 255 );
	}
}


//
// Macro.
//
#define put_fire_pix( x, y, heat )  Data[((x) & UMask)+(((y) & VMask)<<UBits)] = (heat);
#define put_tech_pix( x, y, depth ) ZBuffer[((x) & UMask)+(((y) & VMask)<<UBits)] = (depth);
#define put_wat_pixa( x, y, depth )	WaterA[((x) & U2Mask)+(((y) & V2Mask)<<UBits)] = (depth);
#define put_wat_pixb( x, y, depth )	WaterB[((x) & U2Mask)+(((y) & V2Mask)<<UBits)] = (depth);


//
// Saved registers for assembler.
//
DWord	SavedESP;
DWord	SavedEBP;


/*-----------------------------------------------------------------------------
    FDemoBitmap implementation.
-----------------------------------------------------------------------------*/

//
// Demo bitmap constructor.
//
FDemoBitmap::FDemoBitmap()
	:	FBitmap(),
		EffectPtr( nullptr ),
		UMask( 0 ),
		VMask( 0 ),
		Phase( 0 ),
		PaletteRef( nullptr )
{
	// Not really proper place for initialization.
	static Bool TabInit = false;
	if( !TabInit )
	{
		InitTables();
		TabInit	= true;
	}
}


//
// Demo bitmap destructor.
//
FDemoBitmap::~FDemoBitmap()
{
	if( EffectPtr )
		MemFree( EffectPtr );
}


//
// Initialize dynamic bitmap.
//
void FDemoBitmap::Init( Integer InU, Integer InV )
{
	assert(GIsEditor);

	// Initialize FBitmap fields.
	USize				= InU;
	VSize				= InV;
	UBits				= IntLog2(USize);
	VBits				= IntLog2(VSize);
	RenderInfo			= -1;
	PanUSpeed			= 0.f;
	PanVSpeed			= 0.f;
	Saturation			= 1.f;
	bDynamic			= true;
	BlendMode			= BLEND_Regular;
	Filter				= BFILTER_Bilinear;
	Format				= BF_Palette8;
	bRedrawn			= false;

	// FDemoBitmap fields.
	UMask				= USize-1;
	VMask				= VSize-1;
	Phase				= 0;

	// Simple palette.
	Palette.Allocate( 256 );
	paletteSimple( &Palette.Colors[0] );

	// Allocate demo bitmap data.
	EffectPtr			= (Byte*)MemAlloc( USize * VSize );
}


//
// Process effect after field edit.
//
void FDemoBitmap::EditChange()
{
	FBitmap::EditChange();

	// Validate palette.
	if( PaletteRef )
		if( PaletteRef->USize * PaletteRef->VSize < 256 )
			PaletteRef	= nullptr;

	// Copy data from palette.
	if( PaletteRef )
	{
		if( PaletteRef->Format == BF_Palette8 )
		{
			// Copy via other palette.
			Byte* Inds = (Byte*)PaletteRef->GetData();
			TColor* Pal	= (TColor*)&PaletteRef->Palette.Colors[0];

			for( Integer i=0; i<256; i++ )
				Palette.Colors[i] = Pal[Inds[i]];
		}
		else
		{
			// Copy directly from other's data.
			TColor* Other = (TColor*)PaletteRef->GetData();

			for( Integer i=0; i<256; i++ )
				Palette.Colors[i] = Other[i];
		}
	}
}


//
// Restore some fields after loading.
//
void FDemoBitmap::PostLoad()
{
	FBitmap::PostLoad();

	UMask			= USize - 1;
	VMask			= VSize - 1;
	Phase			= 0;
	bDynamic		= true;

	// Allocate data.
	EffectPtr		= (Byte*)MemAlloc( USize * VSize );
}


//
// Effect bitmap serialization.
//
void FDemoBitmap::SerializeThis( CSerializer& S )
{
	FBitmap::SerializeThis( S );
	Serialize( S, PaletteRef );
}


//
// Return pointer to the dynamic bitmap data.
//
void* FDemoBitmap::GetData()
{
	return EffectPtr;
}


//
// Return the size of data block.
//
SizeT FDemoBitmap::GetBlockSize()
{
	// 8-Bits image.
	return USize*VSize*sizeof(Byte);
}


/*-----------------------------------------------------------------------------
    FPlasmaBitmap implementation.
-----------------------------------------------------------------------------*/

//
// Plasma constructor.
//
FPlasmaBitmap::FPlasmaBitmap()
	:	FDemoBitmap(),
		HeatBuffer( nullptr )
{
}


//
// Plasma destructor.
//
FPlasmaBitmap::~FPlasmaBitmap()
{
	if( HeatBuffer )
		delete[] HeatBuffer;
}


//
// Plasma initialization.
//
void FPlasmaBitmap::Init( Integer InU, Integer InV )
{
	FDemoBitmap::Init( InU, InV );

	AnimSpeed		= 10.f;
	PlasmaA			= 2.f;
	PlasmaB			= 1.f;
	PlasmaC			= 1.f;
	PlasmaD			= 0.5f;
	PaletteShift	= 0;

	SetPlasmaTable();
	palettePlasma( &Palette.Colors[0] );

	// Allocate plasma heat buffer.
	HeatBuffer		= new Byte[USize*VSize >> 2];
}


//
// Compute a plasma heat buffer.
//
void FPlasmaBitmap::CalculatePlasma()
{
	Integer U2Size	= USize >> 1;
	Integer V2Size	= VSize >> 1;
	Integer U2Bits	= UBits - 1;
	Integer A, B, C, D;

	for( Integer V=0; V<V2Size; V++ )
	{
		Byte*	HeatLine	= &HeatBuffer[V << U2Bits];

		B		= PlasmaTable[1][(V-Phase) & 0xff];

		for( Integer U=0; U<U2Size; U++ )
		{
			A	= PlasmaTable[0][(U-Phase) & 0xff];
			C	= PlasmaTable[2][((U+V)+Phase) & 0xff];
			D	= PlasmaTable[3][(SineTab[(U*2+Phase) & 0xff]+SineTab[(V*2+Phase) & 0xff]) & 0xff];

			HeatLine[U]		= A + B + C + D;
		}
	}
}


//
// Render the plasma.
//
void FPlasmaBitmap::RenderPlasma()
{
	Byte*	Data		= (Byte*)GetData();
	Integer U2Size		= USize >> 1;
	Integer V2Size		= VSize >> 1;
	Integer U2Bits		= UBits - 1;
	Integer U2Mask		= UMask >> 1;
	Integer V2Mask		= VMask >> 1;

	for( Integer V=0; V<V2Size; V++ )
	{
		// Get lines and apply some phase panning.
		Byte*	HeatLine	= &HeatBuffer[((V+0+Phase) & V2Mask) << U2Bits];
		Byte*	NextLine	= &HeatBuffer[((V+1+Phase) & V2Mask) << U2Bits];
		Byte*	Data0Line	= &Data[V*2+0 << UBits];
		Byte*	Data1Line	= &Data[V*2+1 << UBits];

		for( Integer U=0; U<U2Size; U++ )
		{
			// Sample heat values.
			Integer A	= HeatLine[(U+HeatLine[(U+0)&U2Mask]+0) & U2Mask];
			Integer B	= HeatLine[(U+HeatLine[(U+1)&U2Mask]+1) & U2Mask];
			Integer C	= NextLine[(U+NextLine[(U+0)&U2Mask]+0) & U2Mask];
			Integer D	= NextLine[(U+NextLine[(U+1)&U2Mask]+1) & U2Mask];
			
			// Super sampling.
			Data0Line[U*2+0]	= A;
			Data0Line[U*2+1]	= (A+B) >> 1;
			Data1Line[U*2+0]	= (A+C) >> 1;
			Data1Line[U*2+1]	= (A+B+C+D) >> 2;
		}
	}
}


//
// Mouse click on plasma.
//
void FPlasmaBitmap::MouseClick( Integer Button, Integer X, Integer Y )
{
	if( Button == 1 )
	{
		// Set some "random" phase.
		Phase	= X + Y;
	}
}


//
// Mouse move plasma.
//
void FPlasmaBitmap::MouseMove( Integer Button, Integer X, Integer Y )
{
	if( Button == 1 )
	{
		// Apply palette shift.
		Integer Delta	= X - (Integer)PaletteShift;

		// Figure out another way to shift palette.
		// This way sucks!
		if( Delta > 0 )
		{
			// Shift palette leftward.
			while( Delta-- > 0 )
			{
				TColor C	= Palette.Colors[0];

				for( Integer i=0; i<255; i++ )
					Palette.Colors[i] = Palette.Colors[i+1];

				Palette.Colors[255] = C;
			}
		}
		else
		{
			// Shift palette rightward.
			while( Delta++ < 0 )
			{
				TColor C	= Palette.Colors[255];

				for( Integer i=254; i>=0; i-- )
					Palette.Colors[i+1]	= Palette.Colors[i];

				Palette.Colors[0]	= C;
			}
		}
	}

	PaletteShift	= X;
}


//
// Some field has been changed.
//
void FPlasmaBitmap::EditChange()
{ 
	FDemoBitmap::EditChange(); 
	
	// Refresh plasma tables.
	SetPlasmaTable();
}


//
// Redraw plasma bitmap.
//
void FPlasmaBitmap::Redraw()
{
	++Phase;
	CalculatePlasma();
	RenderPlasma();

	// Mark bitmap as redrawn.
	bRedrawn	= true;
}


//
// Compute plasma tables.
//
void FPlasmaBitmap::SetPlasmaTable()
{
	for( Integer i=0; i<256; i++ )
	{
		PlasmaTable[0][i]	= Round( Sin(i*(PlasmaA*6.283f/USize*2.f))*31.5f + 32.f );
		PlasmaTable[1][i]	= Round( Sin(i*(PlasmaB*6.283f/USize*2.f))*31.5f + 32.f );
		PlasmaTable[2][i]	= Round( Sin(i*(PlasmaC*6.283f/USize*2.f))*31.5f + 32.f );
		PlasmaTable[3][i]	= Round( Sin(i*(PlasmaD*6.283f/USize*2.f))*31.5f + 32.f );
	}
}


//
// Plasma after loading initialization.
//
void FPlasmaBitmap::PostLoad()
{
	FDemoBitmap::PostLoad();
	SetPlasmaTable();
	PaletteShift	= 0;
	HeatBuffer		= new Byte[USize*VSize >> 2];
}


//
// Plasma serialization.
//
void FPlasmaBitmap::SerializeThis( CSerializer& S )
{
	FDemoBitmap::SerializeThis( S );

	Serialize( S, PlasmaA );
	Serialize( S, PlasmaB );
	Serialize( S, PlasmaC );
	Serialize( S, PlasmaD );
}


//
// Reset the plasma.
//
void FPlasmaBitmap::Erase()
{
	Phase = Random( 0x7fffffff );
}


/*-----------------------------------------------------------------------------
    FFireBitmap implementation.
-----------------------------------------------------------------------------*/

//
// Fire constructor.
//
FFireBitmap::FFireBitmap()
	:	FDemoBitmap()
{
}


//
// Fire initialization.
//
void FFireBitmap::Init( Integer InU, Integer InV )
{
	FDemoBitmap::Init( InU, InV );

	// Default parameters.
	bRising			= false;
	FireHeat		= 220;
	AnimSpeed		= 10.f;
	NumSparks		= 0;
	SetFireTable();

	// Fire pal.
	paletteFire( &Palette.Colors[0] );
}


//
// Redraw the fire.
//
void FFireBitmap::Redraw()
{
	Phase++;

	// Render sparks.
	RedrawSparks();

	// Apply fire filter.
	if( bRising )
		RenderRisingFire();
	else 
		RenderStillFire();

	// Force to refresh.
	bRedrawn	= true;
}


//
// Serialize fire bitmap.
//
void FFireBitmap::SerializeThis( CSerializer& S )
{
	FDemoBitmap::SerializeThis( S );

	Serialize( S, bRising );
	Serialize( S, FireHeat );

	if( S.GetMode() == SM_Save )
	{
		// Save only long-lived sparks.
		for( Integer i=0; i<NumSparks; )
		{
			if( Sparks[i].Type >= _SPARK_Particle )
			{
				Sparks[i] = Sparks[NumSparks--];
			}
			else
				i++;
		}

		Serialize( S, NumSparks );
		S.SerializeData( Sparks, sizeof(TSpark)*NumSparks );
	}
	else
	{
		// Load or count.
		Serialize( S, NumSparks );
		S.SerializeData( Sparks, sizeof(TSpark)*NumSparks );
	}
}


//
// Import the fire bitmap.
//
void FFireBitmap::Import( CImporterBase& Im )
{
	FDemoBitmap::Import( Im );
	IMPORT_INTEGER( NumSparks );

	// Import each spark as two integers.
	for( Integer i=0; i<NumSparks; i++ )
	{
		*(Integer*)(&Sparks[i].Type)	=	Im.ImportInteger( *String::Format( L"Sparks[%d].A", i ) );
		*(Integer*)(&Sparks[i].ParamA)	=	Im.ImportInteger( *String::Format( L"Sparks[%d].B", i ) );
	}
}		


//
// Export the fire bitmap.
//
void FFireBitmap::Export( CExporterBase& Ex )
{
	FDemoBitmap::Export( Ex );

	// Save only long-lived sparks.
	for( Integer i=0; i<NumSparks; )
	{
		if( Sparks[i].Type >= _SPARK_Particle )
		{
			Sparks[i] = Sparks[NumSparks--];
		}
		else
			i++;
	}

	EXPORT_INTEGER( NumSparks );

	// Export each spark as two integers.
	for( Integer i=0; i<NumSparks; i++ )
	{
		Ex.ExportInteger( *String::Format( L"Sparks[%d].A", i ), *(Integer*)(&Sparks[i].Type) );
		Ex.ExportInteger( *String::Format( L"Sparks[%d].B", i ), *(Integer*)(&Sparks[i].ParamA) );
	}
}


//
// Initialize fire after loading.
//
void FFireBitmap::PostLoad()
{
	FDemoBitmap::PostLoad();
	SetFireTable();
}


//
// When some field changed in fire.
//
void FFireBitmap::EditChange()
{
	FDemoBitmap::EditChange();  
	
	// Refresh fire algorithm tables.
	SetFireTable(); 
}


//
// Delete sparks near cursor.
//
void FFireBitmap::DeleteSparks( Integer X, Integer Y, Integer Size )
{
	if( (X<0)||(Y<0)||(X>=USize)||(Y>=VSize) )
		return;

	for( Integer i=0; i<NumSparks; )
	{
		if( Abs(X-Sparks[i].X)<=Size && Abs(Y-Sparks[i].Y)<=Size )
		{
			Sparks[i] = Sparks[NumSparks--];
		}
		else
			i++;
	}
}


//
// Render rising fire.
//
void FFireBitmap::RenderRisingFire()
{
	Byte*	Data	= (Byte*)GetData();
	Byte	Seed	= Phase;

#if 0
	// Naive algorithm.
	for( Integer V=0; V<VSize; V++ )
		for( Integer U=0; U<USize; U++ )
		{
			Integer Value = FireTable[ Data[U+(((V+2)&VMask)<<UBits)]+Data[U+(((V+1)&VMask)<<UBits)] ];

			if( Value )
			{
				Integer Offset = ShiftTab[++Seed];
				Data[ ((U+Offset)&UMask) +(V<<UBits)] = Value;
			}
			else
			{
				Data[U+(V << UBits)]	= 0;
			}
		}
#else
	// Cool algorithm.
	Byte*	ThisLine	= &Data[(VSize-2) << UBits];
	Byte*	NextLine	= &Data[(VSize-1) << UBits];

	for( Integer V=0; V<VSize; V++ )
	{
		Byte*	NextLine2	= &Data[V << UBits];

		for( Integer U=0; U<USize; U++ )
		{
			Byte	Value	= FireTable[NextLine[U]+NextLine2[U]];
			if( Value )
			{
				Integer Bias = ShiftTab[++Seed];
				ThisLine[(U+Bias) & UMask]	= Value;
			}
			else
			{
				ThisLine[U]	= 0;
			}
		}

		ThisLine	= NextLine;
		NextLine	= NextLine2;
	}
#endif
}


//
// Render still fire.
//
void FFireBitmap::RenderStillFire()
{
	Byte*	Data	= (Byte*)GetData();
	Byte	Seed	= Phase;

#if 0
	// Naive algo.
	for( Integer V=0; V<VSize; V++ )
		for( Integer U=0; U<USize; U++ )
		{
			Integer Value = FireTable[ Data[U+(V << UBits)]+Data[U+(((V+1)&VMask)<<UBits)] ];

			if( Value )
			{
				Integer Offset = ShiftTab[++Seed];
				Data[ ((U+Offset)&UMask) +(V<<UBits)] = Value;
			}
			else
			{
				Data[U+(V << UBits)]	= 0;
			}
		}
#else
	// Improved algorithm.
	Byte*	ThisLine	= &Data[(VSize-1) << UBits];
	for( Integer V=0; V<VSize; V++ )
	{
		Byte*	NextLine	= &Data[V << UBits];

		for( Integer U=0; U<USize; U++ )
		{
			Byte Value = FireTable[ThisLine[U]+NextLine[U]];
			if( Value )
			{
				Integer Bias = ShiftTab[++Seed];
				ThisLine[(U+Bias) & UMask]	= Value;
			}
			else
			{
				ThisLine[U]	= 0;
			}
		}

		ThisLine	= NextLine;
	}
#endif
}


//
// Draw a lighting bolt.
//
void FFireBitmap::DrawLighting( Integer X1, Integer Y1, Integer X2, Integer Y2, Byte Heat1, Byte Heat2 )
{
	// Don't draw collapsed.
	if( X1 == X2 && Y1 == Y2 )
		return;

	// Preinitialize.
	// Used some fixed math magic in 24:8 format.
	Byte*	Data		= (Byte*)GetData();
	Integer	Length		= Trunc(FastSqrt(Sqr<Float>(X1-X2)+Sqr<Float>(Y1-Y2))*256.f);
	Integer	LastX		= X1 << 8;
	Integer LastY		= Y1 << 8;
	Integer	FromX		= X1 << 8;
	Integer FromY		= Y1 << 8;
	Integer	DestX		= X2 << 8;
	Integer DestY		= Y2 << 8;
	Integer Dx			= ((DestX-LastX) << 8) / Length;
	Integer Dy			= ((DestY-LastY) << 8) / Length;
	Integer Nx			= -Dy;
	Integer Ny			= +Dx;
	Integer Walk		= 0; 
	Integer Heat		= Heat1 << 8;
	Integer Dh			= ((Heat2-Heat1) << 8) / (Length>>8);

	while( Walk < Length )
	{
		Integer X, Y;
		Walk += (256*5) + (RandomByte()*256*4 >> 8);

		if( Walk > Length )
		{
			// Go to the end point.
			X	= DestX;
			Y	= DestY;
		}
		else
		{
			// Make some jag.
			Integer Jag	= (RandomByte()*10) - (5*256);

			X	= FromX + (Dx*Walk >> 8) + (Nx*Jag >> 8);
			Y	= FromY + (Dy*Walk >> 8) + (Ny*Jag >> 8);
		}

		Integer SegX	= X-LastX;
		Integer SegY	= Y-LastY;
		Integer Seg		= Max( Abs(SegX), Abs(SegY) );
		Integer i		= Seg >> 8;

		SegX	= (SegX << 8) / Seg;
		SegY	= (SegY << 8) / Seg;

		while( i > 0 )
		{
			Data[(LastX >> 8 & UMask)+((LastY >> 8 & VMask)<<UBits)] = Heat >> 8;
			LastX	+= SegX;
			LastY	+= SegY;
			Heat	+= Dh;
			--i;
		}

		LastX	= X;
		LastY	= Y;
	}
}


//
// Advance spark location linear, XVel and YVel is a velocity.
// Speed is a probability of the move. 
//
inline void AdvanceSpark(	
							FFireBitmap::TSpark& Spark, 
							Integer XVel, 
							Integer YVel, 
							Integer UMask, 
							Integer VMask 
						)
{
	if( XVel < 0 )
	{
		if( (RandomByte() & 127) < -XVel )
			Spark.X = (Spark.X - 1) & UMask;
	}
	else
	{
		if( (RandomByte() & 127) < +XVel )
			Spark.X = (Spark.X + 1) & UMask;
	}

	if( YVel < 0 )
	{
		if( (RandomByte() & 127) < -YVel )
			Spark.Y = (Spark.Y - 1) & VMask;
	}
	else
	{
		if( (RandomByte() & 127) < +YVel )
			Spark.Y = (Spark.Y + 1) & VMask;
	}
}


//
// Redraw all sparks.
//
void FFireBitmap::RedrawSparks()
{
	Byte* Data = (Byte*)GetData();

	for( Integer i=0; i<NumSparks; i++ )
	{
		TSpark& Spark = Sparks[i];

		switch( Spark.Type )
		{
			case SPARK_Point:
			{
				// Steady fire point.
				put_fire_pix( Spark.X, Spark.Y, Spark.Heat );
				break;
			}
			case SPARK_RandomPoint:
			{
				// Random fire point.
				put_fire_pix( Spark.X, Spark.Y, Spark.Heat*RandomByte() >> 8 );
				break;
			}
			case SPARK_Phase:
			{
				// Phased.
				put_fire_pix( Spark.X, Spark.Y, Spark.Heat );
				Spark.Heat += Spark.ParamA;
				break;
			}
			case SPARK_Jitter:
			{
				// Vertical jitter.
				Byte	V = SineTab[Spark.ParamC];
				put_fire_pix( Spark.X, Spark.Y+((V*Spark.ParamA)>>8), Spark.Heat );
				Spark.ParamC += Spark.ParamB;
				break;
			}
			case SPARK_Twister:
			{
				// Horizontal jitter.
				Byte	V = SineTab[Spark.ParamC];
				put_fire_pix( Spark.X+((V*Spark.ParamA)>>8), Spark.Y, Spark.Heat );
				Spark.ParamC += Spark.ParamB;
				break;
			}
			case SPARK_Fireball:
			{
				// Simple fireball emitter.
				if( NumSparks<MAX_FIRE_SPARKS && RandomByte()<128 )
				{
					Integer k = NumSparks++;
					TSpark& Other = Sparks[k];

					Other.Type		= _SPARK_Particle;
					Other.Heat		= Spark.Heat;
					Other.X			= Spark.X;
					Other.Y			= Spark.Y;
					Other.ParamA	= RandomByte();
					Other.ParamB	= RandomByte();
					Other.ParamC	= 255;
					Other.ParamD	= 5;
				}
				break;
			}
			case SPARK_JetRightward:
			{
				// Jet to right.
				if( NumSparks<MAX_FIRE_SPARKS && RandomByte()<64 )
				{
					Integer k = NumSparks++;
					TSpark& Other = Sparks[k];

					Other.Type		= _SPARK_Particle;
					Other.Heat		= Spark.Heat;
					Other.X			= Spark.X;
					Other.Y			= Spark.Y;
					Other.ParamA	= 190 + (RandomByte() & 63);
					Other.ParamB	= 160;
					Other.ParamC	= Spark.ParamA;
					Other.ParamD	= 1;
				}
				break;
			}
			case SPARK_JetLeftward:
			{
				// Jet to left.
				if( NumSparks<MAX_FIRE_SPARKS && RandomByte()<64 )
				{
					Integer k = NumSparks++;
					TSpark& Other = Sparks[k];

					Other.Type		= _SPARK_Particle;
					Other.Heat		= Spark.Heat;
					Other.X			= Spark.X;
					Other.Y			= Spark.Y;
					Other.ParamA	= (RandomByte() & 63);
					Other.ParamB	= 160;
					Other.ParamC	= Spark.ParamA;
					Other.ParamD	= 1;
				}
				break;
			}
			case SPARK_JetUpward:
			{
				// Jet to up.
				if( NumSparks<MAX_FIRE_SPARKS && RandomByte()<64 )
				{
					Integer k = NumSparks++;
					TSpark& Other = Sparks[k];

					Other.Type		= _SPARK_Particle;
					Other.Heat		= Spark.Heat;
					Other.X			= Spark.X;
					Other.Y			= Spark.Y;
					Other.ParamA	= 96 + (RandomByte() & 63);
					Other.ParamB	= 63;
					Other.ParamC	= Spark.ParamA;
					Other.ParamD	= 2;
				}
				break;
			}
			case SPARK_JetDownward:
			{
				// Jet to down.
				if( NumSparks<MAX_FIRE_SPARKS && RandomByte()<64 )
				{
					Integer k = NumSparks++;
					TSpark& Other = Sparks[k];

					Other.Type		= _SPARK_Particle;
					Other.Heat		= Spark.Heat;
					Other.X			= Spark.X;
					Other.Y			= Spark.Y;
					Other.ParamA	= 96 + (RandomByte() & 63);
					Other.ParamB	= 191;
					Other.ParamC	= Spark.ParamA;
					Other.ParamD	= 2;
				}
				break;
			}
			case SPARK_Spermatozoa:
			{
				// Spawn some spermatozoids.
				if( NumSparks<MAX_FIRE_SPARKS && RandomByte()<15 )
				{
					Integer k = NumSparks++;
					TSpark& Other = Sparks[k];

					Other.Type		= _SPARK_Particle;
					Other.Heat		= Spark.Heat;
					Other.X			= (Spark.X+(RandomByte() & 0x1f)) & UMask;
					Other.Y			= (Spark.Y+(RandomByte() & 0x1f)) & VMask;
					Other.ParamA	= RandomByte();
					Other.ParamB	= RandomByte();
					Other.ParamC	= Spark.ParamA;
					Other.ParamD	= 0;
				}

				if( (RandomByte() & 0xf) == 0 )
					Spark.X	= UMask & (Spark.X+(RandomByte() & 0xf)-7);

				if( (RandomByte() & 0xf) == 0 )
					Spark.Y	= VMask & (Spark.Y+(RandomByte() & 0xf)-7);

				break;
			}
			case SPARK_Whirligig:
			{
				// Whirligig effect.
				if( NumSparks<MAX_FIRE_SPARKS )
				{
					Integer k = NumSparks++;
					TSpark& Other = Sparks[k];

					Other.Type		= _SPARK_Orbit;
					Other.Heat		= Spark.Heat;
					Other.X			= Spark.X;
					Other.Y			= Spark.Y;
					Other.ParamA	= Spark.ParamD;
					Other.ParamB	= 0;
					Other.ParamC	= Spark.ParamC;
					Other.ParamD	= Spark.ParamB;
				}

				Spark.ParamD += Spark.ParamA;
				break;
			}
			case SPARK_Cloud:
			{
				// Whirligig effect.
				if( NumSparks<MAX_FIRE_SPARKS )
				{
					Integer k = NumSparks++;
					TSpark& Other = Sparks[k];

					Other.Type		= _SPARK_Particle;
					Other.Heat		= 255;
					Other.X			= UMask & (Spark.X+((RandomByte()*Spark.ParamC) >> 8));
					Other.Y			= VMask & (Spark.Y+((RandomByte()*Spark.ParamC) >> 8));
					Other.ParamA	= Spark.ParamA;
					Other.ParamB	= Spark.ParamB;
					Other.ParamC	= Spark.ParamD;
					Other.ParamD	= 0;
				}
				break;
			}
			case SPARK_BallLighting:
			{
				// Ball lighting effect.
				if( RandomByte() > Spark.ParamA )
				{
					Integer Angle = RandomByte();
					DrawLighting
							( 
								Spark.X, 
								Spark.Y, 
								Spark.X+(SineTab[(Angle + 0)&0xff]*Spark.ParamB>>8)-(Spark.ParamB >> 1),
								Spark.Y+(SineTab[(Angle + 64)&0xff]*Spark.ParamB>>8)-(Spark.ParamB >> 1),
								Spark.Heat,
								Spark.Heat >> 3
							);
				}
				break;
			}
			case SPARK_RandomLighting:
			{
				// Random lighting effect.
				if( RandomByte() > Spark.ParamA )
				{
					DrawLighting
							(
								RandomByte() & UMask, 
								RandomByte() & VMask, 
								RandomByte() & UMask, 
								RandomByte() & VMask, 
								Spark.Heat,
								Spark.Heat
							);
				}
				break;
			}
			case SPARK_LineLighting:
			{
				// Line lighting effect.
				if( RandomByte() > Spark.ParamA )
				{
					DrawLighting
							( 
								Spark.X, 
								Spark.Y, 
								Spark.ParamC, 
								Spark.ParamD, 
								Spark.Heat,
								Spark.Heat
							);
				}
				break;
			}
			case SPARK_RampLighting:
			{
				// Line lighting effect.
				if( RandomByte() > Spark.ParamA )
				{
					DrawLighting
							( 
								Spark.X, 
								Spark.Y, 
								Spark.ParamC, 
								Spark.ParamD, 
								Spark.Heat,
								Spark.Heat >> 3
							);
				}
				break;
			}
			case _SPARK_Particle:
			{
				// A temporal particle.
				Spark.ParamC--;
				Spark.Heat -= Spark.ParamD;
				if( Spark.ParamC && Spark.Heat > Spark.ParamD )
				{
					put_fire_pix( Spark.X, Spark.Y, Spark.Heat );
					AdvanceSpark
							( 
								Spark, 
								Spark.ParamA-128, 
								Spark.ParamB-128, 
								UMask, 
								VMask 
							);
				}
				else
				{
					NumSparks--;
					Sparks[i--] = Sparks[NumSparks];
					continue;
				}
				break;
			}
			case _SPARK_Orbit:
			{
				// Orbit spin spark.
				Spark.ParamD--;
				if( Spark.ParamD < 255 )
				{
					put_fire_pix( Spark.X, Spark.Y, Spark.Heat );
					Spark.ParamA += Spark.ParamC;
					AdvanceSpark
							( 
								Spark, 
								SineTab[(Spark.ParamA+64) & 0xff]-128,
								SineTab[(Spark.ParamA) & 0xff]-128, 
								UMask, 
								VMask 
							);
				}
				else
				{
					NumSparks--;
					Sparks[i--] = Sparks[NumSparks];
					continue;
				}
				break;		
			}
		}
	}
}


//
// Add a new spark to the fire bitmap.
//
void FFireBitmap::AddSpark( Integer X, Integer Y )
{
	// Check out of bounds.
	if(	( X < 0 ) || 
		( Y < 0 ) || 
		( X >= USize ) || 
		( Y >= VSize ) || 
		( NumSparks >= MAX_FIRE_SPARKS-1 ) )
			return;

	Integer i = NumSparks++;
	TSpark& Spark = Sparks[i];

	// Common info.
	Spark.X			= X;
	Spark.Y			= Y;
	Spark.Type		= DrawParams.DrawType;
	Spark.Heat		= DrawParams.Heat;

	switch( Spark.Type )
	{
		case SPARK_Phase:
		{
			// Phased heat.
			Spark.Heat		= Phase;
			Spark.ParamA	= DrawParams.Frequency;
			break;
		}
		case SPARK_Jitter:
		case SPARK_Twister:
		{
			// Orbit sparks.
			Spark.ParamA	= DrawParams.Size;
			Spark.ParamB	= DrawParams.Speed;
			Spark.ParamC	= DrawParams.Frequency * Phase;
			break;
		}
		case SPARK_JetUpward:
		case SPARK_JetLeftward:
		case SPARK_JetRightward:
		case SPARK_JetDownward:
		{
			// Jets in all directions.
			Spark.ParamA	= DrawParams.Life;
			break;
		}
		case SPARK_Spermatozoa:
		{
			// Spermatozoids emitter.
			Spark.ParamA	= DrawParams.Life;
			break;
		}
		case SPARK_Whirligig:
		{
			// Whirligig effect.
			Spark.ParamA	= DrawParams.Frequency;
			Spark.ParamB	= DrawParams.Life;
			Spark.ParamC	= Max( 1, ~DrawParams.Area >> 5 );
			Spark.ParamD	= 0;
			break;
		}
		case SPARK_Cloud:
		{
			// Cloud of sparks.
			Integer XVel	= (SineTab[(DrawParams.Direction+64) & 0xff] * DrawParams.Speed) >> 8;
			Integer YVel	= (SineTab[(DrawParams.Direction   ) & 0xff] * DrawParams.Speed) >> 8;
			Spark.ParamA	= 128 - DrawParams.Speed/2 + XVel;
			Spark.ParamB	= 128 - DrawParams.Speed/2 + YVel;
			Spark.ParamC	= DrawParams.Area;
			Spark.ParamD	= DrawParams.Life;
			break;
		}
		case SPARK_BallLighting:
		case SPARK_RandomLighting:
		{
			// Simple lighting.
			Spark.ParamA	= DrawParams.Frequency;
			Spark.ParamB	= DrawParams.Size;
			break;
		}
		case SPARK_RampLighting:
		case SPARK_LineLighting:
		{
			// Lighting bolt.
			Spark.ParamA	= DrawParams.Frequency;
			Spark.ParamB	= 0;
			Spark.ParamC	= Spark.X ^ 3;
			Spark.ParamD	= Spark.Y ^ 3;
			break;
		}
	}
}


//
// Erase fire.
//
void FFireBitmap::Erase()
{
	NumSparks	= 0;
}


//
// Compute fire table.
//
void FFireBitmap::SetFireTable()
{
	for( Integer iHeat=0; iHeat<512; iHeat++ )
	{
		// Pretty strange formula, but it's works well.
		Float Value = ((Float)iHeat/2.f - 16.f * RandomF());
		Value		*= Lerp( 10.f/128.f, 150.f/128.f, (Float)FireHeat/255.f ); 
		FireTable[iHeat]	= Clamp( Round(Value), 0x00, 0xff );
	}
}


// Whether draw lighting bolt?
Bool	GCapFire	= false;


//
// User click at fire bitmap.
//
void FFireBitmap::MouseClick( Integer Button, Integer X, Integer Y )
{
	if( Button == 1 )
	{
		// Left button.
		if( GCapFire )
		{
			for( Integer i=NumSparks-1; i>=0; i-- )
				if( Sparks[i].Type == SPARK_LineLighting || 
					Sparks[i].Type == SPARK_RampLighting )
				{
					Sparks[i].ParamC	= X;
					Sparks[i].ParamD	= Y;
					break;
				}
			GCapFire	= false;
			return;
		}

		// Add new spark.
		AddSpark( X, Y );

		// Start draw lighting?
		if( (	DrawParams.DrawType == SPARK_LineLighting || 
				DrawParams.DrawType == SPARK_RampLighting) && 
			!GCapFire )
				GCapFire	= true;
	}
	else if( Button == 2 )
	{
		// Right button.
		DeleteSparks( X, Y, 10 );
		GCapFire	= false;
	}
}


//
// User move cursor at fire bitmap.
//
void FFireBitmap::MouseMove( Integer Button, Integer X, Integer Y )
{
	// Whether play with lighting bolt?
	if( GCapFire )
	{
		for( Integer i=NumSparks-1; i>=0; i-- )
			if( Sparks[i].Type == SPARK_LineLighting || 
				Sparks[i].Type == SPARK_RampLighting )
			{
				Sparks[i].ParamC	= X;
				Sparks[i].ParamD	= Y;
				break;
			}
		return;
	}

	if( Button == 1 )
	{
		// Left button.
		if( DrawParams.DrawType < SPARK_LineLighting  )
			AddSpark( X, Y );
	}
	else if( Button == 2 )
	{
		// Right button.
		DeleteSparks( X, Y, 10 );
		GCapFire	= false;
	}

	// Draw torch cursor.
	Byte* Data = (Byte*)GetData(); 
	put_fire_pix( X, Y, 255 );
}


/*-----------------------------------------------------------------------------
    FWaterBitmap implementation.
-----------------------------------------------------------------------------*/

//
// Water bitmap constructor.
//
FWaterBitmap::FWaterBitmap()
	:	FDemoBitmap(),
		Image( nullptr ),
		ZBuffer( nullptr )
{
}


//
// Initialize water bitmap.
//
void FWaterBitmap::Init( Integer InU, Integer InV )
{
	FDemoBitmap::Init( InU, InV );

	// Allocate z-buffer.
	ZBuffer	= new Byte[USize*VSize >> 1];
	MemSet( ZBuffer, USize*VSize >> 1, 128 );

	WaterAmpl	= 128;
	NumDrops	= 0;
	Image		= nullptr;
	AnimSpeed	= 20.f;

	SetDistortionTable();
	paletteSimple( &Palette.Colors[0] );
}


//
// User click water bitmap.
//
void FWaterBitmap::MouseClick( Integer Button, Integer X, Integer Y )
{
	if( Image )
	{
		if( Button == 1 )
			AddDrop( X, Y );
		else if( Button == 2 )
			DeleteDrops( X, Y, 5 );
	}
}


//
// User move cursor over the water.
//
void FWaterBitmap::MouseMove( Integer Button, Integer X, Integer Y )
{
	if( Image )
	{
		if( Button == 1 && DrawParams.DrawType < DROP_Oscillator )
			AddDrop( X, Y );
		else if( Button == 2 )
			DeleteDrops( X, Y, 5 );

		// Draw a wavy cursor.
		X >>= 1;
		Y >>= 1;
		Byte* WaterA = ZBuffer;
		Byte* WaterB = ZBuffer + (USize >> 1);
		Integer U2Mask = UMask >> 1;
		Integer V2Mask = VMask >> 1;

		put_wat_pixa( X, Y, 145 );
		put_wat_pixb( X, Y, ~145 );
	}
}


//
// Serialize water bitmap.
//
void FWaterBitmap::SerializeThis( CSerializer& S )
{
	FDemoBitmap::SerializeThis( S );
	Serialize( S, WaterAmpl );
	Serialize( S, Image );
	Serialize( S, NumDrops );

	// Only active drops.
	S.SerializeData( Drops, NumDrops * sizeof(TDrop) );
}


//
// Export the water.
//
void FWaterBitmap::Export( CExporterBase& Ex )
{
	FDemoBitmap::Export( Ex );
	EXPORT_INTEGER( NumDrops );

	// Export each drop as two integers.
	for( Integer i=0; i<NumDrops; i++ )
	{
		Ex.ExportInteger( *String::Format( L"Drops[%d].A", i ), *(Integer*)(&Drops[i].Type) );
		Ex.ExportInteger( *String::Format( L"Drops[%d].B", i ), *(Integer*)(&Drops[i].ParamA) );
	}
}


//
// Import the water.
//
void FWaterBitmap::Import( CImporterBase& Im )
{
	FDemoBitmap::Import( Im );
	IMPORT_INTEGER( NumDrops );

	// Import each drop as two integers.
	for( Integer i=0; i<NumDrops; i++ )
	{
		*(Integer*)(&Drops[i].Type)		=	Im.ImportInteger( *String::Format( L"Drops[%d].A", i ) );
		*(Integer*)(&Drops[i].ParamA)	=	Im.ImportInteger( *String::Format( L"Drops[%d].B", i ) );
	}
}


//
// Delete drops near cursor.
//
void FWaterBitmap::DeleteDrops( Integer X, Integer Y, Integer Size )
{
	if( (X<0)||(Y<0)||(X>=USize)||(Y>=VSize) )
		return;

	X /= 2;
	Y /= 2;

	for( Integer i=0; i<NumDrops; )
	{
		if( Abs(X-Drops[i].X)<=Size && Abs(Y-Drops[i].Y)<=Size )
		{
			Drops[i] = Drops[NumDrops--];
		}
		else
			i++;
	}
}


//
// Set a water dispersion table.
//
void FWaterBitmap::SetDistortionTable()
{
	for( Integer i=0; i<1024; i++ )
	{
		Integer Val		= Trunc((i-511)*((Float)WaterAmpl/512.f));
		DistTable[i]	= Clamp( Val, -128, 127 );
	}
}


//
// Apply water dispersion.
//
void FWaterBitmap::RenderWater()
{
	if( Phase & 1 )
	{
		assert(Image);

		Byte* Data = (Byte*)Image->GetData();
		Byte* Water	= (Byte*)GetData();

		for( Integer V=0; V<VSize; V++ )
		{
			Byte* WaterLine = &Water[V << UBits];
			Byte* DataLine  = &Data[V << UBits];

			for( Integer U=0; U<USize; U++ )
			{
				WaterLine[U] = DataLine[(U+WaterLine[U]) & UMask];
			}
		}
	}
}


//
// Calculate the water surface.
//
void FWaterBitmap::CalculateWater()
{
	Byte* Data			= (Byte*)GetData();
	Integer U2Size		= USize >> 1;
	Integer V2Size		= VSize >> 1;
	Integer U2Mask		= UMask >> 1;
	Integer V2Mask		= VMask >> 1;
	
	if( Phase & 1 )
	{
		// Apply odd water filter.
		Byte*	ThisWater1	= ZBuffer + ((V2Size-1) << UBits);
		Byte*	ThisWater2	= ThisWater1 + U2Size;

		for( Integer V=0; V<V2Size; V++ )
		{
			Byte*	NextWater1	= ZBuffer + (V << UBits);
			Byte*	NextWater2	= NextWater1 + U2Size;
			Byte*	Data0		= &Data[((V*2+0) & VMask) << UBits];
			Byte*	Data1		= &Data[((V*2+1) & VMask) << UBits];

			for( Integer U=0; U<U2Size; U++ )
			{
				// Sample values.
				Integer A = ThisWater1[U];
				Integer B = ThisWater1[(U+1) & U2Mask];
				Integer C = NextWater1[U];
				Integer D = NextWater1[(U+1) & U2Mask];
				Integer E = ThisWater1[(U+2) & U2Mask];
				Integer F = NextWater1[(U+2) & U2Mask];
				Integer G = ThisWater1[(U+3) & U2Mask];
				Integer H = NextWater1[(U+3) & U2Mask];

				// Put pixel.
				ThisWater2[U] = WaveTab[512+A+B+C+D-2*ThisWater2[U]];

				// Compute deltas.
				Integer DH = D-H;
				Integer BG = B-G;
				Integer AE = A-E;
				Integer CF = C-F;

				// Resample water depth to bitmap.
				Data0[U*2+0]	= DistTable[512+AE*2];
				Data1[U*2+0]	= DistTable[512+CF+AE];
				Data0[U*2+1]	= DistTable[512+BG+AE];
				Data1[U*2+1]	= DistTable[512+(DH+BG+AE+CF)/2];
			}

			ThisWater1 = NextWater1;
			ThisWater2 = NextWater2;
		}
	}
	else
	{
		// Apply even water filter.
		Byte*	PrevWater1	= ZBuffer + ((V2Size-1) << UBits);
		Byte*	PrevWater2	= PrevWater1 + U2Size;

		for( Integer V=0; V<V2Size; V++ )
		{
			Byte*	ThisWater1	= ZBuffer + (V << UBits);
			Byte*	ThisWater2	= ThisWater1 + U2Size;

			for( Integer U=0; U<U2Size; U++ )
			{
				// Sample values.
				Integer A = PrevWater2[(U-1) & U2Mask];
				Integer B = PrevWater2[U];
				Integer C = ThisWater2[(U-1) & U2Mask];
				Integer D = ThisWater2[U];

				// Put pixel.
				ThisWater1[U] = WaveTab[512+A+B+C+D-2*ThisWater1[U]];
			}

			PrevWater1 = ThisWater1;
			PrevWater2 = ThisWater2;
		}
	}
}


//
// Erase the water bitmap.
//
void FWaterBitmap::Erase()
{
	NumDrops	= 0;
}


//
// Redraw the water bitmap.
//
void FWaterBitmap::Redraw()
{
	if( Image )
	{
		// Redraw image, if it dynamic.
		if( Image->bDynamic )
			Image->Tick();

		// Render water.
		Phase++;
		RedrawDrops();
		CalculateWater();
		RenderWater();
		bRedrawn	= true;
	}
	else
	{
		// Nothing to draw.
		MemZero( GetData(), USize*VSize*sizeof(Byte) );
		bRedrawn	= true;
	}
}


//
// Initialize water after loading.
//
void FWaterBitmap::PostLoad()
{
	FDemoBitmap::PostLoad();
	SetDistortionTable();

	// Allocate Z-Buffer.
	ZBuffer	= new Byte[USize*VSize >> 1];
	MemSet( ZBuffer, USize*VSize >> 1, 128 );
}


//
// Water bitmap destructor.
//
FWaterBitmap::~FWaterBitmap()
{
	// Delete Z-buffer.
	if( ZBuffer )
		delete[] ZBuffer;
}


//
// Some field changed in water.
//
void FWaterBitmap::EditChange()
{
	FDemoBitmap::EditChange();

	// Validate the image.
	if( Image )
		if(		( Image->USize != USize )||
				( Image->VSize != VSize )||	
				( Image->Format != BF_Palette8 )||
				( Image == this )  )
										Image	= nullptr;

	// Copy palette.
	if( Image )
		MemCopy
			( 
				&Palette.Colors[0], 
				&Image->Palette.Colors[0], 
				sizeof(TColor)*Image->Palette.Colors.Num() 
			);

	// Update the dispersion table.
	SetDistortionTable();
}


//
// Add a new drop to the water bitmap.
//
void FWaterBitmap::AddDrop( Integer X, Integer Y )
{
	// Check out of bounds.
	if(	( X < 0 ) || 
		( Y < 0 ) || 
		( X >= USize ) || 
		( Y >= VSize ) || 
		( NumDrops >= MAX_WATER_DROPS-1 ) )
			return;

	Integer i = NumDrops++;
	TDrop& Drop = Drops[i];

	// Common info.
	Drop.X			= X >> 1;
	Drop.Y			= Y >> 1;
	Drop.Type		= DrawParams.DrawType;
	Drop.Depth		= DrawParams.Depth;

	switch( Drop.Type )
	{
		case DROP_RandomPoint:
		{
			// Noisy.
			Drop.ParamA	= DrawParams.Amplitude;
			break;
		}
		case DROP_Tap:
		case DROP_RainDrops:
		{
			// Rain like.
			Drop.ParamA	= DrawParams.Frequency;
			Drop.ParamB	= 0;
			break;
		}
		case DROP_Surfer:
		{
			// Surfer slider.
			Drop.ParamA	= DrawParams.Speed;
			break;
		}
		case DROP_Oscillator:
		case DROP_VertLine:
		case DROP_HorizLine:
		{
			// Oscillator based.
			Drop.ParamA	= DrawParams.Amplitude;
			Drop.ParamB	= DrawParams.Frequency;
			Drop.ParamC	= DrawParams.Size >> 1;
			break;
		}
	}
}


//
// Redraw all water drops.
//
void FWaterBitmap::RedrawDrops()
{
	Byte* WaterA	= ZBuffer;
	Byte* WaterB	= ZBuffer + (USize >> 1);
	Integer U2Mask	= UMask >> 1;
	Integer V2Mask	= VMask >> 1;
	Integer XSize	= USize / 2;

	for( Integer i=0; i<NumDrops; i++ )
	{
		TDrop& Drop = Drops[i];

		switch( Drop.Type )
		{
			case DROP_Point:
			{
				// Still point.
				put_wat_pixa( Drop.X, Drop.Y, Drop.Depth );
				put_wat_pixb( Drop.X, Drop.Y, Drop.Depth );
				break;
			}
			case DROP_RandomPoint:
			{
				// Noisy point.
				Byte Depth = 128 + (Drop.ParamA * RandomByte() >> 8);
				put_wat_pixa( Drop.X, Drop.Y, Depth );
				put_wat_pixb( Drop.X, Drop.Y, ~Depth );
				break;
			}
			case DROP_Oscillator:
			{
				// Simple oscillator.
				Byte Depth = 128-Drop.ParamA/2 + (SineTab[Drop.Depth] * Drop.ParamA >> 8);
				put_wat_pixa( Drop.X, Drop.Y, Depth );
				put_wat_pixb( Drop.X, Drop.Y, Depth );
				Drop.Depth += Drop.ParamB;
				break;
			}
			case DROP_RainDrops:
			{
				// Rain puddle.
				Drop.ParamB += Drop.ParamA;
				if( Drop.ParamB < Drop.ParamA )
				{
					Drop.X = Random(USize);
					Drop.Y = Random(VSize);
					put_wat_pixa( Drop.X, Drop.Y, Drop.Depth );
					put_wat_pixb( Drop.X, Drop.Y, ~Drop.Depth );
				}
				break;
			}
			case DROP_Tap:
			{
				// Tap.
				Drop.ParamB += Drop.ParamA;
				if( Drop.ParamB < Drop.ParamA )
				{
					put_wat_pixa( Drop.X, Drop.Y, Drop.Depth );
					put_wat_pixb( Drop.X, Drop.Y, ~Drop.Depth );
				}
				break;
			}
			case DROP_VertLine:
			{
				// Vertical line oscillation.
				Byte Depth = 128-Drop.ParamA/2 + (SineTab[Drop.Depth] * Drop.ParamA >> 8);
				for( Integer Y=0; Y<Drop.ParamC; Y++ )
				{
					put_wat_pixa( Drop.X, Drop.Y+Y, Depth );
					put_wat_pixb( Drop.X, Drop.Y+Y, Depth );
				}
				Drop.Depth += Drop.ParamB;
				break;
			}
			case DROP_HorizLine:
			{
				// Horizontal line oscillation.
				Byte Depth = 128-Drop.ParamA/2 + (SineTab[Drop.Depth] * Drop.ParamA >> 8);
				for( Integer X=0; X<Drop.ParamC; X++ )
				{
					put_wat_pixa( Drop.X+X, Drop.Y, Depth );
					put_wat_pixb( Drop.X+X, Drop.Y, Depth );
				}
				Drop.Depth += Drop.ParamB;
				break;
			}
			case DROP_Surfer:
			{
				// Random moved spot.
				if( RandomByte() < Drop.ParamA )
				{
					Drop.X = U2Mask & (Drop.X-(RandomByte()&3)+(RandomByte()&3));
					Drop.Y = V2Mask & (Drop.Y-(RandomByte()&3)+(RandomByte()&3));
				}
				put_wat_pixa( Drop.X, Drop.Y, Drop.Depth );
				put_wat_pixb( Drop.X, Drop.Y, ~Drop.Depth );
				break;
			}
		}
	}
}


/*-----------------------------------------------------------------------------
	FTechBitmap implementation.
-----------------------------------------------------------------------------*/

//
// Tech bitmap constructor.
//
FTechBitmap::FTechBitmap()
	:	FDemoBitmap(),
		ZBuffer(nullptr)
{
}


//
// Tech bitmap destructor.
//
FTechBitmap::~FTechBitmap()
{
	// Delete Z-buffer.
	if( ZBuffer )
		delete[] ZBuffer;
}


//
// Initialize tech bitmap.
//
void FTechBitmap::Init( Integer InU, Integer InV )
{
	FDemoBitmap::Init( InU, InV );

	// Allocate z-buffer.
	ZBuffer	= new Byte[USize*VSize];
	MemZero( ZBuffer, USize*VSize*sizeof(Byte) );

	BumpMapLight = 150;
	BumpMapAngle = 30;
	AnimSpeed = 20.f;
	NumPanels = 0;

	SetLigthTable();
	paletteTech(&Palette.Colors[0]);
}


//
// Delete panels near cursor.
//
void FTechBitmap::DeletePanels( Integer X, Integer Y, Integer Area )
{
	if( (X<0)||(Y<0)||(X>=USize)||(Y>=VSize) )
		return;

	for( Integer i=0; i<NumPanels; )
	{
		if( Abs(X-Panels[i].X)<=Area && Abs(Y-Panels[i].Y)<=Area )
		{
			Panels[i] = Panels[NumPanels--];
		}
		else
			i++;
	}

	// And apply some blur, just for fun.
	ApplyFilter1
	(
		X - Area/2,
		Y - Area/2,
		Area
	);
}


//
// Erase the tech bitmap.
//
void FTechBitmap::Erase()
{
	NumPanels = 0;
	MemZero( ZBuffer, USize*VSize*sizeof(Byte) );
}


//
// User click water bitmap.
//
void FTechBitmap::MouseClick( Integer Button, Integer X, Integer Y )
{
	if( Button == 1 )
		AddPanel( X, Y );
	else if( Button == 2 )
		DeletePanels( X, Y, 12 );
}


//
// User move cursor over the bitmap.
//
void FTechBitmap::MouseMove( Integer Button, Integer X, Integer Y )
{
	if( Button == 1 )
		AddPanel( X, Y );
	else if( Button == 2 )
		DeletePanels( X, Y, 12 );
}


//
// Serialize tech bitmap.
//
void FTechBitmap::SerializeThis( CSerializer& S )
{
	FDemoBitmap::SerializeThis( S );

	Serialize( S, BumpMapAngle );
	Serialize( S, BumpMapLight );
	Serialize( S, NumPanels );

	// Only active panels.
	S.SerializeData( Panels, NumPanels * sizeof(TPanel) );
}	


//
// Import the tech bitmap.
//
void FTechBitmap::Import( CImporterBase& Im )
{
	FDemoBitmap::Import( Im );
	IMPORT_INTEGER( NumPanels );

	// Import each spark as two integers.
	for( Integer i=0; i<NumPanels; i++ )
	{
		*(Integer*)(&Panels[i].Type)	=	Im.ImportInteger( *String::Format( L"Panels[%d].A", i ) );
		*(Integer*)(&Panels[i].ParamA)	=	Im.ImportInteger( *String::Format( L"Panels[%d].B", i ) );
	}
}


//
// Export the fire bitmap.
//
void FTechBitmap::Export( CExporterBase& Ex )
{
	FDemoBitmap::Export( Ex );
	EXPORT_INTEGER( NumPanels );

	// Export each panel as two integers.
	for( Integer i=0; i<NumPanels; i++ )
	{
		Ex.ExportInteger( *String::Format( L"Panels[%d].A", i ), *(Integer*)(&Panels[i].Type) );
		Ex.ExportInteger( *String::Format( L"Panels[%d].B", i ), *(Integer*)(&Panels[i].ParamA) );
	}
}


//
// Smooth filter.
//
void FTechBitmap::ApplyFilter1( Integer X, Integer Y, Integer Area )
{
	Byte* Source = ZBuffer;

	for( Integer U=X; U<X+Area; U++ )
	{
		Integer UZ = U & UMask;
		Integer UP = (U+1) & UMask;
		Integer UN = (U-1) & UMask;

		for( Integer V=Y; V<Y+Area; V++ )
		{
			Integer VZ = (V & VMask) << UBits;
			Integer VP = ((V+1) & VMask) << UBits;
			Integer VN = ((V-1) & VMask) << UBits;

			Source[UZ+VZ] = 
				(
					(Integer)Source[UP + VZ] +
					(Integer)Source[UN + VZ] +
					(Integer)Source[UZ + VP] +
					(Integer)Source[UZ + VN]
				) / 4;
		}
	}
}


//
// Noisy filter.
//
void FTechBitmap::ApplyFilter2( Integer X, Integer Y, Integer Area )
{
	Byte* Source = ZBuffer;

	for( Integer U=X; U<X+Area; U++ )
	{
		Integer UZ = U & UMask;
		Integer UP = (U+1) & UMask;
		Integer UN = (U-1) & UMask;

		for( Integer V=Y; V<Y+Area; V++ )
		{
			Integer VZ = (V & VMask) << UBits;
			Integer VP = ((V+1) & VMask) << UBits;
			Integer VN = ((V-1) & VMask) << UBits;

			Source[UZ+VZ] = 
				(
					(Integer)Source[UP + VZ] +
					(Integer)Source[UN + VZ] +
					(Integer)Source[UZ + VP] +
					(Integer)Source[UZ + VN]
				) / 4 - 2;
		}
	}
}


//
// Redraw all panels.
//
void FTechBitmap::RedrawPanels()
{
	for( Integer i=0; i<NumPanels; i++ )
	{
		TPanel& Panel = Panels[i];

		switch( Panel.Type )
		{
			case TECH_Ivy: 
			{
				put_tech_pix( Panel.X, Panel.Y, Panel.Depth );
				Panel.X += Panel.ParamD;

				if( (Panel.X & 15) == 0 && NumPanels<MAX_TECH_PANELS )
				{
					TPanel& Other = Panels[NumPanels++];
					Other.Type = _TECH_Ivy1;
					Other.Depth = Panel.Depth;
					Other.X = Panel.X;
					Other.Y = Panel.Y;
					Other.ParamB = 255;
					Other.ParamC = RandomByte();
				}

				if( (Panel.X & 15) == 7 && NumPanels<MAX_TECH_PANELS )
				{
					TPanel& Other = Panels[NumPanels++];
					Other.Type = _TECH_Ivy1;
					Other.Depth = Panel.Depth;
					Other.X = Panel.X;
					Other.Y = Panel.Y;
					Other.ParamB = 1;
					Other.ParamC = RandomByte();
				}

				break;
			}
			case TECH_Wave:
			{
				for( Integer X=0; X<Panel.ParamD; X++ )
				{
					Byte* Pixel = &ZBuffer[((X+Panel.X) & UMask) + ((Panel.Y & VMask) << UBits)];
					if( *Pixel < Panel.ParamC )
						*Pixel = Panel.ParamC;
				}

				Panel.ParamC = SineTab[255 & 4*Panel.Y];
				Panel.Y++;

				break;
			}
			case TECH_Segments:
			{
				for( Integer X=0; X<Panel.ParamD; X++ )
				{
					Byte* Pixel = &ZBuffer[((X+Panel.X) & UMask) + ((Panel.Y & VMask) << UBits)];
					if( *Pixel < Panel.ParamC )
						*Pixel = Panel.ParamC;
				}

				if( !(Phase & 0x1f) )
					Panel.ParamC = RandomByte();
				Panel.Y++;

				break;
			}
			case TECH_Straight:
			{
				Panel.X = (Panel.X + 1) & UMask;
				for( Integer Y=0; Y<Panel.ParamD; Y++ )
				{
					Byte* Pixel = &ZBuffer[Panel.X + (((Y+Panel.Y)& VMask) << UBits)];
					if( *Pixel < Panel.Depth )
						*Pixel = Panel.Depth;
				}
				break;
			}
			case TECH_Circle:
			{
				Panel.ParamC++;
				Integer X = Panel.X + (SineTab[Panel.ParamC]*Panel.ParamA >> 8);
				Integer Y = Panel.Y + (SineTab[255 & (Panel.ParamC+64)]*Panel.ParamA >> 8);

				put_tech_pix( X, Y, SineTab[Panel.ParamC] );

				if( !(Phase & 7) )
					ApplyFilter1( X-4, Y-4, 16 );

				break;
			}
			case TECH_Grinder:
			{
				if(((Phase ^ i) & 3) == 1 )
					ApplyFilter1( Panel.X, Panel.Y, 10 );

				if( RandomByte() < Panel.ParamA )
					Panel.X += RandomByte() & 1 ? +2 : -2;

				if( RandomByte() < Panel.ParamA )
					Panel.Y += RandomByte() & 1 ? +2 : -2;

				break;
			}
			case TECH_Noisy:
			{
				if(((Phase ^ i) & 3) == 1 )
					ApplyFilter2( Panel.X, Panel.Y, 10 );

				if( RandomByte() < Panel.ParamA )
					Panel.X += RandomByte() & 1 ? +2 : -2;

				if( RandomByte() < Panel.ParamA )
					Panel.Y += RandomByte() & 1 ? +2 : -2;

				break;
			}
			case _TECH_Ivy1:
			{
				put_tech_pix( Panel.X, Panel.Y, Panel.Depth );

				Panel.Y += Panel.ParamB;
				Panel.ParamC--;

				if( Panel.ParamC == 0 )
				{
					Panel.Type = _TECH_Ivy2;
					Panel.ParamC = 28;
				}
				break;
			}
			case _TECH_Ivy2:
			{
				Panel.ParamC--;
				if( !(Phase & 7) )
					ApplyFilter1( Panel.X-5, Panel.Y-5, 10 );

				if( Panel.ParamC == 0 )
				{
					NumPanels--;
					Panels[i] = Panels[NumPanels];
				}
				break;
			}
		}
	}
}


//
// Add a new panel to the bitmap.
//
void FTechBitmap::AddPanel( Integer X, Integer Y )
{
	// Check out of bounds.
	if(	( X < 0 ) || 
		( Y < 0 ) || 
		( X >= USize ) || 
		( Y >= VSize ) || 
		( NumPanels >= MAX_TECH_PANELS-1 ) )
			return;

	Integer i = NumPanels++;
	TPanel& Panel = Panels[i];

	// Common info.
	Panel.X		= X;
	Panel.Y		= Y;
	Panel.Type	= DrawParams.DrawType;
	Panel.Depth	= DrawParams.Depth;

	switch(Panel.Type)
	{
		case TECH_Ivy:
			Panel.ParamD = 1;
			break;

		case TECH_Circle:
			Panel.ParamA = DrawParams.Size;
			Panel.ParamC = 0;
			break;

		case TECH_Straight:
		case TECH_Segments:
		case TECH_Wave:
			Panel.ParamC = DrawParams.Depth;
			Panel.ParamD = DrawParams.Size;
			break;

		case TECH_Noisy:
		case TECH_Grinder:
			Panel.ParamA = DrawParams.Time;
			break;
	}
}


//
// Redraw tech bitmap.
//
void FTechBitmap::Redraw()
{
	Phase++;

	RedrawPanels();
	CalculateBumpMap();
	
	bRedrawn = true;
}


//
// Compute bump map using cheap water-like bump map
// algorithm.
//
void FTechBitmap::CalculateBumpMap()
{
	Byte* Data = static_cast<Byte*>(GetData());
	Byte* Source = ZBuffer;

	for( Integer V=0; V<VSize; V++ )
	{
		Byte* DataLine = Data + (V << UBits);
		Byte* SourceLine = Source + (V << UBits);

		for( Integer U=0; U<USize; U++ )
		{
			Integer A = SourceLine[(U+1) & UMask];
			Integer B = SourceLine[(U-1) & UMask];

			DataLine[U] = LightTable[255 + A + (B >> 2) - B];
		}
	}
}


//
// Initialize water after loading.
//
void FTechBitmap::PostLoad()
{
	FDemoBitmap::PostLoad();
	SetLigthTable();

	// Allocate Z-buffer.
	ZBuffer = new Byte[USize*VSize];
	MemZero( ZBuffer, USize*VSize*sizeof(Byte) );
}


//
// Some field changed in water.
//
void FTechBitmap::EditChange()
{
	FDemoBitmap::EditChange();
	SetLigthTable();

	// Validate palette.
	if( PaletteRef )
		if( PaletteRef->USize * PaletteRef->VSize < 256 )
			PaletteRef	= nullptr;

	// Copy data from palette.
	if( PaletteRef )
	{
		if( PaletteRef->Format == BF_Palette8 )
		{
			// Copy via other palette.
			Byte* Inds = (Byte*)PaletteRef->GetData();
			TColor* Pal	= (TColor*)&PaletteRef->Palette.Colors[0];

			for( Integer i=0; i<256; i++ )
				Palette.Colors[i] = Pal[Inds[i]];
		}
		else
		{
			// Copy directly from other's data.
			TColor* Other = (TColor*)PaletteRef->GetData();

			for( Integer i=0; i<256; i++ )
				Palette.Colors[i] = Other[i];
		}
	}
}


//
// Set tech layer lighting table.
//
void FTechBitmap::SetLigthTable()
{
	Float LampAngle = PI * BumpMapAngle / 255.f;

	for( Integer i=0; i<575; i++ )
	{
		Float Normal = ArcTan((255.f-i)/95.f) + PI*0.5f;
		Integer TempLight = Round(Cos(Normal-LampAngle)*200.f);
		Float Reflected = Abs(2.f*Normal - LampAngle - BumpMapLight*PI/255.f);

		if( Reflected < 0.1f )
			TempLight += Round((0.1f-Reflected)*1200.f);

		LightTable[i] = Clamp( TempLight, 0, 255 );
	}
}


/*-----------------------------------------------------------------------------
    FGlassBitmap implementation.
-----------------------------------------------------------------------------*/

//
// Glass constructor.
//
FGlassBitmap::FGlassBitmap()
	:	FDemoBitmap(),
		Glass( nullptr ),
		Image( nullptr )
{
}


//
// Glass destructor.
//
FGlassBitmap::~FGlassBitmap()
{
}


//
// Initialize glass bitmap.
//
void FGlassBitmap::Init( Integer InU, Integer InV )
{
	FDemoBitmap::Init( InU, InV );
	
	bMoveGlass		= true;
	HSpeed			= 0.f;
	VSpeed			= 0.f;
	HOffset			= 0;
	VOffset			= 0;
	AnimSpeed		= 30.f;
}


//
// Reset the glass displacement.
//
void FGlassBitmap::Erase()
{
	VOffset	= 0;
	HOffset	= 0;
}


//
// Tap over glass.
//
void FGlassBitmap::MouseClick( Integer Button, Integer X, Integer Y )
{
}


//
// Shift the glass.
//
void FGlassBitmap::MouseMove( Integer Button, Integer X, Integer Y )
{
	static Integer OldX;
	static Integer OldY;

	if( Button == 1 )
	{
		HOffset	-= X-OldX;
		VOffset	-= Y-OldY;
	}

	OldX = X;
	OldY = Y;
}


//
// Redraw the glass effect.
// 
void FGlassBitmap::Redraw()
{
	if( Image && Glass )
	{
		// Tick other images if they are dynamic.
		if( Image->bDynamic )	Image->Tick();
		if( Glass->bDynamic )	Glass->Tick();

		if( bMoveGlass )
			RenderGlassI();
		else
			RenderGlassII();
	}
	else
		MemZero( GetData(), USize*VSize*sizeof(Byte) );

	bRedrawn	= true;
}


//
// Serialize the glass.
//
void FGlassBitmap::SerializeThis( CSerializer& S )
{
	FDemoBitmap::SerializeThis( S );
	Serialize( S, bMoveGlass );
	Serialize( S, Glass );
	Serialize( S, Image );
	Serialize( S, HSpeed );
	Serialize( S, VSpeed );
	Serialize( S, VOffset );
	Serialize( S, HOffset );
}


//
// Some field has been changed.
// 
void FGlassBitmap::EditChange()
{
	FDemoBitmap::EditChange();

	// Validate the image.
	if( Image )
		if(		( Image->USize != USize )||
				( Image->VSize != VSize )||	
				( Image->Format != BF_Palette8 )||
				( Image == this )  )
										Image	= nullptr;

	if( Glass )
		if(		( Glass->USize != USize )||
				( Glass->VSize != VSize )||	
				( Glass->Format != BF_Palette8 )||
				( Glass == this )  )
										Glass	= nullptr;

	// Copy palette.
	if( Image )
		MemCopy
			( 
				&Palette.Colors[0], 
				&Image->Palette.Colors[0], 
				sizeof(TColor)*Image->Palette.Colors.Num() 
			);
}


//
// Image the glass.
//
void FGlassBitmap::Import( CImporterBase& Im )
{
	FDemoBitmap::Import( Im );
	IMPORT_BYTE(HOffset);
	IMPORT_BYTE(VOffset);
}


//
// Export the glass.
//
void FGlassBitmap::Export( CExporterBase& Ex )
{
	FDemoBitmap::Export( Ex );
	EXPORT_BYTE(HOffset);
	EXPORT_BYTE(VOffset);
}


//
// Movable glass version.
//
void FGlassBitmap::RenderGlassI()
{
	// Compute position.
	Float	Time = GPlat->Now();
	Integer PosX = (HOffset + Round(Time*HSpeed)) & UMask;
	Integer PosY = (VOffset + Round(Time*VSpeed)) & VMask;

	Byte* Dst = (Byte*)GetData();
	Byte* Img = (Byte*)Image->GetData();
	Byte* Gls = (Byte*)Glass->GetData();

	for( Integer V=0; V<VSize; V++ )
	{
		Byte* DstLine	= &Dst[V << UBits];
		Byte* ImgLine	= &Img[V << UBits];
		Byte* GlsLine	= &Gls[((V+PosY) & VMask) << UBits];

		for( Integer U=0; U<USize; U++ )
		{
			DstLine[U]	= ImgLine[(GlsLine[(PosX+U) & UMask]+U) & UMask];
		}
	}
}


//
// Movable image version. 
//
void FGlassBitmap::RenderGlassII()
{
	// Compute position.
	Float	Time = GPlat->Now();
	Integer PosX = (HOffset + Round(Time*HSpeed)) & UMask;
	Integer PosY = (VOffset + Round(Time*VSpeed)) & VMask;

	Byte* Dst = (Byte*)GetData();
	Byte* Img = (Byte*)Image->GetData();
	Byte* Gls = (Byte*)Glass->GetData();

	for( Integer V=0; V<VSize; V++ )
	{
		Byte* DstLine	= &Dst[V << UBits];
		Byte* ImgLine	= &Img[((V+PosY) & VMask) << UBits];
		Byte* GlsLine	= &Gls[V << UBits];

		for( Integer U=0; U<USize; U++ )
		{
			DstLine[U]	= ImgLine[(GlsLine[U]+U+PosX) & UMask];
		}
	}
}


/*-----------------------------------------------------------------------------
    FHarmonicBitmap implementation.
-----------------------------------------------------------------------------*/

//
// Harmonic constructor.
//
FHarmonicBitmap::FHarmonicBitmap()
	:	FDemoBitmap(),
		Image( nullptr )
{
}


//
// Harmonic destructor.
//
FHarmonicBitmap::~FHarmonicBitmap()
{
}


//
// Initialize harmonic bitmap.
//
void FHarmonicBitmap::Init( Integer InU, Integer InV )
{
	FDemoBitmap::Init( InU, InV );
	Direction	= WAVE_Horizontal;
	WaveAmpl	= 32;
	WaveFreq	= 16;
	VOffset		= 0;
	HOffset		= 0;
	AnimSpeed	= 20.f;
}


//
// Cleanup harmonic.
//
void FHarmonicBitmap::Erase()
{
	VOffset	= 0;
	HOffset	= 0;
}


//
// Harmonic click.
//
void FHarmonicBitmap::MouseClick( Integer Button, Integer X, Integer Y )
{
}


//
// Shift the harmonic.
//
void FHarmonicBitmap::MouseMove( Integer Button, Integer X, Integer Y )
{
	static Integer OldX;
	static Integer OldY;

	if( Button == 1 )
	{
		HOffset	-= X-OldX;
		VOffset	-= Y-OldY;
	}

	OldX = X;
	OldY = Y;
}


//
// Redraw the harmonic.
//
void FHarmonicBitmap::Redraw()
{
	if( Image )
	{
		// Redraw Image if it's dynamic.
		if( Image->bDynamic )
			Image->Tick();

		Phase += 4;

		if( Direction == WAVE_Horizontal )
			RenderHarmonicH();
		else
			RenderHarmonicV();
	}
	else
		MemZero( GetData(), USize*VSize*sizeof(Byte) );

	bRedrawn	= true;
}


//
// Some harmonic variable has been changed.
//
void FHarmonicBitmap::EditChange()
{
	FDemoBitmap::EditChange();

	// Validate the image.
	if( Image )
		if(		( Image->USize != USize )||
				( Image->VSize != VSize )||	
				( Image->Format != BF_Palette8 )||
				( Image == this )  )
										Image	= nullptr;

	// Copy palette.
	if( Image )
		MemCopy
			( 
				&Palette.Colors[0], 
				&Image->Palette.Colors[0], 
				sizeof(TColor)*Image->Palette.Colors.Num() 
			);
}


//
// Bitmap serialization.
//
void FHarmonicBitmap::SerializeThis( CSerializer& S )
{
	FDemoBitmap::SerializeThis( S );
	SerializeEnum( S, Direction );
	Serialize( S, WaveAmpl );
	Serialize( S, WaveFreq );
	Serialize( S, Image );
	Serialize( S, VOffset );
	Serialize( S, HOffset );
}


//
// Harmonic import.
//
void FHarmonicBitmap::Import( CImporterBase& Im )
{
	FDemoBitmap::Import( Im );
	IMPORT_BYTE(VOffset);
	IMPORT_BYTE(HOffset);
}


//
// Harmonic export.
//
void FHarmonicBitmap::Export( CExporterBase& Ex )
{
	FDemoBitmap::Export( Ex );
	EXPORT_BYTE(VOffset);
	EXPORT_BYTE(HOffset);
}


//
// Render vertical harmonic.
//
void FHarmonicBitmap::RenderHarmonicV()
{
	Byte* Dst	= (Byte*)GetData();
	Byte* Src	= (Byte*)Image->GetData();

	for( Integer U=0; U<USize; U++ )
	{
		Byte*	DstRow	= &Dst[U];
		Byte*	SrcRow	= &Src[(U+HOffset) & VMask];
		Integer WaveOff	= VOffset + ((SineTab[(Phase+((U*WaveFreq)>>4)) & 0xff]*WaveAmpl) >> 8);

		for( Integer V=0; V<VSize; V++ )
		{
			DstRow[V << UBits]	= SrcRow[((V+WaveOff) & VMask) << UBits];
		}
	}
}


//
// Render horizontal harmonics.
//
void FHarmonicBitmap::RenderHarmonicH()
{
	Byte* Dst	= (Byte*)GetData();
	Byte* Src	= (Byte*)Image->GetData();

	for( Integer V=0; V<VSize; V++ )
	{
		Byte*	DstLine	= &Dst[V << UBits];
		Byte*	SrcLine	= &Src[((V+VOffset) & VMask) << UBits];
		Integer WaveOff	= HOffset + ((SineTab[(Phase+((V*WaveFreq)>>4)) & 0xff]*WaveAmpl) >> 8);

		for( Integer U=0; U<USize; U++ )
		{
			DstLine[U] = SrcLine[(U+WaveOff) & UMask];
		}
	}
}


/*-----------------------------------------------------------------------------
    Registration.
-----------------------------------------------------------------------------*/

REGISTER_CLASS_CPP( FDemoBitmap, FBitmap, CLASS_Abstract )
{
}


REGISTER_CLASS_CPP( FPlasmaBitmap, FDemoBitmap, CLASS_None )
{
	ADD_PROPERTY( PlasmaA, PROP_Editable );
	ADD_PROPERTY( PlasmaB, PROP_Editable );
	ADD_PROPERTY( PlasmaC, PROP_Editable );
	ADD_PROPERTY( PlasmaD, PROP_Editable );
	ADD_PROPERTY( PaletteRef, PROP_Editable );
}


REGISTER_CLASS_CPP( FFireBitmap, FDemoBitmap, CLASS_None )
{
	BEGIN_ENUM(ESparkType);
		ENUM_ELEM(SPARK_Point);
		ENUM_ELEM(SPARK_RandomPoint);
		ENUM_ELEM(SPARK_Phase);
		ENUM_ELEM(SPARK_Jitter);
		ENUM_ELEM(SPARK_Twister);
		ENUM_ELEM(SPARK_Fireball);
		ENUM_ELEM(SPARK_JetUpward);
		ENUM_ELEM(SPARK_JetLeftward);
		ENUM_ELEM(SPARK_JetRightward);
		ENUM_ELEM(SPARK_JetDownward);
		ENUM_ELEM(SPARK_Spermatozoa);
		ENUM_ELEM(SPARK_Whirligig);
		ENUM_ELEM(SPARK_Cloud);
		ENUM_ELEM(SPARK_LineLighting);
		ENUM_ELEM(SPARK_RampLighting);
		ENUM_ELEM(SPARK_RandomLighting);
		ENUM_ELEM(SPARK_BallLighting);
	END_ENUM;

	ADD_PROPERTY( bRising, PROP_Editable );
	ADD_PROPERTY( FireHeat,PROP_Editable );
	ADD_PROPERTY( PaletteRef, PROP_Editable );
}


REGISTER_CLASS_CPP( FWaterBitmap, FDemoBitmap, CLASS_None )
{
	BEGIN_ENUM(EDropType);
		ENUM_ELEM(DROP_Point);
		ENUM_ELEM(DROP_RandomPoint);
		ENUM_ELEM(DROP_Tap);
		ENUM_ELEM(DROP_Surfer);
		ENUM_ELEM(DROP_RainDrops);
		ENUM_ELEM(DROP_Oscillator);
		ENUM_ELEM(DROP_VertLine);
		ENUM_ELEM(DROP_HorizLine);
	END_ENUM;

	ADD_PROPERTY( WaterAmpl, PROP_Editable );
	ADD_PROPERTY( Image, PROP_Editable );
}


REGISTER_CLASS_CPP( FTechBitmap, FDemoBitmap, CLASS_None )
{
	BEGIN_ENUM(EPanelType);
		ENUM_ELEM(TECH_Ivy);
		ENUM_ELEM(TECH_Circle);
		ENUM_ELEM(TECH_Straight);
		ENUM_ELEM(TECH_Segments);
		ENUM_ELEM(TECH_Grinder);
		ENUM_ELEM(TECH_Noisy);
		ENUM_ELEM(TECH_Wave);
	END_ENUM;

	ADD_PROPERTY( BumpMapLight, PROP_Editable );
	ADD_PROPERTY( BumpMapAngle, PROP_Editable );
	ADD_PROPERTY( PaletteRef, PROP_Editable );
}


REGISTER_CLASS_CPP( FGlassBitmap, FDemoBitmap, CLASS_None )
{
	ADD_PROPERTY( bMoveGlass, PROP_Editable );
	ADD_PROPERTY( Glass, PROP_Editable );
	ADD_PROPERTY( Image, PROP_Editable );
	ADD_PROPERTY( HSpeed, PROP_Editable );
	ADD_PROPERTY( VSpeed, PROP_Editable );
}


REGISTER_CLASS_CPP( FHarmonicBitmap, FDemoBitmap, CLASS_None )
{
	BEGIN_ENUM(EWaveDirection);
		ENUM_ELEM(WAVE_Horizontal);
		ENUM_ELEM(WAVE_Vertical);
	END_ENUM;

	ADD_PROPERTY( Direction, PROP_Editable );
	ADD_PROPERTY( Image, PROP_Editable );
	ADD_PROPERTY( WaveAmpl, PROP_Editable );
	ADD_PROPERTY( WaveFreq, PROP_Editable );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/