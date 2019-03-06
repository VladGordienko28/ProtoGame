/*=============================================================================
	FrResBrowser.cpp: A resource browser panel.
	Created by Vlad Gordienko, Jul. 2016.
	Redesigned and reimplemented by Vlad Gordienko, Apr-May. 2018.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
	Resource browser colors.
-----------------------------------------------------------------------------*/

// Page colors.
#define COLOR_ASSETS_PAGE				TColor( 0x32, 0x52, 0x78, 0xff )
#define COLOR_SCRIPTS_PAGE				TColor( 0x32, 0x78, 0x32, 0xff )
#define COLOR_LEVELS_PAGE				TColor( 0x72, 0x56, 0x38, 0xff )


/*-----------------------------------------------------------------------------
	WResourceBrowser implementation.
-----------------------------------------------------------------------------*/

//
// Resource browser constructor.
//
WResourceBrowser::WResourceBrowser( WContainer* InOwner, WWindow* InRoot )
	:	WContainer( InOwner, InRoot ),
		Selected( nullptr )
{
	// Setup own variables.
	SetSize( 300, 300 );
	Caption	= L"Resource Browser";
	Padding = TArea( FORM_HEADER_SIZE, 0, 0, 0 );

	// Main splitter.
	WVSplitBox* SplitBox = new WVSplitBox( this, Root );
	SplitBox->Align		= AL_Client;
	SplitBox->RatioRule	= VRR_PreferBottom;
	SplitBox->TopMin	= 200;
	SplitBox->BottomMin	= 200;

	// Resource tab.
	TopPanel = new WTabControl( SplitBox, Root );
	TopPanel->SetHeaderSide(ETH_Bottom);

	// Create pages.
	TopPanel->AddTabPage(AssetsPage = new WAssetsPage(this, TopPanel, Root));
	TopPanel->AddTabPage(ScriptsPage = new WScriptsPage(this, TopPanel, Root));
	TopPanel->AddTabPage(LevelsPage = new WLevelsPage(this, TopPanel, Root));
	TopPanel->ActivateTabPage(AssetsPage);

	// Stub control.
	BottomPanel = new WPanel( SplitBox, Root );
}


//
// Resource browser destructor.
//
WResourceBrowser::~WResourceBrowser()
{
}


//
// Resource browser painting.
//
void WResourceBrowser::OnPaint( CGUIRenderBase* Render )
{
	WContainer::OnPaint(Render);
	TPoint Base = ClientToWindow(TPoint::Zero);

	// Draw header.
	Render->DrawRegion
	(
		Base,
		TSize( Size.Width, FORM_HEADER_SIZE ),
		GUI_COLOR_FORM_BG,
		GUI_COLOR_FORM_BG,
		BPAT_Solid
	);
	Render->DrawRegion
	(
		Base, 
		TSize( Size.Width, FORM_HEADER_SIZE ),
		TColor( 0x33, 0x33, 0x33, 0xff ),
		GUI_COLOR_FORM_BORDER,
		BPAT_Diagonal
	);
	Render->DrawText
	( 
		TPoint( Base.X + 5, Base.Y+(FORM_HEADER_SIZE-Root->Font1->Height)/2 ), 
		Caption, 
		GUI_COLOR_TEXT, 
		Root->Font1 
	);
}


//
// Refresh resources list.
//
void WResourceBrowser::Refresh()
{
	// Delegate it to pages.
	AssetsPage->Refresh();
	ScriptsPage->Refresh();
	LevelsPage->Refresh();
}


//
// Activate browser's resource.
//
void WResourceBrowser::ActivateResource( FResource* Resource )
{
	if( !Resource )
		return;

	Selected = Resource;

	if( Selected->IsA(FSound::MetaClass) )
	{
		GEditor->GAudio->PlayFX( As<FSound>(Selected), 1.f, 1.f );
	}
	else if( Selected->IsA(FMusic::MetaClass) )
	{
		AssetsPage->MusicPlayer->Show( Root->Size.Width/3, Root->Size.Height/3 );
		AssetsPage->MusicPlayer->SetMusic( As<FMusic>(Selected) );
	}
	else if( Selected->IsA(FFont::MetaClass) )
	{
		AssetsPage->FontViewDialog->Show( Root->Size.Width/3, Root->Size.Height/3 );
		AssetsPage->FontViewDialog->SetFont( As<FFont>(Selected) );
	}
	else
		GEditor->OpenPageWith( Selected );
}


/*-----------------------------------------------------------------------------
	WLevelConstructor implementation.
-----------------------------------------------------------------------------*/

//
// An level construction dialog.
//
class WLevelConstructor: public WForm
{
public:
	// WLevelConstructor interface.
	WLevelConstructor( WLevelsPage* InPage, WWindow* InRoot )
		:	WForm( InRoot, InRoot ),
			Page( InPage )
	{
		// Initialize the form.
		Caption			= L"Level Constructor";
		bCanClose		= true;
		bSizeableH		= false;
		bSizeableW		= false;
		SetSize( 240, 123 );

		WPanel* TopPanel	= new WPanel( this, Root );
		TopPanel->SetLocation( 8, 28 );
		TopPanel->SetSize( 224, 57 );	

		NameLabel			= new WLabel( TopPanel, Root );
		NameLabel->Caption	= L"Name:";
		NameLabel->SetLocation( 6, 8 );

		NameEdit			= new WEdit( TopPanel, Root );
		NameEdit->EditType	= EDIT_String;
		NameEdit->Location	= TPoint( 56, 7 );
		NameEdit->SetSize( 160, 18 );

		OkButton				= new WButton( this, Root );
		OkButton->Caption		= L"Ok";
		OkButton->EventClick	= WIDGET_EVENT(WLevelConstructor::ButtonOkClick);
		OkButton->SetLocation( 38, 90 );
		OkButton->SetSize( 64, 25 );

		CancelButton				= new WButton( this, Root );
		CancelButton->Caption		= L"Cancel";
		CancelButton->EventClick	= WIDGET_EVENT(WLevelConstructor::ButtonCancelClick);
		CancelButton->SetLocation( 138, 90 );
		CancelButton->SetSize( 64, 25 );

		Hide();
	}

	// WForm interface.
	void OnClose() override
	{
		WForm::OnClose();
		Hide();
	}
	void Show( Int32 X = 0, Int32 Y = 0 ) override
	{
		WForm::Show( X, Y );
		NameEdit->Text			= L"";
	}

private:
	// Internal.
	WLevelsPage*	Page;
	WLabel*			NameLabel;
	WEdit*			NameEdit;
	WButton*		OkButton;
	WButton*		CancelButton;

	// Notifications.
	void ButtonCancelClick( WWidget* Sender )
	{
		Hide();
	}
	void ButtonOkClick( WWidget* Sender )
	{
		if( NameEdit->Text.len() == 0 )
		{
			Root->ShowMessage
			(
				L"Please specify the name of level",
				L"Level Constructor",
				true
			);
			return;
		}
		if( GObjectDatabase->FindObject( NameEdit->Text ) )
		{
			Root->ShowMessage
			(
				String::format( L"Object '%s' already exists", *NameEdit->Text ),
				L"Level Constructor",
				true
			);	
			return;
		}

		FLevel* Level	= NewObject<FLevel>( NameEdit->Text );
		Level->RndFlags	= RND_Editor;
		Hide();
		Page->Refresh();
		GEditor->OpenPageWith( Level );
	}
};


/*-----------------------------------------------------------------------------
	WLevelsList implementation.
-----------------------------------------------------------------------------*/

//
// List of levels.
//
class WLevelsList: public WListBox
{
public:
	// WLevelsList interface.
	WLevelsList( WLevelsPage* InPage, WWindow* InRoot )
		:	WListBox( InPage, InRoot ),
			Page( InPage )
	{
		Popup			= new WPopupMenu( InRoot, InRoot );
		Popup->AddItem( L"Open Level", WIDGET_EVENT(WLevelsList::PopOpenClick) );
		Popup->AddItem( L"" );
		Popup->AddItem( L"Rename...", WIDGET_EVENT(WLevelsList::PopRenameClick) );
		Popup->AddItem( L"Show Properties", WIDGET_EVENT(WLevelsList::PopShowPropertiesClick) );
		Popup->AddItem( L"" );
		Popup->AddItem( L"Delete Level", WIDGET_EVENT(WLevelsList::PopDeleteClick) );
	}
	~WLevelsList()
	{
		delete Popup;
	}

	// WWidget interface.
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y ) override
	{
		WListBox::OnMouseUp( Button, X, Y );
		if( Button == MB_Right && ItemIndex != -1 )
			Popup->Show(Root->MousePos);
	}

private:
	// Internal.
	WPopupMenu*		Popup;
	WLevelsPage*	Page;

	// Popup events.
	void PopOpenClick( WWidget* Sender )
	{
		Page->ListLevelsDblClick( Sender );
	}
	void PopRenameClick( WWidget* Sender )
	{
		if( ItemIndex != -1 )
			Page->Browser->AssetsPage->RenameDialog->SetResource((FLevel*)Items[ItemIndex].Data);
	}
	void PopShowPropertiesClick( WWidget* Sender )
	{
		Page->ListLevelsChange( Sender );
	}
	void PopDeleteClick( WWidget* Sender )
	{
		Page->ButtonDeleteLevelClick( Sender );
	}
};


/*-----------------------------------------------------------------------------
	WLevelsPage implementation.
-----------------------------------------------------------------------------*/

//
// Levels page constructor.
//
WLevelsPage::WLevelsPage( WResourceBrowser* InBrowser, WContainer* InOwner, WWindow* InRoot )
	:	WTabPage( InOwner, InRoot ),
		Browser( InBrowser )
{
	// Initialize own variables.
	bCanClose		= false;
	Caption			= L"Levels";
	TabWidth		= 55;
	Color			= COLOR_LEVELS_PAGE;

	WPanel* HeaderPanel		= new WPanel( this, Root );
	HeaderPanel->Align		= AL_Top;
	HeaderPanel->bDrawEdges	= true;
	HeaderPanel->SetSize( 50, 26 );

	AddLevelButton				= new WPictureButton( HeaderPanel, Root );
	AddLevelButton->Tooltip		= L"Create New Level";
	AddLevelButton->Picture		= Root->Icons;
	AddLevelButton->Offset		= TPoint( 0, 128 );
	AddLevelButton->Scale		= TSize( 16, 16 );
	AddLevelButton->Location	= TPoint( 2, 2 );
	AddLevelButton->EventClick	= WIDGET_EVENT(WLevelsPage::ButtonAddLevelClick);
	AddLevelButton->SetSize( 22, 22 );

	ProjectPropsButton				= new WPictureButton( HeaderPanel, Root );
	ProjectPropsButton->Tooltip		= L"Show Project Properties";
	ProjectPropsButton->Picture		= Root->Icons;
	ProjectPropsButton->Offset		= TPoint( 16, 128 );
	ProjectPropsButton->Scale		= TSize( 16, 16 );
	ProjectPropsButton->Location	= TPoint( 23, 2 );
	ProjectPropsButton->EventClick	= WIDGET_EVENT(WLevelsPage::ButtonProjectPropsClick);
	ProjectPropsButton->SetSize( 22, 22 );

	DeleteLevelButton				= new WPictureButton( HeaderPanel, Root );
	DeleteLevelButton->Tooltip		= L"Delete Level";
	DeleteLevelButton->Picture		= Root->Icons;
	DeleteLevelButton->Offset		= TPoint( 32, 128 );
	DeleteLevelButton->Scale		= TSize( 16, 16 );
	DeleteLevelButton->Location		= TPoint( Size.Width-22-8, 2 );
	DeleteLevelButton->Align		= AL_Right;
	DeleteLevelButton->Margin		= TArea( 2, 2, 0, 2 );
	DeleteLevelButton->EventClick	= WIDGET_EVENT(WLevelsPage::ButtonDeleteLevelClick);
	DeleteLevelButton->SetSize( 22, 22 );

	// List of levels.
	LevelsList						= new WLevelsList( this, Root );
	LevelsList->Margin				= TArea( -1, 0, 0, 0 );
	LevelsList->Align				= AL_Client;
	LevelsList->ItemsHeight			= 20;
	LevelsList->EventChange			= WIDGET_EVENT(WLevelsPage::ListLevelsChange);
	LevelsList->EventDblClick		= WIDGET_EVENT(WLevelsPage::ListLevelsDblClick);

	// Level constructor.
	LevelConstructor = new WLevelConstructor( this, Root );
}


//
// Levels page destructor.
//
WLevelsPage::~WLevelsPage()
{
	// Release constructor.
	freeandnil(LevelConstructor);
}


//
// Redraw the levels page.
//
void WLevelsPage::OnPaint( CGUIRenderBase* Render )
{
	WTabPage::OnPaint( Render );

	// Turn on or turn off buttons.
	DeleteLevelButton->bEnabled		= GProject != nullptr && LevelsList->ItemIndex != -1;
	AddLevelButton->bEnabled		= GProject != nullptr;
	ProjectPropsButton->bEnabled	= GProject != nullptr;
}


