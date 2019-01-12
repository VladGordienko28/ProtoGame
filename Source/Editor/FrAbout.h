/*=============================================================================
    FrAbout.h: An about panel.
    Copyright Jan.2017 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    WAboutPanel.
-----------------------------------------------------------------------------*/

//
// Panel to show information about Me and
// my engine.
//
class WAboutPanel: public WPanel
{
public:
	// Variables.
	String		Lines[4];

	// WAboutPanel interface.
	WAboutPanel( WWindow* InRoot )
		:	WPanel( InRoot, InRoot )
	{
		bStayOnTop		= true;
		SetSize( 300, 200 );
		SetLocation
		( 
			(Owner->Size.Width-Size.Width)/2, 
			(Owner->Size.Height-Size.Height)/2 
		);

		for( Int32 i=Root->Children.FindItem(this); i<Root->Children.Num()-1; i++ )
			Root->Children.Swap( i, i+1 );

		Root->SetFocused( this );

		Lines[0]	= String::Format( L"Fluorine Engine" );
		Lines[1]	= String::Format( L"Version: %s", FLU_VERSION );
		Lines[2]	= String::Format( L"Build on: %s", WIDEN(__DATE__) );
		Lines[3]	= String::Format( L"%s", FLU_COPYRIGHT );
	}
	~WAboutPanel()
	{
	}

	// WWidget interface.
	void OnDeactivate()
	{
		WPanel::OnDeactivate();
		delete this;
	}
	void OnPaint( CGUIRenderBase* Render )
	{
		WPanel::OnPaint( Render );
		TPoint	Base	= ClientToWindow(TPoint::Zero);

		// Top panel.
		Render->DrawRegion
		(
			TPoint( Base.X+1, Base.Y+1 ),
			TSize( Size.Width-2, 78 ),
			TColor( 0x22, 0x22, 0x22, 0xff ),
			TColor( 0x22, 0x22, 0x22, 0xff ),
			BPAT_Solid
		);
		Render->DrawPicture
		(
			TPoint( Base.X+(Size.Width-256)/2, Base.Y+12 ),
			TSize( 256, 64 ),
			TPoint( 0, 192 ),
			TSize( 256, 64 ),
			Root->Icons
		);

		// Text.
		Render->DrawText( TPoint(Base.X+10, Base.Y+85),  Lines[0], GUI_COLOR_TEXT, WWindow::Font1 );
		Render->DrawText( TPoint(Base.X+10, Base.Y+100), Lines[1], GUI_COLOR_TEXT, WWindow::Font1 );
		Render->DrawText( TPoint(Base.X+10, Base.Y+115), Lines[2], GUI_COLOR_TEXT, WWindow::Font1 );

#if 1
		// Thanks.
		Render->DrawText
		(
			TPoint( Base.X+55, Base.Y+145 ),
			L"Thank you for your interest :)",
			TColor::HSLToRGB( GFrameStamp/2, 0xff, 0x80 ),
			WWindow::Font1
		);
#endif

		// Copyright.
		Render->DrawText
		(
			TPoint( Base.X+(Size.Width-WWindow::Font1->TextWidth(*Lines[3]))/2, Base.Y+Size.Height-18 ),
			Lines[3],
			GUI_COLOR_TEXT,
			WWindow::Font1
		);
	}
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/