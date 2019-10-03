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
{
	SetFramesTable();
}


//
// Find animation sequence by it name. If no sequence
// found return nullptr.
//
Int32 FAnimation::FindSequence( String InName )
{
	for( Int32 iSeq=0; iSeq<Sequences.size(); iSeq++ )
		if( Sequences[iSeq].Name == InName )
			return iSeq;

	return -1;
}


//
// Return the texture coords for the frame iFrame.
// If iFrame are invalid, return an identity rectangle.
//
math::Rect FAnimation::GetTexCoords( Int32 iFrame )
{
	return iFrame>=0 && iFrame<Frames.size() ? 
				Frames[iFrame] : 
				math::Rect( math::Vector( 0.5f, 0.5f ), 1.f );
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
	Sequences.setSize(Im.ImportInteger(L"NumSeqs"));

	for( Int32 iSeq=0; iSeq<Sequences.size(); iSeq++ )
	{
		TAnimSequence& S = Sequences[iSeq];

		S.Name	= Im.ImportString(*String::format( L"Sequences[%d].Name", iSeq ));
		S.Start	= Im.ImportInteger(*String::format( L"Sequences[%d].Start", iSeq ));
		S.Count	= Im.ImportInteger(*String::format( L"Sequences[%d].Count", iSeq ));
	}
}


//
// Export animation.
//
void FAnimation::Export( CExporterBase& Ex )
{
	FResource::Export( Ex );
	Ex.ExportInteger( L"NumSeqs", Sequences.size() );

	for( Int32 iSeq=0; iSeq<Sequences.size(); iSeq++ )
	{
		TAnimSequence& S = Sequences[iSeq];

		Ex.ExportString( *String::format( L"Sequences[%d].Name", iSeq ), S.Name );
		Ex.ExportInteger( *String::format( L"Sequences[%d].Start", iSeq ), S.Start );
		Ex.ExportInteger( *String::format( L"Sequences[%d].Count", iSeq ), S.Count );
	}
}


//
// Precompute texture coords for each frame
// in the animation.
//
void FAnimation::SetFramesTable()
{
	Frames.empty();

	if( !Sheet || FrameH*FrameW==0 )
		return;

	Int32 XSize	= 1 << Sheet->getUBits();
	Int32 YSize	= 1 << Sheet->getVBits();

	// Without horrible whitespace after frames, for 
	// power-of-two size.
	Int32 FrmPerX	= math::floor((Float)(XSize+SpaceX) / (Float)(FrameW+SpaceX));		
	Int32 FrmPerY	= math::floor((Float)(YSize+SpaceY) / (Float)(FrameH+SpaceY));

	for( Int32 Y=0; Y<FrmPerY; Y++ )
	for( Int32 X=0; X<FrmPerX; X++ )
	{
		math::Rect R;

		R.min.x	= (Float)((X+0)*(FrameW+SpaceX)) / (Float)(XSize);
		R.min.y	= (Float)((Y+0)*(FrameH+SpaceY)) / (Float)(YSize);

		R.max.x	= (Float)((X+1)*(FrameW+SpaceX)-SpaceX) / (Float)(XSize);
		R.max.y	= (Float)((Y+1)*(FrameH+SpaceY)-SpaceY) / (Float)(YSize);

		// Flip V.
		exchange( R.min.y, R.max.y );

		Frames.push( R );
	}
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/