//
// Refresh the list of levels.
//
void WLevelsPage::Refresh()
{
	LevelsList->Empty();

	if( !GProject )
		return;

	// Collect all levels.
	for( Int32 i=0; i<GProject->GObjects.size(); i++ )
		if	(	
				GProject->GObjects[i] && 
				GProject->GObjects[i]->IsA(FLevel::MetaClass) 
			)
		{
			FLevel* Level = (FLevel*)GProject->GObjects[i];
			if( !Level->IsTemporal() )
				LevelsList->AddItem( Level->GetName(), Level );
		}
}


//
// Count all references.
//
void WLevelsPage::CountRefs( CSerializer& S )
{
	Bool bRefresh = false;

	for( Int32 i=0; i<LevelsList->Items.size(); i++ )
	{
		FLevel* Level = (FLevel*)LevelsList->Items[i].Data;
		Serialize( S, Level );
		if( !Level )
		{
			// Found with bad reference.
			bRefresh	= true;
			break;
		}
	}

	if( bRefresh )
		Refresh();
}


//
// Show project properties.
//
void WLevelsPage::ButtonProjectPropsClick( WWidget* Sender )
{
	assert(GProject->Info);
	GEditor->Inspector->SetEditObject( GProject->Info );
}


//
// User click yes, when destroy level.
//
void WLevelsPage::MessageDeleteYesClick( WWidget* Sender )
{
	Root->Modal->OnClose();

	FLevel* Level = (FLevel*)LevelsList->Items[LevelsList->ItemIndex].Data;

	DestroyObject( Level, true );
	Refresh();
}


//
// When delete button clicked.
//
void WLevelsPage::ButtonDeleteLevelClick( WWidget* Sender )
{
	if( LevelsList->ItemIndex != -1 )
	{
		FLevel* Level = (FLevel*)LevelsList->Items[LevelsList->ItemIndex].Data;

		Root->AskYesNo
		(
			*String::format( L"Do you really want to destroy level '%s'?", *Level->GetName() ),
			L"Project Explorer",
			true,
			WIDGET_EVENT(WLevelsPage::MessageDeleteYesClick)
		);
	}
}


//
// When level item changed.
//
void WLevelsPage::ListLevelsChange( WWidget* Sender )
{
	if( LevelsList->ItemIndex != -1 )
	{
		FLevel* Level = As<FLevel>((FObject*)LevelsList->Items[LevelsList->ItemIndex].Data);
		GEditor->Inspector->SetEditObject( Level );
	}
}


//
// Open a level constructor.
//
void WLevelsPage::ButtonAddLevelClick( WWidget* Sender )
{
	if( !GProject )
		return;

	// Open dialog.
	LevelConstructor->Show
	( 
		Root->Size.Width/3, 
		Root->Size.Height/3  
	);
}


//
// When double clicked on item.
//
void WLevelsPage::ListLevelsDblClick( WWidget* Sender )
{
	if( LevelsList->ItemIndex != -1 )
	{
		// Open page.
		GEditor->OpenPageWith
		(
			(FLevel*)LevelsList->Items[LevelsList->ItemIndex].Data
		);
	}
}


/*-----------------------------------------------------------------------------
	WScriptsView implementation.
-----------------------------------------------------------------------------*/

//
// Tree of the scripts.
//
class WScriptsView: public WTreeView
{
public:
	// WScriptsView interface.
	WScriptsView( WScriptsPage* InPage, WWindow* InRoot )
		:	WTreeView( InPage, InRoot ),
			Page( InPage ),
			bReadyForDrag( false )
	{
		Popup		= new WPopupMenu( InRoot, InRoot );
		Popup->AddItem( L"Edit Script...",		WIDGET_EVENT(WScriptsView::PopEditScriptClick) );
		Popup->AddItem( L"Edit Properties...",	WIDGET_EVENT(WScriptsView::PopEditPropertiesClick) );
		Popup->AddItem( L"" );
		Popup->AddItem( L"Script Usage",		WIDGET_EVENT(WScriptsView::PopScriptUsageClick) );
	}
	~WScriptsView()
	{
		delete Popup;
	}

	// WWidget interface.
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y ) override
	{
		WTreeView::OnMouseUp( Button, X, Y );
		bReadyForDrag = false;
		if( Button == MB_Right && GetSelected() != -1 )
			Popup->Show(Root->MousePos);
	}
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y ) override
	{
		WTreeView::OnMouseDown( Button, X, Y );
		bReadyForDrag = Button == MB_Left && GetSelectedScript() != nullptr;
	}
	void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y ) override
	{
		WTreeView::OnMouseMove( Button, X, Y );
		FScript* Selected = GetSelectedScript();
		if( bReadyForDrag && Selected )
		{
			BeginDrag(Selected);
		}
		bReadyForDrag = false;
	}
	void OnDblClick( EMouseButton Button, Int32 X, Int32 Y ) override
	{
		WTreeView::OnDblClick( Button, X, Y );
		bReadyForDrag = false;
	}

private:
	// Internal.
	WPopupMenu*		Popup;
	WScriptsPage*	Page;
	Bool			bReadyForDrag;

	// Popup events.
	void PopEditScriptClick( WWidget* Sender )
	{
		Page->ButtonEditClick(this);
	}
	void PopEditPropertiesClick( WWidget* Sender )
	{
		FScript* Script = GetSelectedScript();
		if( Script )
		{
			GEditor->Inspector->SetEditObject(Script);
		}
	}
	void PopScriptUsageClick( WWidget* Sender )
	{
		FScript* Script = GetSelectedScript();
		if( GObjectDatabase && Script )
		{
			Int32 NumEntity = 0;
			for( Int32 i=0; i < GObjectDatabase->GObjects.size(); i++ )
				if( GObjectDatabase->GObjects[i] )
				{
					FEntity* Entity = As<FEntity>(GObjectDatabase->GObjects[i]);
					if( Entity && Entity->Script == Script )
						NumEntity++;
				}
			Root->ShowMessage
			(
				String::format( L"%i entities being to '%s' script.", NumEntity, *Script->GetName() ),
				String::format( L"Scipt '%s' Info: ", *Script->GetName() ),
				true
			);
		}
	}
	FScript* GetSelectedScript()
	{
		Int32 i = GetSelected();
		return i != -1 ? (FScript*)DataOf(i) : nullptr;
	}
};


/*-----------------------------------------------------------------------------
	WScriptsPage implementation.
-----------------------------------------------------------------------------*/

//
// Scripts page constructor.
//
WScriptsPage::WScriptsPage( WResourceBrowser* InBrowser, WContainer* InOwner, WWindow* InRoot )
	:	WTabPage( InOwner, InRoot ),
		Browser( InBrowser )
{
	// Initialize own variables.
	bCanClose		= false;
	Caption			= L"Scripts";
	TabWidth		= 55;
	Color			= COLOR_SCRIPTS_PAGE;

	// Panels.
	WPanel* Header		= new WPanel( this, Root );
	Header->Align		= AL_Top;
	Header->SetSize( 200, 26 );

	WPanel* Footer		= new WPanel( this, Root );
	Footer->Align		= AL_Bottom;
	Footer->SetSize( 200, 24 );

	EntityOnlyCheck				= new WCheckBox( Footer, Root );
	EntityOnlyCheck->Caption	= L"Entity Scripts Only?";
	EntityOnlyCheck->Location	= TPoint( 5, 5 );
	EntityOnlyCheck->EventClick	= WIDGET_EVENT(WScriptsPage::CheckEntityOnlyChange);

	CreateButton				= new WPictureButton( Header, Root );
	CreateButton->Tooltip		= L"Create Script";
	CreateButton->Picture		= Root->Icons;
	CreateButton->Offset		= TPoint( 16, 96 );
	CreateButton->Scale			= TSize( 16, 16 );
	CreateButton->Location		= TPoint( 2, 2 );
	CreateButton->EventClick	= WIDGET_EVENT(WScriptsPage::ButtonCreateClick);
	CreateButton->SetSize( 22, 22 );

	EditButton					= new WPictureButton( Header, Root );
	EditButton->Tooltip			= L"Edit Script";
	EditButton->Picture			= Root->Icons;
	EditButton->Offset			= TPoint( 64, 32 );
	EditButton->Scale			= TSize( 16, 16 );
	EditButton->Location		= TPoint( 23, 2 );
	EditButton->EventClick		= WIDGET_EVENT(WScriptsPage::ButtonEditClick);
	EditButton->SetSize( 22, 22 );

	CompileAllButton				= new WPictureButton( Header, Root );
	CompileAllButton->Tooltip		= L"Compile All Scripts";
	CompileAllButton->Picture		= Root->Icons;
	CompileAllButton->Offset		= TPoint( 64, 0 );
	CompileAllButton->Scale			= TSize( 16, 16 );
	CompileAllButton->Location		= TPoint( 44, 2 );
	CompileAllButton->EventClick	= WIDGET_EVENT(WScriptsPage::ButtonCompileAllClick);
	CompileAllButton->SetSize( 22, 22 );

	RemoveButton				= new WPictureButton( Header, Root );
	RemoveButton->Tooltip		= L"Delete Script";
	RemoveButton->Picture		= Root->Icons;
	RemoveButton->Offset		= TPoint( 48, 96 );
	RemoveButton->Scale			= TSize( 16, 16 );
	RemoveButton->Align			= AL_Right;
	RemoveButton->Margin		= TArea( 2, 2, 0, 2 );
	RemoveButton->EventClick	= WIDGET_EVENT(WScriptsPage::ButtonRemoveClick);
	RemoveButton->SetSize( 22, 22 );

	// Main Scripts Viewer.
	ScriptsView					= new WScriptsView( this, Root );
	ScriptsView->Margin			= TArea( -1, -1, 0, 0 );
	ScriptsView->Align			= AL_Client;
	ScriptsView->EventDblClick	= WIDGET_EVENT(WScriptsPage::ViewDblClick);
	ScriptsView->EventClick		= WIDGET_EVENT(WScriptsPage::ViewSelectionChange);
}


//
// Scripts page destructor.
//
WScriptsPage::~WScriptsPage()
{
}


//
// Scripts page painting.
//
void WScriptsPage::OnPaint( CGUIRenderBase* Render )
{
	WTabPage::OnPaint(Render);

	// Turn on or turn off buttons.
	CreateButton->bEnabled		= 
	CompileAllButton->bEnabled	= GProject != nullptr;
	EditButton->bEnabled		=
	RemoveButton->bEnabled		= GProject != nullptr && ScriptsView->GetSelected() != -1;
}	


//
// Refresh list of scripts.
//
void WScriptsPage::Refresh()
{
	// No project.
	if( !GProject )
	{
		ScriptsView->Empty();
		return;
	}

	// Prepare.
	ScriptsView->Empty();

	// For each object in project.
	for( Int32 i=0; i<GProject->GObjects.size(); i++ )
		if( GProject->GObjects[i] && GProject->GObjects[i]->IsA(FScript::MetaClass) )
		{
			FScript* Script = (FScript*)GProject->GObjects[i];

			if( EntityOnlyCheck->bChecked && Script->IsStatic() )
				continue;

			// Add to list.
			ScriptsView->AddNode
			(
				Script->GetName(),
				-1,
				Script
			);
		}

	// Sort by name.
	ScriptsView->AlphabetSort();
}


//
// Count all references.
//
void WScriptsPage::CountRefs( CSerializer& S )
{
	Bool bRefresh = false;

	for( Int32 i=0; i<ScriptsView->Nodes.size(); i++ )
	{
		FScript* Script = (FScript*)ScriptsView->Nodes[i].Data;
		Serialize( S, Script );
		if( !Script )
		{
			// Found with bad reference.
			bRefresh	= true;
			break;
		}
	}

	if( bRefresh )
		Refresh();
}


//
// Remove button clicked.
//
void WScriptsPage::ButtonRemoveClick( WWidget* Sender )
{
	Int32 iSelected = ScriptsView->GetSelected();
	FScript* Script = iSelected != -1 ? (FScript*)ScriptsView->DataOf(iSelected) : nullptr;

	if( Script )
	{
		// Dirty way, but work fine, just let assets page kill
		// script as regular resource.
		Browser->Selected = Script;
		Browser->AssetsPage->ButtonRemoveClick(this);
	}
}


//
// Open a script constructor.
//
void WScriptsPage::ButtonCreateClick( WWidget* Sender )
{
	Browser->AssetsPage->PopNewScriptClick(this);
}


//
// Open script editor.
//
void WScriptsPage::ButtonEditClick( WWidget* Sender )
{
	Int32 iSelected = ScriptsView->GetSelected();
	FScript* Script = iSelected != -1 ? (FScript*)ScriptsView->DataOf(iSelected) : nullptr;

	if( Script )
		Browser->ActivateResource(Script);
}


