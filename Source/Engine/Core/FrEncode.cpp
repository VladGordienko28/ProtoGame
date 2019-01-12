/*=============================================================================
    FrEncode.cpp: Data compression implementation.
    Copyright Nov.2016 Vlad Gordienko.
=============================================================================*/

#include "..\Engine.h"

/*-----------------------------------------------------------------------------
    Bits manipulation.
-----------------------------------------------------------------------------*/

//
// Bits walking variables.
//
static	Integer	GBitsNum	= 0;
static	SizeT	GBitsAccum	= 0;


//
// Write a NumBits to the array.
//
inline void WriteBits( Byte*& Arr, SizeT Bits, SizeT NumBits )
{
	GBitsAccum		|= Bits << (32-NumBits-GBitsNum);
	GBitsNum		+= NumBits;

	while( GBitsNum >= 8 )
	{
		*Arr++			=	GBitsAccum >> 24;
		GBitsAccum		<<=	8;
		GBitsNum		-=	8;
	}
}


//
// Read a NumBits from the array.
//
inline SizeT ReadBits( Byte*& Arr, SizeT NumBits )
{
	while( GBitsNum <= 24 )
	{
		GBitsAccum		|= (SizeT)(*Arr++) << (24-GBitsNum);
		GBitsNum		+= 8;	
	}

	SizeT	Tmp = GBitsAccum >> (32-NumBits);
	GBitsAccum		<<=	NumBits;
	GBitsNum		-=	NumBits;

	return Tmp;
}


/*-----------------------------------------------------------------------------
    CLZWCompressor.
-----------------------------------------------------------------------------*/

//
// LZW constants.
//
#define LZW_BITS			12
#define	LZW_HASH_BIAS		(LZW_BITS-8)
#define LZW_MAX_VALUE		((1<<LZW_BITS)-1)
#define LZW_MAX_CODE		(LZW_MAX_VALUE-1)		
#define LZW_TABLE_SIZE		5021


//
// LZW algorithm table entry.
//
static struct TLZWEntry
{
public:
	Byte		Append;
	SizeT		Code;
	SizeT		Prefix;
} GLZWTable[LZW_TABLE_SIZE];
static Byte GLZWStack[LZW_TABLE_SIZE]; 


//
// Unpack the chunk of LZW data.
//
Byte* DecodeString( Byte* Buffer, SizeT Code )
{
	while( Code > 255 )
	{
		*Buffer++	= GLZWTable[Code].Append;
		Code		= GLZWTable[Code].Prefix;
	}
	*Buffer	= Code;
	return Buffer;
}


//
// Find a lzw entry in global table.
//
Integer FindLZWEntry( Integer Prefix, SizeT Value )
{
	Integer iEntry	= Prefix ^ (Value << LZW_HASH_BIAS);
	Integer Bias	= iEntry == 0 ? 1 : LZW_TABLE_SIZE-iEntry;

	for( ; ; )
	{
		if( GLZWTable[iEntry].Code == -1 )
			return iEntry;

		if	( 
				(GLZWTable[iEntry].Prefix == Prefix) && 
				(GLZWTable[iEntry].Append == Value) 
			)
				return iEntry;

		iEntry	-= Bias;
		if( iEntry < 0 )
			iEntry	+= LZW_TABLE_SIZE;
	}
}


//
// LZW constructor.
//
CLZWCompressor::CLZWCompressor()
{
}


//
// Encode data using LZW compression.
//
void CLZWCompressor::Encode( const void* InBuffer, SizeT InSize, void*& OutBuffer, SizeT& OutSize )
{
	// Allocate out buffer, with some extra memory
	// just in case.
	OutBuffer	= MemAlloc(InSize+InSize*2+8);
	OutSize		= 0;

	Byte*	In		= (Byte*)InBuffer;	
	Byte*	Out		= (Byte*)OutBuffer;
	Byte*	Eof		= In + InSize;

	// Output length of source data.
	*(SizeT*)Out	= InSize;
	Out	+= sizeof(SizeT);

	// Setup LZW table.
	for( SizeT i=0; i<LZW_TABLE_SIZE; i++ )
		GLZWTable[i].Code	= -1;

	// Reset bits walking.
	GBitsAccum	= 0;
	GBitsNum	= 0;

	SizeT	NextCode	= 256, 
			Code		= 0, 
			Value		= 0;

	// Read first code.
	Code	= InSize > 0 ? *In++ : 256;

	// Read code by code.
	while( In < Eof )
	{
		Value			= *In++;
		SizeT iEntry	= FindLZWEntry( Code, Value );

		if( GLZWTable[iEntry].Code != -1 )
		{
			// Entry found.
			Code	= GLZWTable[iEntry].Code;
		}
		else
		{
			// Entry not found.
			// Add to dictionary.
			if( NextCode <= LZW_MAX_CODE )
			{
				GLZWTable[iEntry].Code		= NextCode++;
				GLZWTable[iEntry].Prefix	= Code;
				GLZWTable[iEntry].Append	= Value;
			}

			// Write bits.
			WriteBits( Out, Code, LZW_BITS );
			Code	= Value;
		}
	}

	// Write markers.
	WriteBits( Out, Code, LZW_BITS );
	WriteBits( Out, LZW_MAX_VALUE, LZW_BITS );
	WriteBits( Out, 0, LZW_BITS );

	// Set out buffer length.
	OutSize		= (SizeT)(Out - OutBuffer);
	OutBuffer	= MemRealloc( OutBuffer, OutSize );
}


