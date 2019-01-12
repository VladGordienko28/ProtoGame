/*=============================================================================
    FrEdToolBar.cpp: An editor main toolbar.
    Copyright Aug.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    WEditorToolBar implementation.
-----------------------------------------------------------------------------*/

//
// ToolBar constructor.
//
WEditorToolBar::WEditorToolBar( WContainer* InOwner, WWindow* InRoot )
	:	WToolBar( InOwner, InRoot )
{
	// Initialize own fields.
	SetSize( 150, 35 );

	// Create buttons.
	NewProjectButton					= new WPictureButton( this, InRoot );
	NewProjectButton->Tooltip			= L"New Project";
	NewProjectButton->Scale				= TSize( 16, 16 );
	NewProjectButton->Offset			= TPoint( 0, 64 );
	NewProjectButton->Picture			= Root->Icons;
	NewProjectButton->EventClick		= WIDGET_EVENT(WEditorToolBar::ButtonNewProjectClick);
	NewProjectButton->SetSize( 25, 25 );
	this->AddElement( NewProjectButton );

	OpenProjectButton					= new WPictureButton( this, InRoot );
	OpenProjectButton->Tooltip			= L"Open Project";
	OpenProjectButton->Scale			= TSize( 16, 16 );
	OpenProjectButton->Offset			= TPoint( 16, 64 );
	OpenProjectButton->Picture			= Root->Icons;
	OpenProjectButton->EventClick		= WIDGET_EVENT(WEditorToolBar::ButtonOpenProjectClick);
	OpenProjectButton->SetSize( 25, 25 );
	this->AddElement( OpenProjectButton );

	SaveProjectButton					= new WPictureButton( this, InRoot );
	SaveProjectButton->Tooltip			= L"Save Project";
	SaveProjectButton->Scale			= TSize( 16, 16 );
	SaveProjectButton->Offset			= TPoint( 32, 64 );
	SaveProjectButton->Picture			= Root->Icons;
	SaveProjectButton->EventClick		= WIDGET_EVENT(WEditorToolBar::ButtonSaveProjectClick);
	SaveProjectButton->SetSize( 25, 25 );
	this->AddElement( SaveProjectButton );
	this->AddElement( nullptr );

	UndoButton							= new WPictureButton( this, InRoot );
	UndoButton->Tooltip					= L"Undo";
	UndoButton->Scale					= TSize( 16, 16 );
	UndoButton->Offset					= TPoint( 48, 64 );
	UndoButton->Picture					= Root->Icons;
	UndoButton->EventClick				= WIDGET_EVENT(WEditorToolBar::ButtonUndoClick);
	UndoButton->SetSize( 25, 25 );
	this->AddElement( UndoButton );

	RedoButton							= new WPictureButton( this, InRoot );
	RedoButton->Tooltip					= L"Redo";
	RedoButton->Scale					= TSize( 16, 16 );
	RedoButton->Offset					= TPoint( 64, 64 );
	RedoButton->Picture					= Root->Icons;
	RedoButton->EventClick				= WIDGET_EVENT(WEditorToolBar::ButtonRedoClick);
	RedoButton->SetSize( 25, 25 );
	this->AddElement( RedoButton );
	this->AddElement( nullptr );

	PlayModeCombo						= new WComboBox( this, InRoot );
	PlayModeCombo->SetSize( 96, 25 );
	PlayModeCombo->AddItem( L"Debug", nullptr );
	PlayModeCombo->AddItem( L"Release", nullptr );
	PlayModeCombo->SetItemIndex( 0, false );
	this->AddElement( PlayModeCombo );

	PlayButton							= new WPictureButton( this, InRoot );
	PlayButton->Tooltip					= L"Play Level!";
	PlayButton->Scale					= TSize( 16, 16 );
	PlayButton->Offset					= TPoint( 80, 64 );
	PlayButton->Picture					= Root->Icons;
	PlayButton->EventClick				= WIDGET_EVENT(WEditorToolBar::ButtonPlayClick);
	PlayButton->SetSize( 25, 25 );
	this->AddElement( PlayButton );
	this->AddElement(nullptr);

	PauseButton							= new WPictureButton( this, InRoot );
	PauseButton->Tooltip				= L"Pause Level";
	PauseButton->Scale					= TSize( 16, 16 );
	PauseButton->Offset					= TPoint( 96, 64 );
	PauseButton->Picture					= Root->Icons;
	PauseButton->EventClick				= WIDGET_EVENT(WEditorToolBar::ButtonPauseClick);
	PauseButton->SetSize( 25, 25 );
	this->AddElement( PauseButton );

	StopButton							= new WPictureButton( this, InRoot );
	StopButton->Tooltip					= L"Stop Level";
	StopButton->Scale					= TSize( 16, 16 );
	StopButton->Offset					= TPoint( 112, 64 );
	StopButton->Picture					= Root->Icons;
	StopButton->EventClick				= WIDGET_EVENT(WEditorToolBar::ButtonStopClick);
	StopButton->SetSize( 25, 25 );
	this->AddElement( StopButton );
	this->AddElement( nullptr );

	BuildButton							= new WPictureButton( this, InRoot );
	BuildButton->Tooltip				= L"Build Game...";
	BuildButton->Scale					= TSize( 16, 16 );
	BuildButton->Offset					= TPoint( 128, 64 );
	BuildButton->Picture				= Root->Icons;
	BuildButton->EventClick				= WIDGET_EVENT(WEditorToolBar::ButtonBuildClick);
	BuildButton->SetSize( 25, 25 );
	this->AddElement( BuildButton );
}


