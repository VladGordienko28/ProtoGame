/*=============================================================================
    FrAnimPage.cpp: Animation page implementation.
    Copyright Oct.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

//
// Preview animation play speed.
//
#define ANIM_PREVIEW_SPEED	0.2f	// 5 fps.


/*-----------------------------------------------------------------------------
    WAnimationPlayer.
-----------------------------------------------------------------------------*/

//
// An animation player.
//
class WAnimationPlayer: public WPanel
{
public:
	// Variables.
	FAnimation*			Animation;
	Int32*				FramePtr;

	// WAnimationPlayer interface.
	WAnimationPlayer( FAnimation* InAnimation, Int32* InFramePtr, WContainer* InOwner, WWindow* InRoot );
	void OnPaint( CGUIRenderBase* Render );
};


/*-----------------------------------------------------------------------------
    WAnimationPage implementation.
-----------------------------------------------------------------------------*/

//
// Animation page constructor.
//
WAnimationPage::WAnimationPage( FAnimation* InAnimation, WContainer* InOwner, WWindow* InRoot )
	:	WEditorPage( InOwner, InRoot ),
		Animation( InAnimation ),
		Scale( 1.f ),
		Pan( 0, 0 ),
		iFrame( -1 ),
		bPreview( false )
{
	// Initialize own fields.
	Padding						= TArea( 0, 0, 0, 0 );
	Animation					= InAnimation;
	Caption						= Animation->GetName();
	TabWidth					= Root->Font1->textWidth( *Caption ) + 30;
	PageType					= PAGE_Animation;
	Color						= PAGE_COLOR_ANIMATION;

	// Create toolbar and buttons.
	ToolBar						= new WToolBar( this, Root );
	ToolBar->SetSize( 3000, 28 );

	ZoomInButton				= new WButton( ToolBar, Root );
	ZoomInButton->Caption		= L"+";
	ZoomInButton->Tooltip		= L"Zoom In";
	ZoomInButton->EventClick	= WIDGET_EVENT(WAnimationPage::ButtonZoomInClick);	
	ZoomInButton->SetSize( 25, 22 );
	ToolBar->AddElement( ZoomInButton );

	ZoomOutButton				= new WButton( ToolBar, Root );
	ZoomOutButton->Caption		= L"-";
	ZoomOutButton->Tooltip		= L"Zoom Out";
	ZoomOutButton->EventClick	= WIDGET_EVENT(WAnimationPage::ButtonZoomOutClick);
	ZoomOutButton->SetSize( 25, 22 );
	ToolBar->AddElement( ZoomOutButton );

	LeftPanel					= new WPanel( this, Root );
	LeftPanel->Align			= AL_Left;
	LeftPanel->SetSize( 216, 260 );						

	PreviewPanel				= new WPanel( LeftPanel, Root );
	PreviewPanel->SetLocation( 4, 4 );
	PreviewPanel->SetSize( 208, 233 );								

	PlayerPanel					= new WAnimationPlayer( Animation, &iFrame, PreviewPanel, Root );	
	PlayerPanel->SetSize( 200, 200 );
	PlayerPanel->SetLocation( 4, 4 );

	PlayButton					= new WButton( PreviewPanel, Root );
	PlayButton->Caption			= L"Play";
	PlayButton->Tooltip			= L"Play Animation";
	PlayButton->bEnabled		= false;
	PlayButton->EventClick		= WIDGET_EVENT(WAnimationPage::ButtonPlayClick);
	PlayButton->SetLocation( 4, 208 );
	PlayButton->SetSize( 95, 20 );

	StopButton					= new WButton( PreviewPanel, Root );
	StopButton->Caption			= L"Stop";
	StopButton->Tooltip			= L"Stop Animation";
	StopButton->bEnabled		= false;
	StopButton->EventClick		= WIDGET_EVENT(WAnimationPage::ButtonStopClick);
	StopButton->SetLocation( 110, 208 );
	StopButton->SetSize( 95, 20 );

	SeqPanel					= new WPanel( LeftPanel, Root );
	SeqPanel->SetLocation( 4, 241 );
	SeqPanel->SetSize( 208, 246 );		

	SeqsList					= new WListBox( SeqPanel, Root );
	SeqsList->EventChange		= WIDGET_EVENT(WAnimationPage::ListSequencesChange);
	SeqsList->SetLocation( 4, 4 );
	SeqsList->SetSize( 200, 170 );

	SeqNameEdit					= new WEdit( SeqPanel, Root );
	SeqNameEdit->EditType		= EDIT_String;
	SeqNameEdit->bEnabled		= false;
	SeqNameEdit->EventChange	= WIDGET_EVENT(WAnimationPage::EditNameChange);
	SeqNameEdit->SetSize( 200, 18 );
	SeqNameEdit->SetLocation( 4, 178 );

	StartLabel					= new WLabel( SeqPanel, Root );
	StartLabel->Caption			= L"From:";
	StartLabel->SetLocation( 4, 200 );

	StartFrmEdit				= new WEdit( SeqPanel, Root );
	StartFrmEdit->EditType		= EDIT_Integer;
	StartFrmEdit->bEnabled		= false;
	StartFrmEdit->Text			= L"0";
	StartFrmEdit->EventChange	= WIDGET_EVENT(WAnimationPage::EditStartChange);
	StartFrmEdit->SetSize( 50, 18 );
	StartFrmEdit->SetLocation( 45, 200 );

	CountLabel					= new WLabel( SeqPanel, Root );
	CountLabel->Caption			= L"Count:";
	CountLabel->SetLocation( 110, 200 );
	
	CountFrmsEdit				= new WEdit( SeqPanel, Root );
	CountFrmsEdit->EditType		= EDIT_Integer;
	CountFrmsEdit->bEnabled		= false;
	CountFrmsEdit->Text			= L"0";
	CountFrmsEdit->EventChange	= WIDGET_EVENT(WAnimationPage::EditCountChange);
	CountFrmsEdit->SetSize( 50, 18 );
	CountFrmsEdit->SetLocation( 154, 200 );

	NewSeqButton				= new WButton( SeqPanel, Root );
	NewSeqButton->Caption		= L"New";
	NewSeqButton->Tooltip		= L"New Sequence";
	NewSeqButton->EventClick	= WIDGET_EVENT(WAnimationPage::ButtonNewClick);
	NewSeqButton->SetLocation( 4, 222 );
	NewSeqButton->SetSize( 95, 20 );

	DeleteSeqButton				= new WButton( SeqPanel, Root );
	DeleteSeqButton->Caption	= L"Delete";
	DeleteSeqButton->Tooltip	= L"Delete Sequence";
	DeleteSeqButton->EventClick	= WIDGET_EVENT(WAnimationPage::ButtonDeleteClick);
	DeleteSeqButton->bEnabled	= false;
	DeleteSeqButton->SetLocation( 110, 222 );
	DeleteSeqButton->SetSize( 95, 20 );

	// Fill lists by animation.
	UpdateControls();
}