//
// Decode LZW compressed data.
//
void CLZWCompressor::Decode( const void* InBuffer, SizeT InSize, void*& OutBuffer, SizeT& OutSize )
{
	// Setup buffers.
	Byte*		In		= (Byte*)InBuffer;

	// Out buffer size.
	OutSize		= *(SizeT*)In;
	In			+= sizeof(SizeT);

	// Allocate out buffer size.
	OutBuffer		= MemAlloc(OutSize);
	Byte*	Out		= (Byte*)OutBuffer;

	// Setup LZW table.
	MemZero( GLZWStack, sizeof(GLZWStack) );
	for( SizeT i=0; i<LZW_TABLE_SIZE; i++ )
		GLZWTable[i].Code	= -1;

	// Reset bits walking.
	GBitsAccum	= 0;
	GBitsNum	= 0;

	// Setup LZW.
	SizeT	Value=0;
	DWord	NextCode=0, NewCode=0, OldCode=0;
	DWord	Count=0;

	NextCode	= 256;
	OldCode		= ReadBits( In, LZW_BITS );
	if( OldCode == 256 )
		return;
	Value	= OldCode;

	*Out++	= OldCode;

	while( (NewCode = ReadBits(In, LZW_BITS)) != LZW_MAX_VALUE )
	{
		// Check for some problems.
		Byte*	Str;
		if( NewCode >= NextCode )
		{
			GLZWStack[0]	= (Byte)Value;
			Str				= DecodeString( GLZWStack+1, OldCode );
		}
		else
			Str				= DecodeString( GLZWStack+0, NewCode );

		// Write unpacked string.
		Value	= *Str;
		while( Str >= GLZWStack )
			*Out++	= *Str--;

		// Update lzw table.
		if( NextCode <= LZW_MAX_CODE )
		{
			GLZWTable[NextCode].Prefix	= OldCode;
			GLZWTable[NextCode].Append	= Value;
			NextCode++;
		}
		OldCode	= NewCode;
	}
}


//
// Forecast the size of the source data 
// after a LZW compression.
//
SizeT CLZWCompressor::ForecastSize( const void* InBuffer, SizeT InSize )
{
	Byte*	In		= (Byte*)InBuffer;	
	Byte*	Eof		= In + InSize;
	SizeT	NumBits	= 0;

	// Output length of source data.
	NumBits	+= sizeof(SizeT)*8;

	// Setup LZW table.
	for( SizeT i=0; i<LZW_TABLE_SIZE; i++ )
		GLZWTable[i].Code	= -1;

	// Reset bits walking.
	GBitsAccum	= 0;
	GBitsNum	= 0;

	SizeT	NextCode	= 256, 
			Code		= 0, 
			Value		= 0;

	// Read first code.
	Code	= InSize > 0 ? *In++ : 256;

	// Read code by code.
	while( In < Eof )
	{
		Value			= *In++;
		SizeT iEntry	= FindLZWEntry( Code, Value );

		if( GLZWTable[iEntry].Code != -1 )
		{
			// Entry found.
			Code	= GLZWTable[iEntry].Code;
		}
		else
		{
			// Entry not found.
			// Add to dictionary.
			if( NextCode <= LZW_MAX_CODE )
			{
				GLZWTable[iEntry].Code		= NextCode++;
				GLZWTable[iEntry].Prefix	= Code;
				GLZWTable[iEntry].Append	= Value;
			}

			// Write bits.
			NumBits	+= LZW_BITS;
			Code	= Value;
		}
	}

	// Write markers.
	NumBits	+= LZW_BITS*3;

	// Compute output size.
	return NumBits / 8;
}