//
// Recompile all scripts.
//
void WScriptsPage::ButtonCompileAllClick( WWidget* Sender )
{
	// Let editor compile all scripts.
	GEditor->Inspector->Empty();   
	GEditor->CompileAllScripts(GEditor->TaskDialog);
}


//
// Show only entity scripts or not.
//
void WScriptsPage::CheckEntityOnlyChange( WWidget* Sender )
{
	Refresh();
}


//
// Double click on script view.
//
void WScriptsPage::ViewDblClick( WWidget* Sender )
{
	ButtonEditClick( this );
}


//
// Change selection.
//
void WScriptsPage::ViewSelectionChange( WWidget* Sender )
{
	Int32 iSelected = ScriptsView->GetSelected();
	Browser->Selected = iSelected != -1 ? (FScript*)ScriptsView->DataOf(iSelected) : nullptr;
}


//
// Script page just opened.
//
void WScriptsPage::OnOpen()
{
	WTabPage::OnOpen();
	ScriptsView->SetSelected( -1, true );
}


/*-----------------------------------------------------------------------------
	WResourcePane implementation.
-----------------------------------------------------------------------------*/

// Pane color preferences.
#define COLOR_RESPANE_ICON				TColor( 0x25, 0xff, 0xff, 0xff )
#define COLOR_RESPANE_ICON_PAD			TColor( 0x30, 0x30, 0x30, 0xff )
#define COLOR_RESPANE_ICON_BORDER		TColor( 0xc8, 0xc8, 0xc8, 0xff )
#define COLOR_RESPANE_HIGHLIGHT			TColor( 0x32, 0xa4, 0xf0, 0xff )

// Pane constants.
#define PANE_ICONS_BORDER_SIZE			(1)
#define PANE_ICONS_STEP					(16)


//
// Browser pane constructor.
//
WResourcePane::WResourcePane( WAssetsPage* InPage, WWindow* InRoot )
	:	WPanel( InPage, InRoot ),
		Page( InPage ),
		bReadyForDrag( false ),
		Icons()
{
	// Create scrollbar.
	ScrollBar				= new WSlider( this, InRoot );
	ScrollBar->Orientation	= SLIDER_Vertical;
	ScrollBar->Align		= AL_Right;
	ScrollBar->EventChange	= WIDGET_EVENT(WResourcePane::ScrollChange);
	ScrollBar->SetSize( 12, 50 );
}


//
// Pane destructor.
//
WResourcePane::~WResourcePane()
{
}


//
// Update resources list.
//
void WResourcePane::Refresh()
{
	// No project.
	if( !GProject )
	{
		Icons.empty();
		return;
	}

	// Precompute.
	String	NameFil	= String::upperCase( Page->NameFilter->Text );
	Icons.empty();


	// For each object.
	for( Int32 i=0; i<GProject->GObjects.size(); i++ )
		if( GProject->GObjects[i] && GProject->GObjects[i]->IsA(FResource::MetaClass) )
		{
			FResource*	Res = (FResource*)GProject->GObjects[i];

			// Reject some resources.
			if( Res->IsA(FLevel::MetaClass) || Res->IsA(FProjectInfo::MetaClass) )
				continue;

			// Apply name filter.
			if( NameFil )
			{						
				if( String::pos(NameFil, String::upperCase(Res->GetName())) == -1 )
					continue;
			}

			// Class filter.
			if( Page->ClassFilter )
			{
				if( Page->ClassFilter->IsA(FFont::MetaClass) )
				{
					// Show fonts and it bitmaps.
					if( !(Res->IsA(FFont::MetaClass) || (Res->GetOwner() && Res->GetOwner()->IsA(FFont::MetaClass))) )
						continue;
				}
				else
				{
					// Not font filter.
					if( !Res->IsA(Page->ClassFilter) )
						continue;
				}
			}

			// Don't show folded resource, except font pages only.
			if( Res->GetOwner() && !(Page->ClassFilter && Page->ClassFilter->IsA(FFont::MetaClass)) )
				continue;

			// Initialize icon.
			TIcon Icon;
			Icon.Position	= TPoint::Zero;		
			Icon.Resource	= Res;
			Icon.Picture	= nullptr;
			Icon.TypeName	= L"[BadRes]";

			if( Res->IsA(FTexture::MetaClass) )
			{
				//
				// Draw texture as bitmap or material, of course.
				//
				Icon.Picture	= As<FTexture>(Res);
				Icon.TypeName	= Res->IsA(FMaterial::MetaClass) ? L"[Material]" : 
									(Icon.Picture->USize==256 && Icon.Picture->VSize==1) ? L"[Palette]" : L"[Bitmap]";
				Icon.PicOffset	= TPoint( 0, 0 );
				Icon.PicSize	= TSize( Icon.Picture->USize, Icon.Picture->VSize );

				// Compute new scale, but keep aspect ratio.
				Int32 Scale = max( 1, max( Icon.Picture->USize, Icon.Picture->VSize )/RES_ICON_SIZE );
				Icon.Scale.Width	= max( 1, Icon.Picture->USize/Scale );
				Icon.Scale.Height	= max( 1, Icon.Picture->VSize/Scale );
			}
			else if( Res->IsA(FAnimation::MetaClass) )
			{
				//
				// Draw first frame of animation.
				//
				FAnimation* Anim	= As<FAnimation>(Res);
				Icon.TypeName		= L"[Anim]";
				if( Anim->Sheet && Anim->Frames.size() && Anim->Sequences.size() )
				{
					// Valid animation frame.
					math::Rect Frame = Anim->GetTexCoords(Anim->Sequences[0].Start);
					Icon.Picture			= Anim->Sheet;

					Icon.PicOffset.X	= Int32( (Float)Anim->Sheet->USize*Frame.min.x );
					Icon.PicOffset.Y	= Int32( (Float)Anim->Sheet->VSize*Frame.max.y );
					Icon.PicSize.Width	= Int32( (Float)Anim->Sheet->USize*(Frame.max.x-Frame.min.x) );
					Icon.PicSize.Height	= Int32( (Float)Anim->Sheet->VSize*(Frame.min.y-Frame.max.y) );

					Float Scale = max( 1, max( Icon.PicSize.Width, Icon.PicSize.Height )/RES_ICON_SIZE );
					Icon.Scale.Width	= max<Int32>( 1, Icon.PicSize.Width/Scale );
					Icon.Scale.Height	= max<Int32>( 1, Icon.PicSize.Height/Scale );
				}
				else
				{
					// Bad animation.
					Icon.Picture		= FBitmap::NullBitmap();
					Icon.PicOffset		= TPoint( 0, 0 );
					Icon.PicSize		= TSize( Icon.Picture->USize, Icon.Picture->VSize );
					Icon.Scale			= TSize( 32, 32 );
				}
			}
			else if( Res->IsA(FScript::MetaClass) )
			{
				//
				// Draw script resource as Jigsaw.
				//
				FScript* Script = As<FScript>(Res);
				Icon.TypeName	= L"[Script]";
				Icon.Picture	= Root->Icons;
				Icon.PicOffset	= TPoint( 0, 256-96 );
				Icon.PicSize	= TSize( 32, 32 );
				Icon.Scale		= TSize( 32, 32 );

				// Maybe draw as billboard.
				for( Int32 e=0; e<Script->Components.size(); e++ )
				{
					FExtraComponent* Com = Script->Components[e];
					if( Com->IsA(FAnimatedSpriteComponent::MetaClass) )
					{
						// Draw as animation.
						FAnimation* Anim	= As<FAnimatedSpriteComponent>(Com)->Animation;
						if( Anim && Anim->Sheet && Anim->Frames.size() && Anim->Sequences.size() )
						{
							math::Rect Frame = Anim->GetTexCoords(Anim->Sequences[0].Start);
							Icon.Picture			= Anim->Sheet;

							Icon.PicOffset.X	= Int32( (Float)Anim->Sheet->USize*Frame.min.x );
							Icon.PicOffset.Y	= Int32( (Float)Anim->Sheet->VSize*Frame.max.y );
							Icon.PicSize.Width	= Int32( (Float)Anim->Sheet->USize*(Frame.max.x-Frame.min.x) );
							Icon.PicSize.Height	= Int32( (Float)Anim->Sheet->VSize*(Frame.min.y-Frame.max.y) );

							Float Scale = max( 1, max( Icon.PicSize.Width, Icon.PicSize.Height )/RES_ICON_SIZE );
							Icon.Scale.Width	= max<Int32>( 1, Icon.PicSize.Width/Scale );
							Icon.Scale.Height	= max<Int32>( 1, Icon.PicSize.Height/Scale );
							break;
						}
						else
							goto NoSprite;
					}
					else if( Com->IsA(FSpriteComponent::MetaClass) )
					{
						// Single sprite.
						FSpriteComponent* Sprite = As<FSpriteComponent>(Com);
						if( Sprite->Texture )
						{
							math::Rect	Frame		= Sprite->TexCoords;
							Icon.Picture			= Sprite->Texture;

							Icon.PicOffset.X	= Int32( Frame.min.x );
							Icon.PicOffset.Y	= Int32( Frame.min.y );
							Icon.PicSize.Width	= Int32( (Frame.max.x-Frame.min.x) );
							Icon.PicSize.Height	= Int32( (Frame.max.y-Frame.min.y) );

							Float Scale = max( 1, max( Icon.PicSize.Width, Icon.PicSize.Height )/RES_ICON_SIZE );
							Icon.Scale.Width	= max<Int32>( 1, Icon.PicSize.Width/Scale );
							Icon.Scale.Height	= max<Int32>( 1, Icon.PicSize.Height/Scale );
							break;
						}
						else
							goto NoSprite;
					}			
					else if( Com->IsA(FDecoComponent::MetaClass) )
					{
						// Deco sprite.
						FDecoComponent* Sprite = As<FDecoComponent>(Com);
						if( Sprite->Texture )
						{
							math::Rect	Frame		= Sprite->TexCoords;
							Icon.Picture			= Sprite->Texture;

							Icon.PicOffset.X	= Int32( (Float)Icon.Picture->USize*Frame.min.x );
							Icon.PicOffset.Y	= Int32( (Float)Icon.Picture->VSize*Frame.max.y );
							Icon.PicSize.Width	= Int32( (Float)Icon.Picture->USize*(Frame.max.x-Frame.min.x) );
							Icon.PicSize.Height	= Int32( (Float)Icon.Picture->VSize*(Frame.min.y-Frame.max.y) );

							Float Scale = max( 1, max( Icon.PicSize.Width, Icon.PicSize.Height )/RES_ICON_SIZE );
							Icon.Scale.Width	= max<Int32>( 1, Icon.PicSize.Width/Scale );
							Icon.Scale.Height	= max<Int32>( 1, Icon.PicSize.Height/Scale );
							break;
						}
						else
							goto NoSprite;
					}	
					continue;

					{
						// No sprite.
					NoSprite:;
						Icon.Picture	= FBitmap::NullBitmap();
						Icon.PicOffset	= TPoint( 0, 0 );
						Icon.PicSize	= TSize( Icon.Picture->USize, Icon.Picture->VSize );
						Icon.Scale		= TSize( 32, 32 );
						break;
					}
				}
			}
			else if( Res->IsA(FSound::MetaClass) )
			{
				//
				// Draw sound resource as Speaker.
				//
				Icon.TypeName	= L"[Sound]";
				Icon.Picture	= Root->Icons;
				Icon.PicOffset	= TPoint( 32, 256-96 );
				Icon.PicSize	= TSize( 32, 32 );
				Icon.Scale		= TSize( 32, 32 );
			}
			else if( Res->IsA(FMusic::MetaClass) )
			{
				//
				// Draw music resource as AudioTape.
				//
				Icon.TypeName	= L"[Music]";
				Icon.Picture	= Root->Icons;
				Icon.PicOffset	= TPoint( 64, 256-96 );
				Icon.PicSize	= TSize( 32, 32 );
				Icon.Scale		= TSize( 32, 32 );
			}
			else if( Res->IsA(FSkeleton::MetaClass) )
			{
				//
				// Draw skeleton resource as Jolly Roger.
				//
				Icon.TypeName	= L"[Skeleton]";
				Icon.Picture	= Root->Icons;
				Icon.PicOffset	= TPoint( 666, 256-96 );
				Icon.PicSize	= TSize( 32, 32 );
				Icon.Scale		= TSize( 32, 32 );
			}
			else if( Res->IsA(FFont::MetaClass) )
			{
				//
				// Draw font as first page.
				//
				FFont* Font		= As<FFont>(Res);
				Icon.TypeName	= L"[Font]";
				Icon.Picture	= Font->Bitmaps[0];
				Icon.PicOffset	= TPoint( 0, 0 );
				Icon.PicSize	= TSize( GLYPHS_ATLAS_SIZE, GLYPHS_ATLAS_SIZE );
				Icon.Scale		= TSize( RES_ICON_SIZE, RES_ICON_SIZE );
			}

			Icons.push(Icon);	
		}

	// Alphabet sorting.
	Icons.sort([]( const TIcon& A, const TIcon& B )->Bool
	{
		return String::insensitiveCompare
		( 
			A.Resource->GetName(), 
			B.Resource->GetName() 
		) < 0;
	});

	// Scroll to previous location and update icons
	// locations.
	ScrollChange( this );
}


