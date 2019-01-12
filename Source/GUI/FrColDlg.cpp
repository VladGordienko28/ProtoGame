/*=============================================================================
    FrColDlg.cpp: Color chooser dialog.
    Copyright Dec.2016 Vlad Gordienko.
=============================================================================*/

#include "GUI.h"

/*-----------------------------------------------------------------------------
    Color dialog bitmaps.
-----------------------------------------------------------------------------*/

//
// Bitmaps.
//
static	TStaticBitmap*		SLBitmap	= nullptr;
static	TStaticBitmap*		HBitmap		= nullptr;
static	TStaticBitmap*		ABitmap		= nullptr;


//
// Initialize bitmaps.
//
void InitHSLBitmaps()
{
	// HBitmap.
	if( !HBitmap )
	{
		HBitmap					= new TStaticBitmap();
		HBitmap->Format			= BF_RGBA;
		HBitmap->USize			= 1;
		HBitmap->VSize			= 256;
		HBitmap->UBits			= IntLog2(HBitmap->USize);
		HBitmap->VBits			= IntLog2(HBitmap->VSize);
		HBitmap->Filter			= BFILTER_Nearest;
		HBitmap->BlendMode		= BLEND_Regular;
		HBitmap->RenderInfo		= -1;
		HBitmap->PanUSpeed		= 0.f;
		HBitmap->PanVSpeed		= 0.f;
		HBitmap->Saturation		= 1.f;
		HBitmap->AnimSpeed		= 0.f;
		HBitmap->bDynamic		= false;
		HBitmap->bRedrawn		= false;
		HBitmap->Data.SetNum(256*sizeof(TColor));

		TColor* Data = (TColor*)HBitmap->GetData();
		for( Int32 i=0; i<256; i++ )
			Data[i]	= TColor::HSLToRGB( i, 0xff, 0x80 );
	}

	// ABitmap.
	if( !ABitmap )
	{
		ABitmap					= new TStaticBitmap();
		ABitmap->Format			= BF_RGBA;
		ABitmap->USize			= 4;
		ABitmap->VSize			= 4;
		ABitmap->UBits			= IntLog2(ABitmap->USize);
		ABitmap->VBits			= IntLog2(ABitmap->VSize);
		ABitmap->Filter			= BFILTER_Nearest;
		ABitmap->BlendMode		= BLEND_Alpha;
		ABitmap->RenderInfo		= -1;
		ABitmap->PanUSpeed		= 0.f;
		ABitmap->PanVSpeed		= 0.f;
		ABitmap->Saturation		= 1.f;
		ABitmap->AnimSpeed		= 0.f;
		ABitmap->bDynamic		= true;
		ABitmap->bRedrawn		= false;
		ABitmap->Data.SetNum(4*4*sizeof(TColor));

		TColor* Data = (TColor*)ABitmap->GetData();
		MemSet( Data, 4*4*sizeof(TColor), 0xff );
		Data[0] = Data[1] = Data[14] = Data[15] =
		Data[10] = Data[11] = Data[4] = Data[5] = TColor( 0x80, 0x80, 0x80, 0xff );
	}

	// SLBitmap.
	if( !SLBitmap )
	{
		SLBitmap				= new TStaticBitmap();
		SLBitmap->Format		= BF_RGBA;
		SLBitmap->USize			= 256;
		SLBitmap->VSize			= 256;
		SLBitmap->UBits			= IntLog2(SLBitmap->USize);
		SLBitmap->VBits			= IntLog2(SLBitmap->VSize);
		SLBitmap->Filter		= BFILTER_Nearest;
		SLBitmap->BlendMode		= BLEND_Regular;
		SLBitmap->RenderInfo	= -1;
		SLBitmap->PanUSpeed		= 0.f;
		SLBitmap->PanVSpeed		= 0.f;
		SLBitmap->Saturation	= 1.f;
		SLBitmap->AnimSpeed		= 0.f;
		SLBitmap->bDynamic		= true;
		SLBitmap->bRedrawn		= true;
		SLBitmap->Data.SetNum(256*256*sizeof(TColor));
	}
}


/*-----------------------------------------------------------------------------
    WColorChooser implementation.
-----------------------------------------------------------------------------*/

//
// Globals.
//
TColor		WColorChooser::SharedColor	= COLOR_White;


