/*=============================================================================
	FrFontView.h: Lame Font-View dialog.
	Created by Vlad Gordienko, Dec. 2016.
=============================================================================*/

/*-----------------------------------------------------------------------------
	WFontViewDialog.
-----------------------------------------------------------------------------*/

//
// An font view dialog.
//
class WFontViewDialog: public WForm, public CRefsHolder
{
public:
	// WFontViewDialog interface.
	WFontViewDialog( WContainer* InOwner, WWindow* InRoot )
		:	WForm( InOwner, InRoot ),
			Font( nullptr )
	{
		// Initialize the form.
		bCanClose		= true;
		bSizeableH		= false;
		bSizeableW		= false;
		Padding			= TArea( 22, 0, 1, 1 );
		SetSize( 400, 120 );

		Edit			= new WEdit( this, InRoot );
		Edit->Text		= L"The quick brown fox";
		Edit->Align		= AL_Top;
		Edit->EditType	= EDIT_String;
		Edit->SetSize( 150, 18 );

		Hide();
		SetFont( nullptr );
	}
	void SetFont( FFont* NewFont )
	{
		Font		= NewFont;
		if( Font )
			Caption	= String::Format( L"Font Viewer [%s]", *Font->GetName() );
		else
			Caption	= L"Font Viewer";
	}

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render )
	{
		WForm::OnPaint(Render);
		Render->SetClipArea
		(
			ClientToWindow(TPoint::Zero),
			TSize( Size.Width-1, Size.Height-1 )
		);

		if( Font )
			Render->DrawText
			(
				ClientToWindow(TPoint::Zero) + TPoint( (Size.Width-Font->TextWidth(*Edit->Text))/2, 40+(Size.Height-80+Font->Height)/2 ),
				*Edit->Text,
				COLOR_White,
				Font
			);
	}

	// WForm interface.
	void Show( Int32 X = 0, Int32 Y = 0 )
	{
		WForm::Show( X, Y );
		SetFont( nullptr );
	}
	void Hide()
	{
		WForm::Hide();
		SetFont( nullptr );
	}
	void OnClose()
	{
		WForm::OnClose();
		Hide();
	}

	// CRefsHolder interface.
	void CountRefs( CSerializer& S )
	{
		Serialize( S, Font );
		SetFont( Font );
		if( !Font )
			Hide();
	}

private:
	// Variables.
	FFont*			Font;
	WEdit*			Edit;
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/