//
// User click on resource panel.
//
void WResourcePane::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y )
{
	WPanel::OnMouseDown( Button, X, Y );

	if( Button == MB_Left || Button == MB_Right )
	{
		// Prepare for drag.
		bReadyForDrag	= true;

		// Get clicked resource.
		FResource* Res = nullptr;
		for( Int32 i=0; i<Icons.size(); i++ )
		{
			TIcon& Icon = Icons[i];

			if	(
					X >= Icon.Position.X &&
					Y >= Icon.Position.Y &&
					X < Icon.Position.X+RES_ICON_SIZE &&
					Y < Icon.Position.Y+RES_ICON_SIZE
				)
			{
				// Gotcha.
				Res	= Icon.Resource;
				break;
			}
		}

		// Select it!
		Page->Browser->Selected	= Res;
	}

	// Popup it!
	if( Button == MB_Right && Page->Browser->Selected )
	{
		bReadyForDrag	= false;
		Page->ShowResourceMenu(Page->Browser->Selected);
	}
}


//
// User hover mouse above panel.
//
void WResourcePane::OnMouseMove( EMouseButton Button, Int32 X, Int32 Y )
{
	WPanel::OnMouseMove( Button, X, Y );

	if( bReadyForDrag && Page->Browser->Selected )
	{
		// Start drag object.
		BeginDrag( Page->Browser->Selected );
		bReadyForDrag	= false;
	}
}


//
// User just release mouse button.
//
void WResourcePane::OnMouseUp( EMouseButton Button, Int32 X, Int32 Y )
{
	WPanel::OnMouseUp( Button, X, Y );
	bReadyForDrag = false;
}


//
// Double click on browser.
//
void WResourcePane::OnDblClick( EMouseButton Button, Int32 X, Int32 Y )
{
	WPanel::OnDblClick( Button, X, Y );
	bReadyForDrag		= false;
	Page->Browser->ActivateResource(Page->Browser->GetSelected());
} 


//
// Scroll resource icons.
//
void WResourcePane::ScrollChange( WWidget* Sender )
{
	// Prepare for walking.
	Int32 TotalWidth	= Size.Width - PANE_ICONS_STEP;
	Int32 XCount		= TotalWidth / (RES_ICON_SIZE + PANE_ICONS_STEP);
	Int32 XSpacing		= (TotalWidth - XCount*(RES_ICON_SIZE + PANE_ICONS_STEP)) / max(1, XCount);
	Int32	XWalk		= PANE_ICONS_STEP;
	Int32	YWalk		= PANE_ICONS_STEP;

	// Walk through all icons and compute initial location,
	// without scroll yet.
	for( Int32 i=0; i<Icons.size(); i++ )
	{
		TIcon& Icon = Icons[i];

		Icon.Position.X	= XWalk;
		Icon.Position.Y	= YWalk;

		// Iterate to next.
		XWalk += RES_ICON_SIZE + 16 + XSpacing;
		if( XWalk+RES_ICON_SIZE > Size.Width - (ScrollBar->Size.Width+5) )
		{
			XWalk	= PANE_ICONS_STEP;
			YWalk	+= RES_ICON_SIZE + 24;
		}
	}

	// Apply scroll for each icon.
	Int32 YScroll = max( 0, YWalk-RES_ICON_SIZE-30 )*ScrollBar->Value / 100;
	for( Int32 i=0; i<Icons.size(); i++ )
		Icons[i].Position.Y -= YScroll;
}


//
// Scroll browser via mouse wheel.
//
void WResourcePane::OnMouseScroll( Int32 Delta )
{
	ScrollBar->Value = clamp
	( 
		ScrollBar->Value - Delta/40, 
		0, 
		100 
	);
	ScrollChange( this );
}


//
// Count references and remove unused.
//
void WResourcePane::CountRefs( CSerializer& S )
{
	// Detect whether changed some icon.
	Bool bChanged = false;
	for( Int32 i=0; i<Icons.size(); i++ )
	{
		auto& Icon = Icons[i];
		Serialize( S, Icon.Resource );

		if( Icons[i].Resource == nullptr )
		{
			bChanged	= true;
			break;
		}
	}

	// Update if was changed.
	if( bChanged )
		Refresh();
}


//
// Browser pane resize.
//
void WResourcePane::OnResize()
{
	WPanel::OnResize();

	// Force to repose icons.
	ScrollChange( this );
}


//
// Render all icons.
//
void WResourcePane::OnPaint( CGUIRenderBase* Render )
{
	WPanel::OnPaint( Render );
	TPoint Base = ClientToWindow(TPoint::Zero);

	// Clip icons.
	Render->SetClipArea
	(
		TPoint( Base.X, Base.Y+1 ), 
		TSize( Size.Width, Size.Height-2 )
	);

	// Iterate through all icons.
	for( Int32 i=0; i<Icons.size(); i++ )
	{
		TIcon& Icon = Icons[i];
		Bool bSelected = Icon.Resource == Page->Browser->GetSelected();

		// Visibility test.
		if( Icon.Position.Y <= -80 )		continue;
		if( Icon.Position.Y > Size.Height )	continue;

		// Draw icon border.
		Render->DrawRegion
		( 
			TPoint
			( 
				Base.X+Icon.Position.X - PANE_ICONS_BORDER_SIZE, 
				Base.Y + Icon.Position.Y - PANE_ICONS_BORDER_SIZE 
			), 
			TSize
			( 
				RES_ICON_SIZE + PANE_ICONS_BORDER_SIZE*2, 
				RES_ICON_SIZE + PANE_ICONS_BORDER_SIZE*2 
			), 
			COLOR_RESPANE_ICON_PAD,
			bSelected ? COLOR_RESPANE_HIGHLIGHT : COLOR_RESPANE_ICON_BORDER,
			BPAT_Solid 
		);

		// Icon label.
		Render->DrawText
		(			
			TPoint( Base.X + Icon.Position.X + 1, Base.Y + Icon.Position.Y + RES_ICON_SIZE + 1 ), 
			Icon.Resource->GetName(), 
			bSelected ? COLOR_RESPANE_HIGHLIGHT : GUI_COLOR_TEXT, 
			Root->Font1 
		);

		// Draw picture.
		if( Icon.Picture )
		{
			// Temporally switch style for root icons.
			if( Icon.Picture == Root->Icons )
				Root->Icons->BlendMode	= BLEND_Translucent;

			// Draw it.
			Render->DrawPicture
			( 
				Base + Icon.Position + TPoint( (RES_ICON_SIZE-Icon.Scale.Width)/2, (RES_ICON_SIZE-Icon.Scale.Height)/2 ),
				Icon.Scale, 
				Icon.PicOffset, 
				Icon.PicSize, 
				Icon.Picture
			);

			// Restore root icons default style.
			if( Icon.Picture == Root->Icons )
				Root->Icons->BlendMode	= BLEND_Masked;
		}

		// Draw overlay label.
		Int32 Width = Root->Font1->TextWidth( Icon.TypeName );
		Render->DrawText
		( 
			Base + Icon.Position + TPoint( RES_ICON_SIZE/2-Width/2, RES_ICON_SIZE - 17 ),
			Icon.TypeName, 
			bSelected ? COLOR_RESPANE_HIGHLIGHT * 0.75f : GUI_COLOR_TEXT, 
			Root->Font1 
		);	
	}
}


/*-----------------------------------------------------------------------------
	WImportDialog implementation.
-----------------------------------------------------------------------------*/

//
// A resource import dialog.
// 
class WImportDialog: public WForm
{
public:
	// WImportDialog interface.
	WImportDialog( WResourceBrowser* InBrowser, WWindow* InRoot )
		:	WForm( InRoot, InRoot ),
			Browser( InBrowser )
	{
		// Initialize the form.
		Caption			= L"Resource Importer";
		bCanClose		= true;
		bSizeableH		= false;
		bSizeableW		= false;
		SetSize( 240, 123 );

		TopPanel			= new WPanel( this, Root );
		TopPanel->SetLocation( 8, 28 );
		TopPanel->SetSize( 224, 57 );		

		NameLabel			= new WLabel( TopPanel, Root );
		NameLabel->Caption	= L"Name:";
		NameLabel->SetLocation( 8, 8 );

		NameEdit			= new WEdit( TopPanel, Root );
		NameEdit->EditType	= EDIT_String;
		NameEdit->Location	= TPoint( 56, 7 );
		NameEdit->SetSize( 160, 18 );

		GroupLabel			= new WLabel( TopPanel, Root );
		GroupLabel->Caption	= L"Group:";
		GroupLabel->SetLocation( 8, 32 );

		GroupEdit			= new WEdit( TopPanel, Root );
		GroupEdit->EditType	= EDIT_String;
		GroupEdit->Location	= TPoint( 56, 31 );
		GroupEdit->SetSize( 160, 18 );

		OkButton				= new WButton( this, Root );
		OkButton->Caption		= L"Ok";
		OkButton->EventClick	= WIDGET_EVENT(WImportDialog::ButtonOkClick);
		OkButton->SetLocation( 38, 90 );
		OkButton->SetSize( 64, 25 );

		CancelButton				= new WButton( this, Root );
		CancelButton->Caption		= L"Cancel";
		CancelButton->EventClick	= WIDGET_EVENT(WImportDialog::ButtonCancelClick);
		CancelButton->SetLocation( 138, 90 );
		CancelButton->SetSize( 64, 25 );

		Hide();
	}

	// WForm interface.
	void OnClose()
	{
		WForm::OnClose();
		Hide();
	}
	void Show( Int32 X, Int32 Y )
	{
		// Start with dialog.
		if( ExecuteOpenFileDialog
			( 
				GEditor->hWnd,
				FileName, 
				fm::getCurrentDirectory(), 
				L"Any Resource File\0*.bmp;*.tga;*.png;*.wav;*.ogg;*.flf\0\0" 
			) )
		{
			// Show the form.
			WForm::Show( X, Y );

			// Fill fields.
			NameEdit->Text		= fm::getFileName( *FileName );
			GroupEdit->Text		= L"<unused>";
		}
	}

	// Notifications.
	void ButtonCancelClick( WWidget* Sender )
	{
		Hide();
	}
	void ButtonOkClick( WWidget* Sender )
	{
		if( NameEdit->Text.len() == 0 )
		{
			Root->ShowMessage( L"Please specify the name of resource", L"Importer", true );
			return;
		}
		if( GObjectDatabase->FindObject( NameEdit->Text ) )
		{
			Root->ShowMessage
			(
				String::format( L"Object '%s' already exists", *NameEdit->Text ),
				L"Resource Importer",
				true
			);
			return;
		}

		FResource* Res = GEditor->ImportResource( FileName, NameEdit->Text );
		if( Res )
		{
			// Res has been imported.
			Res->Group	= GroupEdit->Text;
		}
		else 
		{
			// Resource import failure.
			debug( L"Import '%s' failed.", *FileName );
		}

		Hide();
		Browser->Refresh();
		// Do not open, just imported resource!
	}

private:
	// Importer internal.
	String				FileName;

	// Controls.
	WResourceBrowser*	Browser;
	WLabel*				NameLabel;
	WLabel*				GroupLabel;
	WEdit*				NameEdit;
	WEdit*				GroupEdit;
	WPanel*				TopPanel;
	WButton*			OkButton;
	WButton*			CancelButton;
};


/*-----------------------------------------------------------------------------
	WDemoEffectBuilder implementation.
-----------------------------------------------------------------------------*/