//
// Color chooser constructor.
//
WColorChooser::WColorChooser( WWindow* InRoot, Bool InbUseAlpha, TNotifyEvent InOk )
	:	WForm( InRoot, InRoot ),
		bUseAlpha( InbUseAlpha )
{
	// Dialog fields.
	bSizeableW			= false;
	bSizeableH			= false;
	bCanClose			= true;
	Caption				= L"Color Chooser";
	SetSize( 408, 328 );	
	SetLocation
	( 
		(Root->Size.Width-Size.Width)/2, 
		(Root->Size.Height-Size.Height)/2 
	);

	// Allocate controls.
	OkButton					= new WButton( this, Root );
	OkButton->Caption			= L"Ok";
	OkButton->EventClick		= WIDGET_EVENT(WColorChooser::ButtonOkClick);
	OkButton->SetSize( 75, 25 );
	OkButton->SetLocation( 10, 293 );

	CancelButton				= new WButton( this, Root );
	CancelButton->Caption		= L"Cancel";
	CancelButton->EventClick	= WIDGET_EVENT(WColorChooser::ButtonCancelClick);
	CancelButton->SetSize( 75, 25 );
	CancelButton->SetLocation( 90, 293 );

	RSpinner				= new WSpinner( this, Root );
	RSpinner->EditType		= EDIT_Integer;
	RSpinner->EventChange	= WIDGET_EVENT(WColorChooser::UpdateFromRGB);
	RSpinner->SetRange( 0, 255, 1 );
	RSpinner->SetSize( 36, 18 );
	RSpinner->SetLocation( 312, 102 );

	GSpinner				= new WSpinner( this, Root );
	GSpinner->EditType		= EDIT_Integer;
	GSpinner->EventChange	= WIDGET_EVENT(WColorChooser::UpdateFromRGB);
	GSpinner->SetRange( 0, 255, 1 );
	GSpinner->SetSize( 36, 18 );
	GSpinner->SetLocation( 312, 122 );

	BSpinner				= new WSpinner( this, Root );
	BSpinner->EditType		= EDIT_Integer;
	BSpinner->EventChange	= WIDGET_EVENT(WColorChooser::UpdateFromRGB);
	BSpinner->SetRange( 0, 255, 1 );
	BSpinner->SetSize( 36, 18 );
	BSpinner->SetLocation( 312, 142 );

	if( bUseAlpha )
	{
		ASpinner				= new WSpinner( this, Root );
		ASpinner->EditType		= EDIT_Integer;
		ASpinner->EventChange	= WIDGET_EVENT(WColorChooser::UpdateFromA);
		ASpinner->SetRange( 0, 255, 1 );
		ASpinner->SetSize( 36, 18 );
		ASpinner->SetLocation( 312, 162 );
	}
	else
	{
		ASpinner	= nullptr;
	}

	HSpinner				= new WSpinner( this, Root );
	HSpinner->EditType		= EDIT_Integer;
	HSpinner->EventChange	= WIDGET_EVENT(WColorChooser::UpdateFromHSL);
	HSpinner->SetRange( 0, 255, 1 );
	HSpinner->SetSize( 36, 18 );
	HSpinner->SetLocation( 362, 102 );

	SSpinner				= new WSpinner( this, Root );
	SSpinner->EditType		= EDIT_Integer;
	SSpinner->EventChange	= WIDGET_EVENT(WColorChooser::UpdateFromHSL);
	SSpinner->SetRange( 0, 255, 1 );
	SSpinner->SetSize( 36, 18 );
	SSpinner->SetLocation( 362, 122 );

	LSpinner				= new WSpinner( this, Root );
	LSpinner->EditType		= EDIT_Integer;
	LSpinner->EventChange	= WIDGET_EVENT(WColorChooser::UpdateFromHSL);
	LSpinner->SetRange( 0, 255, 1 );
	LSpinner->SetSize( 36, 18 );
	LSpinner->SetLocation( 362, 142 );

	// Init bitmaps, if they wasn't.
	InitHSLBitmaps();

	// Setup.
	EventOk		= InOk;
	Selected	= SharedColor;
	bMoveSL		= false;
	bMoveH		= false;
	RSpinner->SetValue(Selected.R, false);
	GSpinner->SetValue(Selected.G, false);
	BSpinner->SetValue(Selected.B, false);
	UpdateFromRGB( this );

	if( bUseAlpha )
		ASpinner->SetValue(Selected.A, false);

	// Store old modal, and current modal.
	OldModal		= Root->Modal;
	Root->Modal		= this;
}


//
// Color chooser destructor.
//
WColorChooser::~WColorChooser()
{
	// Restore old modal.
	Root->Modal	= OldModal;
}


//
// When dialog closed, its hides.
//
void WColorChooser::OnClose()
{
	WForm::OnClose();
	Hide();
}


//
// When user click 'Ok' button.
//
void WColorChooser::ButtonOkClick( WWidget* Sender )
{
	// Store color.
	SharedColor	= Selected;

	// Notify.
	EventOk( Sender );
	OnClose();
}


//
// When user click 'Cancel' button.
//
void WColorChooser::ButtonCancelClick( WWidget* Sender )
{
	OnClose();
}


