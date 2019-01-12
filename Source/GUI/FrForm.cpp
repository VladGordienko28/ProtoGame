/*=============================================================================
    FrForm.cpp: Form classes.
    Copyright Oct.2016 Vlad Gordienko.
=============================================================================*/

#include "GUI.h"

/*-----------------------------------------------------------------------------
    WForm implementation.
-----------------------------------------------------------------------------*/

//
// Form constructor.
//
WForm::WForm(  WContainer* InOwner, WWindow* InRoot  )
	:	WContainer( InOwner, InRoot ),
		bSizeableW( true ),
		bSizeableH( true ),
		bCanClose( true ),
		MinWindth( 128 ),
		MinHeight( FORM_HEADER_SIZE + 14 ),
		bIsMove( false ),
		bIsSize(false),
		HoldOffset( 0, 0 )
{
	// Set container properties.
	SetSize( 256, 256 );
	Padding		= TArea( FORM_HEADER_SIZE, 11, 0, 0 );	

	// Form stay on top.
	bStayOnTop	= true;
}


//
// User press button on form.
//
void WForm::OnMouseDown( EMouseButton Button, Integer X, Integer Y )
{
	WContainer::OnMouseDown( Button, X, Y );
	TPoint RealPos = ClientToWindow(TPoint(X, Y));

	// Reset flags.
	bIsMove		= false;
	bIsSize		= false;

	if( Button == MB_Left )
	{
		if( Y < FORM_HEADER_SIZE )
		{
			// Move or close button.
			if( X > Size.Width-15 && bCanClose )
			{
				OnClose();
			}
			else
				bIsMove	 = true;

			HoldOffset	= Location - RealPos;
		}
		else if( Y > Size.Height-8 && X > Size.Width-8 && ( bSizeableH || bSizeableW ))
		{
			// Resize.
			bIsSize			= true;
			HoldOffset		= TPoint( Size.Width-X, Size.Height-Y );
		}	
	}
}


//
// User release button.
//
void WForm::OnMouseUp( EMouseButton Button, Integer X, Integer Y )
{
	WContainer::OnMouseUp( Button, X, Y );

	bIsMove		= false;
	bIsSize		= false;
}


//
// Mouse move.
//
void WForm::OnMouseMove( EMouseButton Button, Integer X, Integer Y )
{
	WContainer::OnMouseMove( Button, X, Y );

	if( bIsMove )
	{
		TPoint RealPos	= ClientToWindow(TPoint( X, Y ));
		Location		= HoldOffset + RealPos;
	}
	if( bIsSize )
	{
		TPoint RealPos	= ClientToWindow( TPoint( X, Y ) );

		Integer NewWidth, NewHeight;

		NewWidth		= X + HoldOffset.X;
		NewWidth		= Clamp( NewWidth, MinWindth, Owner->Size.Width );

		NewHeight		= Y + HoldOffset.Y;
		NewHeight		= Clamp( NewHeight, MinHeight, Owner->Size.Height );

		if( bSizeableW && (Size.Width-NewWidth != 0) && (RealPos.X < Owner->Size.Width-13) )
		{
			SetSize( NewWidth, Size.Height );
		}
		if( bSizeableH && (Size.Height-NewHeight != 0) && (RealPos.Y < Owner->Size.Height-12) )
		{
			SetSize( Size.Width, NewHeight );
		}
	}

	// Change cursor style.
	if	(	
			bIsSize ||
			(( bSizeableH || bSizeableW ) && 
			Y > Size.Height-8 && 
			X > Size.Width-8) 
		)
		Cursor	= CR_SizeNWSE;
	else
		Cursor	= CR_Arrow;
}