//
// Demo effect builder form.
//
class WDemoEffectBuilder: public WForm
{
public:
	// WDemoEffectBuilder interface.
	WDemoEffectBuilder( WResourceBrowser* InBrowser, WWindow* InRoot )	
		:	WForm( InRoot, InRoot ),
			Browser( InBrowser )
	{
		// Initialize form.
		SetSize( 220, 175 );
		Caption		= L"Demo Effect Builder";
		bCanClose	= true;
		bSizeableH	= false;
		bSizeableW	= false;

		// Create controls.
		ClassLabel				= new WLabel( this, InRoot );
		ClassLabel->Caption		= L"Class: ";
		ClassLabel->Location	= TPoint( 10, 30 );

		ClassCombo				= new WComboBox( this, InRoot );	
		ClassCombo->Location	= TPoint( 50, 29 );
		ClassCombo->SetSize( 160, 18 );
		for( Int32 i=0; i<CClassDatabase::GClasses.size(); i++ )
		{
			CClass*	Class	= CClassDatabase::GClasses[i];
			if( Class->IsA(FDemoBitmap::MetaClass) && !(Class->Flags & CLASS_Abstract) )
				ClassCombo->AddItem( Class->GetAltName(), Class );
		}
		ClassCombo->AlphabetSort();
		ClassCombo->ItemIndex	= 0;
		ClassCombo->EventChange	= WIDGET_EVENT(WDemoEffectBuilder::ComboClassChange);

		NameLabel				= new WLabel( this, InRoot );
		NameLabel->Caption		= L"Name: ";
		NameLabel->Location		= TPoint( 10, 54 );

		NameEdit				= new WEdit( this, InRoot );
		NameEdit->Location		= TPoint( 50, 53 );
		NameEdit->SetSize( 160, 18 );
		NameEdit->EditType		= EDIT_String;

		GroupLabel				= new WLabel( this, InRoot );
		GroupLabel->Caption		= L"Group: ";
		GroupLabel->Location	= TPoint( 10, 78 );

		GroupEdit				= new WEdit( this, InRoot );
		GroupEdit->Location		= TPoint( 50, 77 );
		GroupEdit->SetSize( 160, 18 );
		GroupEdit->EditType		= EDIT_String;

		WidthLabel				= new WLabel( this, InRoot );
		WidthLabel->Caption		= L"Width: ";
		WidthLabel->Location	= TPoint( 30, 104 );

		HeightLabel				= new WLabel( this, InRoot );
		HeightLabel->Caption	= L"Height: ";
		HeightLabel->Location	= TPoint( 120, 104 );

		WidthCombo				= new WComboBox( this, InRoot );
		WidthCombo->Location	= TPoint( 20, 120 );
		WidthCombo->SetSize( 64, 18 );

		HeightCombo				= new WComboBox( this, InRoot );
		HeightCombo->Location	= TPoint( 110, 120 );
		HeightCombo->SetSize( 64, 18 );

		for( Int32 i=5; i<=8; i++ )
		{
			WidthCombo->AddItem( String::format( L"%d", 1 << i ), nullptr );
			HeightCombo->AddItem( String::format( L"%d", 1 << i ), nullptr );
		}

		WidthCombo->ItemIndex	= 3;
		HeightCombo->ItemIndex	= 3;

		OkButton				= new WButton( this, Root );
		OkButton->EventClick	= WIDGET_EVENT(WDemoEffectBuilder::ButtonOkClick);
		OkButton->Caption		= L"Ok";
		OkButton->SetSize( 200, 20 );
		OkButton->Location		= TPoint( 8, 147 );

		Hide();
	}

	// WForm interface.
	void Show( Int32 X = 0, Int32 Y = 0 )
	{
		WForm::Show( X, Y );
		NameEdit->Text		= L"";
	}
	void OnClose()
	{
		WForm::OnClose();
		Hide();
	}

	// Controls notifications.
	void ComboClassChange( WWidget* Sender )
	{
	}
	void ButtonOkClick( WWidget* Sender )
	{
		if( NameEdit->Text.len() == 0 )
		{
			Root->ShowMessage( L"Please specify the name of effect", L"Effect Builder", true );
			return;
		}
		if( GObjectDatabase->FindObject( NameEdit->Text ) )
		{
			Root->ShowMessage
			(
				String::format( L"Object \"%s\" already exists", *NameEdit->Text ),
				L"Effect Builder",
				true
			);	
			return;
		}

		CClass* DemoClass	= (CClass*)ClassCombo->Items[ClassCombo->ItemIndex].Data;
		assert(DemoClass);
		UInt32	USize		= 1 << (WidthCombo->ItemIndex + 5);
		UInt32	VSize		= 1 << (HeightCombo->ItemIndex + 5);
		assert(USize>=2 && USize<=256);
		assert(VSize>=2 && VSize<=256);

		FBitmap* Bitmap		= NewObject<FBitmap>( DemoClass, NameEdit->Text, nullptr );
		Bitmap->Init( USize, VSize );
		Bitmap->Group		= GroupEdit->Text;

		Hide();
		Browser->Refresh();
		GEditor->OpenPageWith( Bitmap );
	}

private:
	// Controls.
	WResourceBrowser*	Browser;
	WComboBox*			ClassCombo;		
	WEdit*				NameEdit;
	WEdit*				GroupEdit;
	WComboBox*			WidthCombo;
	WComboBox*			HeightCombo;
	WButton*			OkButton;
	WLabel*				ClassLabel;
	WLabel*				NameLabel;
	WLabel*				GroupLabel;
	WLabel*				WidthLabel;
	WLabel*				HeightLabel;
};


/*-----------------------------------------------------------------------------
	WScriptBuilder implementation.
-----------------------------------------------------------------------------*/

//
// Script builder form.
//
class WScriptBuilder: public WForm
{
public:
	// WScriptBuilder interface.
	WScriptBuilder( WResourceBrowser* InBrowser, WWindow* InRoot )
		:	WForm( InRoot, InRoot ),
			Browser( InBrowser )
	{
		// Initialize the form.
		Caption		= L"Script Builder";
		bCanClose	= true;
		bSizeableH	= false;
		bSizeableW	= false;
		SetSize( 445, 417 );

		NameLabel				= new WLabel( this, InRoot );
		NameLabel->Caption		= L"Name: ";
		NameLabel->Location		= TPoint( 100, 30 );

		GroupLabel				= new WLabel( this, InRoot );
		GroupLabel->Caption		= L"Group: ";
		GroupLabel->Location	= TPoint( 100, 54 );

		GroupEdit				= new WEdit( this, InRoot );
		GroupEdit->Location		= TPoint( 145, 53 );
		GroupEdit->EditType		= EDIT_String;
		GroupEdit->SetSize( 160, 18 );

		BaseLabel				= new WLabel( this, InRoot );
		BaseLabel->Caption		= L"Base: ";
		BaseLabel->Location		= TPoint( 100, 78 );

		BaseCombo				= new WComboBox( this, InRoot );	
		BaseCombo->Location		= TPoint( 145, 77 );
		BaseCombo->SetSize( 160, 18 );
		for( Int32 i=0; i<CClassDatabase::GClasses.size(); i++ )
		{
			CClass*	Class	= CClassDatabase::GClasses[i];
			if( Class->IsA(FBaseComponent::MetaClass) && !(Class->Flags & CLASS_Abstract) )
				BaseCombo->AddItem( Class->GetAltName(), Class );
		}
		BaseCombo->AlphabetSort();
		BaseCombo->ItemIndex	= -1;

		NameEdit				= new WEdit( this, InRoot );
		NameEdit->Location		= TPoint( 145, 29 );
		NameEdit->EditType		= EDIT_String;
		NameEdit->SetSize( 160, 18 );

		ExtraPanel				= new WPanel( this, InRoot );
		ExtraPanel->Location	= TPoint( 10, 107 );
		ExtraPanel->SetSize( 425, 256 );	

		ExtraList				= new WListBox( ExtraPanel, InRoot );
		ExtraList->Location		= TPoint( 10, 10 );
		ExtraList->SetSize( 180, 200 );
		for( Int32 i=0; i<CClassDatabase::GClasses.size(); i++ )
		{
			CClass*	Class	= CClassDatabase::GClasses[i];
			if( Class->IsA(FExtraComponent::MetaClass) && !(Class->Flags & CLASS_Abstract) )
				ExtraList->AddItem( Class->GetAltName(), Class );
		}
		ExtraList->AlphabetSort();
		ExtraList->ItemIndex		= -1;	
		ExtraList->EventChange		= WIDGET_EVENT(WScriptBuilder::ListExtraChange);
		ExtraList->EventDblClick	= WIDGET_EVENT(WScriptBuilder::ButtonAddExtraClick);

		AddExtraButton				= new WButton( ExtraPanel, InRoot );
		AddExtraButton->Caption		= L">>";
		AddExtraButton->Tooltip		= L"Add Extra Component";
		AddExtraButton->Location	= TPoint( 200, 90 );
		AddExtraButton->EventClick	= WIDGET_EVENT(WScriptBuilder::ButtonAddExtraClick);
		AddExtraButton->SetSize( 25, 22 );

		RemoveExtraButton				= new WButton( ExtraPanel, InRoot );
		RemoveExtraButton->Caption		= L"<<";
		RemoveExtraButton->Tooltip		= L"Remove Extra Component";
		RemoveExtraButton->Location		= TPoint( 200, 120 );
		RemoveExtraButton->EventClick	= WIDGET_EVENT(WScriptBuilder::ButtonRemoveExtraClick);
		RemoveExtraButton->SetSize( 25, 22 );

		ExtraUsedList					= new WListBox( ExtraPanel, InRoot );
		ExtraUsedList->Location			= TPoint( 235, 10 );
		ExtraUsedList->ItemIndex		= -1;	
		ExtraUsedList->EventDblClick	= WIDGET_EVENT(WScriptBuilder::ButtonRemoveExtraClick);
		ExtraUsedList->EventChange		= WIDGET_EVENT(WScriptBuilder::ListUsedClick);
		ExtraUsedList->SetSize( 180, 200 );

		HasTextCheck			= new WCheckBox( this, InRoot );
		HasTextCheck->Caption	= L"Has Text?";
		HasTextCheck->Tooltip	= L"Whether Script Has Editable Text?";
		HasTextCheck->Location	= TPoint( 15, 372 );
		HasTextCheck->bChecked	= true;

		StaticCheck				= new WCheckBox( this, InRoot );
		StaticCheck->Caption	= L"Static Script?";
		StaticCheck->Tooltip	= L"Whether Static Script or Not?";
		StaticCheck->Location	= TPoint( 15, 391 );
		StaticCheck->bChecked	= false;
		StaticCheck->EventClick	= WIDGET_EVENT(WScriptBuilder::CheckStaticClick);

		OkButton				= new WButton( this, Root );
		OkButton->Caption		= L"Ok";
		OkButton->Location		= TPoint( 371, 378 );
		OkButton->EventClick	= WIDGET_EVENT(WScriptBuilder::ButtonOkClick);
		OkButton->SetSize( 64, 25 );

		ExtraNameEdit				= new WEdit( ExtraPanel, InRoot );
		ExtraNameEdit->Location		= TPoint( 235, 222 );
		ExtraNameEdit->EditType		= EDIT_String;
		ExtraNameEdit->EventChange	= WIDGET_EVENT(WScriptBuilder::EditExtraNameChange);
		ExtraNameEdit->SetSize( 180, 18 );

		ClassLabel				= new WLabel( ExtraPanel, InRoot );
		ClassLabel->Caption		= L"";
		ClassLabel->Location	= TPoint( 15, 223 );

		Hide();
	}
	void UpdateSingleCompFlags()
	{
		for( Int32 i=0; i<ExtraList->Items.size(); i++ )
			ExtraList->Items[i].bEnabled = true;

		for( Int32 iUsed=0; iUsed<ExtraUsedList->Items.size(); iUsed++ )
		{
			CClass* UsedClass = (CClass*)ExtraUsedList->Items[iUsed].Data;
			if( UsedClass->Flags & CLASS_SingleComp )
			{
				for( Int32 i=0; i<ExtraList->Items.size(); i++ )
					if( ExtraList->Items[i].Data == UsedClass )
					{
						ExtraList->Items[i].bEnabled = false;
						break;
					}
			}
		}

		if( ExtraList->ItemIndex != -1 && !ExtraList->Items[ExtraList->ItemIndex].bEnabled )
			ExtraList->SetItemIndex( -1, true );
	}


	// WForm interface.
	void Show( Int32 X = 0, Int32 Y = 0 )
	{
		WForm::Show( X, Y );

		// Reset everything.
		HasTextCheck->bChecked		= true;
		NameEdit->Text				= L"";
		BaseCombo->ItemIndex		= -1;
		ExtraList->ItemIndex		= -1;
		ExtraUsedList->Empty();
		ExtraNameEdit->Text			= L"";
		AddExtraButton->bEnabled	= false;
		RemoveExtraButton->bEnabled	= false;
		ExtraNameEdit->bEnabled		= false;
		StaticCheck->SetChecked( false, true );
		UpdateSingleCompFlags();
	}
	void OnClose()
	{
		WForm::OnClose();
		Hide();
	}