//
// Tick the animation page.
//
void WAnimationPage::TickPage( Float Delta )
{
	WEditorPage::TickPage( Delta );

	if( bPreview && SeqsList->ItemIndex != -1 )
	{
		// Tick animation.
		static Float AnimTime = 0.f;

		TAnimSequence* Seq = (TAnimSequence*)SeqsList->Items[SeqsList->ItemIndex].Data;

		AnimTime += Delta;
		if( AnimTime >= ANIM_PREVIEW_SPEED )
		{
			iFrame		= (iFrame - Seq->Start + 1) % Seq->Count + Seq->Start;
			AnimTime	= 0.f;
		}
	}
	else
	{
		// Animation are not played.
		if( SeqsList->ItemIndex != -1 )
		{
			// Draw still frame.
			TAnimSequence* Seq = (TAnimSequence*)SeqsList->Items[SeqsList->ItemIndex].Data;
			iFrame	= Seq->Start;
		}
		else
		{
			// Nothing to draw.
			iFrame	= -1;
		}
	}
}


//
// Delete selected animation.
//
void WAnimationPage::ButtonDeleteClick( WWidget* Sender )
{
	if( SeqsList->ItemIndex != -1 )
	{
		Animation->Sequences.removeShift( SeqsList->ItemIndex );
		UpdateControls();
	}
}


//
// Play animation.
//
void WAnimationPage::ButtonPlayClick( WWidget* Sender )
{
	if( SeqsList->ItemIndex != -1 )
	{
		TAnimSequence* Seq = (TAnimSequence*)SeqsList->Items[SeqsList->ItemIndex].Data;
		iFrame		= Seq->Start;
		bPreview	= true;

		PlayButton->bEnabled	= false;
		StopButton->bEnabled	= true;
	}
}