//
// When dialog hides, its suicide.
//
void WColorChooser::Hide()
{
	WForm::Hide();
	delete this;
}


//
// Redraw color picker.
//
void WColorChooser::OnPaint( CGUIRenderBase* Render )
{ 
	// Call parent.
	WForm::OnPaint(Render); 
	TPoint Base	= ClientToWindow(TPoint::Zero);

	// Decompose selected color.
	Int32	H, S, L;
	H = HSpinner->GetIntValue();
	S = SSpinner->GetIntValue();
	L = LSpinner->GetIntValue();

	// Draw SL panel.
	Render->DrawPicture
	(
		TPoint( Base.X+10, Base.Y+30 ),
		TSize( 256, 256 ),
		TPoint( 0, 0 ),
		TSize( 256, 256 ),
		SLBitmap
	);
	Render->DrawPicture
	(
		TPoint( Base.X+10+S-5, Base.Y+30+L-5 ),
		TSize( 11, 11 ),
		TPoint( 39, 9 ),
		TSize( 11, 11 ),
		WWindow::Icons		
	);
	
	// Draw H panel.
	Render->DrawPicture
	(
		TPoint( Base.X+271, Base.Y+30 ),
		TSize( 15, 256 ),
		TPoint( 0, 0 ),
		TSize( 1, 256 ),
		HBitmap
	);
	Render->DrawPicture
	(
		TPoint( Base.X+286, Base.Y+30+H-5 ),
		TSize( 8, 9 ),
		TPoint( 39, 0 ),
		TSize( 8, 9 ),
		WWindow::Icons		
	);

	// Draw labels next to edits.
	Render->DrawText( TPoint(Base.X+302, Base.Y+104), L"R", 1, GUI_COLOR_TEXT, WWindow::Font1 );
	Render->DrawText( TPoint(Base.X+302, Base.Y+124), L"G", 1, GUI_COLOR_TEXT, WWindow::Font1 );
	Render->DrawText( TPoint(Base.X+302, Base.Y+144), L"B", 1, GUI_COLOR_TEXT, WWindow::Font1 );
	Render->DrawText( TPoint(Base.X+352, Base.Y+104), L"H", 1, GUI_COLOR_TEXT, WWindow::Font1 );
	Render->DrawText( TPoint(Base.X+352, Base.Y+124), L"S", 1, GUI_COLOR_TEXT, WWindow::Font1 );
	Render->DrawText( TPoint(Base.X+352, Base.Y+144), L"L", 1, GUI_COLOR_TEXT, WWindow::Font1 );

	if( bUseAlpha )
		Render->DrawText( TPoint(Base.X+302, Base.Y+164), L"A", 1, GUI_COLOR_TEXT, WWindow::Font1 );		

	// Draw new color.
	Render->DrawRegion
	(
		TPoint( Base.X+298, Base.Y+45 ),
		TSize( 50, 50 ),
		Selected,
		Selected,
		BPAT_Solid
	);
	Render->DrawText
	( 
		TPoint( Base.X+298, Base.Y+30 ), 
		L"New" , 3, 
		GUI_COLOR_TEXT, 
		WWindow::Font1 
	);
	if( bUseAlpha && Selected.A != 255 )
	{
		SetAlphaBlend(255-Selected.A);
		Render->DrawPicture
		(
			TPoint( Base.X+298, Base.Y+45 ),
			TSize( 50, 50 ),
			TPoint( 0, 0 ),
			TSize( 10, 10 ),
			ABitmap
		);
	}

	// Draw old color.
	Render->DrawRegion
	(
		TPoint( Base.X+348, Base.Y+45 ),
		TSize( 50, 50 ),
		SharedColor,
		SharedColor,
		BPAT_Solid
	);
	Render->DrawText
	( 
		TPoint( Base.X+348, Base.Y+30 ), 
		L"Old" , 3, 
		GUI_COLOR_TEXT, 
		WWindow::Font1 
	);
	if( bUseAlpha && SharedColor.A != 255 )
	{
		SetAlphaBlend(255-SharedColor.A);
		Render->DrawPicture
		(
			TPoint( Base.X+348, Base.Y+45 ),
			TSize( 50, 50 ),
			TPoint( 2, 0 ),
			TSize( 10, 10 ),
			ABitmap
		);
	}
}