	// Controls notification.
	void ButtonRemoveExtraClick( WWidget* Sender )
	{
		if( ExtraUsedList->ItemIndex != -1 )
		{
			// Remove from the list extra component.
			ExtraUsedList->Remove( ExtraUsedList->ItemIndex );
			ExtraUsedList->SetItemIndex( -1, true );
			UpdateSingleCompFlags();
		}
	}
	void ButtonAddExtraClick( WWidget* Sender )
	{
		if( ExtraList->ItemIndex != -1 )
		{
			// Add a new extra component.
			CClass* ComClass = (CClass*)ExtraList->Items[ExtraList->ItemIndex].Data;

			// Make friendly name.
			assert(String::pos( L"Component", ComClass->GetAltName() ) != -1);
			ExtraUsedList->AddItem( String::copy( ComClass->GetAltName(), 0, ComClass->GetAltName().len()-9 ), ComClass );
			UpdateSingleCompFlags();
		}
	}
	void ListUsedClick( WWidget* Sender )
	{
		if( ExtraUsedList->ItemIndex != -1 )
		{
			ClassLabel->Caption	= ((CClass*)ExtraUsedList->Items[ExtraUsedList->ItemIndex].Data)->GetAltName() + L":";
			ExtraNameEdit->Text	= ExtraUsedList->Items[ExtraUsedList->ItemIndex].Name;
		}
		else
		{
			ExtraNameEdit->Text	= L"";
			ClassLabel->Caption	= L"";
		}
		RemoveExtraButton->bEnabled	= ExtraUsedList->ItemIndex != -1;
		ExtraNameEdit->bEnabled		= ExtraUsedList->ItemIndex != -1;
	}
	void EditExtraNameChange( WWidget* Sender )
	{
		if( ExtraUsedList->ItemIndex != -1 )
		{
			ExtraUsedList->Items[ExtraUsedList->ItemIndex].Name = ExtraNameEdit->Text;
		}
	}
	void ListExtraChange( WWidget* Sender )
	{
		AddExtraButton->bEnabled	= ExtraList->ItemIndex != -1;
	}
	void ButtonOkClick( WWidget* Sender )
	{
		if( NameEdit->Text.len() == 0 )
		{
			Root->ShowMessage( L"Please specify the name of script", L"Script Builder", true );
			return;
		}
		if( GObjectDatabase->FindObject( NameEdit->Text ) )
		{
			Root->ShowMessage
			(
				String::format( L"Object \"%s\" already exists", *NameEdit->Text ),
				L"Script Builder",
				true
			);
			return;
		}
		if( !StaticCheck->bChecked && BaseCombo->ItemIndex == -1 )
		{
			Root->ShowMessage( L"Please specify the base component", L"Script Builder", true );
			return;
		}
		if( !StaticCheck->bChecked )
		{
			for( Int32 i=0; i<ExtraUsedList->Items.size(); i++ )
			{
				CClass* Class = (CClass*)ExtraUsedList->Items[i].Data;
				String  Name  = ExtraUsedList->Items[i].Name;

				if( Name.len() == 0 )
				{
					Root->ShowMessage
					(
						String::format( L"Please specify the name of %d extra component", i ),
						L"Script Builder",
						true
					);
					return;
				}

				for( Int32 j=i+1; j<ExtraUsedList->Items.size(); j++ )
					if( ExtraUsedList->Items[j].Name == Name )
					{
						Root->ShowMessage
						(
							String::format( L"Component name '%s' redefined", *Name ),
							L"Script Builder",
							true
						);
						return;
					}
			}
		}

		// The last one paranoid validation.
		assert(StaticCheck->bChecked ? HasTextCheck->bChecked : true);

		// Create script.
		FScript* Script			= NewObject<FScript>( NameEdit->Text );
		Script->Group			= GroupEdit->Text;
		Script->StaticsBuffer	= new CInstanceBuffer(Script->Statics); 
		Script->ScriptFlags		|= HasTextCheck->bChecked ? SCRIPT_Scriptable : SCRIPT_None;
		Script->ScriptFlags		|= StaticCheck->bChecked ? SCRIPT_Static : SCRIPT_None;
		Script->FileName		= Script->IsScriptable() ? String::format( L"%s.flu", *Script->GetName() ) : L"";

		if( !Script->IsStatic() )
		{
			Script->InstanceBuffer	= Script->IsScriptable() ? new CInstanceBuffer(Script->Properties) : nullptr;

			// Base.
			CClass* BaseClass = (CClass*)BaseCombo->Items[BaseCombo->ItemIndex].Data;
			FBaseComponent* Base = NewObject<FBaseComponent>( BaseClass, L"Base", Script );
			Base->InitForScript( Script );

			// Extras.
			for( Int32 i=0; i<ExtraUsedList->Items.size(); i++ )
			{
				CClass*	ExtraClass	= (CClass*)ExtraUsedList->Items[i].Data;
				String	ExtraName	= ExtraUsedList->Items[i].Name;

				FExtraComponent* Extra	= NewObject<FExtraComponent>( ExtraClass, ExtraName, Script );
				Extra->InitForScript( Script );
			}
		}

		// If script has text - write some text to compile successfully.
		if( Script->IsScriptable() )
		{
			if( Script->IsStatic() )
			{
				// Default script for static script;
				Script->Text.push(String::format( L"/**" ));
				Script->Text.push(String::format( L" * static @%s: ...", *Script->GetName() ));
				Script->Text.push(String::format( L" * @Author: ..." ));
				Script->Text.push(String::format( L" */" ));
				Script->Text.push(String::format( L"static script %s", *Script->GetName() ));
				Script->Text.push(String::format( L"{" ));
				Script->Text.push(String::format( L" " ));
				Script->Text.push(String::format( L"}" ));
			}
			else
			{
				// Default script for regular script;
				Script->Text.push(String::format( L"/**" ));
				Script->Text.push(String::format( L" * @%s: ...", *Script->GetName() ));
				Script->Text.push(String::format( L" * @Author: ..." ));
				Script->Text.push(String::format( L" */" ));
				Script->Text.push(String::format( L"script %s", *Script->GetName() ));
				Script->Text.push(String::format( L"{" ));
				Script->Text.push(String::format( L" " ));
				Script->Text.push(String::format( L"}" ));
			}
		}

		Hide();
		Browser->Refresh();
		if( Script->IsScriptable() )
			GEditor->OpenPageWith( Script );
	}
	void CheckStaticClick( WWidget* Sender )
	{
		if( StaticCheck->bChecked )
		{
			// Reset values.
			BaseCombo->SetItemIndex( -1, true );
			ExtraUsedList->Empty();
			ExtraNameEdit->Text = L"";
			UpdateSingleCompFlags();

			// Any way, it should be turned on.
			HasTextCheck->bChecked = true;
		}

		// Turn off some widgets.
		ExtraNameEdit->bEnabled =
		RemoveExtraButton->bEnabled =
		AddExtraButton->bEnabled =
		ExtraUsedList->bEnabled =
		ExtraList->bEnabled =
		BaseLabel->bEnabled =
		HasTextCheck->bEnabled = 
		BaseCombo->bEnabled = !StaticCheck->bChecked;
	}

private:
	// Controls.
	WResourceBrowser*	Browser;
	WLabel*				NameLabel;
	WLabel*				BaseLabel;
	WLabel*				GroupLabel;
	WLabel*				ClassLabel;
	WPanel*				ExtraPanel;
	WCheckBox*			HasTextCheck;
	WCheckBox*			StaticCheck;
	WButton*			OkButton;
	WEdit*				NameEdit;
	WEdit*				GroupEdit;
	WComboBox*			BaseCombo;
	WListBox*			ExtraList;
	WListBox*			ExtraUsedList;
	WButton*			AddExtraButton;
	WButton*			RemoveExtraButton;
	WEdit*				ExtraNameEdit;
};


/*-----------------------------------------------------------------------------
	WAnimationBuilder implementation.
-----------------------------------------------------------------------------*/

//
// An animation builder dialog.
// 
class WAnimationBuilder: public WForm
{
public:
	// WAnimationBuilder interface.
	WAnimationBuilder( WResourceBrowser* InBrowser, WWindow* InRoot )
		:	WForm( InRoot, InRoot ),
			Browser( InBrowser )
	{
		// Initialize the form.
		Caption			= L"Animation Builder";
		bCanClose		= true;
		bSizeableH		= false;
		bSizeableW		= false;
		SetSize( 240, 123 );

		TopPanel			= new WPanel( this, Root );
		TopPanel->SetLocation( 8, 28 );
		TopPanel->SetSize( 224, 57 );		

		NameLabel			= new WLabel( TopPanel, Root );
		NameLabel->Caption	= L"Name:";
		NameLabel->SetLocation( 8, 8 );

		NameEdit			= new WEdit( TopPanel, Root );
		NameEdit->EditType	= EDIT_String;
		NameEdit->Location	= TPoint( 56, 7 );
		NameEdit->SetSize( 160, 18 );

		GroupLabel			= new WLabel( TopPanel, Root );
		GroupLabel->Caption	= L"Group:";
		GroupLabel->SetLocation( 8, 32 );

		GroupEdit			= new WEdit( TopPanel, Root );
		GroupEdit->EditType	= EDIT_String;
		GroupEdit->Location	= TPoint( 56, 31 );
		GroupEdit->SetSize( 160, 18 );

		OkButton				= new WButton( this, Root );
		OkButton->Caption		= L"Ok";
		OkButton->EventClick	= WIDGET_EVENT(WAnimationBuilder::ButtonOkClick);
		OkButton->SetLocation( 38, 90 );
		OkButton->SetSize( 64, 25 );

		CancelButton				= new WButton( this, Root );
		CancelButton->Caption		= L"Cancel";
		CancelButton->EventClick	= WIDGET_EVENT(WAnimationBuilder::ButtonCancelClick);
		CancelButton->SetLocation( 138, 90 );
		CancelButton->SetSize( 64, 25 );

		Hide();
	}

	// WForm interface.
	void OnClose()
	{
		WForm::OnClose();
		Hide();
	}
	void Show( Int32 X = 0, Int32 Y = 0 )
	{
		WForm::Show( X, Y );
		NameEdit->Text		= L"";
	}

	// Notifications.
	void ButtonCancelClick( WWidget* Sender )
	{
		Hide();
	}
	void ButtonOkClick( WWidget* Sender )
	{
		if( NameEdit->Text.len() == 0 )
		{
			Root->ShowMessage( L"Please specify the name of animation", L"Animation Builder", true );
			return;
		}
		if( GObjectDatabase->FindObject( NameEdit->Text ) )
		{
			Root->ShowMessage
			(
				String::format( L"Object \"%s\" already exists", *NameEdit->Text ),
				L"Rename Dialog",
				true
			);
			return;
		}

		FAnimation* Animation	= NewObject<FAnimation>( NameEdit->Text );
		Animation->Group		= GroupEdit->Text;

		Hide();
		Browser->Refresh();
		GEditor->OpenPageWith( Animation );
	}

private:
	// Controls.
	WResourceBrowser*	Browser;
	WLabel*				NameLabel;
	WLabel*				GroupLabel;
	WEdit*				NameEdit;
	WEdit*				GroupEdit;
	WPanel*				TopPanel;
	WButton*			OkButton;
	WButton*			CancelButton;
};


/*-----------------------------------------------------------------------------
	WSkeletonBuilder implementation.
-----------------------------------------------------------------------------*/

//
// An skeleton builder dialog.
// 
class WSkeletonBuilder: public WForm
{
public:
	// WSkeletonBuilder interface.
	WSkeletonBuilder( WResourceBrowser* InBrowser, WWindow* InRoot )
		:	WForm( InRoot, InRoot ),
			Browser( InBrowser )
	{
		// Initialize the form.
		Caption			= L"Skeleton Builder";
		bCanClose		= true;
		bSizeableH		= false;
		bSizeableW		= false;
		SetSize( 240, 123 );

		TopPanel			= new WPanel( this, Root );
		TopPanel->SetLocation( 8, 28 );
		TopPanel->SetSize( 224, 57 );		

		NameLabel			= new WLabel( TopPanel, Root );
		NameLabel->Caption	= L"Name:";
		NameLabel->SetLocation( 8, 8 );

		NameEdit			= new WEdit( TopPanel, Root );
		NameEdit->EditType	= EDIT_String;
		NameEdit->Location	= TPoint( 56, 7 );
		NameEdit->SetSize( 160, 18 );

		GroupLabel			= new WLabel( TopPanel, Root );
		GroupLabel->Caption	= L"Group:";
		GroupLabel->SetLocation( 8, 32 );

		GroupEdit			= new WEdit( TopPanel, Root );
		GroupEdit->EditType	= EDIT_String;
		GroupEdit->Location	= TPoint( 56, 31 );
		GroupEdit->SetSize( 160, 18 );

		OkButton				= new WButton( this, Root );
		OkButton->Caption		= L"Ok";
		OkButton->EventClick	= WIDGET_EVENT(WSkeletonBuilder::ButtonOkClick);
		OkButton->SetLocation( 38, 90 );
		OkButton->SetSize( 64, 25 );

		CancelButton				= new WButton( this, Root );
		CancelButton->Caption		= L"Cancel";
		CancelButton->EventClick	= WIDGET_EVENT(WSkeletonBuilder::ButtonCancelClick);
		CancelButton->SetLocation( 138, 90 );
		CancelButton->SetSize( 64, 25 );

		Hide();
	}

	// WForm interface.
	void OnClose()
	{
		WForm::OnClose();
		Hide();
	}
	void Show( Int32 X = 0, Int32 Y = 0 )
	{
		WForm::Show( X, Y );
		NameEdit->Text		= L"";
	}

