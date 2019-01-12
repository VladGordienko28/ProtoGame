/*=============================================================================
    FrAnim.cpp: FAnimation.
    Copyright Aug.2016 Vlad Gordienko.
	Totally rewritten Oct.2016.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    FAnimation implementation.
-----------------------------------------------------------------------------*/

//
// Animation constructor.
//
FAnimation::FAnimation()
	:	FResource(),
		Sheet( nullptr ),
		Sequences(),
		FrameW( 32 ),
		FrameH( 32 ),
		SpaceX( 0 ),
		SpaceY( 0 ),
		Frames()
{}


//
// Find animation sequence by it name. If no sequence
// found return nullptr.
//
Int32 FAnimation::FindSequence( String InName )
{
	for( Int32 iSeq=0; iSeq<Sequences.Num(); iSeq++ )
		if( Sequences[iSeq].Name == InName )
			return iSeq;

	return -1;
}


//
// Return the texture coords for the frame iFrame.
// If iFrame are invalid, return an identity rectangle.
//
TRect FAnimation::GetTexCoords( Int32 iFrame )
{
	return iFrame>=0 && iFrame<Frames.Num() ? 
				Frames[iFrame] : 
				TRect( TVector(0.5f, 0.5f), 1.f );
}


// Sequence serialization.
void Serialize( CSerializer& S, TAnimSequence& V )
{
	Serialize( S, V.Name );
	Serialize( S, V.Start );
	Serialize( S, V.Count );
}


//
// Serialize the animation.
//
void FAnimation::SerializeThis( CSerializer& S )
{
	FResource::SerializeThis( S );

	Serialize( S, Sheet );
	Serialize( S, Sequences );
	Serialize( S, FrameW );
	Serialize( S, FrameH );
	Serialize( S, SpaceX );
	Serialize( S, SpaceY );
}


//
// Some field changed.
//
void FAnimation::EditChange()
{
	FResource::EditChange();
	SetFramesTable();
}


//
// After loading initialization.
//
void FAnimation::PostLoad()
{
	FResource::PostLoad();
	SetFramesTable();
}


//
// Import animation.
//
void FAnimation::Import( CImporterBase& Im )
{
	FResource::Import( Im );
	Sequences.SetNum(Im.ImportInteger(L"NumSeqs"));

	for( Int32 iSeq=0; iSeq<Sequences.Num(); iSeq++ )
	{
		TAnimSequence& S = Sequences[iSeq];

		S.Name	= Im.ImportString(*String::Format( L"Sequences[%d].Name", iSeq ));
		S.Start	= Im.ImportInteger(*String::Format( L"Sequences[%d].Start", iSeq ));
		S.Count	= Im.ImportInteger(*String::Format( L"Sequences[%d].Count", iSeq ));
	}
}


//
// Export animation.
//
void FAnimation::Export( CExporterBase& Ex )
{
	FResource::Export( Ex );
	Ex.ExportInteger( L"NumSeqs", Sequences.Num() );

	for( Int32 iSeq=0; iSeq<Sequences.Num(); iSeq++ )
	{
		TAnimSequence& S = Sequences[iSeq];

		Ex.ExportString( *String::Format( L"Sequences[%d].Name", iSeq ), S.Name );
		Ex.ExportInteger( *String::Format( L"Sequences[%d].Start", iSeq ), S.Start );
		Ex.ExportInteger( *String::Format( L"Sequences[%d].Count", iSeq ), S.Count );
	}
}


//
// Precompute texture coords for each frame
// in the animation.
//
void FAnimation::SetFramesTable()
{
	Frames.Empty();

	if( !Sheet || FrameH*FrameW==0 )
		return;

	Int32 XSize	= 1 << Sheet->UBits;
	Int32 YSize	= 1 << Sheet->VBits;

	// Without horrible whitespace after frames, for 
	// power-of-two size.
	Int32 FrmPerX	= Floor((Float)(XSize+SpaceX) / (Float)(FrameW+SpaceX));		
	Int32 FrmPerY	= Floor((Float)(YSize+SpaceY) / (Float)(FrameH+SpaceY));

	for( Int32 Y=0; Y<FrmPerY; Y++ )
	for( Int32 X=0; X<FrmPerX; X++ )
	{
		TRect R;

		R.Min.X	= (Float)((X+0)*(FrameW+SpaceX)) / (Float)(XSize);
		R.Min.Y	= (Float)((Y+0)*(FrameH+SpaceY)) / (Float)(YSize);

		R.Max.X	= (Float)((X+1)*(FrameW+SpaceX)-SpaceX) / (Float)(XSize);
		R.Max.Y	= (Float)((Y+1)*(FrameH+SpaceY)-SpaceY) / (Float)(YSize);

		// Flip V.
		Exchange( R.Min.Y, R.Max.Y );

		Frames.Push( R );
	}
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/