//
// Process mouse movement over dialog.
//
void WColorChooser::OnMouseMove( EMouseButton Button, Int32 X, Int32 Y )
{ 
	WForm::OnMouseMove( Button, X, Y ); 

	if( bMoveH )
	{
		// Process Hue movement.
		Int32	Hue	= Clamp( Y-30, 0x00, 0xff );

		HSpinner->SetValue( Hue, false );
		UpdateFromHSL( this );
	}
	if( bMoveSL )
	{
		// Process Saturation/Lightness movement.
		Int32	Saturation	= Clamp( X-10, 0x00, 0xff ),
				Lightness	= Clamp( Y-30, 0x00, 0xff );

		SSpinner->SetValue( Saturation, false );
		LSpinner->SetValue( Lightness, false );
		UpdateFromHSL( this );
	}
}


//
// User just hold mouse button.
//
void WColorChooser::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y )
{ 
	WForm::OnMouseDown( Button, X, Y ); 

	if( Button != MB_Left )
		return;

	// Test for SL area.
	if( X>=10 && Y>=30 && X<=266 && Y<=286 )
	{
		bMoveSL	= true;
		OnMouseMove( Button, X, Y );
	}

	// Test for H area.
	if( X>=271 && Y>=30 && X<=294 && Y<=286 )
	{
		bMoveH	= true;
		OnMouseMove( Button, X, Y );
	}
}	


//
// User just release mouse button.
//
void WColorChooser::OnMouseUp( EMouseButton Button, Int32 X, Int32 Y )
{ 
	WForm::OnMouseUp( Button, X, Y ); 

	bMoveH	= false;
	bMoveSL	= false;
}


//
// Update color, when changed via RGB edits.
//
void WColorChooser::UpdateFromRGB( WWidget* Sender )
{
	Int32	R, G, B;
	R	= RSpinner->GetIntValue();
	G	= GSpinner->GetIntValue();
	B	= BSpinner->GetIntValue();

	Selected	= TColor
	(
		Clamp( R, 0x00, 0xff ),
		Clamp( G, 0x00, 0xff ),
		Clamp( B, 0x00, 0xff ),
		Selected.A
	);

	UInt8 H, S, L;
	TColor::RGBToHSL( Selected, H, S, L );
	HSpinner->SetValue( H, false );
	SSpinner->SetValue( S, false );
	LSpinner->SetValue( L, false );

	RefreshSL();
}


//
// Update color, when changed via HSL edits.
//
void WColorChooser::UpdateFromHSL( WWidget* Sender )
{
	Int32 H, S, L;

	H = HSpinner->GetIntValue();
	S = SSpinner->GetIntValue();
	L = LSpinner->GetIntValue();

	UInt8 Alpha = Selected.A;
	Selected	= TColor::HSLToRGB
	(
		Clamp( H, 0x00, 0xff ),
		Clamp( S, 0x00, 0xff ),
		Clamp( L, 0x00, 0xff )
	);
	Selected.A	= Alpha;

	RSpinner->SetValue(Selected.R, false);
	GSpinner->SetValue(Selected.G, false);
	BSpinner->SetValue(Selected.B, false);

	RefreshSL();
}


//
// Update Alpha value.
//
void WColorChooser::UpdateFromA( WWidget* Sender )
{
	Selected.A	= ASpinner->GetIntValue();
}


//
// Redraw saturation/lightness bitmap.
//
void WColorChooser::RefreshSL()
{
	TColor*	Data	= (TColor*)SLBitmap->GetData();

	// Decompose selected color.
	Int32	Hue;
	Hue = HSpinner->GetIntValue();

	for( Int32 L=0; L<256; L++ )
	{
		TColor	Col1	= TColor( L, L, L, 0xff );
		TColor	Col2	= TColor::HSLToRGB( Hue, 255, L );

		TColor*	Line	= &Data[L*256];

		// Use some fixed math 16:16 magic :3
		Int32	RWalk	= Col1.R * 65536,
				GWalk	= Col1.G * 65536,
				BWalk	= Col1.B * 65536,
				RStep	= (Col2.R-Col1.R) * 65536 / 256,
				GStep	= (Col2.G-Col1.G) * 65536 / 256,
				BStep	= (Col2.B-Col1.B) * 65536 / 256;

		for( Int32 S=0; S<256; S++ )
		{
			Line[S]	= TColor( RWalk>>16, GWalk>>16, BWalk>>16, 0xff );

			RWalk	+= RStep;
			GWalk	+= GStep;
			BWalk	+= BStep;
		}
	}

	// Force to reload.
	SLBitmap->bRedrawn	= true;
}


//
// Update Alpha value of Checker texture.
//
void WColorChooser::SetAlphaBlend( UInt8 Alpha )
{
	assert(bUseAlpha);
	TColor*	Data	= (TColor*)ABitmap->GetData();

	// Don't redraw, if not required.
	if( Alpha == Data[0].A )
		return;

	for( Int32 i=0; i<ABitmap->USize*ABitmap->VSize; i++ )
		Data[i].A = Alpha;

	// Force to reload.
	ABitmap->bRedrawn	= true;
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/