	// Notifications.
	void ButtonCancelClick( WWidget* Sender )
	{
		Hide();
	}
	void ButtonOkClick( WWidget* Sender )
	{
		if( NameEdit->Text.len() == 0 )
		{
			Root->ShowMessage( L"Please specify the name of skeleton", L"Skeleton Builder", true );
			return;
		}
		if( GObjectDatabase->FindObject( NameEdit->Text ) )
		{
			Root->ShowMessage
			(
				String::format( L"Object \"%s\" already exists", *NameEdit->Text ),
				L"Rename Dialog",
				true
			);
			return;
		}

		FSkeleton* Skeleton = NewObject<FSkeleton>( NameEdit->Text );
		Skeleton->Group = GroupEdit->Text;

		Hide();
		Browser->Refresh();
		GEditor->OpenPageWith( Skeleton );
	}

private:
	// Controls.
	WResourceBrowser*	Browser;
	WLabel*				NameLabel;
	WLabel*				GroupLabel;
	WEdit*				NameEdit;
	WEdit*				GroupEdit;
	WPanel*				TopPanel;
	WButton*			OkButton;
	WButton*			CancelButton;
};


/*-----------------------------------------------------------------------------
	WMaterialBuilder implementation.
-----------------------------------------------------------------------------*/

//
// An material builder dialog.
// 
class WMaterialBuilder: public WForm
{
public:
	// WMaterialBuilder interface.
	WMaterialBuilder( WResourceBrowser* InBrowser, WWindow* InRoot )
		:	WForm( InRoot, InRoot ),
			Browser( InBrowser )
	{
		// Initialize the form.
		Caption			= L"Material Builder";
		bCanClose		= true;
		bSizeableH		= false;
		bSizeableW		= false;
		SetSize( 240, 123 );

		TopPanel			= new WPanel( this, Root );
		TopPanel->SetLocation( 8, 28 );
		TopPanel->SetSize( 224, 57 );		

		NameLabel			= new WLabel( TopPanel, Root );
		NameLabel->Caption	= L"Name:";
		NameLabel->SetLocation( 8, 8 );

		NameEdit			= new WEdit( TopPanel, Root );
		NameEdit->EditType	= EDIT_String;
		NameEdit->Location	= TPoint( 56, 7 );
		NameEdit->SetSize( 160, 18 );

		GroupLabel			= new WLabel( TopPanel, Root );
		GroupLabel->Caption	= L"Group:";
		GroupLabel->SetLocation( 8, 32 );

		GroupEdit			= new WEdit( TopPanel, Root );
		GroupEdit->EditType	= EDIT_String;
		GroupEdit->Location	= TPoint( 56, 31 );
		GroupEdit->SetSize( 160, 18 );

		OkButton				= new WButton( this, Root );
		OkButton->Caption		= L"Ok";
		OkButton->EventClick	= WIDGET_EVENT(WMaterialBuilder::ButtonOkClick);
		OkButton->SetLocation( 38, 90 );
		OkButton->SetSize( 64, 25 );

		CancelButton				= new WButton( this, Root );
		CancelButton->Caption		= L"Cancel";
		CancelButton->EventClick	= WIDGET_EVENT(WMaterialBuilder::ButtonCancelClick);
		CancelButton->SetLocation( 138, 90 );
		CancelButton->SetSize( 64, 25 );

		Hide();
	}

	// WForm interface.
	void OnClose()
	{
		WForm::OnClose();
		Hide();
	}
	void Show( Int32 X = 0, Int32 Y = 0 )
	{
		WForm::Show( X, Y );
		NameEdit->Text		= L"";
	}

	// Notifications.
	void ButtonCancelClick( WWidget* Sender )
	{
		Hide();
	}
	void ButtonOkClick( WWidget* Sender )
	{
		if( NameEdit->Text.len() == 0 )
		{
			Root->ShowMessage( L"Please specify the name of material", L"Material Builder", true );
			return;
		}
		if( GObjectDatabase->FindObject( NameEdit->Text ) )
		{
			Root->ShowMessage
			(
				String::format( L"Object \"%s\" already exists", *NameEdit->Text ),
				L"Rename Dialog",
				true
			);
			return;
		}

		FMaterial* Material = NewObject<FMaterial>(NameEdit->Text);
		Material->Group = GroupEdit->Text;

		Hide();
		Browser->Refresh();
		GEditor->OpenPageWith(Material);
	}

private:
	// Controls.
	WResourceBrowser*	Browser;
	WLabel*				NameLabel;
	WLabel*				GroupLabel;
	WEdit*				NameEdit;
	WEdit*				GroupEdit;
	WPanel*				TopPanel;
	WButton*			OkButton;
	WButton*			CancelButton;
};


/*-----------------------------------------------------------------------------
	WAssetsPage implementation.
-----------------------------------------------------------------------------*/

//
// Assets page constructor.
//
WAssetsPage::WAssetsPage( WResourceBrowser* InBrowser, WContainer* InOwner, WWindow* InRoot )
	:	WTabPage( InOwner, InRoot ),
		Browser( InBrowser ),
		ClassFilter( nullptr )
{
	// Initialize own variables.
	bCanClose		= false;
	Caption			= L"Assets";
	TabWidth		= 55;
	Color			= COLOR_ASSETS_PAGE;

	// Create controls.
	WPanel* HeaderPanel		= new WPanel( this, Root );
	HeaderPanel->bDrawEdges	= true;
	HeaderPanel->Align		= AL_Top;
	HeaderPanel->Padding	= TArea( 0, 3, 2, 2 );
	HeaderPanel->SetSize( 50, 47 );

	ImportButton				= new WPictureButton( HeaderPanel, Root );
	ImportButton->Tooltip		= L"Import Resource";
	ImportButton->Picture		= Root->Icons;
	ImportButton->Offset		= TPoint( 0, 96 );
	ImportButton->Scale			= TSize( 16, 16 );
	ImportButton->Location		= TPoint( 2, 2 );
	ImportButton->EventClick	= WIDGET_EVENT(WAssetsPage::ButtonImportClick);
	ImportButton->SetSize( 22, 22 );

	CreateButton				= new WPictureButton( HeaderPanel, Root );
	CreateButton->Tooltip		= L"Create Resource";
	CreateButton->Picture		= Root->Icons;
	CreateButton->Offset		= TPoint( 16, 96 );
	CreateButton->Scale			= TSize( 16, 16 );
	CreateButton->Location		= TPoint( 23, 2 );
	CreateButton->EventClick	= WIDGET_EVENT(WAssetsPage::ButtonCreateClick);
	CreateButton->SetSize( 22, 22 );

	ClassButton					= new WPictureButton( HeaderPanel, Root );
	ClassButton->Tooltip		= L"Type Filter";
	ClassButton->Picture		= Root->Icons;
	ClassButton->Offset			= TPoint( 32, 96 );
	ClassButton->Scale			= TSize( 16, 16 );
	ClassButton->Location		= TPoint( 44, 2 );
	ClassButton->EventClick		= WIDGET_EVENT(WAssetsPage::ButtonClassClick);
	ClassButton->SetSize( 22, 22 );

	RemoveButton				= new WPictureButton( HeaderPanel, Root );
	RemoveButton->Tooltip		= L"Remove Resource";
	RemoveButton->Picture		= Root->Icons;
	RemoveButton->Offset		= TPoint( 48, 96 );
	RemoveButton->Scale			= TSize( 16, 16 );
	RemoveButton->Location		= TPoint( 70, 2 );
	RemoveButton->EventClick	= WIDGET_EVENT(WAssetsPage::ButtonRemoveClick);
	RemoveButton->SetSize( 22, 22 );

	TileViewButton				= new WPictureButton( HeaderPanel, Root );
	TileViewButton->Tooltip		= L"Tile View";
	TileViewButton->Picture		= Root->Icons;
	TileViewButton->Offset		= TPoint( 80, 96 );
	TileViewButton->Scale		= TSize( 16, 16 );
	TileViewButton->EventClick	= WIDGET_EVENT(WAssetsPage::ButtonTileViewClick);
	TileViewButton->Align		= AL_Right;
	TileViewButton->Margin		= TArea( 2, 2, 0, 0 );
	TileViewButton->bDown		= true;
	TileViewButton->SetSize( 22, 22 );

	ListViewButton				= new WPictureButton( HeaderPanel, Root );
	ListViewButton->Tooltip		= L"List View";
	ListViewButton->Picture		= Root->Icons;
	ListViewButton->Offset		= TPoint( 64, 96 );
	ListViewButton->Scale		= TSize( 16, 16 );
	ListViewButton->EventClick	= WIDGET_EVENT(WAssetsPage::ButtonListViewClick);
	ListViewButton->Align		= AL_Right;
	ListViewButton->Margin		= TArea( 2, 2, 0, -1 );
	ListViewButton->SetSize( 22, 22 );

	// Name filter.
	NameFilter					= new WEdit( HeaderPanel, Root );
	NameFilter->EditType		= EDIT_String;
	NameFilter->Location		= TPoint( 2, 44 );
	NameFilter->EventChange		= WIDGET_EVENT(WAssetsPage::EditNameChange);
	NameFilter->Align			= AL_Bottom;
	NameFilter->SetSize( Size.Width-4, 18 );

	// Resource list.
	ResourceList				= new WResourceList( this, Root );
	ResourceList->Align			= AL_Client;
	ResourceList->Margin		= TArea( -1, 0, 0, 0 );
	ResourceList->bVisible		= false;

	// Resource pane.
	ResourcePane				= new WResourcePane( this, Root );
	ResourcePane->Align			= AL_Client;
	ResourcePane->Margin		= TArea( -1, 0, 0, 0 );

	// Per Resource popup.
	ResourcePopup				= new WPopupMenu( Root, Root );
	ResourcePopup->AddItem( L"Edit",		WIDGET_EVENT(WAssetsPage::PopEditClick) );
	ResourcePopup->AddItem( L"Rename...",	WIDGET_EVENT(WAssetsPage::PopRenameClick) );
	ResourcePopup->AddItem( L"" );
	ResourcePopup->AddItem( L"Remove",		WIDGET_EVENT(WAssetsPage::PopRemoveClick) );

	// New popup.
	NewPopup	= new WPopupMenu( Root, Root );
	NewPopup->AddItem( L"New Animation",		WIDGET_EVENT(WAssetsPage::PopNewAnimationClick) );
	NewPopup->AddItem( L"New Script",			WIDGET_EVENT(WAssetsPage::PopNewScriptClick) );
	NewPopup->AddItem( L"New Demoscene Effect", WIDGET_EVENT(WAssetsPage::PopNewEffectClick) );
	NewPopup->AddItem( L"New Skeleton",			WIDGET_EVENT(WAssetsPage::PopNewSkeletonClick) );
	NewPopup->AddItem( L"New Material",			WIDGET_EVENT(WAssetsPage::PopNewMaterialClick) );

	// Class popup.
	ClassPopup	= new WPopupMenu( Root, Root );
	ClassPopup->AddItem( L"All", WIDGET_EVENT(WAssetsPage::PopClassAllClick), true );
	ClassPopup->AddItem( L"" );
	for( Int32 i=0; i<CClassDatabase::GClasses.size(); i++ )
	{
		CClass* Class = CClassDatabase::GClasses[i];
		if	(	
				!(Class->Flags & CLASS_Abstract) &&
				Class->IsA(FResource::MetaClass) && 
				!Class->IsA(FLevel::MetaClass) &&
				!Class->IsA(FProjectInfo::MetaClass) &&
				(Class->Super == FResource::MetaClass || Class->Super == FTexture::MetaClass)
			)
		{
			ClassPopup->AddItem( Class->GetAltName(), WIDGET_EVENT(WAssetsPage::PopClassClick), true );
		}
	}
	for( Int32 i=0; i<ClassPopup->Items.size(); i++ )
		ClassPopup->Items[i].bChecked	= true;

	// Create dialogs.
	DemoEffectBuilder		= new WDemoEffectBuilder( Browser, Root );
	AnimationBuilder		= new WAnimationBuilder( Browser, Root );
	SkeletonBuilder			= new WSkeletonBuilder( Browser, Root );
	MaterialBuilder			= new WMaterialBuilder( Browser, Root );
	ScriptBuilder			= new WScriptBuilder( Browser, Root );

	ImportDialog			= new WImportDialog( Browser, Root );
	MusicPlayer				= new WMusicPlayer( Root, Root );
	FontViewDialog			= new WFontViewDialog( Root, Root );
	RenameDialog			= new WRenameDialog( Root );
}


//
// Assets page destructor.
//
WAssetsPage::~WAssetsPage()
{
	// Destroy not-really owned widgets.
	freeandnil(ResourcePopup);
	freeandnil(NewPopup);
	freeandnil(ClassPopup);

	// Delete dialogs.
	freeandnil(DemoEffectBuilder);
	freeandnil(AnimationBuilder);
	freeandnil(SkeletonBuilder);
	freeandnil(MaterialBuilder);
	freeandnil(ScriptBuilder);

	freeandnil(ImportDialog);
	freeandnil(MusicPlayer);
	freeandnil(FontViewDialog);
	freeandnil(RenameDialog);
}


