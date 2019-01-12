/*=============================================================================
    FrGamBld.cpp: Game builder.
    Copyright Dec.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    WGameBuilderDialog implementation.
-----------------------------------------------------------------------------*/

//
// Game builder constructor.
//
WGameBuilderDialog::WGameBuilderDialog( WWindow* InRoot )
	:	WForm( InRoot, InRoot )
{
	// Initialize own variables.
	Caption						= L"Game Builder";
	bCanClose					= true;
	bSizeableH					= false;
	bSizeableW					= false;
	SetSize( 270, 185 );	

	// Allocate controls.
	TargetLabel					= new WLabel( this, Root );
	TargetLabel->Caption		= L"Target:";
	TargetLabel->SetLocation( 10, 30 );

	TargetCombo					= new WComboBox( this, Root );
#if FLU_X64
	TargetCombo->AddItem( L"Win64", nullptr );
#else
	TargetCombo->AddItem( L"Win32", nullptr );
#endif

	TargetCombo->ItemIndex		= 0;
	TargetCombo->SetSize( 160, 18 );
	TargetCombo->SetLocation( 100, 29 );

	ConfigLabel					= new WLabel( this, Root );
	ConfigLabel->Caption		= L"Configuration:";
	ConfigLabel->SetLocation( 10, 54 );

	ConfigCombo					= new WComboBox( this, Root );
	ConfigCombo->AddItem( L"Release", nullptr );
	ConfigCombo->ItemIndex		= 0;
	ConfigCombo->SetSize( 160, 18 );
	ConfigCombo->SetLocation( 100, 53 );

	LaunchCheck					= new WCheckBox( this, Root );
	LaunchCheck->Tooltip		= L"Whether launch game after building?";
	LaunchCheck->Caption		= L"Launch After Build?";
	LaunchCheck->bChecked		= false;
	LaunchCheck->EventClick		= WIDGET_EVENT(WGameBuilderDialog::CheckLaunchClick);
	LaunchCheck->SetLocation( 60, 90 );

	CmdLabel					= new WLabel( this, Root );
	CmdLabel->Caption			= L"Cmd Line: ";
	CmdLabel->bEnabled			= false;
	CmdLabel->SetLocation( 10, 114 );

	ParmsEdit					= new WEdit( this, Root );
	ParmsEdit->EditType			= EDIT_String;
	ParmsEdit->bEnabled			= false;
	ParmsEdit->SetSize( 160, 18 );
	ParmsEdit->SetLocation( 100, 113 );

	BuildButton					= new WButton( this, Root );
	BuildButton->Caption		= L"Build";
	BuildButton->EventClick		= WIDGET_EVENT(WGameBuilderDialog::ButtonBuildClick);
	BuildButton->SetLocation( 180, 150 );
	BuildButton->SetSize( 80, 25 );

	CancelButton				= new WButton( this, Root );
	CancelButton->Caption		= L"Cancel";
	CancelButton->EventClick	= WIDGET_EVENT(WGameBuilderDialog::ButtonCancelClick);
	CancelButton->SetLocation( 90, 150 );
	CancelButton->SetSize( 80, 25 );

	Hide();
}


//
// Game builder destructor.
//
WGameBuilderDialog::~WGameBuilderDialog()
{
}


//
// Show the dialog.
//
void WGameBuilderDialog::Show( Int32 X, Int32 Y )
{
	if( !bVisible )
	{
		bVisible	= true;

		SetLocation
		(
			(Owner->Size.Width - Size.Width)/2,
			(Owner->Size.Height - Size.Height)/2
		);
	}
}


//
// Hide the dialog.
//
void WGameBuilderDialog::Hide()
{
	WForm::Hide();
}


//
// Build button click.
//
void WGameBuilderDialog::ButtonBuildClick( WWidget* Sender )
{
	BuildGame();
}


//
// When form close.
//
void WGameBuilderDialog::OnClose()
{
	WForm::OnClose();
	Hide();
}


//
// Cancel button clicked.
//
void WGameBuilderDialog::ButtonCancelClick( WWidget* Sender )
{
	Hide();
}


//
// Launch box checked.
//
void WGameBuilderDialog::CheckLaunchClick( WWidget* Sender )
{
	CmdLabel->bEnabled	=
	ParmsEdit->bEnabled	= LaunchCheck->bChecked;
}


/*-----------------------------------------------------------------------------
    Game building.
-----------------------------------------------------------------------------*/

//
// Rebuild game.
//
void WGameBuilderDialog::RebuildGame()
{
	// Same as BuildGame() for now.
	BuildGame();
}


//
// Build game.
//
void WGameBuilderDialog::BuildGame()
{
	// Project should be saved.
	if( !GProject )
		return;
	if( !GProject->FileName )
		if( !GEditor->SaveProject() )
			return;

	// Allocate target directory.
	String Directory = GetFileDir(GProject->FileName) + L"\\Release";
	CreateDirectory( *Directory, nullptr );

	// Shutdown play pages.
	{
		Bool bAgain	= true;
		while( bAgain )
		{
			bAgain	= false;
			for( Int32 i=0; i<GEditor->EditorPages->Pages.Num(); i++ )
			{
				WEditorPage* EdPage = (WEditorPage*)GEditor->EditorPages->Pages[i];
				if( EdPage->PageType == PAGE_Play )
				{
					EdPage->Close( true );
					bAgain	= true;
					break;
				}
			}
		}
	}

	// Now save project.
	if( !GEditor->SaveGame( Directory, GProject->ProjName ) )
	{
		Root->ShowMessage( L"Couldn't save project", L"Project", true );
		return;
	}

	// Launch game!
	if( LaunchCheck->bChecked )
	{
		String ExeDir = GDirectory+L"\\"+CLIENT_EXE_NAME;

		if( !GPlat->FileExists(ExeDir) )
		{
			Root->ShowMessage( L"Couldn't run game. '" CLIENT_EXE_NAME L"' not found.", L"Fluorine", true );
			return;
		}
		GPlat->Launch
		(
			*ExeDir,
			*String::Format( L"project=\"%s\" %s", *(Directory+L"\\"+GProject->ProjName+PROJ_FILE_EXT), *ParmsEdit->Text )
		);
	}
	else
		Root->ShowMessage( L"Game successfully created.", L"Fluorine", true );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/