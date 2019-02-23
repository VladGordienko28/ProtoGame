/*=============================================================================
    FrMusPlay.h: Music player dialog.
    Copyright Nov.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    WMusicPlayer.
-----------------------------------------------------------------------------*/

//
// A music player dialog.
//
class WMusicPlayer: public WForm, public CRefsHolder
{
public:
	// WMusicPlayer interface.
	WMusicPlayer( WContainer* InOwner, WWindow* InRoot )
		:	WForm( InOwner, InRoot ),
			Music( nullptr )
	{
		// Initialize own fields.
		bCanClose			= true;
		bSizeableH			= false;
		bSizeableW			= false;
		Caption				= L"Music Player";
		SetSize( 283, 105 );	

		// Controls.
		Panel					= new WPanel( this, Root );
		Panel->SetSize( 275, 48 );
		Panel->SetLocation( 4, 24 );

		FNLabel					= new WLabel( Panel, Root );
		FNLabel->Caption		= L"File Name:";
		FNLabel->SetLocation( 8, 4 );

		FSLabel					= new WLabel( Panel, Root );
		FSLabel->Caption		= L"File Size:";
		FSLabel->SetLocation( 8, 24 );

		FNValue					= new WLabel( Panel, Root );
		FNValue->Caption		= L"..";
		FNValue->SetLocation( 85, 4 );

		FSValue					= new WLabel( Panel, Root );
		FSValue->Caption		= L"0 Kb";
		FSValue->SetLocation( 85, 24 );

		PlayButton				= new WButton( this, Root );
		PlayButton->Caption		= L"Play";
		PlayButton->EventClick	= WIDGET_EVENT(WMusicPlayer::ButtonPlayClick);
		PlayButton->SetLocation( 50, 76 );
		PlayButton->SetSize( 80, 25 );

		StopButton			= new WButton( this, Root );
		StopButton->Caption	= L"Stop";
		StopButton->EventClick	= WIDGET_EVENT(WMusicPlayer::ButtonStopClick);
		StopButton->SetLocation( 150, 76 );
		StopButton->SetSize( 80, 25 );

		Hide();
		SetMusic( nullptr );
	}
	void SetMusic( FMusic* NewMusic )
	{
		Music	= NewMusic;
		if( NewMusic )
		{
			String RealFN	= GDirectory + L"\\" + Music->FileName;
			Bool bExists	= GPlat->FileExists(RealFN);
			UInt32 MusicSize	= 0;
			String FN		= Music->FileName;
			if( bExists )
			{
				CFileLoader Loader(RealFN);
				MusicSize	= Loader.TotalSize();
			}
#if 0
			{
				// Replace '\' to '/' in file name.
				Integer i;
				while( (i = String::Pos( L"\\", FN )) != -1 )
					FN[i]	= L'/';
			}
#endif
			PlayButton->bEnabled	= bExists;
			StopButton->bEnabled	= bExists;
			FNValue->Caption		= String::format( L"..%s", *FN );
			FSValue->Caption		= String::format( L"%i Kb", MusicSize/1024 );
			Caption					= String::format( L"Music Player [%s]", *Music->GetName() );
		}
		else
		{
			PlayButton->bEnabled	= false;
			StopButton->bEnabled	= false;
			FNValue->Caption		= L"..";
			FSValue->Caption		= L"0 Kb";
			Caption					= L"Music Player";
		}
	}

	// WForm interface.
	void Show( Int32 X = 0, Int32 Y = 0 )
	{
		WForm::Show( X, Y );
		SetMusic( nullptr );
	}
	void Hide()
	{
		WForm::Hide();
		SetMusic( nullptr );
	}
	void OnClose()
	{
		WForm::OnClose();
		Hide();
	}

	// Notifications.
	void ButtonPlayClick( WWidget* Sender )
	{
		GApp->GAudio->PlayMusic( Music, 1.5f );
	}
	void ButtonStopClick( WWidget* Sender )
	{
		GApp->GAudio->PlayMusic( nullptr, 2.5f );
	}

	// CRefsHolder interface.
	void CountRefs( CSerializer& S )
	{
		Serialize( S, Music );
		SetMusic( Music );
	}

private:
	// Internal.
	FMusic*			Music;
	WPanel*			Panel;
	WButton*		PlayButton;
	WButton*		StopButton;
	WLabel*			FNLabel;
	WLabel*			FSLabel;
	WLabel*			FNValue;
	WLabel*			FSValue;
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/