//
// Assets page painting.
//
void WAssetsPage::OnPaint( CGUIRenderBase* Render )
{
	WTabPage::OnPaint(Render);

	// Turn on or turn off buttons.
	RemoveButton->bEnabled		= GProject != nullptr && Browser->Selected != nullptr;
	ImportButton->bEnabled		= GProject != nullptr;
	ClassButton->bEnabled		= GProject != nullptr;
	CreateButton->bEnabled		= GProject != nullptr;
}


//
// Refresh the assets page.
//
void WAssetsPage::Refresh()
{
	ResourceList->Refresh();
	ResourcePane->Refresh();
}


//
// Switch to list-view.
//
void WAssetsPage::ButtonListViewClick( WWidget* Sender )
{
	if( !ListViewButton->bDown )
	{
		ListViewButton->bDown = true;
		TileViewButton->bDown = false;

		ResourceList->bVisible = true;
		ResourcePane->bVisible = false;
		WidgetProc( WPE_Resize, TWidProcParms() );
	}
}


//
// Switch to tile-view.
//
void WAssetsPage::ButtonTileViewClick( WWidget* Sender )
{
	if( !TileViewButton->bDown )
	{
		ListViewButton->bDown = false;
		TileViewButton->bDown = true;

		ResourceList->bVisible = false;
		ResourcePane->bVisible = true;
		WidgetProc( WPE_Resize, TWidProcParms() );
	}
}


//
// Show resource context menu.
//
void WAssetsPage::ShowResourceMenu( FResource* Resource )
{
	// Popup it!
	if( Resource )
	{
		Browser->Selected = Resource;
		ResourcePopup->Show(Root->MousePos);
		ResourcePopup->Items[0].bEnabled	= !Resource->IsA(FSound::MetaClass) && !Resource->IsA(FFont::MetaClass);
	}
}


//
// Import resource click.
//
void WAssetsPage::ButtonImportClick( WWidget* Sender )
{
	ImportDialog->Show
	( 
		Root->Size.Width/3, 
		Root->Size.Height/3 
	);
}


//
// Popup 'Resource constructor'
//
void WAssetsPage::ButtonCreateClick( WWidget* Sender )
{
	NewPopup->Show( Root->MousePos );
}


//
// Popup 'Class filter'.
//
void WAssetsPage::ButtonClassClick( WWidget* Sender )
{
	ClassPopup->Show( Root->MousePos );
}


//
// Destroy selected resource.
//
void WAssetsPage::ButtonRemoveClick( WWidget* Sender )
{
	FResource* Res = Browser->GetSelected();
	if( !Res )
		return;

	if( !Res->IsA(FScript::MetaClass) )
	{
		// Some resource.
		Int32 S	= MessageBox
		(
			GEditor->hWnd,
			*String::format( L"Do you really want to destroy resource '%s'?", *Res->GetName() ),
			L"Resource Browser",
			MB_YESNO | MB_ICONQUESTION
		);

		if( S == IDYES )
		{
			// If its a font, also kill it bitmaps.
			FFont* Font = As<FFont>(Res);
			if( Font )
			{
				for( Int32 i=0; i<Font->Bitmaps.size(); i++ )
					if( Font->Bitmaps[i] )
						DestroyObject( Font->Bitmaps[i], true );
			}

			// If its a material, also kill it layers.
			FMaterial* Material = As<FMaterial>(Res);
			if( Material )
			{
				for( Int32 i=0; i<Material->Layers.size(); i++ )
					DestroyObject( Material->Layers[i], true );
			}

			// Destroy resource.
			DestroyObject( Res, true );
			Browser->Refresh();
		}
	}
	else
	{
		// A script resource.
		FScript* Script	= As<FScript>(Res);

		// Count entities being this script.
		Int32 NumEnts = 0;
		for( Int32 i=0; i<GProject->GObjects.size(); i++ )
			if( GProject->GObjects[i] && GProject->GObjects[i]->IsA(FEntity::MetaClass) )
			{
				FEntity* Entity	= As<FEntity>(GProject->GObjects[i]);
				if( Entity->Script == Script )
					NumEnts++;
			}

		Int32 S	= MessageBox
		(
			GEditor->hWnd,
			*String::format( L"Do you really want to destroy script '%s' and %d entities being it?", *Script->GetName(), NumEnts ),
			L"Resource Browser",
			MB_YESNO | MB_ICONQUESTION
		);

		if( S == IDYES )
		{
			// Release all refs to this script.
			GEditor->DropAllScripts();

			// Kill all entities.
			for( Int32 i=0; i<GProject->GObjects.size(); i++ )
				if( GProject->GObjects[i] && GProject->GObjects[i]->IsA(FEntity::MetaClass) )
				{
					FEntity* Entity	= As<FEntity>(GProject->GObjects[i]);
					if( Entity->Script == Script )
					{
						Entity->Level->Entities.removeUnique( Entity );
						DestroyObject( Entity, true );
					}
				}

			// Destroy script itself.
			DestroyObject( Script, true );
			Browser->Refresh();
		}
	}
}


//
// Open an effect builder.
//
void WAssetsPage::PopNewEffectClick( WWidget* Sender )
{
	DemoEffectBuilder->Show
	( 
		Root->Size.Width/3, 
		Root->Size.Height/3 
	);
}


//
// Open an animation builder.
//
void WAssetsPage::PopNewAnimationClick( WWidget* Sender )
{
	AnimationBuilder->Show
	( 
		Root->Size.Width/3, 
		Root->Size.Height/3 
	);
}


//
// Open a skeleton builder.
//
void WAssetsPage::PopNewSkeletonClick( WWidget* Sender )
{
	SkeletonBuilder->Show
	( 
		Root->Size.Width/3, 
		Root->Size.Height/3 
	);
}


//
// Open a material builder.
//
void WAssetsPage::PopNewMaterialClick( WWidget* Sender )
{
	MaterialBuilder->Show
	(
		Root->Size.Width/3, 
		Root->Size.Height/3 
	);
}


//
// Open a script builder.
//
void WAssetsPage::PopNewScriptClick( WWidget* Sender )
{
	ScriptBuilder->Show
	( 
		Root->Size.Width/3, 
		Root->Size.Height/3 
	);
}


//
// Popup 'Edit' clicked.
//
void WAssetsPage::PopEditClick( WWidget* Sender )
{
	if( Browser->Selected )
		GEditor->OpenPageWith( Browser->Selected );
}


//
// Popup 'Rename' clicked.
//
void WAssetsPage::PopRenameClick( WWidget* Sender )
{
	if( Browser->Selected )
		RenameDialog->SetResource( Browser->Selected );
}


//
// Popup 'Remove' clicked.
//
void WAssetsPage::PopRemoveClick( WWidget* Sender )
{
	ButtonRemoveClick( Sender );
}


// Hack, used to filter by class.
static Bool GUglyFilter[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

//
// On class filter checked.
//
void WAssetsPage::PopClassClick( WWidget* Sender )
{
	assert(arraySize(GUglyFilter) >= ClassPopup->Items.size());

	// 'Not all are now'.
	Int32 iChecked = 0;
	for( iChecked=1; iChecked<ClassPopup->Items.size(); iChecked++ )
		if( ClassPopup->Items[iChecked].bChecked != GUglyFilter[iChecked] )
			break;

	for( Int32 i=0; i<ClassPopup->Items.size(); i++ )
	{
		ClassPopup->Items[i].bChecked	= false;
		GUglyFilter[i]					= false;
	}

	ClassPopup->Items[iChecked].bChecked	= true;
	GUglyFilter[iChecked]					= true;

	String RealName = String(L"F") + ClassPopup->Items[iChecked].Text;
	ClassFilter = CClassDatabase::StaticFindClass( *RealName );

	// And refresh list.
	Refresh();
}


//
// On class filter 'All' checked.
//
void WAssetsPage::PopClassAllClick( WWidget* Sender )
{
	assert(arraySize(GUglyFilter) >= ClassPopup->Items.size());

	// If 'all' selected, mark everything.
	for( Int32 i=0; i<ClassPopup->Items.size(); i++ )
	{
		ClassPopup->Items[i].bChecked	= true;
		GUglyFilter[i]					= true;
	}

	ClassFilter	= nullptr;

	// Just refresh list.
	Refresh();
}


//
// Name filter has been changed.
//
void WAssetsPage::EditNameChange( WWidget* Sender )
{
	Refresh();
}


/*-----------------------------------------------------------------------------
	WResourceList implementation.
-----------------------------------------------------------------------------*/

//
// Resource list constructor.
//
WResourceList::WResourceList( WAssetsPage* InPage, WWindow* InRoot )
	:	WListBox( InPage, InRoot ),
		Page( InPage ),
		bReadyForDrag(false)
{
	// Own list variables.
	ItemsHeight	= 18;
}


//
// Resource list destructor.
//
WResourceList::~WResourceList()
{
}


//
// Mouse down on list.
//
void WResourceList::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y )
{
	if( Button == MB_Left )
	{
		// Left click on browser.
		WListBox::OnMouseDown( Button, X, Y );
		bReadyForDrag = ItemIndex != -1 && Page->Browser->Selected != nullptr;
	}
	else if( Button == MB_Right )
	{
		// Right click on resource browser.
		Int32 i = YToIndex(Y);
		SetItemIndex( i, true );

		if( ItemIndex != -1  )
			Page->ShowResourceMenu(Page->Browser->Selected);
		
		bReadyForDrag = false;
	}
}


//
// User hover mouse over list.
//
void WResourceList::OnMouseMove( EMouseButton Button, Int32 X, Int32 Y )
{
	WListBox::OnMouseMove( Button, X, Y );

	if( bReadyForDrag && Page->Browser->Selected )
	{
		// Begin drag resource.
		BeginDrag(Page->Browser->Selected);
	}

	bReadyForDrag = false;
}


//
// Mouse down on list.
//
void WResourceList::OnMouseUp( EMouseButton Button, Int32 X, Int32 Y )
{
	WListBox::OnMouseUp( Button, X, Y );
	bReadyForDrag = false;
}


//
// Dbl click on resource item.
//
void WResourceList::OnDoubleClick()
{
	WListBox::OnDoubleClick();
	bReadyForDrag = false;
	Page->Browser->ActivateResource(Page->Browser->Selected);
}


//
// When item changed.
//
void WResourceList::OnChange()
{
	if( ItemIndex != -1 )
	{
		FResource* Res = (FResource*)Items[ItemIndex].Data;
		assert(Res);
		Page->Browser->Selected = Res;
	}
}


//
// Refresh list of resources.
//
void WResourceList::Refresh()
{
	// No project.
	if( !GProject )
	{
		Empty();
		return;
	}

	// Precompute.
	String	NameFil = String::upperCase( Page->NameFilter->Text );
	Page->Browser->Selected = nullptr;

	// Build list of all matching resources.
	Empty();

	// For each object in project.
	for( Int32 i=0; i<GProject->GObjects.size(); i++ )
		if( GProject->GObjects[i] && GProject->GObjects[i]->IsA(FResource::MetaClass) )
		{
			FResource*	Res = (FResource*)GProject->GObjects[i];

			// Reject some resources.
			if( Res->IsA(FLevel::MetaClass) || Res->IsA(FProjectInfo::MetaClass) )
				continue;

			// Apply name filter.
			if( NameFil )
			{						
				if( String::pos(NameFil, String::upperCase(Res->GetName())) == -1 )
					continue;
			}

			// Class filter.
			if( Page->ClassFilter )
			{
				if( Page->ClassFilter->IsA(FFont::MetaClass) )
				{
					// Show fonts and it bitmaps.
					if( !(Res->IsA(FFont::MetaClass) || (Res->GetOwner() && Res->GetOwner()->IsA(FFont::MetaClass))) )
						continue;
				}
				else
				{
					// Not font filter.
					if( !Res->IsA(Page->ClassFilter) )
						continue;
				}
			}

			// Don't show folded resource, except font pages only.
			if( Res->GetOwner() && !(Page->ClassFilter && Page->ClassFilter->IsA(FFont::MetaClass)) )
				continue;

			// Retrive resource icon.
			TPoint IconPos = TPoint( 224, 176 );

			if( Res->IsA(FTexture::MetaClass) )
			{
				// Art resource.
				IconPos = TPoint( 208, 176 );
			}
			else if( Res->IsA(FSound::MetaClass) || Res->IsA(FMusic::MetaClass) )
			{
				IconPos = TPoint( 192, 176 );
			}
			else if( Res->IsA(FScript::MetaClass) )
			{
				// Script resource.
				IconPos = TPoint( 240, 176 );
			}

			// Add to list box.
			AddPictureItem( Res->GetName(), WWindow::Icons, IconPos, TSize(16, 16), Res );
		}

	// Alphabet sorting.
	AlphabetSort();
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/