/*=============================================================================
    FrEdMenu.cpp: Editor main menu bar.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    WEditorMainMenu implementation.
-----------------------------------------------------------------------------*/

//
// Editor menu constructor.
//
WEditorMainMenu::WEditorMainMenu( WContainer* InOwner, WWindow* InRoot )
	:	WMainMenu( InOwner, InRoot )
{
	// Create sub menus.
	FileSubmenu			= new WMenu( Root, Root );
	FileSubmenu->AddItem( L"New Project",			WIDGET_EVENT(WEditorMainMenu::NewProjectClick) );
	FileSubmenu->AddItem( L"Open Project",			WIDGET_EVENT(WEditorMainMenu::OpenProjectClick) );
	FileSubmenu->AddItem( L"" );
	FileSubmenu->AddItem( L"Save Project",			WIDGET_EVENT(WEditorMainMenu::SaveProjectClick) );
	FileSubmenu->AddItem( L"Save Project As",		WIDGET_EVENT(WEditorMainMenu::SaveProjectAsClick) );
	FileSubmenu->AddItem( L"" );
	FileSubmenu->AddItem( L"Close Project",			WIDGET_EVENT(WEditorMainMenu::CloseProjectClick) );
	FileSubmenu->AddItem( L"" );
	FileSubmenu->AddItem( L"Exit",					WIDGET_EVENT(WEditorMainMenu::ExitClick) );

	EditSubmenu			= new WMenu( Root, Root );
	EditSubmenu->AddItem( L"Undo",					WIDGET_EVENT(WEditorMainMenu::UndoClick) );
	EditSubmenu->AddItem( L"Redo",					WIDGET_EVENT(WEditorMainMenu::RedoClick) );
	EditSubmenu->AddItem( L"" );
	EditSubmenu->AddItem( L"Cut",					WIDGET_EVENT(WEditorMainMenu::CutClick) );
	EditSubmenu->AddItem( L"Copy",					WIDGET_EVENT(WEditorMainMenu::CopyClick) );
	EditSubmenu->AddItem( L"Paste",					WIDGET_EVENT(WEditorMainMenu::PasteClick) );
	EditSubmenu->AddItem( L"Delete",				WIDGET_EVENT(WEditorMainMenu::DeleteClick) );
	EditSubmenu->AddItem( L"" );
	EditSubmenu->AddItem( L"Select All",			WIDGET_EVENT(WEditorMainMenu::SelectAllClick) );
	EditSubmenu->AddItem( L"" );
	EditSubmenu->AddItem( L"Preferences...",		WIDGET_EVENT(WEditorMainMenu::PreferencesClick) );

	ProjectSubmenu			= new WMenu( Root, Root );
	ProjectSubmenu->AddItem( L"Add Level...",		WIDGET_EVENT(WEditorMainMenu::AddLevelClick) );
	ProjectSubmenu->AddItem( L"" );
	ProjectSubmenu->AddItem( L"Compile Scripts",	WIDGET_EVENT(WEditorMainMenu::CompileScriptsClick) );
	ProjectSubmenu->AddItem( L"" );
	ProjectSubmenu->AddItem( L"Project Properties",	WIDGET_EVENT(WEditorMainMenu::ProjectPropertiesClick) );

	ResourceSubmenu			= new WMenu( Root, Root );
	ResourceSubmenu->AddItem( L"Import Resource...",WIDGET_EVENT(WEditorMainMenu::ImportResourceClick) );

	BuildSubmenu			= new WMenu( Root, Root );
	BuildSubmenu->AddItem( L"Build Game...",		WIDGET_EVENT(WEditorMainMenu::BuildGameClick) );
	BuildSubmenu->AddItem( L"Rebuild Game",			WIDGET_EVENT(WEditorMainMenu::RebuildGameClick) );

	HelpSubmenu			= new WMenu( Root, Root );
	HelpSubmenu->AddItem( L"Help...",				WIDGET_EVENT(WEditorMainMenu::HelpClick) );
	HelpSubmenu->AddItem( L"" );
	HelpSubmenu->AddItem( L"About",					WIDGET_EVENT(WEditorMainMenu::AboutClick) );

	// Add to list.
	this->AddSubMenu( L"File",		FileSubmenu );
	this->AddSubMenu( L"Edit",		EditSubmenu );
	this->AddSubMenu( L"Project",	ProjectSubmenu );
	this->AddSubMenu( L"Resource",	ResourceSubmenu );
	this->AddSubMenu( L"Build",		BuildSubmenu );
	this->AddSubMenu( L"Help",		HelpSubmenu );
}