//
// Stop animation playing.
//
void WAnimationPage::ButtonStopClick( WWidget* Sender )
{
	bPreview	= false;

	PlayButton->bEnabled	= true;
	StopButton->bEnabled	= false;
}


//
// Change index of start frame in the selected
// sequence.
//
void WAnimationPage::EditStartChange( WWidget* Sender )
{
	if( SeqsList->ItemIndex != -1 )
	{
		TAnimSequence* Seq = (TAnimSequence*)SeqsList->Items[SeqsList->ItemIndex].Data;
		StartFrmEdit->Text.toInteger( Seq->Start, 0 );
		Seq->Start	= clamp( Seq->Start, 0, 255 );
	}
}


//
// Change number of frames in the selected
// sequence.
//
void WAnimationPage::EditCountChange( WWidget* Sender )
{
	if( SeqsList->ItemIndex != -1 )
	{
		TAnimSequence* Seq = (TAnimSequence*)SeqsList->Items[SeqsList->ItemIndex].Data;
		CountFrmsEdit->Text.toInteger( Seq->Count, 0 );
		Seq->Count	= clamp( Seq->Count, 1, 255 );
	}
}


//
// Change sequence name.
//
void WAnimationPage::EditNameChange( WWidget* Sender )
{
	if( SeqsList->ItemIndex != -1 )
	{
		TAnimSequence* Seq = (TAnimSequence*)SeqsList->Items[SeqsList->ItemIndex].Data;
		Seq->Name = SeqsList->Items[SeqsList->ItemIndex].Name = SeqNameEdit->Text;
	}
}


//
// When selected sequence changed in list.
//
void WAnimationPage::ListSequencesChange( WWidget* Sender )
{
	if( SeqsList->ItemIndex != -1 )
	{
		// Regular item.
		TAnimSequence* Selected = (TAnimSequence*)SeqsList->Items[SeqsList->ItemIndex].Data;

		SeqNameEdit->Text			= Selected->Name;
		StartFrmEdit->Text			= String::format( L"%i", Selected->Start );
		CountFrmsEdit->Text			= String::format( L"%i", Selected->Count );

		SeqNameEdit->bEnabled		= true;
		StartFrmEdit->bEnabled		= true;
		CountFrmsEdit->bEnabled		= true;
		DeleteSeqButton->bEnabled	= true;
		PlayButton->bEnabled		= true;
		StopButton->bEnabled		= false;
	}
	else
	{
		// Bad item.
		SeqNameEdit->Text			= L"";
		StartFrmEdit->Text			= L"0";
		CountFrmsEdit->Text			= L"0";

		SeqNameEdit->bEnabled		= false;
		StartFrmEdit->bEnabled		= false;
		CountFrmsEdit->bEnabled		= false;
		DeleteSeqButton->bEnabled	= false;
		PlayButton->bEnabled		= false;
		StopButton->bEnabled		= false;
	}

	// Stop animation anyway.
	bPreview	= false;
}


//
// Add new animation sequence.
//
void WAnimationPage::ButtonNewClick( WWidget* Sender )
{
	// Generate unique name.
	String SeqName = L"";
	for( Int32 i=0; ; i++ )
	{
		String TestName = String::format( L"Sequence_%d", i );
		if( Animation->FindSequence(TestName) == -1 )
		{
			SeqName	= TestName;
			break;
		}
	}

	// Allocate new sequence.
	TAnimSequence NewSeq;
	NewSeq.Start	= 0;
	NewSeq.Count	= 1;
	NewSeq.Name		= SeqName;
	Animation->Sequences.push( NewSeq );

	// Refresh list.
	UpdateControls();

	// Highlight new sequence.
	if( Animation->Sequences.size() > 0 )
		SeqsList->SetItemIndex( SeqsList->Items.size()-1, true );
}


//
// Refresh list of sequences.
//
void WAnimationPage::UpdateControls()
{
	SeqsList->Empty();
	for( Int32 i=0; i<Animation->Sequences.size(); i++ )
		SeqsList->AddItem( Animation->Sequences[i].Name, &Animation->Sequences[i] );

	ListSequencesChange( this );
}