//
// Toolbar paint.
//
void WEditorToolBar::OnPaint( CGUIRenderBase* Render )
{
	// Call parent.
	WToolBar::OnPaint(Render);

	WEditorPage*	Page	= GEditor->GetActivePage();
	EPageType		Type	= Page ? Page->PageType : PAGE_None;

	// Turn on, or turn off buttons.
	SaveProjectButton->bEnabled	= GProject != nullptr;
	UndoButton->bEnabled		= Type == PAGE_Level || Type == PAGE_Script;
	RedoButton->bEnabled		= Type == PAGE_Level || Type == PAGE_Script;
	PlayModeCombo->bEnabled		= Type == PAGE_Level;
	PlayButton->bEnabled		= Type == PAGE_Level || (Type == PAGE_Play && ((FLevel*)Page->GetResource())->bIsPause);
	PauseButton->bEnabled		= Type == PAGE_Play && !((FLevel*)Page->GetResource())->bIsPause;
	StopButton->bEnabled		= Type == PAGE_Play;
	BuildButton->bEnabled		= GProject != nullptr && Type != PAGE_Play;
}


/*-----------------------------------------------------------------------------
    Buttons click.
-----------------------------------------------------------------------------*/

void WEditorToolBar::ButtonNewProjectClick( WWidget* Sender )
{
	GEditor->NewProject();
}


void WEditorToolBar::ButtonOpenProjectClick( WWidget* Sender )
{
	GEditor->OpenProject();
}


void WEditorToolBar::ButtonSaveProjectClick( WWidget* Sender )
{
	GEditor->SaveProject();
}


void WEditorToolBar::ButtonUndoClick( WWidget* Sender )
{
	WEditorPage* EdPage		= GEditor->GetActivePage();
	if( EdPage )	
		EdPage->Undo();
}


void WEditorToolBar::ButtonRedoClick( WWidget* Sender )
{
	WEditorPage* EdPage		= GEditor->GetActivePage();
	if( EdPage )	
		EdPage->Redo();
}


void WEditorToolBar::ButtonPlayClick( WWidget* Sender )
{
	WEditorPage* EdPage	= GEditor->GetActivePage();
	if( EdPage )
	{
		if( EdPage->PageType == PAGE_Level )
			GEditor->PlayLevel( ((WLevelPage*)EdPage)->Level );
		if( EdPage->PageType == PAGE_Play )
			((FLevel*)EdPage->GetResource())->bIsPause	= false;	
	}
}


void WEditorToolBar::ButtonPauseClick( WWidget* Sender )
{
	WEditorPage* EdPage	= GEditor->GetActivePage();
	if( EdPage && EdPage->PageType == PAGE_Play )
		((FLevel*)EdPage->GetResource())->bIsPause	= true;
}


void WEditorToolBar::ButtonStopClick( WWidget* Sender )
{
	WEditorPage* EdPage	= GEditor->GetActivePage();
	if( EdPage && EdPage->PageType == PAGE_Play )
		EdPage->Close();
}


void WEditorToolBar::ButtonBuildClick( WWidget* Sender )
{
	GEditor->GameBuilder->Show();
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/