//
// Redraw an editor menu.
//
void WEditorMainMenu::OnPaint( CGUIRenderBase* Render )
{
	WMainMenu::OnPaint(Render);

	WEditorPage*	Page	= GEditor->GetActivePage();
	EPageType		Type	= Page ? Page->PageType : PAGE_None;

	// Turn on or turn off some menus.
	FileSubmenu->Items[3].bEnabled		= 
	FileSubmenu->Items[4].bEnabled		= 
	FileSubmenu->Items[6].bEnabled		= GProject != nullptr;

	EditSubmenu->Items[0].bEnabled		= 
	EditSubmenu->Items[1].bEnabled		= 
	EditSubmenu->Items[3].bEnabled		= 
	EditSubmenu->Items[4].bEnabled		= 
	EditSubmenu->Items[6].bEnabled		= Type == PAGE_Level || Type == PAGE_Script;
	EditSubmenu->Items[5].bEnabled		= 
	EditSubmenu->Items[8].bEnabled		= Type == PAGE_Script;
	EditSubmenu->Items[10].bEnabled		= false;

	ProjectSubmenu->Items[0].bEnabled	=
	ProjectSubmenu->Items[2].bEnabled	=
	ProjectSubmenu->Items[4].bEnabled	= GProject != nullptr;

	ResourceSubmenu->Items[0].bEnabled	= false && GProject != nullptr;

	BuildSubmenu->Items[0].bEnabled		=
	BuildSubmenu->Items[1].bEnabled		= GProject != nullptr;
}


/*-----------------------------------------------------------------------------
    'About'
-----------------------------------------------------------------------------*/

void WEditorMainMenu::HelpClick( WWidget* Sender )
{
	static aud::Sound::Ptr sound0 = res::ResourceManager::get<aud::Sound>( L"Experimental.Bola", res::EFailPolicy::FATAL );
	static aud::Sound::Ptr sound1 = res::ResourceManager::get<aud::Sound>( L"Experimental.SGrunt", res::EFailPolicy::FATAL );

	GEditor->m_audioDevice->playSFX( sound0->getHandle(), 1.f, 0.6f );
	threading::sleep( 100 );
	GEditor->m_audioDevice->playSFX( sound1->getHandle(), 1.f, 1.f );

	Root->ShowMessage( L"Help file not found", L"Fluorine", true );
}


void WEditorMainMenu::AboutClick( WWidget* Sender )
{
	WAboutPanel* About	= new WAboutPanel( Root );
	Root->SetFocused( About );
}


/*-----------------------------------------------------------------------------
    'Resource'
-----------------------------------------------------------------------------*/

void WEditorMainMenu::ImportResourceClick( WWidget* Sender )
{
}


/*-----------------------------------------------------------------------------
    'Build'
-----------------------------------------------------------------------------*/

void WEditorMainMenu::BuildGameClick( WWidget* Sender )
{
	GEditor->GameBuilder->Show();
}


void WEditorMainMenu::RebuildGameClick( WWidget* Sender )
{
	GEditor->GameBuilder->RebuildGame();
}


/*-----------------------------------------------------------------------------
    'File'
-----------------------------------------------------------------------------*/

void WEditorMainMenu::NewProjectClick( WWidget* Sender )
{
	GEditor->NewProject();
}


void WEditorMainMenu::OpenProjectClick( WWidget* Sender )
{
	GEditor->OpenProject();
}


void WEditorMainMenu::SaveProjectClick( WWidget* Sender )
{
	GEditor->SaveProject();
}