//
// Draw the animation sprite sheet table.
//
void WAnimationPage::OnPaint( CGUIRenderBase* Render )
{
	TPoint	Base	= ClientToWindow(TPoint( LeftPanel->Size.Width, 0 ));
	TSize	Size	= TSize( this->Size.Width - LeftPanel->Size.Width, this->Size.Height );

	// Clip to page.
	Render->SetClipArea( Base, Size );	

	// Draw cool backdrop, with pattern!
	Render->DrawRegion
					( 
						Base, 
						TSize( Size.Width, Size.Height ), 
						math::Color( 0x30, 0x30, 0x30, 0xff ), 
						math::Color( 0x30, 0x30, 0x30, 0xff ), 
						BPAT_Solid 
					);

	Render->DrawRegion
					( 
						Base, 
						TSize( Size.Width, Size.Height ), 
						math::Color( 0x3f, 0x3f, 0x3f, 0xff ), 
						math::Color( 0x3f, 0x3f, 0x3f, 0xff ), 
						BPAT_PolkaDot 
					);

	// Draw animation sprite sheet.
	if( Animation && Animation->Sheet )
	{
		FTexture* Texture	= Animation->Sheet;
		Int32 X = Base.X - Pan.X + ( Size.Width - Texture->getUSize() * Scale ) / 2;
		Int32 Y = Base.Y - Pan.Y + ( Size.Height - Texture->getVSize() * Scale ) / 2;
		Int32 W = Texture->getUSize() * Scale;
		Int32 H = Texture->getVSize() * Scale;

		// Draw border.
		Render->DrawRegion
					( 
						TPoint( X, Y-1 ), 
						TSize( W+1, H+1 ), 
						math::colors::BLACK, 
						math::colors::BLACK, 
						BPAT_None 
					);

		// Draw image.
		Render->DrawImage
						( 
							TPoint( X, Y ), 
							TSize( W, H ), 
							TPoint( 0, 0 ), 
							TSize( Texture->getUSize(), Texture->getVSize() ), 
							As<FBitmap>(Texture)->m_image  
						);

		// Draw cells.
		Int32 FrmPerX	= math::floor((Float)(Animation->Sheet->getUSize()+Animation->SpaceX) / (Float)(Animation->FrameW+Animation->SpaceX));		
		Int32 FrmPerY	= math::floor((Float)(Animation->Sheet->getVSize()+Animation->SpaceY) / (Float)(Animation->FrameH+Animation->SpaceY));

		// Vertical lines.
		for( Int32 iX=0; iX<=FrmPerX; iX++ )
		{
			TSize Size	= TSize( 1, H );
			TPoint Pos	= TPoint
							(
								X + iX*(Animation->FrameW+Animation->SpaceX) * Scale, 
								Y
							);
			if( iX != 0 )
				Render->DrawRegion( Pos, Size, math::colors::BLACK, math::colors::BLACK, BPAT_Solid );
			if( Animation->SpaceX && iX<FrmPerX )
				Render->DrawRegion(TPoint(Pos.X+Animation->FrameW*Scale, Pos.Y), Size, math::colors::BLACK, math::colors::BLACK, BPAT_Solid );
		}

		// Horizontal lines.
		for( Int32 iY=0; iY<=FrmPerY; iY++ )
		{
			TSize Size	= TSize( W, 1 );
			TPoint Pos	= TPoint
							(
								X, 
								Y + iY*(Animation->FrameH+Animation->SpaceY) * Scale
							);
			if( iY != 0 )
				Render->DrawRegion( Pos, Size, math::colors::BLACK, math::colors::BLACK, BPAT_Solid );
			if( Animation->SpaceY && iY<FrmPerY )
				Render->DrawRegion(TPoint(Pos.X, Pos.Y+Animation->FrameH*Scale), Size, math::colors::BLACK, math::colors::BLACK, BPAT_Solid );
		}

		// Number each tile.
		for( Int32 iY=0; iY<FrmPerY; iY++ )
		for( Int32 iX=0; iX<FrmPerX; iX++ )
		{
			Int32 iTile	= iY*FrmPerX + iX;
			TPoint Pos	= TPoint
							(
								X + iX*(Animation->FrameW+Animation->SpaceX) * Scale + 3,  
								Y + iY*(Animation->FrameH+Animation->SpaceY) * Scale + 3
							);

			Render->DrawText( Pos, String::format(L"%i", iTile), GUI_COLOR_TEXT, Root->Font2 );
		}
	}

	// Draw animation's sprite sheet zoom.
	Render->DrawText
				( 
					TPoint( Base.X + 10, Base.Y + 38 ), 
					String::format( L"x%.2f", Scale ), 
					math::colors::WHITE, 
					Root->Font1 
				);
}