//
// Redraw form.
//
void WForm::OnPaint( CGUIRenderBase* Render )
{
	WContainer::OnPaint(Render);

	// Clamp form into parent area.
	Location.X	= Max( Location.X, 10 );
	Location.Y	= Max( Location.Y, 10 );
	Location.X	= Min( Location.X, Owner->Size.Width-10-Size.Width );
	Location.Y	= Min( Location.Y, Owner->Size.Height-10-Size.Height );

	TPoint Base = ClientToWindow(TPoint::Zero);

	// Form background.
	Render->DrawRegion
					( 
						Base, 
						Size,
						GUI_COLOR_FORM_BG, 
						GUI_COLOR_FORM_BORDER,
						BPAT_Solid 
					);

	// Draw cool pattern header.
	Render->DrawRegion
					( 
						TPoint( Base.X+2, Base.Y+1 ),
						TSize( Size.Width-3, FORM_HEADER_SIZE ),
						GUI_COLOR_FORM_HEADER,
						GUI_COLOR_FORM_HEADER,
						BPAT_Diagonal 
					);

	// Caption.
	TSize TextSize	= TSize( Root->Font1->TextWidth(*Caption), Root->Font1->Height );

	Render->DrawText
				( 
					TPoint( Base.X+5, Base.Y+10-TextSize.Height/2),
					Caption,
					GUI_COLOR_TEXT,
					Root->Font1 
				);

	// Draw 'close' icon.
	if( bCanClose )
		Render->DrawPicture
						( 
							TPoint( Base.X+Size.Width-14, Base.Y+4 ), 
							TSize( 11, 11 ), 
							TPoint( 0, 11 ), 
							TSize( 11, 11 ), 
							Root->Icons 
						);

	// Resize marker.
	if( bSizeableH || bSizeableW )
		Render->DrawPicture
						( 
							TPoint( Base.X+Size.Width-10, Base.Y+Size.Height-10 ), 
							TSize( 8, 8 ), 
							TPoint( 15, 9 ), 
							TSize( 8, 8 ), 
							Root->Icons 
						);
}


//
// Show the form.
//
void WForm::Show( Integer X, Integer Y )
{
	Location	= TPoint( X, Y );
	bVisible	= true;
}


//
// Hide the form.
//
void WForm::Hide()
{
	bVisible	= false;
}


/*-----------------------------------------------------------------------------
    WMessageBox implementation.
-----------------------------------------------------------------------------*/

//
// Message box construction.
//
WMessageBox::WMessageBox( WWindow* InRoot, String InText, String InCaption, Bool InbModal )
	:	WForm( InRoot, InRoot )
{
	// Setup own variables.
	OkButton			= nullptr;
	CancelButton		= nullptr;
	YesButton			= nullptr;
	NoButton			= nullptr;
	bModal				= InbModal;
	bNotification		= false;
	Caption				= InCaption;
	bCanClose			= true;

	// Break text into lines and resize dialog to
	// fit lines.
	Lines				= String::WrapText( InText, 320/WWindow::Font1->TextWidth(L"a") );
	SetSize( 340, 90+13*Lines.Num() );
	SetLocation( (Root->Size.Width-Size.Width)/2, (Root->Size.Height-Size.Height)/2 );

	// Make form modal, if user wants it.
	if( bModal )
	{
		// Store old modal, and current modal.
		OldModal		= Root->Modal;
		Root->Modal		= this;
	}
}


//
// Message box destruction.
//
WMessageBox::~WMessageBox()
{
	// If dialog is modal, set old modal as current
	// modal.
	if( bModal )
	{
		Root->Modal	= OldModal;
	}
}


//
// Message box with buttons: [Ok].
//
void WMessageBox::SetOk( TNotifyEvent InOk )
{
	OkButton					= new WButton( this, Root );
	OkButton->EventClick		= InOk ? InOk : WIDGET_EVENT(WMessageBox::ButtonCloseClick);
	OkButton->Caption			= L"Ok";
	OkButton->SetLocation( Size.Width-100, Size.Height-40 );
	OkButton->SetSize( 88, 26 );

	bCanClose	= true;
}