/*-----------------------------------------------------------------------------
    CFutileCompressor.
-----------------------------------------------------------------------------*/

//
// Futile compressor constructor.
//
CFutileCompressor::CFutileCompressor()
{
}


//
// Encode data using Futile compression.
//
void CFutileCompressor::Encode( const void* InBuffer, SizeT InSize, void*& OutBuffer, SizeT& OutSize )
{
	OutBuffer	= MemMalloc(InSize);
	OutSize		= InSize;
	MemCopy( OutBuffer, InBuffer, InSize );
}


//
// Decode Futile compressed data.
//
void CFutileCompressor::Decode( const void* InBuffer, SizeT InSize, void*& OutBuffer, SizeT& OutSize )
{
	OutBuffer	= MemMalloc(InSize);
	OutSize		= InSize;
	MemCopy( OutBuffer, InBuffer, InSize );
}


//
// Forecast the size of the source data 
// after a hideous compression.
//
SizeT CFutileCompressor::ForecastSize( const void* InBuffer, SizeT InSize )
{
	return InSize;
}


/*-----------------------------------------------------------------------------
    CRLECompressor.
-----------------------------------------------------------------------------*/

// Threshold for RLE processing.
// Should be at least RLE_THRESH same characters
// in sequence.
#define RLE_THRESH		5


//
// RLE constructor.
//
CRLECompressor::CRLECompressor()
{
}


//
// Encode data using RLE compression.
//
void CRLECompressor::Encode( const void* InBuffer, SizeT InSize, void*& OutBuffer, SizeT& OutSize )
{
	// Allocate out buffer, with some extra memory
	// just in case.
	OutBuffer	= MemMalloc(InSize+InSize/2+4);
	OutSize		= 0;

	Byte*	In	= (Byte*)InBuffer;	
	Byte*	Out	= (Byte*)OutBuffer;

	// Write source length.
	*(SizeT*)Out	= InSize;
	OutSize			+= sizeof(SizeT);

	// Process data.
	for( SizeT iWalk=0; iWalk<InSize; )
	{
		SizeT Count = 0;
		Byte C = In[iWalk];

		// Process run-length.
		while( In[iWalk]==C && iWalk<InSize && Count<0xff )
		{
			Count++;
			iWalk++;
		}

		// Write it.
		for( SizeT i=0; i<Min<SizeT>(RLE_THRESH, Count); i++ )
			Out[OutSize++]	= C;
		if( Count >= RLE_THRESH )
		{
			Byte Len = Count-RLE_THRESH;		
			Out[OutSize++]	= Len;
		}
	}

	// Set-up real size for out buffer.
	OutBuffer	= MemRealloc( OutBuffer, OutSize );
}


//
// Decode RLE compressed data.
//
void CRLECompressor::Decode( const void* InBuffer, SizeT InSize, void*& OutBuffer, SizeT& OutSize )
{
	// Setup.
	Byte*	In		= (Byte*)InBuffer;
	SizeT	iWalk	= 0;

	OutSize		= *(SizeT*)In;
	iWalk		+= sizeof(SizeT);

	OutBuffer	= MemMalloc(OutSize);
	Byte*	Out		= (Byte*)OutBuffer;
	SizeT	oWalk	= 0;
	
	// Process data.
	while( iWalk<InSize )
	{
		SizeT Count = 0;
		Byte C = In[iWalk];

		// Process run-length.
		while( In[iWalk]==C && iWalk<InSize && Count<RLE_THRESH )
		{
			Count++;
			iWalk++;
		}

		// Output.
		for( SizeT i=0; i<Count; i++ )
			Out[oWalk++]	= C;   
		if( Count == RLE_THRESH )
		{
			Byte Len = In[iWalk++];
			for( SizeT i=0; i<Len; i++ )
				Out[oWalk++]	= C;
		}
	}
}


//
// Forecast the size of the source data 
// after a RLE compression.
//
SizeT CRLECompressor::ForecastSize( const void* InBuffer, SizeT InSize )
{
	SizeT	TotalSize	= 0;
	Byte*	In	= (Byte*)InBuffer;	

	TotalSize	+= sizeof(SizeT);

	// Process data.
	for( SizeT iWalk=0; iWalk<InSize; )
	{
		SizeT Count = 0;
		Byte C = In[iWalk];

		// Process run-length.
		while( In[iWalk]==C && iWalk<InSize && Count<0xff )
		{
			Count++;
			iWalk++;
		}

		// Write it.
		TotalSize	+= Min<SizeT>(RLE_THRESH, Count);
		if( Count >= RLE_THRESH )
			TotalSize += 1;		
	}

	return TotalSize;
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/