//
// When user drag something over animation panel.
//
void WAnimationPage::OnDragOver( void* Data, Int32 X, Int32 Y, Bool& bAccept )
{
	FObject* Res = (FObject*)Data;
	bAccept	= Res && Res->IsA(FTexture::MetaClass);
}


//
// User has drop something on panel.
//
void WAnimationPage::OnDragDrop( void* Data, Int32 X, Int32 Y )
{
	FObject* Res = (FObject*)Data;

	assert(Res && Res->IsA(FTexture::MetaClass));
	assert(Animation);

	Animation->Sheet	= As<FTexture>(Res);
}


//
// Zoom in sprite sheet.
//
void WAnimationPage::ButtonZoomInClick( WWidget* Sender )
{
	Scale	= min( 4.f, 2.f*Scale );
}


//
// Zoom out sprite sheet.
//
void WAnimationPage::ButtonZoomOutClick( WWidget* Sender )
{
	Scale	= max( 0.25f, Scale*0.5f );
}



//
// When page has been opened/reopened.
//
void WAnimationPage::OnOpen()
{
	// Let inspector show animation's properties.
	if( Animation )
		GEditor->Inspector->SetEditObject( Animation );
}


//
// Ask user for page closing.
//
Bool WAnimationPage::OnQueryClose()
{
	// Ok, let user close the page without
	// any asking.
	return true;
}


//
// User has down on animation page.
//
void WAnimationPage::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y )
{
	WEditorPage::OnMouseDown( Button, X, Y );
}


//
// User has release mouse button.
//
void WAnimationPage::OnMouseUp( EMouseButton Button, Int32 X, Int32 Y )
{
	WEditorPage::OnMouseUp( Button, X, Y );
}


//
// Mouse move animation sprite sheet.
//
void WAnimationPage::OnMouseMove( EMouseButton Button, Int32 X, Int32 Y )
{
	WEditorPage::OnMouseMove( Button, X, Y );
	static TPoint OldPos = TPoint( 0, 0 ); 

	// Pan bitmap.
	if( Button == MB_Left )
	{
		Pan.X		-= X - OldPos.X;
		Pan.Y		-= Y - OldPos.Y;
	}

	// Store last location.
	OldPos.X	= X;
	OldPos.Y	= Y;
}


/*-----------------------------------------------------------------------------
    WAnimationPlayer implementation.
-----------------------------------------------------------------------------*/

//
// Animation preview player constructor.
//
WAnimationPlayer::WAnimationPlayer( FAnimation* InAnimation, Int32* InFramePtr, WContainer* InOwner, WWindow* InRoot )
	:	WPanel( InOwner, InRoot ),
		Animation( InAnimation ),
		FramePtr( InFramePtr )

{
}


//
// Draw preview animation.
//
void WAnimationPlayer::OnPaint( CGUIRenderBase* Render )
{
	WPanel::OnPaint( Render );

	// Check animation.
	if( !Animation || !Animation->Sheet )
		return;

	// Check frame.
	Int32 Frame = *FramePtr;
	if( Frame < 0 || Frame >= Animation->Frames.size() )
		return;

	math::Rect R = Animation->GetTexCoords( Frame );

	// Clip to viewport.
	TPoint Base = ClientToWindow(TPoint::Zero);
	Render->SetClipArea( Base, Size );

	Int32	X	= math::floor( R.min.x * Animation->Sheet->getUSize() ),
			Y	= math::floor( R.min.y * Animation->Sheet->getVSize() ),
			W	= math::floor( (R.max.x-R.min.x) * Animation->Sheet->getUSize() ),
			H	= math::floor( (R.max.y-R.min.y) * Animation->Sheet->getVSize() );

	Int32 DestX	= Base.X + (Size.Width - W) / 2,
			DestY	= Base.Y + (Size.Height - H) / 2;

	// Draw frame.
	Render->DrawImage
					( 
						TPoint( DestX, DestY ),
						TSize( W, H ),
						TPoint( X, Y ),
						TSize( W, H ),
						As<FBitmap>(Animation->Sheet)->m_image
					);

	// Write frame.
	Render->DrawText
				(
					TPoint( Base.X+4, Base.Y+4 ),
					String::format( L"Frame = %i", Frame ),
					math::colors::WHITE,
					Root->Font1
				);
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/