void WEditorMainMenu::SaveProjectAsClick( WWidget* Sender )
{ 
	GEditor->SaveAsProject();  
}


void WEditorMainMenu::ExitClick( WWidget* Sender )
{
	SendMessage( GEditor->hWnd, WM_CLOSE, 0, 0 );
}


void WEditorMainMenu::CloseProjectClick( WWidget* Sender )
{
	GEditor->CloseProject();
}


/*-----------------------------------------------------------------------------
    'Edit'
-----------------------------------------------------------------------------*/

void WEditorMainMenu::UndoClick( WWidget* Sender )
{
	WEditorPage* EdPage		= GEditor->GetActivePage();
	if( EdPage )
		EdPage->Undo();
}


void WEditorMainMenu::RedoClick( WWidget* Sender )
{
	WEditorPage* EdPage		= GEditor->GetActivePage();
	if( EdPage )
		EdPage->Redo();
}


void WEditorMainMenu::SelectAllClick( WWidget* Sender )
{
	WEditorPage* EdPage		= GEditor->GetActivePage();
	if( EdPage && EdPage->PageType == PAGE_Script )
	{
		WScriptPage* Page = (WScriptPage*)EdPage;
		Page->CodeEditor->SelectAll();
	}
}


void WEditorMainMenu::DeleteClick( WWidget* Sender )
{
	WEditorPage* EdPage		= GEditor->GetActivePage();
	if( !EdPage )
		return;

	if( EdPage->PageType == PAGE_Script )
	{
		WScriptPage* Page = (WScriptPage*)EdPage;
		Page->CodeEditor->BeginTransaction();
		Page->CodeEditor->ClearSelected();
		Page->CodeEditor->EndTransaction();
	}
	else if( EdPage->PageType == PAGE_Level )
	{
		WLevelPage* Page = (WLevelPage*)EdPage;
		Page->PopDeleteClick( Sender );
	}
}


void WEditorMainMenu::CutClick( WWidget* Sender )
{
	WEditorPage* EdPage		= GEditor->GetActivePage();
	if( !EdPage )
		return;

	if( EdPage->PageType == PAGE_Script )
	{
		WScriptPage* Page = (WScriptPage*)EdPage;
		Page->CodeEditor->PopCutClick( Sender );
	}
	else if( EdPage->PageType == PAGE_Level )
	{
		WLevelPage* Page = (WLevelPage*)EdPage;
		Page->PopCutClick( Sender );
	}
}


void WEditorMainMenu::CopyClick( WWidget* Sender )
{
	WEditorPage* EdPage		= GEditor->GetActivePage();
	if( !EdPage )
		return;

	if( EdPage->PageType == PAGE_Script )
	{
		WScriptPage* Page = (WScriptPage*)EdPage;
		Page->CodeEditor->PopCopyClick( Sender );
	}
	else if( EdPage->PageType == PAGE_Level )
	{
		WLevelPage* Page = (WLevelPage*)EdPage;
		Page->PopCopyClick( Sender );
	}
}


void WEditorMainMenu::PasteClick( WWidget* Sender )
{
	WEditorPage* EdPage		= GEditor->GetActivePage();
	if( !EdPage )
		return;

	if( EdPage->PageType == PAGE_Script )
	{
		WScriptPage* Page = (WScriptPage*)EdPage;
		Page->CodeEditor->PopPasteClick( Sender );
	}
}


void WEditorMainMenu::PreferencesClick( WWidget* Sender )
{
}


/*-----------------------------------------------------------------------------
    'Project'
-----------------------------------------------------------------------------*/

void WEditorMainMenu::ProjectPropertiesClick( WWidget* Sender )
{
	if( GProject && GProject->Info )
		GEditor->Inspector->SetEditObject(GProject->Info);
}


void WEditorMainMenu::AddLevelClick( WWidget* Sender )
{
	GEditor->Browser->LevelsPage->ButtonAddLevelClick(this);
}


void WEditorMainMenu::CompileScriptsClick( WWidget* Sender )
{
	GEditor->CompileAllScripts( GEditor->TaskDialog );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/