//
// Message box with buttons: [Ok] [Cancel].
//
void WMessageBox::SetOkCancel( TNotifyEvent InCancel, TNotifyEvent InOk )
{
	OkButton					= new WButton( this, Root );
	OkButton->EventClick		= InOk ? InOk : WIDGET_EVENT(WMessageBox::ButtonCloseClick);
	OkButton->Caption			= L"Ok";
	OkButton->SetLocation( Size.Width-198, Size.Height-40 );
	OkButton->SetSize( 88, 26 );

	CancelButton				= new WButton( this, Root );
	CancelButton->EventClick	= InCancel ? InCancel : WIDGET_EVENT(WMessageBox::ButtonCloseClick);
	CancelButton->Caption		= L"Cancel";
	CancelButton->SetLocation( Size.Width-100, Size.Height-40 );
	CancelButton->SetSize( 88, 26 );

	bCanClose	= false;
}


//
// Message box with buttons: [Yes] [No].
//
void WMessageBox::SetYesNo( TNotifyEvent InYes, TNotifyEvent InNo )
{
	YesButton					= new WButton( this, Root );
	YesButton->EventClick		= InYes ? InYes : WIDGET_EVENT(WMessageBox::ButtonCloseClick);
	YesButton->Caption			= L"Yes";
	YesButton->SetLocation( Size.Width-198, Size.Height-40 );
	YesButton->SetSize( 88, 26 );

	NoButton					= new WButton( this, Root );
	NoButton->EventClick		= InNo ? InNo : WIDGET_EVENT(WMessageBox::ButtonCloseClick);
	NoButton->Caption			= L"No";
	NoButton->SetLocation( Size.Width-100, Size.Height-40 );
	NoButton->SetSize( 88, 26 );

	bCanClose	= false;
}


//
// Message box with buttons: [Yes] [No] [Cancel].
//
void WMessageBox::SetYesNoCancel( TNotifyEvent InYes, TNotifyEvent InNo, TNotifyEvent InCancel )
{
	YesButton					= new WButton( this, Root );
	YesButton->EventClick		= InYes ? InYes : WIDGET_EVENT(WMessageBox::ButtonCloseClick);
	YesButton->Caption			= L"Yes";
	YesButton->SetLocation( Size.Width-296, Size.Height-40 );
	YesButton->SetSize( 88, 26 );

	NoButton					= new WButton( this, Root );
	NoButton->EventClick		= InNo ? InNo : WIDGET_EVENT(WMessageBox::ButtonCloseClick);
	NoButton->Caption			= L"No";
	NoButton->SetLocation( Size.Width-198, Size.Height-40 );
	NoButton->SetSize( 88, 26 );

	CancelButton					= new WButton( this, Root );
	CancelButton->EventClick		= InCancel ? InCancel : WIDGET_EVENT(WMessageBox::ButtonCloseClick);
	CancelButton->Caption			= L"Cancel";
	CancelButton->SetLocation( Size.Width-100, Size.Height-40 );
	CancelButton->SetSize( 88, 26 );

	bCanClose	= false;
}


//
// Special notification dialog.
//
void WMessageBox::SetNotification()
{
	bNotification	= true;
	bCanClose		= true;
}


//
// When dialog hides, its suicide.
//
void WMessageBox::Hide()
{
	WForm::Hide();
	delete this;
}


//
// When dialog closed, its hides.
//
void WMessageBox::OnClose()
{
	WForm::OnClose();
	Hide();
}


//
// Redraw message box.
//
void WMessageBox::OnPaint( CGUIRenderBase* Render )
{
	WForm::OnPaint( Render );

	TPoint Base = ClientToWindow(TPoint::Zero);

	Render->DrawRegion
	(
		TPoint( Base.X+1, Base.Y+Size.Height-50 ),
		TSize( Size.Width-2, 49 ),
		TColor( 0x55, 0x55, 0x55, 0xff ),
		TColor( 0x55, 0x55, 0x55, 0xff ),
		BPAT_Solid
	);

	for( Integer iLine=0; iLine<Lines.Num(); iLine++ )
		Render->DrawText
		(
			TPoint( Base.X+20, Base.Y+30+iLine*13 ),
			Lines[iLine],
			GUI_COLOR_TEXT,
			WWindow::Font1
		);
}


//
// When message box lost focus.
//
void WMessageBox::OnDeactivate()
{
	WForm::OnDeactivate();
	if( bNotification )
		Hide();
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/