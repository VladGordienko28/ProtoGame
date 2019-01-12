/*=============================================================================
    FrScriptPage.cpp: Script page.
    Copyright Aug.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    WFindDialog.
-----------------------------------------------------------------------------*/

//
// A Text find dialog.
//
class WFindDialog: public WForm
{
public:
	// Variables.
	WCodeEditor*		CodeEditor;
	WPanel*				Panel;
	WLabel*				FindLabel;
	WEdit*				TextEdit;
	WCheckBox*			CaseCheckBox;
	WButton*			CancelButton;
	WButton*			FindButton;

	// WFindDialog interface.
	WFindDialog( WCodeEditor* InCodeEditor, WContainer* InOwner, WWindow* InRoot )
		:	WForm( InOwner, InRoot ),
			CodeEditor( InCodeEditor )
	{
		// Initialize form.
		Caption						= L"Find";
		bCanClose					= true;
		bSizeableH					= false;
		bSizeableW					= false;
		SetSize( 250, 140 );		
		
		Panel						= new WPanel( this, InRoot );
		Panel->Location				= TPoint( 8, 28 );
		Panel->SetSize( 234, 70 );	

		FindLabel					= new WLabel( Panel, InRoot );
		FindLabel->Caption			= L"Find:";
		FindLabel->Location			= TPoint( 8, 6 );

		TextEdit					= new WEdit( Panel, InRoot );
		TextEdit->Location			= TPoint( 8, 25 );
		TextEdit->EditType			= EDIT_String;
		TextEdit->EventAccept		= WIDGET_EVENT(WFindDialog::ButtonFindClick);
		TextEdit->SetSize( 218, 18 );

		CaseCheckBox				= new WCheckBox( Panel, InRoot );
		CaseCheckBox->Caption		= L"Match Case?";
		CaseCheckBox->bChecked		= false;
		CaseCheckBox->Location		= TPoint( 15, 49 );

		CancelButton				= new WButton( this, Root );
		CancelButton->Caption		= L"Cancel";
		CancelButton->Location		= TPoint( 24, 106 );
		CancelButton->EventClick	= WIDGET_EVENT(WFindDialog::ButtonCancelClick);
		CancelButton->SetSize( 64, 25 );

		FindButton					= new WButton( this, Root );
		FindButton->Caption			= L"Find";
		FindButton->Location		= TPoint( 162, 106 );
		FindButton->EventClick		= WIDGET_EVENT(WFindDialog::ButtonFindClick);
		FindButton->SetSize( 64, 25 );

		Hide();
	}

	// WForm interface.
	void Show( Int32 X = 0, Int32 Y = 0 )
	{
		Location	= TPoint( X, Y );
		bVisible	= true;
		Root->SetFocused(TextEdit);
	}
	void OnClose()
	{
		WForm::OnClose();
		Hide();
	}
	void Hide()
	{
		WForm::Hide();
		Root->SetFocused(CodeEditor);
	}

	// Widgets notifications.
	void ButtonCancelClick( WWidget* Sender )
	{
		Hide();
	}
	void ButtonFindClick( WWidget* Sender );
};


/*-----------------------------------------------------------------------------
	WGotoLineDialog.
-----------------------------------------------------------------------------*/

//
// A goto line dialog.
//
class WGotoLineDialog: public WForm
{
public:
	// Variables.
	WCodeEditor*		CodeEditor;
	WPanel*				Panel;
	WLabel*				TopLabel;
	WSpinner*			LineSpinner;
	WButton*			CancelButton;
	WButton*			GoButton;

	// WGotoLineDialog interface.
	WGotoLineDialog( WCodeEditor* InCodeEditor, WContainer* InOwner, WWindow* InRoot )
		:	WForm( InOwner, InRoot ),
			CodeEditor( InCodeEditor )
	{
		// Initialize form.
		Caption						= L"Goto Line";
		bCanClose					= true;
		bSizeableH					= false;
		bSizeableW					= false;
		SetSize( 250, 122 );		
		
		Panel						= new WPanel( this, InRoot );
		Panel->Location				= TPoint( 8, 28 );
		Panel->SetSize( 234, 52 );	

		TopLabel					= new WLabel( Panel, InRoot );
		TopLabel->Caption			= L"Line Number (1 - 1):";
		TopLabel->Location			= TPoint( 8, 6 );

		LineSpinner					= new WSpinner( Panel, InRoot );
		LineSpinner->Location		= TPoint( 8, 25 );
		LineSpinner->EventAccept	= WIDGET_EVENT(WGotoLineDialog::ButtonGoClick);
		LineSpinner->SetSize( 218, 19 );
		LineSpinner->SetRange( 0, 1, 1 );

		CancelButton				= new WButton( this, Root );
		CancelButton->Caption		= L"Cancel";
		CancelButton->Location		= TPoint( 24, 88 );
		CancelButton->EventClick	= WIDGET_EVENT(WGotoLineDialog::ButtonCancelClick);
		CancelButton->SetSize( 64, 25 );

		GoButton					= new WButton( this, Root );
		GoButton->Caption			= L"Go";
		GoButton->Location			= TPoint( 162, 88 );
		GoButton->EventClick		= WIDGET_EVENT(WGotoLineDialog::ButtonGoClick);
		GoButton->SetSize( 64, 25 );

		Hide();
	}

	// WForm interface.
	void Show( Int32 X = 0, Int32 Y = 0 )
	{
		WForm::Show( X, Y );

		TopLabel->Caption = String::Format( L"Line Number (%i - %i):", 1, CodeEditor->Lines.Num() );
		LineSpinner->SetRange( 1, CodeEditor->Lines.Num(), 1 );
		LineSpinner->SetValue( CodeEditor->CaretYBegin+1, false );
	}
	void Hide()
	{
		WForm::Hide();
		Root->SetFocused(CodeEditor);
	}
	void OnClose()
	{
		WForm::OnClose();
		Hide();
	}

	// Widgets notifications.
	void ButtonCancelClick( WWidget* Sender )
	{
		Hide();
	}
	void ButtonGoClick( WWidget* Sender )
	{
		Int32 iLine = Clamp( LineSpinner->GetIntValue(), 1, CodeEditor->Lines.Num() )-1;
		CodeEditor->ScrollToLine(iLine);
		Hide();
	}
};


/*-----------------------------------------------------------------------------
    WAlterDialog.
-----------------------------------------------------------------------------*/

//
// Dialog to edit script's components.
//
class WAlterDialog: public WForm
{
public:
	// Variables.
	WScriptPage*		Page;
	WLabel*				BaseLabel;
	WLabel*				ClassLabel;
	WPanel*				ExtraPanel;
	WButton*			AlterButton;
	WComboBox*			BaseCombo;
	WListBox*			ExtraList;
	WListBox*			ExtraUsedList;
	WButton*			AddExtraButton;
	WButton*			RemoveExtraButton;
	WEdit*				ExtraNameEdit;

	// WAlterDialog interface.
	WAlterDialog( WScriptPage* InPage, WWindow* InRoot )
		:	WForm( InPage, InRoot ),
			Page( InPage )
	{
		// Initialize the form.
		Caption		= L"Components Alter";
		bCanClose	= true;
		bSizeableH	= false;
		bSizeableW	= false;
		SetSize( 445, 356 );

		BaseLabel				= new WLabel( this, InRoot );
		BaseLabel->Caption		= L"Base: ";
		BaseLabel->Location		= TPoint( 100, 30 );

		BaseCombo				= new WComboBox( this, InRoot );	
		BaseCombo->Location		= TPoint( 145, 29 );
		BaseCombo->SetSize( 160, 18 );
		for( Int32 i=0; i<CClassDatabase::GClasses.Num(); i++ )
		{
			CClass*	Class	= CClassDatabase::GClasses[i];
			if( Class->IsA(FBaseComponent::MetaClass) && !(Class->Flags & CLASS_Abstract) )
			{
				BaseCombo->AddItem( Class->GetAltName(), Class );
			}
		}
		BaseCombo->AlphabetSort();
		BaseCombo->ItemIndex	= -1;

		ExtraPanel				= new WPanel( this, InRoot );
		ExtraPanel->Location	= TPoint( 10, 59 );
		ExtraPanel->SetSize( 425, 256 );

		ExtraList				= new WListBox( ExtraPanel, InRoot );
		ExtraList->Location		= TPoint( 10, 10 );
		ExtraList->SetSize( 180, 200 );
		for( Int32 i=0; i<CClassDatabase::GClasses.Num(); i++ )
		{
			CClass*	Class	= CClassDatabase::GClasses[i];
			if( Class->IsA(FExtraComponent::MetaClass) && !(Class->Flags & CLASS_Abstract) )
				ExtraList->AddItem( Class->GetAltName(), Class );
		}
		ExtraList->AlphabetSort();
		ExtraList->ItemIndex		= -1;	
		ExtraList->EventChange		= WIDGET_EVENT(WAlterDialog::ListExtraChange);
		ExtraList->EventDblClick	= WIDGET_EVENT(WAlterDialog::ButtonAddExtraClick);

		AddExtraButton				= new WButton( ExtraPanel, InRoot );
		AddExtraButton->Caption		= L">>";
		AddExtraButton->Tooltip		= L"Add Extra Component";
		AddExtraButton->Location	= TPoint( 200, 90 );
		AddExtraButton->EventClick	= WIDGET_EVENT(WAlterDialog::ButtonAddExtraClick);
		AddExtraButton->SetSize( 25, 22 );

		RemoveExtraButton				= new WButton( ExtraPanel, InRoot );
		RemoveExtraButton->Caption		= L"<<";
		RemoveExtraButton->Tooltip		= L"Remove Extra Component";
		RemoveExtraButton->Location		= TPoint( 200, 120 );
		RemoveExtraButton->EventClick	= WIDGET_EVENT(WAlterDialog::ButtonRemoveExtraClick);
		RemoveExtraButton->SetSize( 25, 22 );

		ExtraUsedList					= new WListBox( ExtraPanel, InRoot );
		ExtraUsedList->Location			= TPoint( 235, 10 );
		ExtraUsedList->ItemIndex		= -1;	
		ExtraUsedList->EventDblClick	= WIDGET_EVENT(WAlterDialog::ButtonRemoveExtraClick);
		ExtraUsedList->EventChange		= WIDGET_EVENT(WAlterDialog::ListUsedClick);
		ExtraUsedList->SetSize( 180, 200 );

		AlterButton					= new WButton( this, Root );
		AlterButton->Caption		= L"Alter";
		AlterButton->Location		= TPoint( 371, 321 );
		AlterButton->EventClick		= WIDGET_EVENT(WAlterDialog::ButtonAlterClick);
		AlterButton->SetSize( 64, 25 );

		ExtraNameEdit				= new WEdit( ExtraPanel, InRoot );
		ExtraNameEdit->Location		= TPoint( 235, 222 );
		ExtraNameEdit->EditType		= EDIT_String;
		ExtraNameEdit->EventChange	= WIDGET_EVENT(WAlterDialog::EditExtraNameChange);
		ExtraNameEdit->SetSize( 180, 18 );

		ClassLabel				= new WLabel( ExtraPanel, InRoot );
		ClassLabel->Caption		= L"";
		ClassLabel->Location	= TPoint( 15, 223 );

		Hide();
	}
	void UpdateSingleCompFlags()
	{
		for( Int32 i=0; i<ExtraList->Items.Num(); i++ )
			ExtraList->Items[i].bEnabled = true;

		for( Int32 iUsed=0; iUsed<ExtraUsedList->Items.Num(); iUsed++ )
		{
			CClass* UsedClass = (CClass*)ExtraUsedList->Items[iUsed].Data;
			if( UsedClass->Flags & CLASS_SingleComp )
			{
				for( Int32 i=0; i<ExtraList->Items.Num(); i++ )
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
	void OnClose()
	{
		WForm::OnClose();
		Hide();
	}
	void Show( Int32 X = 0, Int32 Y = 0 )
	{
		WForm::Show( X, Y );

		// Base.
		for( Int32 i=0; i<BaseCombo->Items.Num(); i++ )
			if( Page->Script->Base->GetClass() == BaseCombo->Items[i].Data )
			{
				BaseCombo->ItemIndex	= i;
				break;
			}

		// Extra components.
		ExtraUsedList->Empty();
		for( Int32 e=0; e<Page->Script->Components.Num(); e++ )
			ExtraUsedList->AddItem
			(
				Page->Script->Components[e]->GetName(),
				Page->Script->Components[e]->GetClass()
			);

		// Misc.
		ExtraNameEdit->Text			= L"";
		AddExtraButton->bEnabled	= false;
		RemoveExtraButton->bEnabled	= false;
		ExtraNameEdit->bEnabled		= false;
		UpdateSingleCompFlags();
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
	void ListExtraChange( WWidget* Sender )
	{
		AddExtraButton->bEnabled	= ExtraList->ItemIndex != -1;	
	}
	void ButtonAddExtraClick( WWidget* Sender )
	{
		if( ExtraList->ItemIndex != -1 )
		{
			// Add a new extra component.
			CClass* ComClass = (CClass*)ExtraList->Items[ExtraList->ItemIndex].Data;

			// Make friendly name.
			assert(String::Pos( L"Component", ComClass->GetAltName() ) != -1);
			ExtraUsedList->AddItem( String::Copy( ComClass->GetAltName(), 0, ComClass->GetAltName().Len()-9 ), ComClass );
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
	void ButtonAlterClick( WWidget* Sender )
	{
		FScript* Script	= Page->Script;

		if( BaseCombo->ItemIndex == -1 )
		{
			Root->ShowMessage( L"Please specify the base component", L"Components Editor", true );
			return;
		}
		for( Int32 i=0; i<ExtraUsedList->Items.Num(); i++ )
		{
			CClass* Class = (CClass*)ExtraUsedList->Items[i].Data;
			String  Name  = ExtraUsedList->Items[i].Name;

			if( Name.Len() == 0 )
			{
				Root->ShowMessage
				(
					String::Format( L"Please specify the name of %d extra component", i ),
					L"Components Editor",
					true
				);
				return;
			}

			for( Int32 j=i+1; j<ExtraUsedList->Items.Num(); j++ )
				if( ExtraUsedList->Items[j].Name == Name )
				{
					Root->ShowMessage
					(
						String::Format( L"Component name '%s' redefined", *Name ),
						L"Components Editor",
						true
					);
					return;
				}
		}

		// Count entities being this script.
		Int32 NumEnts = 0;
		for( Int32 i=0; i<GProject->GObjects.Num(); i++ )
			if( GProject->GObjects[i] && GProject->GObjects[i]->IsA(FEntity::MetaClass) )
			{
				FEntity* Entity	= As<FEntity>(GProject->GObjects[i]);
				if( Entity->Script == Script )
					NumEnts++;
			}

			// Ask?
			Root->AskYesNo
			(
				String::Format( L"Do you really want to alter script '%s' and destroy %d entities being it?", *Script->GetName(), NumEnts ),
				L"Components Editor", 
				true, 
				WIDGET_EVENT(WAlterDialog::ButtonYesClick)
			);
	}
	void ButtonYesClick( WWidget* Sender )
	{
		Root->Modal->OnClose();

		// Release all refs to this script.
		GEditor->DropAllScripts();

		// Kill all entities.
		for( Int32 i=0; i<GProject->GObjects.Num(); i++ )
			if( GProject->GObjects[i] && GProject->GObjects[i]->IsA(FEntity::MetaClass) )
			{
				FEntity* Entity	= As<FEntity>(GProject->GObjects[i]);
				if( Entity->Script == Page->Script )
				{
					Entity->Level->Entities.RemoveUnique( Entity );
					DestroyObject( Entity, true );
				}
			}

		// Base.
		CClass* BaseClass	= (CClass*)BaseCombo->Items[BaseCombo->ItemIndex].Data;
		if( BaseClass != Page->Script->Base->GetClass() )
		{
			DestroyObject( Page->Script->Base, true );
			FBaseComponent* Base	= NewObject<FBaseComponent>( BaseClass, L"Base", Page->Script );
			Base->InitForScript(Page->Script);
		}

		// Extra components.
		for( Int32 e=0; e<Page->Script->Components.Num(); e++ )
		{
			FExtraComponent* Com = Page->Script->Components[e];
			Bool bFound = false;
			for( Int32 i=0; i<ExtraUsedList->Items.Num(); i++ )
				if( Com->GetName()==ExtraUsedList->Items[i].Name && Com->GetClass()==ExtraUsedList->Items[i].Data )
				{
					bFound	= true;
					break;
				}

			if( !bFound )
			{
				DestroyObject( Page->Script->Components[e], true );
				Page->Script->Components.RemoveUnique(nullptr);
				e--;
			}
		}
		for( Int32 i=0; i<ExtraUsedList->Items.Num(); i++ )
		{
			CClass*	ExtraClass	= (CClass*)ExtraUsedList->Items[i].Data;
			String	ExtraName	= ExtraUsedList->Items[i].Name;

			FComponent* Source = Page->Script->FindComponent(ExtraName);
			if( Source && Source->GetClass()==ExtraClass )
				continue;

			FExtraComponent* Extra	= NewObject<FExtraComponent>( ExtraClass, ExtraName, Page->Script );
			Extra->InitForScript( Page->Script );
		}

		Hide();
		GEditor->Inspector->SetEditObject(Page->Script);
	}
};


/*-----------------------------------------------------------------------------
    WScriptPage implementation.
-----------------------------------------------------------------------------*/

//
// Script page constructor.
//
WScriptPage::WScriptPage( FScript* InScript, WContainer* InOwner, WWindow* InRoot )
	:	WEditorPage( InOwner, InRoot ),
		Script( InScript )
{
	// Initialize page.
	Padding			= TArea( 0, 0, 0, 0 );
	PageType		= PAGE_Script;
	Script			= InScript;
	Caption			= InScript->GetName();
	TabWidth		= Root->Font1->TextWidth( *Caption ) + 35;
	Color			= PAGE_COLOR_SCRIPT;

	// Create toolbar and buttons.
	ToolBar = new WToolBar( this, Root );
	ToolBar->Align = AL_Top;
	ToolBar->SetSize( 3000, 28 );

	SaveButton				= new WPictureButton( ToolBar, Root );
	SaveButton->Scale		= TSize( 16, 16 );
	SaveButton->Offset		= TPoint( 32, 64 );
	SaveButton->Picture		= Root->Icons;
	SaveButton->Tooltip		= L"Save Script Text";
	SaveButton->EventClick	= WIDGET_EVENT(WScriptPage::ButtonSaveClick);
	SaveButton->SetSize( 22, 22 );
	ToolBar->AddElement( SaveButton );
	ToolBar->AddElement( nullptr );

	CompileAllButton				= new WPictureButton( ToolBar, Root );
	CompileAllButton->Scale			= TSize( 16, 16 );
	CompileAllButton->Offset		= TPoint( 64, 0 );
	CompileAllButton->Picture		= Root->Icons;
	CompileAllButton->Tooltip		= L"Compile All!";
	CompileAllButton->EventClick	= WIDGET_EVENT(WScriptPage::ButtonCompileAllClick);
	CompileAllButton->SetSize( 22, 22 );
	ToolBar->AddElement( CompileAllButton );
	ToolBar->AddElement( nullptr );

	FindDialogButton				= new WPictureButton( ToolBar, Root );
	FindDialogButton->Scale			= TSize( 16, 16 );
	FindDialogButton->Offset		= TPoint( 80, 0 );
	FindDialogButton->Picture		= Root->Icons;
	FindDialogButton->Tooltip		= L"Find Text";
	FindDialogButton->SetSize( 22, 22 );
	FindDialogButton->EventClick	= WIDGET_EVENT(WScriptPage::ButtonFindDialogClick);
	ToolBar->AddElement( FindDialogButton );
	ToolBar->AddElement( nullptr );

	AlterButton					= new WPictureButton( ToolBar, Root );
	AlterButton->Scale			= TSize( 16, 16 );
	AlterButton->Offset			= TPoint( 96, 0 );
	AlterButton->Picture		= Root->Icons;
	AlterButton->bEnabled		= !Script->IsStatic();
	AlterButton->Tooltip		= L"Show Script Alter";
	AlterButton->SetSize( 22, 22 );
	AlterButton->EventClick		= WIDGET_EVENT(WScriptPage::ButtonAlterClick);
	ToolBar->AddElement( AlterButton );

	// Separate panels using splitter.	
	WVSplitBox* SplitBox = new WVSplitBox( this, Root );
	SplitBox->TopMin = 200;
	SplitBox->BottomMin = 125;
	SplitBox->BottomMax = 400;
	SplitBox->Align = AL_Client;
	SplitBox->RatioRule = VRR_PreferBottom;
	
	// Code editor.
	CodeEditor			= new WCodeEditor( this, SplitBox, Root );
	CodeEditor->Margin	= TArea( 0, 1, 0, 0 );
	
	// Compiler messages output widget.
	Output			= new WCompilerOutput( SplitBox, Root );
	Output->AddMessage( L"--- FluScript 1.0. ---",	nullptr, -1,	COLOR_LightSteelBlue );
	Output->AddMessage( L"---",						nullptr, -1,	COLOR_LightSteelBlue );
	
	// Find dialog.
	FindDialog			= new WFindDialog( CodeEditor, this, Root );

	// Goto line dialog.
	GotoLineDialog		= new WGotoLineDialog( CodeEditor, this, Root );

	// Components view dialog.
	AlterDialog			= new WAlterDialog( this, Root );
}


//
// When page has been opened/reopened.
//
void WScriptPage::OnOpen()
{ 
	WEditorPage::OnOpen();

	// Let inspector show bitmap's properties.
	if( Script )
		GEditor->Inspector->SetEditObject( Script );   
}


//
// Tick the page.
//
void WScriptPage::TickPage( Float Delta )
{
	WEditorPage::TickPage(Delta);
}


//
// Redraw the page.
//
void WScriptPage::OnPaint( CGUIRenderBase* Render )
{
	WEditorPage::OnPaint(Render);
	
	// Draw flat-shade backdrop.
	Render->DrawRegion
					( 
						ClientToWindow(TPoint::Zero),
						Size,
						GUI_COLOR_PANEL,
						GUI_COLOR_PANEL,
						BPAT_Solid 
					);
					
	// Set buttons enable.
	SaveButton->bEnabled	= CodeEditor->bDirty;
}


//
// Ask for script saving before page close.
//
Bool WScriptPage::OnQueryClose()
{
	SaveScriptText( true );
	return true;
}


//
// Save script from the editor to
// the script.
//
void WScriptPage::SaveScriptText( Bool bAsk )
{
	if( Script && Script->IsScriptable() && CodeEditor->bDirty )
	{
		// Ask user.
		if	( bAsk && MessageBox
							( 
								GEditor->hWnd, 
								*String::Format( L"Save changes to script '%s'?", *Script->GetName() ), 
								L"Confirm", 
								MB_YESNO | MB_ICONASTERISK | MB_TASKMODAL 
							) == IDNO
			)
				return;

		// Save it.
		Script->Text.Empty();
		for( Int32 i=0; i<CodeEditor->Lines.Num(); i++ )
			Script->Text.Push(CodeEditor->Lines[i].Text);

		// Mark as not dirty.
		CodeEditor->SetDirty( false );

		// Notify.
		notice( L"Script: Script '%s' text is updated", *Script->GetName() );
	}
}


//
// Save script button clicked.
//
void WScriptPage::ButtonSaveClick( WWidget* Sender )
{
	if( Script->IsScriptable() )
		SaveScriptText( false );
}


//
// Open find dialog button clicked.
//
void WScriptPage::ButtonFindDialogClick( WWidget* Sender )
{
	FindDialog->Show( Size.Width/3, Size.Height/3 );
}


//
// Compile scripts button click.
//
void WScriptPage::ButtonCompileAllClick( WWidget* Sender )
{
	// Let editor compile all scripts.
	GEditor->CompileAllScripts(GEditor->TaskDialog);

	// Restore inspector's properties after compilation.
	GEditor->Inspector->SetEditObject( Script );   
}


//
// Open components editor.
//
void WScriptPage::ButtonAlterClick( WWidget* Sender )
{
	AlterDialog->Show( Size.Width/4, Size.Height/4 );
}


//
// Script page destructor.
//
WScriptPage::~WScriptPage()
{
}


//
// Highlight the error after script
// compilation failure.
//
void WScriptPage::HighlightError( Int32 iLine )
{
	CodeEditor->ScrollToLine( iLine );
	CodeEditor->bFlashy	= true;
}


//
// Perform Undo operation.
//
void WScriptPage::Undo()
{
	CodeEditor->Undo();
}


//
// Perform Redo operation.
//
void WScriptPage::Redo()
{
	CodeEditor->Redo();
}


/*-----------------------------------------------------------------------------
    WCodeEditor implementation.
-----------------------------------------------------------------------------*/

// Is should auto fill whitespace into new line?
#define NEW_LINE_WHITESPACE		1 

// Backspace tabs instead of spaces.
#define BACKSPACE_TAB			1

// Is should use autocomplete panel?
#define USE_AUTOCOMPLETE		1
#define	AUTO_KEYWORD			1


//
// Syntax highlight colors.
//
enum EHighlightType
{
	HIGH_Text,
	HIGH_Keyword,
	HIGH_Comment,
	HIGH_Quote,
	HIGH_Label,
	HIGH_Resource,
	HIGH_MAX
};


static const TColor GHightlight[HIGH_MAX] =
{
	TColor( 0xff, 0xff, 0xff, 0xff ),	//	HIGH_Text.
	TColor( 0x87, 0xce, 0xeb, 0xff ),	//	HIGH_Keyword.
	TColor( 0x00, 0xfa, 0x9a, 0xff ),	//	HIGH_Comment.
	TColor( 0xff, 0xa0, 0x7a, 0xff ),	//	HIGH_Quote.
	TColor( 0xda, 0x70, 0xd6, 0xff ),	//	HIGH_Label.
	TColor( 0xeb, 0xde, 0x87, 0xff )	//	HIGH_Resource.
};


//
// Code editor constructor.
//
WCodeEditor::WCodeEditor( WScriptPage* InPage, WContainer* InOwner, WWindow* InRoot )
	:	WContainer( InOwner, InRoot ),
		Page( InPage ),
		Script( InPage->Script ),
		Pool( 32*1024 ),
		bDirty( false ),
		bFlashy( false ),
		bDrag( false ),
		bReadyDrag( false ),
		ScrollTop( 0 ),
		Lines(),
		UndoTop(0),
		UndoStack(),
		bUndoLock(false),
		LastHighlightTime( GPlat->Now() ),
		LastTypeTime( GPlat->Now() ),
		AutoDialog(nullptr),
		CharSize( Root->Font2->TextWidth(L"A"), Root->Font2->Height )
{
	// Initialize own fields.
	Padding		= TArea( 0, 0, 0, 0 );

	// Create widgets.
	ScrollBar				= new WSlider( this, InRoot );
	ScrollBar->Align		= AL_Right;
	ScrollBar->EventChange	= WIDGET_EVENT(WCodeEditor::ScrollBarChange);
	ScrollBar->SetOrientation( SLIDER_Vertical );
	ScrollBar->SetSize( 12, 50 );

	PopUp					= new WPopupMenu( this, InRoot );
	PopUp->AddItem( L"Cut",		WIDGET_EVENT(WCodeEditor::PopCutClick) );
	PopUp->AddItem( L"Copy",	WIDGET_EVENT(WCodeEditor::PopCopyClick) );
	PopUp->AddItem( L"Paste",	WIDGET_EVENT(WCodeEditor::PopPasteClick) );

	// Setup editor.
	CaretXBegin	= CaretXEnd = CaretYBegin = CaretYEnd = 0;
	bDrag		= false;
	bReadyDrag	= false;

	// Copy text from buffer.
	if( Script->IsScriptable() )
	{
		// Script has an editable text.
		for( Int32 i=0; i<Script->Text.Num(); i++ )
		{
			TLine Line;
			Line.Text	= Script->Text[i];
			Line.First	= nullptr;
			Lines.Push( Line );
		}
	}
	else
	{
		// Script has no text, insert some message.
		TLine Line;
		Line.Text	= String::Format( L"// Script '%s' has no text.", *Script->GetName() );	
		Line.First	= nullptr;
		Lines.Push( Line );
	}

	// Highlight it!
	HighlightAll();
	HighlightBrackets(true);
}


//
// Code editor destructor.
//
WCodeEditor::~WCodeEditor()
{
	Lines.Empty();
	Pool.PopAll();

	// Destroy Undo/Redo stack.
	for( Int32 i=0; i<UndoStack.Num(); i++ )
		if( UndoStack[i] )
			mem::free(UndoStack[i]);
	UndoStack.Empty();
}


//
// Mark code as dirty or not, also mark page header
// with asterisk.
//
void WCodeEditor::SetDirty( Bool InbDirty )
{
	if( Script->IsScriptable() )
	{
		if( InbDirty && !bDirty )
		{
			// Mark text as dirty.
			Page->Caption	= String::Format( L"%s*", *Script->GetName() );
			bDirty			= true;
		}
		if( !InbDirty && bDirty )
		{
			// Mark text as not dirty.
			Page->Caption	= String::Format( L"%s", *Script->GetName() );
			bDirty			= false;
		}
	}
}


//
// When code editor turn on.
//
void WCodeEditor::OnActivate()
{
	WContainer::OnActivate();
}


//
// When code editor turn off.
//
void WCodeEditor::OnDeactivate()
{
	WContainer::OnDeactivate();
} 


//
// Code editor resized.
//
void WCodeEditor::OnResize()
{
	WContainer::OnResize();

	// Kill auto hint.
	freeandnil(AutoDialog);
}


//
// User unpress button.
//
void WCodeEditor::OnKeyUp( Int32 Key )
{
	WContainer::OnKeyUp( Key );

	// Set proper order of caret area, after
	// <Shift> selection.
	if( Key == 0x10 )
	{
		if( CaretYBegin == CaretYEnd )
		{
			if( CaretXBegin > CaretXEnd )
				Exchange( CaretXBegin, CaretXEnd );
		}
		else if( CaretYBegin > CaretYEnd )
		{
			Exchange( CaretYBegin, CaretYEnd );
			Exchange( CaretXBegin, CaretXEnd );
		}
	}
}


//
// Mouse up.
//
void WCodeEditor::OnMouseUp( EMouseButton Button, Int32 X, Int32 Y )
{
	// Call parent.
	WContainer::OnMouseUp( Button, X, Y );

	// User has chance to drag, but he just click.
	if( bReadyDrag && !bDrag )
		if( Button == MB_Left || Button == MB_Right )
		{
			CaretYBegin	= CaretYEnd	= YToLine( Y );
			CaretXBegin	= CaretXEnd	= XToColumn( X, CaretYBegin );
		}

	// Set proper order of caret area.
	if( CaretYBegin == CaretYEnd )
	{
		if( CaretXBegin > CaretXEnd )
			Exchange( CaretXBegin, CaretXEnd );
	}
	else if( CaretYBegin > CaretYEnd )
	{
		Exchange( CaretYBegin, CaretYEnd );
		Exchange( CaretXBegin, CaretXEnd );
	}

	// Drop text.
	if( bReadyDrag && bDrag && DragY >= 0 )
	{
		// Insert drag text.
		if( CaretXBegin!=CaretXEnd || CaretYBegin!=CaretYEnd )
		{
			BeginTransaction();
			{
				// Copy selected text to the buffer.
				TArray<String> Buffer;

				if( CaretYBegin == CaretYEnd )
				{
					// Copy piece of line.
					Int32 NumChars = CaretXEnd - CaretXBegin;
					Buffer.Push(String::Copy( Lines[CaretYBegin].Text, CaretXBegin, NumChars ));
				}
				else
				{
					// Copy a few lines.
					for( Int32 Y=CaretYBegin; Y<=CaretYEnd; Y++ )
						if( Y == CaretYEnd )
						{
							// Last line.
							Buffer.Push(String::Copy( Lines[Y].Text, 0, CaretXEnd ));
						}
						else if( Y == CaretYBegin )
						{
							// First line.
							Buffer.Push(String::Copy( Lines[Y].Text, CaretXBegin, Lines[Y].Text.Len()-CaretXBegin ));
						}
						else
						{
							// Middle line.
							Buffer.Push(Lines[Y].Text);
						}
				}

				// Clear selected.
				if( DragY > CaretYBegin )
				{
					DragY -= CaretYEnd-CaretYBegin;
				}
				ClearSelected();

				// Inert from the buffer.
				// Store rest of the line.
				String Rest			= String::Copy( Lines[DragY].Text, DragX, Lines[DragY].Text.Len()-DragX );
				Lines[DragY].Text	= String::Delete( Lines[DragY].Text, DragX, Lines[DragY].Text.Len()-DragX );

				CaretXBegin	= CaretXEnd = DragX;
				CaretYBegin	= CaretYEnd	= DragY;

				// Insert it.
				for( Int32 i=0; i<Buffer.Num(); i++ )
				{
					if( i == 0 )
					{
						Lines[CaretYBegin].Text += Buffer[0];
					}
					else
					{
						TLine Line;
						Line.First	= nullptr;
						Line.Text	= Buffer[i];
						Lines.Insert( CaretYBegin, 1 );
						CaretYBegin++;
						Lines[CaretYBegin] = Line;
					}
				}

				// Goto end of buffer.
				CaretYEnd	= CaretYBegin;
				CaretXEnd	= CaretXBegin	= Lines[CaretYBegin].Text.Len();

				// Insert the rest.
				Lines[CaretYBegin].Text += Rest;

				// Notify about change.
				OnChange();
			}
			EndTransaction();
		} 
	}

	// Reset drag processing.
	bReadyDrag	= false;
	bDrag		= false;

	// Show popup menu.
	if( Button == MB_Right )
	{
		PopUp->Items[0].bEnabled	=
		PopUp->Items[1].bEnabled	= CaretXBegin!=CaretXEnd || CaretYBegin!=CaretYEnd;

		PopUp->Show( TPoint( X, Y ) );
	}
}


//
// Mouse move code editor.
//
void WCodeEditor::OnMouseMove( EMouseButton Button, Int32 X, Int32 Y )
{
	WContainer::OnMouseMove( Button, X, Y );

	if( bReadyDrag )
	{
		// Yes, we just start drag.
		bDrag		= true;

		// Figure out drag target.
		DragY		= YToLine( Y );	
		DragX		= XToColumn( X, DragY );		

		// Reject it target are inside selection area.
		if( IsInSelection(X, Y) )
		{
			DragX	= -100;
			DragY	= -100;
		}
		if( CaretYBegin != CaretYEnd )
		{
			if( DragY == CaretYBegin && DragX>CaretXBegin )
				DragY = -100;
			else if( DragY == CaretYEnd && DragX<=CaretXEnd )
				DragY = -100;
			else if( DragY > CaretYBegin && DragY < CaretYEnd )
				DragY = -100;
		}
	}
	else if( Button == MB_Left )
	{
		// Move selection area.
		CaretYEnd	= YToLine( Y );
		CaretXEnd	= XToColumn( X, CaretYEnd );

		// Slowly scroll.
		if( Y < 0 )				ScrollTop = Max( 0, ScrollTop-1 );
		if( Y > Size.Height )	ScrollTop = Min( Lines.Num()-1, ScrollTop+1 );
		ScrollBar->Value = 100*ScrollTop / Max( Lines.Num()-1, 1 );
	}

	// Change cursor style.
	if( bDrag )
	{
		// Change cursor while drag.
		Cursor	= DragY < 0 ? CR_No : CR_Arrow;
	}
	else
	{
		// Selection mode.
		Cursor	= (IsInSelection( X, Y ) && ( CaretXEnd != CaretXBegin || CaretYBegin != CaretYEnd )) ? CR_Arrow : CR_IBeam;
	}
}


//
// Text has been just changed.
//
void WCodeEditor::OnChange()
{
	// Mark text as changed.
	SetDirty( true );

	// Undo error flashy.
	bFlashy	= false;

	// Re-highlight the syntax!
	HighlightAll();

	// Don't hide caret for couple seconds.
	LastTypeTime	= GPlat->Now();
}


//
// Key has been down.
//
void WCodeEditor::OnKeyDown( Int32 Key )
{
	WContainer::OnKeyDown( Key );

	// Don't let combo break auto-complete.
	if( Root->bCtrl )
		freeandnil(AutoDialog);

	// Handle shift-selection.
	if( Root->bShift )
	{
		if( Key == 0x25 )
		{
			// <Left> arrow.
			if( --CaretXEnd < 0 )
			{
				if( CaretYEnd != 0 )
				{
					CaretYEnd--;
					CaretXEnd	= Lines[CaretYEnd].Text.Len();
				}
				else
					CaretXEnd	= 0;
			}
		}
		else if( Key == 0x27 )
		{
			// <Right> arrow.
			if( ++CaretXEnd > Lines[CaretYEnd].Text.Len() )
			{
				if( CaretYEnd != Lines.Num()-1 )
				{
					CaretYEnd++;
					CaretXEnd	= 0;
				}
				else
					CaretXEnd	= Lines[CaretYEnd].Text.Len();
			}
		}
		else if( Key == 0x26 )
		{
			// <Up> arrow.
			CaretYEnd	= Max( 0, CaretYEnd-1 );
			CaretXEnd	= Min( CaretXEnd, Lines[CaretYEnd].Text.Len() );
		}
		else if( Key == 0x28 )
		{
			// <Down> arrow.
			CaretYEnd	= Min( CaretYEnd+1, Lines.Num()-1 );
			CaretXEnd = Min( CaretXEnd, Lines[CaretYEnd].Text.Len() );
		}

		ScrollToCaret();
		return;
	}

	// Regular state.
	if( Key == 0x25 )
	{
		// <Left> button.
		if( CaretXBegin==CaretXEnd && CaretYBegin==CaretYEnd )
		{
			CaretXBegin	-= 1;

			if( AutoDialog && CaretXBegin<AutoDialog->iX )
				freeandnil(AutoDialog);
			
			if( CaretXBegin < 0 )
			{
				if( CaretYBegin != 0 )
				{
					CaretYBegin--;
					CaretXBegin	= Lines[CaretYBegin].Text.Len();
				}
				else
					CaretXBegin	= 0;
			}
		}

		CaretYEnd	= CaretYBegin;
		CaretXEnd	= CaretXBegin;

		if( AutoDialog )
			AutoDialog->Filter();

		LastTypeTime	= GPlat->Now();
	}
	else if( Key == 0x27 )
	{
		// <Right> button.
		if( CaretXBegin==CaretXEnd && CaretYBegin==CaretYEnd )
		{
			CaretXBegin	+= 1;
			if( CaretXBegin > Lines[CaretYBegin].Text.Len() )
			{
				freeandnil(AutoDialog);

				if( CaretYBegin != Lines.Num()-1 )
				{
					CaretYBegin++;
					CaretXBegin	= 0;
				}
				else
					CaretXBegin	= Lines[CaretYBegin].Text.Len();
			}

			CaretYEnd	= CaretYBegin;
			CaretXEnd	= CaretXBegin;
		}
		else
		{
			CaretXBegin	= CaretXEnd;
			CaretYBegin	= CaretYEnd;
		}

		if( AutoDialog )
			AutoDialog->Filter();

		LastTypeTime	= GPlat->Now();
	}
	else if( Key == 0x26 )
	{
		// <Up> button.
		if( !AutoDialog )
		{
			CaretYEnd	= CaretYBegin = Max( 0, CaretYBegin-1 );
			CaretXEnd	= CaretXBegin = Min( CaretXBegin, Lines[CaretYBegin].Text.Len() );
		}
		else
			AutoDialog->OnKeyDown( Key );

		LastTypeTime	= GPlat->Now();
	}
	else if( Key == 0x28 )
	{
		// <Down> button.
		if( !AutoDialog )
		{
			CaretYBegin	= CaretYEnd	= Min( CaretYEnd+1, Lines.Num()-1 );
			CaretXBegin	= CaretXEnd = Min( CaretXEnd, Lines[CaretYBegin].Text.Len() );
		}
		else
			AutoDialog->OnKeyDown( Key );

		LastTypeTime	= GPlat->Now();
	}
	else if( Key == 0x2e )
	{
		// <Del> button.
		BeginTransaction();
		{
			if( CaretYBegin==CaretYEnd && CaretXBegin==CaretXEnd )
			{
				if( CaretXEnd == Lines[CaretYBegin].Text.Len() )
				{
					if( CaretYBegin != Lines.Num()-1 )
					{
						Lines[CaretYBegin].Text += Lines[CaretYBegin+1].Text;
						Lines.RemoveShift( CaretYBegin+1 );
					}
					freeandnil(AutoDialog);
				}
				else
					Lines[CaretYBegin].Text = String::Delete( Lines[CaretYBegin].Text, CaretXEnd, 1 );
			}
			else
			{
				ClearSelected();
			}

			OnChange();
		}
		EndTransaction();
	}
	else if( Root->bCtrl && Key == L'A' )
	{
		// <Ctrl> + <A>.
		SelectAll();
	}
	else if( Root->bCtrl && Key == L'C' )
	{
		// <Ctrl> + <C>.
		PopCopyClick( this );
	}
	else if( Root->bCtrl && Key == L'X' )
	{
		// <Ctrl> + <X>.
		PopCutClick( this );
	}
	else if( Root->bCtrl && Key == L'V' )
	{
		// <Ctrl> + <V>.
		PopPasteClick( this );
	}
	else if( Root->bCtrl && Root->bShift && Key == L'Z' )
	{
		// <Ctrl> + <Shift> + <Z>.
		Redo();
	}
	else if( Root->bCtrl && Key == L'Z' )
	{
		// <Ctrl> + <Z>.
		Undo();
	}
	else if( Root->bCtrl && Key == L'G' )
	{
		// <Ctrl> + <G>.
		Page->GotoLineDialog->Show(	Size.Width/3, Size.Height/3 );
	}
	else if( Root->bCtrl && Key == L'F' )
	{
		// <Ctrl> + <F>.
		Page->FindDialog->Show(	Size.Width/3, Size.Height/3 );
	}
	else if( Root->bCtrl && Key == L' ' )
	{
		// <Ctrl> + <Space>.
		if( !AutoDialog )
		{
			String	Line	= Lines[CaretYBegin].Text;
			Int32	iFrom	= CaretXBegin;
			while	( 
						iFrom > 0 && 
						(IsDigit(Line(iFrom-1)) || IsLetter(Line(iFrom-1))) 
					)
				iFrom--;

			CaretXEnd	= CaretXBegin;
			CaretYEnd	= CaretYBegin;

			AutoDialog				= new WAutoComplete( this, Root );
			AutoDialog->iX			= iFrom;
			AutoDialog->MoveToCaret();
			AutoDialog->FillBy( Script );
			AutoDialog->Filter();
		}
	}


	ScrollToCaret();
	HighlightBrackets();
}


//
// Select entire text.
//
void WCodeEditor::SelectAll()
{
	CaretYBegin	= 0;
	CaretYEnd	= Lines.Num()-1;
	CaretXBegin	= 0;
	CaretXEnd	= Lines[CaretYEnd].Text.Len();

	ScrollToCaret();
	HighlightBrackets();
}


//
// Paste text from clipboard.
// 
void WCodeEditor::PopPasteClick( WWidget* Sender )
{
	BeginTransaction();
	{
		// Prepare.
		ClearSelected();

		// Read text.
		String ClipText = GPlat->ClipboardPaste();
		if( ClipText )
		{
			// Store rest of the line.
			String Rest = String::Copy( Lines[CaretYBegin].Text, CaretXBegin, Lines[CaretYBegin].Text.Len()-CaretXBegin );
			Lines[CaretYBegin].Text	= String::Delete( Lines[CaretYBegin].Text, CaretXBegin, Lines[CaretYBegin].Text.Len()-CaretXBegin );

			// Insert it.
			for( Int32 i=0; i<ClipText.Len(); i++ )
			{
				Char C[2] = { ClipText(i), 0 };
				if( *C == '\r' )	continue;  // Windows feature.

				if( *C == '\t' ) Lines[CaretYBegin].Text += L"    ";	// Spaces instead tabulation.

				if( *C != '\n' )
				{
					// Regular symbol.
					Lines[CaretYBegin].Text += C;
				}
				else
				{
					// Insert new line and.
					TLine Line;
					Line.First	= nullptr;
					Line.Text = L"";

					Lines.Insert( CaretYBegin, 1 );
					CaretYBegin++;
					Lines[CaretYBegin] = Line;
				}
			}

			CaretYEnd	= CaretYBegin;
			CaretXEnd	= CaretXBegin	= Lines[CaretYBegin].Text.Len();

			// Insert the rest.
			Lines[CaretYBegin].Text += Rest;
			ScrollToCaret();
			HighlightBrackets();
			OnChange();
		}
	}
	EndTransaction();
}


//
// Cut selected to the clipboard.
//
void WCodeEditor::PopCutClick( WWidget* Sender )
{
	BeginTransaction();
	{
		PopCopyClick( Sender );
		ClearSelected();
		OnChange();
	}
	EndTransaction();
}


//
// Copy selected text to the clipboard.
//
void WCodeEditor::PopCopyClick( WWidget* Sender )
{
	// Don't copy empty selected area.
	if( CaretXBegin==CaretXEnd && CaretYBegin==CaretYEnd )
		return;

	if( CaretYBegin == CaretYEnd )
	{
		// Copy chunk of line, no \n symbols required.
		Int32 NumChars = CaretXEnd - CaretXBegin;
		Char* Text = (Char*)mem::alloc( (NumChars+1)*sizeof(Char) );
		mem::copy( Text, &Lines[CaretYBegin].Text[CaretXBegin], NumChars*sizeof(Char) );
		GPlat->ClipboardCopy( Text );
		mem::free( Text );
	}
	else
	{
		// Copy a few lines.
		Int32 NumChars = 0;

		// Count count, not exactly of course.
		for( Int32 Y=CaretYBegin; Y<=CaretYEnd; Y++ )
			NumChars += Lines[Y].Text.Len() + 4;

		Char* Text = (Char*)mem::alloc( (NumChars+1)*sizeof(Char) );

		// Copy text and insert /n symbol.
		Char* Walk = Text;
		for( Int32 Y=CaretYBegin; Y<=CaretYEnd; Y++ )
			if( Y == CaretYEnd )
			{
				mem::copy( Walk, &Lines[Y].Text[0], CaretXEnd*sizeof(Char) );
				Walk += CaretXEnd;
			}
			else if( Y == CaretYBegin )
			{
				mem::copy( Walk, &Lines[Y].Text[CaretXBegin], (Lines[Y].Text.Len()-CaretXBegin)*sizeof(Char) );
				Walk += Lines[Y].Text.Len()-CaretXBegin;
				*Walk = L'\n';
				Walk++;
			}
			else
			{
				mem::copy( Walk, &Lines[Y].Text[0], Lines[Y].Text.Len()*sizeof(Char) );
				Walk += Lines[Y].Text.Len();
				*Walk = L'\n';
				Walk++;
			}

		GPlat->ClipboardCopy( Text );
		mem::free( Text );
	}
}


//
// Scroll code editor to show caret.
//
void WCodeEditor::ScrollToCaret()
{
	Int32 NumVis	= Size.Height / CharSize.Height;

	// Scroll from the current location.
	while( CaretYEnd < ScrollTop )				ScrollTop--;
	while( CaretYEnd >= ScrollTop+NumVis )		ScrollTop++;

	// Update scroll bar.
	ScrollBar->Value	= 100*ScrollTop / Max( Lines.Num()-1, 1 );
}


//
// User type a char.
//
void WCodeEditor::OnCharType( Char TypedChar )
{
	WContainer::OnCharType( TypedChar );

	// Don't type character, if <Ctrl> used as part of command.
	if( Root->bCtrl )
		return;

	if( TypedChar == 0x08 )
	{
		// <Backspace>.
		BeginTransaction();
		{
			if( CaretYBegin==CaretYEnd && CaretXBegin==CaretXEnd )
			{
				if( CaretXBegin == 0 )
				{
					if( CaretYBegin != 0 )
					{
						CaretYBegin--;
						CaretXBegin	= Lines[CaretYBegin].Text.Len();
						Lines[CaretYBegin].Text += Lines[CaretYBegin+1].Text;
						Lines.RemoveShift( CaretYBegin+1 );
					}
				}
				else
				{
#if BACKSPACE_TAB
					Int32 Dest		= Max( 0, (4*(((CaretXBegin-1)/4)+1))-4 );
					Int32 NumEra		= CaretXBegin - Dest;

					// Test for spaces only.
					Bool	bSpacesOnly	= true;
					for( Int32 i=0; i<NumEra; i++ )
						if( Lines[CaretYBegin].Text(CaretXBegin-i-1) != ' ' )
						{
							bSpacesOnly	= false;
							break;
						}

					if( bSpacesOnly )
					{
						Lines[CaretYBegin].Text = String::Delete( Lines[CaretYBegin].Text, CaretXBegin-NumEra, NumEra );
						CaretXBegin	-= NumEra;
					}
					else
					{
						Lines[CaretYBegin].Text = String::Delete( Lines[CaretYBegin].Text, CaretXBegin-1, 1 );
						CaretXBegin--;
					}
#else
					Lines[CaretYBegin].Text = String::Delete( Lines[CaretYBegin].Text, CaretXBegin-1, 1 );
					CaretXBegin--;
#endif
					if( AutoDialog && CaretXBegin<AutoDialog->iX )
						freeandnil(AutoDialog);

					if( AutoDialog )
					{
						CaretXEnd	= CaretXBegin;
						AutoDialog->Filter();
					}
				}
			}
			else
			{
				ClearSelected();
			}

			CaretYEnd	= CaretYBegin;
			CaretXEnd	= CaretXBegin;
			HighlightBrackets();
		}
		EndTransaction();
	}
	else if( TypedChar == 0x0d )
	{
		// <Enter>.
		if( !AutoDialog )
		{
			BeginTransaction();
			{
				ClearSelected();

				TLine Line;
				Line.First	= nullptr;
				Line.Text	= String::Copy( Lines[CaretYBegin].Text, CaretXBegin, Lines[CaretYBegin].Text.Len()-CaretXBegin );

				Int32 LastLen = Lines[CaretYBegin].Text.Len();
				Lines[CaretYBegin].Text	= String::Delete( Lines[CaretYBegin].Text, CaretXBegin, Lines[CaretYBegin].Text.Len()-CaretXBegin );

				Lines.Insert( CaretYBegin, 1 );
				Lines[CaretYBegin+1] = Line;

#if NEW_LINE_WHITESPACE
				CaretYBegin = CaretYEnd	= CaretYBegin + 1;
				Int32 PrevLineWhite = 0;
				for( PrevLineWhite=0; PrevLineWhite<Lines[CaretYBegin-1].Text.Len(); PrevLineWhite++ )
					if( Lines[CaretYBegin-1].Text(PrevLineWhite) != L' ' )
						break;

				for( Int32 i=0; i<PrevLineWhite; i++ )
					Lines[CaretYBegin].Text += L" ";

				if( CaretXEnd < LastLen )
					CaretXBegin	= CaretXEnd	= 0;
				else
					CaretXBegin	= CaretXEnd	= Lines[CaretYBegin].Text.Len();
#else
				CaretXBegin	= CaretXEnd	= 0;
				CaretYBegin	= CaretYEnd	= CaretYBegin+1;
#endif
				HighlightBrackets();
			}
			EndTransaction();
		}
		else
		{
			AutoDialog->Accept();
		}
	}
	else if( TypedChar == 0x09 )
	{
		// <Tab>.
		ClearSelected();
		Int32 NumSpaces = (4*((CaretXBegin/4)+1))-CaretXBegin;
		Char tmp[8] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
		tmp[NumSpaces]	= '\0';

		Lines[CaretYBegin].Text = String::Copy
											( 
												Lines[CaretYBegin].Text, 
												0, 
												CaretXBegin 
											) 
						+ tmp + 
								  String::Copy
											( 
												Lines[CaretYBegin].Text, 
												CaretXBegin, 
												Lines[CaretYBegin].Text.Len()-CaretXBegin 
											);		

		CaretXBegin = CaretXEnd = CaretXBegin + NumSpaces;
		freeandnil(AutoDialog);
	}
	else if( TypedChar == 0x1b )
	{
		// <Esc> button.
		freeandnil(AutoDialog);
	}
	else
	{
		// Any character.
		// Store only if random.
		Bool	bUndo = RandomF() > 0.85f;

		if( bUndo )	BeginTransaction();
		{
			ClearSelected();

			// Append.
			Char tmp[2] = { TypedChar, '\0' };
			Lines[CaretYBegin].Text = String::Copy
												( 
													Lines[CaretYBegin].Text, 
													0, 
													CaretXBegin 
												) 
							+ tmp + 
									  String::Copy
												( 
													Lines[CaretYBegin].Text, 
													CaretXBegin, 
													Lines[CaretYBegin].Text.Len()-CaretXBegin 
												);		

			CaretXBegin = CaretXEnd = CaretXBegin + 1;

			// If we select something.
			if( AutoDialog && TypedChar==' ' )
				freeandnil(AutoDialog);
			if( AutoDialog )
				AutoDialog->Filter();
		}
		if( bUndo )	EndTransaction();
	}

	ScrollToCaret();
	HighlightBrackets(true);
	OnChange();

	// Maybe it's time for autocomplete?
#if USE_AUTOCOMPLETE
	if( !AutoDialog )
	{
		if( TypedChar == '$' )
		{
			// User type '$', show panel if it not a member
			// of other's entity.
			String ThisLine	= Lines[CaretYBegin].Text;
			Bool bDotDetected = false;
			for( Int32 i=CaretXBegin-2; i>=0; i-- )
				if( ThisLine(i) != ' ' )
				{
					bDotDetected	= ThisLine(i) == '.';
					break;
				}

			// Popup only if no dot found.
			if( !bDotDetected )
			{
				freeandnil(AutoDialog);
				AutoDialog				= new WAutoComplete( this, Root );
				AutoDialog->iX			= CaretXBegin;
				AutoDialog->MoveToCaret();
				AutoDialog->FillBy(Script->Components);
			}
		}
		else if( TypedChar == '.' )
		{
			// Extra component member.
			String ThisLine	= Lines[CaretYBegin].Text;
			Bool bDollarDetected = false;
			Bool bDotDetected = false;
			Int32 i, iDollar;

			for( i=CaretXBegin-2; i>=0; i-- )
				if( !(IsLetter(ThisLine(i))||IsDigit(ThisLine(i))||(ThisLine(i)==' ')) )
				{
					bDollarDetected	= ThisLine(i) == '$';
					iDollar			= i;
					break;
				}

			for( ; i>=0; i-- )
				if( ThisLine(i) != ' ' )
				{
					bDotDetected	= ThisLine(i) == '.';
					break;
				}		

			if( bDollarDetected && !bDotDetected )
			{
				Char ComName[128] = {}, *Walk = ComName;
				for( Int32 j=iDollar+1; j<ThisLine.Len(); j++ )
					if( IsLetter(ThisLine(j)) || IsDigit(ThisLine(j)) )
					{
						*Walk	= ThisLine(j);
						Walk++;
					}
					else
						break;

				FComponent* Extra = Script->FindComponent(ComName);
				if( Extra )
				{
					freeandnil(AutoDialog);
					AutoDialog				= new WAutoComplete( this, Root );
					AutoDialog->iX			= CaretXBegin;
					AutoDialog->MoveToCaret();
					AutoDialog->FillBy(Extra->GetClass());
				}
			}
		}
	}
#endif
}


//
// Mouse button down in the code editor.
//
void WCodeEditor::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y )
{
	WContainer::OnMouseDown( Button, X, Y );

	// Release auto-dialog.
	freeandnil(AutoDialog);

	// Ready for text drag?
	bDrag		=	false;
	bReadyDrag	=	Button == MB_Left && 
					IsInSelection( X, Y ) && 
					( CaretXEnd != CaretXBegin || CaretYBegin != CaretYEnd );

	if( !bReadyDrag )
		if( Button == MB_Left || (Button == MB_Right && !IsInSelection( X, Y )) )
		{
			CaretYBegin	= CaretYEnd	= YToLine( Y );
			CaretXBegin	= CaretXEnd	= XToColumn( X, CaretYBegin );
		}

	HighlightBrackets();
}


//
// Return true, if point(in pixels) are inside
// selection area.
//
Bool WCodeEditor::IsInSelection( Int32 X, Int32 Y )
{
	Int32 Y1, Y2, BeginX, EndX;

	// Sort caret coords.
	if( CaretYBegin <= CaretYEnd )
	{
		Y1		= CaretYBegin;
		Y2		= CaretYEnd;
		BeginX	= CaretXBegin;
		EndX	= CaretXEnd;
	}
	else
	{
		Y1		= CaretYEnd;
		Y2		= CaretYBegin;
		BeginX	= CaretXEnd;
		EndX	= CaretXBegin;
	}

	// Test line.
	Int32 Line = YToLine( Y );
	if( Line < Y1 || Line > Y2 )
		return false;

	// XToColumn(..), without clamp.
	Int32 Column	= Round((Float)(X-15) / (Float)CharSize.Width);  

	if( Line == Y1 )
	{
		// First line.
		return Column >= BeginX && Column <= Lines[Line].Text.Len();
	}
	else if( Line == Y2 )
	{
		// Last line.
		return Column >= 0 && Column <= EndX;
	}
	else
	{
		// Middle line.
		return Column >= 0 && Column <= Lines[Line].Text.Len();
	}
}


//
// Scroll text to show line iLine, used
// for example in error highlight.
//
void WCodeEditor::ScrollToLine( Int32 iLine )
{ 
	// Clamp it.
	iLine	= Clamp( iLine, 0, Lines.Num()-1 );

	// Set caret location.
	CaretYBegin	= CaretYEnd	= iLine;
	CaretXBegin	= CaretXEnd	= 0;
	
	// Set scroll.
	ScrollTop	= Max( 0, iLine-10 );	

	// Update scroll bar.
	ScrollBar->Value	= 100*ScrollTop / Max( Lines.Num()-1, 1 );
}


//
// Clear entire selected text.
//
void WCodeEditor::ClearSelected()
{
	if( CaretYBegin == CaretYEnd )
	{
		// Clean text in the line.
		if( CaretXBegin != CaretXEnd )
			Lines[CaretYBegin].Text	= String::Delete
												( 
													Lines[CaretYBegin].Text, 
													CaretXBegin, 
													CaretXEnd-CaretXBegin 
												);

		CaretXEnd	= CaretXBegin;
	}
	else
	{
		// Cleanup block of text.
		Lines[CaretYBegin].Text		= String::Delete
												( 
													Lines[CaretYBegin].Text, 
													CaretXBegin, 
													Lines[CaretYBegin].Text.Len()-CaretXBegin 
												);

		Lines[CaretYBegin].Text		+= String::Copy
												( 
													Lines[CaretYEnd].Text, 
													CaretXEnd, 
													Lines[CaretYEnd].Text.Len()-CaretXEnd  
												);

		for( Int32 i=CaretYBegin+1; i<=CaretYEnd; i++ )
			Lines.RemoveShift( CaretYBegin+1 );

		CaretXEnd	= CaretXBegin;
		CaretYEnd	= CaretYBegin;
	}

	ScrollToCaret();
}


//
// User just drop something.
//
void WCodeEditor::OnDragDrop( void* Data, Int32 X, Int32 Y )
{
	BeginTransaction();
	{
		// Get resource name.
		FResource* Res = (FResource*)Data;
		assert(Res);
		String Name = Res->IsA(FScript::MetaClass) ? 
								Res->GetName() : 
								String::Format( L"#%s", *Res->GetName() );

		// Insert resource name into line.
		Lines[DragY].Text =		String::Copy
										( 
											Lines[DragY].Text, 
											0, 
											DragX 
										) 

										+ Name +

								String::Copy
										( 
											Lines[DragY].Text, 
											DragX, 
											Lines[DragY].Text.Len()-DragX 
										);		

		// Move caret to drop location.
		CaretYBegin	= CaretYEnd	= DragY;
		CaretXBegin = CaretXEnd = DragX + Name.Len();

		// Switch mode.
		bDrag = false;
		OnChange();
	}
	EndTransaction();
}


//
// User drag something above.
//
void WCodeEditor::OnDragOver( void* Data, Int32 X, Int32 Y, Bool& bAccept )
{
	freeandnil(AutoDialog);
	bAccept	= false;

	// Allow to drop resources here.
	if( Data && ((FObject*)Data)->IsA(FResource::MetaClass) )
	{
		bDrag	= true;
		DragY	= YToLine( Y );
		DragX	= XToColumn( X, DragY );
		bAccept	= true;
	}
}


//
// Scroll text via left scroll bar.
//
void WCodeEditor::ScrollBarChange( WWidget* Sender )
{
	ScrollTop	= ScrollBar->Value * (Lines.Num()-1) / 100;
	ScrollTop	= Clamp( ScrollTop, 0, Lines.Num()-1 );
}


//
// Mouse scroll over code editor.
//
void WCodeEditor::OnMouseScroll( Int32 Delta )
{
	if( !AutoDialog )
	{
		// Scroll text in aspect 1:3.
		ScrollTop	-= Delta / 40;
		ScrollTop	= Clamp( ScrollTop, 0, Lines.Num()-1 );

		// Update scroll bar.
		ScrollBar->Value	= 100*ScrollTop / Max( Lines.Num()-1, 1 );
	}
	else
	{
		// Delegate this event to auto dialog.
		AutoDialog->OnMouseScroll( Delta );
	}
}


//
// Double click on code editor.
//
void WCodeEditor::OnDblClick( EMouseButton Button, Int32 X, Int32 Y )
{
	WContainer::OnDblClick( Button, X, Y );

	TLine& Line = Lines[CaretYBegin];

	// If at end, try to select just something.
	if( CaretXBegin >= Line.Text.Len() )
		CaretXBegin = Max( 0, CaretXBegin-1 );

	// Prepare for selection.
	CaretYEnd	= CaretYBegin;
	CaretXEnd	= CaretXBegin;
	Char C		= Line.Text(CaretXBegin);

	if( IsLetter(C) || IsDigit(C) )
	{
		// Select entire word or identifier.
		while	( 
					CaretXBegin > 0 && 
					(IsDigit(Line.Text(CaretXBegin-1)) || IsLetter(Line.Text(CaretXBegin-1))) 
				)
			CaretXBegin--;

		while	( 
					CaretXEnd < Line.Text.Len() && 
					(IsDigit(Line.Text(CaretXEnd)) || IsLetter(Line.Text(CaretXEnd))) 
				)
			CaretXEnd++;
	} 
	else if( C == ' ' )
	{
		// Select whitespace.
		while( CaretXBegin>0 && Line.Text(CaretXBegin-1)==' ' )
			CaretXBegin--;

		while( CaretXEnd<Line.Text.Len() && Line.Text(CaretXEnd)==' ' )
			CaretXEnd++;
	}
	else
	{
		// Select something else.
		if( CaretXEnd < Line.Text.Len() )
			CaretXEnd++;
	}

	// Reset drag mode.
	bReadyDrag	= false;
	bDrag		= false;
}   


//
// Convert column number to it X location.
//
Int32 WCodeEditor::ColumnToX( Int32 iColumn )
{
	return 15 + iColumn * CharSize.Width;
}


//
// Convert X location in line iLine to column number.
//
Int32 WCodeEditor::XToColumn( Int32 X, Int32 iLine )
{
	return Clamp
			( 
				Round((Float)(X-15) / (Float)CharSize.Width),		
				0, 
				Lines[iLine].Text.Len() 
			);	
}


//
// Convert line number to it Y location.
//
Int32 WCodeEditor::LineToY( Int32 iLine )
{
	return (iLine-ScrollTop) * CharSize.Height;
}


//
// Convert Y location to line number.
//
Int32 WCodeEditor::YToLine( Int32 Y )
{
	return Clamp
			(
				ScrollTop + Y / CharSize.Height,
				0,
				Lines.Num()-1
			);
}


//
// Highlight brackets right to caret.
//
void WCodeEditor::HighlightBrackets( Bool bUnmark )
{
	// Reset selection.
	EnclosingBrackets[0] = TPoint( -1, -1 );
	EnclosingBrackets[1] = TPoint( -1, -1 );

	// Unmark or area of selection.
	if( bUnmark || CaretXBegin != CaretXEnd || CaretYBegin != CaretYEnd )
		return;

	// See if caret is in text.
	Int32 X = CaretXBegin,
			Y = CaretYBegin;

	if( !InRange(Y, 0, Lines.Num()-1) || !InRange(X, 0, Lines[Y].Text.Len()-1) )
		return;

	const Char OpenBracks[] = L"([{";
	const Char CloseBracks[] = L")]}";

	// Read next litera.
	Char C = Lines[Y].Text[X];
	if( !wcschr(OpenBracks, C) && !wcschr(CloseBracks, C) )
		return;

	// Flip litera.
	Char D = '\0';
	switch( C )
	{
		case '(':	D = ')'; break;
		case '[':	D = ']'; break;
		case '{':	D = '}'; break;
		case ')':	D = '('; break;
		case ']':	D = '['; break;
		case '}':	D = '{'; break;
	}

	// Mark first bracket.
	EnclosingBrackets[0].X = X;
	EnclosingBrackets[0].Y = Y;

	// Find other bracket.
	if( wcschr(OpenBracks, C) )
	{
		// Look forward.
		Int32 NestLevel = 0;
		for( Y; Y<Lines.Num(); Y++ )
		{
			for( X; X<Lines[Y].Text.Len(); X++ )
			{
				Char ThisChar = Lines[Y].Text[X];
				if( ThisChar == C ) NestLevel++;
				if( ThisChar == D ) NestLevel--;

				if( ThisChar == D && NestLevel == 0 )
				{
					EnclosingBrackets[1].X = X;
					EnclosingBrackets[1].Y = Y;
					return;
				}
			}

			X = 0;
		}
	}
	else
	{
		// Look backward.
		Int32 NestLevel = 0;
		for( Y; Y>=0; Y-- )
		{
			for( X; X>=0; X-- )
			{
				Char ThisChar = Lines[Y].Text[X];
				if( ThisChar == C ) NestLevel++;
				if( ThisChar == D ) NestLevel--;

				if( ThisChar == D && NestLevel == 0 )
				{
					EnclosingBrackets[1].X = X;
					EnclosingBrackets[1].Y = Y;
					return;
				}
			}

			X = Y > 0 ? Lines[Y-1].Text.Len()-1 : 0;
		}
	}
}


/*-----------------------------------------------------------------------------
    Code editor drawing.
-----------------------------------------------------------------------------*/

//
// Paint the code editor.
//
void WCodeEditor::OnPaint( CGUIRenderBase* Render )
{
	// Call parent.
	WContainer::OnPaint( Render );

	// Turn on clipping.
	TPoint Base = ClientToWindow( TPoint::Zero );
	Render->SetClipArea( Base, TSize( Size.Width, Size.Height ) );

	// Visible lines bounds.
	Int32 iVisFirst	= ScrollTop;
	Int32 iVisLast	= Min( ScrollTop + Size.Height/CharSize.Height, Lines.Num()-1 );
	
	// Draw background.
	Render->DrawRegion
					(
						Base,
						Size,
						TColor( 0x1e, 0x1e, 0x1e, 0xff ),
						GUI_COLOR_SLIDER_BORDER,
						BPAT_Solid
					);
	
	// Draw a little left padding, maybe some day I'll 
	// got some brains and make breakpoints support.
	Render->DrawRegion
					(
						TPoint( Base.X+1, Base.Y+1 ),
						TSize( 15, Size.Height-2 ),
						TColor( 0x33, 0x33, 0x33, 0xff ),
						TColor( 0x33, 0x33, 0x33, 0xff ),
						BPAT_Solid
					);	

	// Text clipping.
	Render->SetClipArea
					( 
						TPoint( Base.X, Base.Y+1 ), 
						TSize( Size.Width, Size.Height-2 ) 
					);

	// Draw selection.
	if( CaretYBegin==CaretYEnd && CaretXBegin==CaretXEnd )
	{
		// Test bounds.
		if( CaretYBegin >= iVisFirst && CaretYBegin <= iVisLast )
		{
			// Simple '|' caret and highlight the line.
			Int32 TextY	= Base.Y + LineToY(CaretYBegin);

			// Highlight the current line.
			Render->DrawRegion
							(
								TPoint( Base.X + 16, TextY + 1 ),
								TSize( Size.Width - 28, CharSize.Height - 1 ),
								bFlashy ? COLOR_IndianRed : TColor( 0x0f, 0x0f, 0x0f, 0xff ),
								TColor( 0x40, 0x40, 0x40, 0xff ),
								BPAT_Solid
							);

			if( IsFocused() && (((Int32)(GPlat->Now()*2.f) & 1) || (GPlat->Now()-LastTypeTime)<1.5f) )
				Render->DrawText
							( 
								TPoint( Base.X + ColumnToX(CaretXBegin) - 2, TextY ), 
								L"|", 
								1,
								COLOR_White, 
								Root->Font2 
							);
		}
	}
	else
	{
		// Draw selection area.
		Int32 Y1, Y2, BeginX, EndX;
		TColor	DrawColor	= IsFocused() ? 
									TColor( 0x26, 0x4f, 0x78, 0xff ) : 
									TColor( 0x34, 0x34, 0x34, 0xff );
		
		// Get sorted bounds.
		if( CaretYBegin <= CaretYEnd )
		{
			Y1		= CaretYBegin;
			Y2		= CaretYEnd;
			BeginX	= CaretXBegin;
			EndX	= CaretXEnd;
		}
		else
		{
			Y1		= CaretYEnd;
			Y2		= CaretYBegin;
			BeginX	= CaretXEnd;
			EndX	= CaretXBegin;
		}

		for( Int32 Y=Max(Y1, iVisFirst); Y<=Min(Y2, iVisLast); Y++ )  
		{
			Int32 X1 = Y==Y1 ? BeginX : 0;
			Int32 X2 = Y==Y2 ? EndX : Lines[Y].Text.Len();		

			Render->DrawRegion
							( 
								TPoint( Base.X + ColumnToX(X1) + 2, Base.Y + LineToY(Y) ),
								TSize( CharSize.Width * (X2-X1) + 2, CharSize.Height ),
								DrawColor,
								DrawColor,
								BPAT_Solid
							);
		}
	}
	
	// Draw drag target.
	if( bDrag && DragY >= 0 )
	{
		Render->DrawText
					( 
						TPoint( Base.X + ColumnToX(DragX) - 2, Base.Y + LineToY(DragY) ), 
						L"|", 
						1,
						COLOR_White, 
						Root->Font2 
					);
	}
	
	// Draw text limit.
	Render->DrawRegion
	(
		TPoint( Base.X+CharSize.Width*120+16, Base.Y+1 ),
		TSize( 0, Size.Height-2 ),
		TColor( 0x40, 0x40, 0x40, 0xff ),
		TColor( 0x40, 0x40, 0x40, 0xff ),
		BPAT_None
	);	

	// Highlight brackets.
	if( EnclosingBrackets[0].Y != -1 && EnclosingBrackets[1].Y != -1 )
	{
		const TColor BRACKET_COLOR = TColor( 0x0e, 0x3a, 0x6c, 0xff );
		
		for( Int32 i=0; i<arr_len(EnclosingBrackets); i++ )
			Render->DrawRegion
			(
				TPoint
				( 
					Base.X + EnclosingBrackets[i].X*CharSize.Width + 17,
					Base.Y + (EnclosingBrackets[i].Y-ScrollTop)*CharSize.Height
				),
				CharSize, 
				BRACKET_COLOR,
				BRACKET_COLOR,
				BPAT_Solid
			);
	}

	// Draw highlighted text.
	for( Int32 iLine=iVisFirst; iLine<=iVisLast; iLine++ )	
	{
		// Prepare.
		TLine& Line			= Lines[iLine];
		Int32 Len			= Line.Text.Len();
		Int32 RndChars	= 0;

		// Walk through the chain of spans and render colored
		// substrings.
		for( TSpan* S=Line.First; S; S=S->Next )
		{
			// Draw substring if span fit.
			if( RndChars < Line.Text.Len() )
				Render->DrawText
							( 
								TPoint(	Base.X + RndChars*CharSize.Width + 17, 
										Base.Y + (iLine-ScrollTop)*CharSize.Height ),
								&Line.Text[RndChars],
								Min( S->Length, Line.Text.Len()-RndChars ),
								GHightlight[S->Type],
								Root->Font2
							);

			// Walk to next.
			RndChars += S->Length;			
		}

		// Render remain piece of string, in case it
		// outside of the span.
		if( RndChars < Len )
		{
			Render->DrawText
						( 
							TPoint(	Base.X + RndChars*CharSize.Width + 17, 
									Base.Y + (iLine-ScrollTop)*CharSize.Height ),
							&Line.Text[RndChars],
							Len - RndChars,
							GHightlight[HIGH_Text],
							Root->Font2
						);
		}
	}

	// Highlight text each second.
	Float Now = GPlat->Now();
	if( (Now - LastHighlightTime) > 4.5f )
	{
		HighlightAll();
		LastHighlightTime	= Now;
	}

	// Output information into status bar.
	if( !(GFrameStamp & 7) )
	{
		GEditor->StatusBar->Panels[0].Text	= String::Format( L"Ln %d", CaretYEnd+1 );
		GEditor->StatusBar->Panels[1].Text	= String::Format( L"Col %d", CaretXEnd+1 );
	}
}


/*-----------------------------------------------------------------------------
	Text searching.
-----------------------------------------------------------------------------*/

//
// Tries to find a text in the line of text start from
// iStart location, if text not found return -1,
// otherwise return start position of text.
//
Int32 FindInLine( const String& Needle, const String& HayStack, Int32 iStart )
{
	const Char* Gotten = wcsstr( &(*HayStack)[Clamp(iStart, 0, HayStack.Len())], *Needle );
	return Gotten ? ((Int32)Gotten - (Int32)*HayStack)/sizeof(Char) : -1;
}


//
// Find the next word in the text and highlight it.
// Return false if no text found.
//
Bool WCodeEditor::FindText( String S, Bool bMatchCase )
{
	// Collapse selection.
	CaretXBegin	= CaretXEnd;
	CaretYBegin	= CaretYEnd;

	// Iterate through all lines.
	for( Int32 i=0, iLine=CaretYEnd, X=CaretXBegin; i<=Lines.Num(); i++, iLine=(iLine+1)%Lines.Num(), X=0 )
	{
		Int32 iFound = bMatchCase ? 
							FindInLine( S, Lines[iLine].Text, X ) :
							FindInLine( String::UpperCase(S), String::UpperCase(Lines[iLine].Text), X );

		if( iFound != -1 )
		{
			// Word found! Highlight it.
			CaretXBegin	= iFound;
			CaretYBegin	= iLine;
			CaretXEnd	= iFound + S.Len();
			CaretYEnd	= iLine;

			// Scroll to show result.
			ScrollToCaret();

			// Notify.
			return true;
		}
	}

	// Nothing found..
	return false;
}


/*-----------------------------------------------------------------------------
    Code Editor Undo/Redo implementation.
-----------------------------------------------------------------------------*/

// Whether compress transactions?
#define UNDO_TEXT_COMPRESS		1


//
// Perform 'Undo' rollback operation.
//
void WCodeEditor::Undo()
{
	if( UndoTop > 0 )
	{
		UndoTop--;
		LoadFromUndoStack(UndoTop);
	}
}


//
// Perform 'Redo' rollback operation.
//
void WCodeEditor::Redo()
{
	if( UndoTop < (UndoStack.Num()-1) )
	{
		UndoTop++;
		LoadFromUndoStack(UndoTop);
	}
}


//
// Enter to undo/redo tracking section.
//
void WCodeEditor::BeginTransaction()
{
	assert(!bUndoLock);
	bUndoLock	= true;

	// Store first initial state.
	if( UndoTop == 0 )
	{
		UndoStack.SetNum(1);
		SaveToUndoStack(0);
		UndoTop	= 0;
	}
}


//
// Leave undo/redo tracking section.
//
void WCodeEditor::EndTransaction()
{
	assert(bUndoLock);
	bUndoLock	= false;

	// Destroy transactions after top.
	for( Int32 i=UndoTop+1; i<UndoStack.Num(); i++ )
		if( UndoStack[i] )
		{
			mem::free( UndoStack[i] );
			UndoStack[i]	= nullptr;
		}

	// Store current state.
	UndoStack.SetNum(UndoTop+2);
	if( UndoStack.Num() > HISTORY_LIMIT )
	{
		// Destroy first record and shift others.
		UndoStack.SetNum(HISTORY_LIMIT);
		mem::free(UndoStack[0]);

		mem::copy
		(
			&UndoStack[0],
			&UndoStack[1],
			(UndoStack.Num()-1)*sizeof(void*)
		);
	}
	UndoTop	= UndoStack.Num() - 1;

	// Create new one.
	UndoStack[UndoTop]	= nullptr;
	SaveToUndoStack(UndoTop);
}


//
// Store a text into iSlot transaction.
//
void WCodeEditor::SaveToUndoStack( Int32 iSlot )
{
	// Release old data.
	if( UndoStack[iSlot] )
	{
		mem::free(UndoStack[iSlot]);
		UndoStack[iSlot]	= nullptr;
	}
	
	// Count how mush required memory.
	UInt32 ReqMem = 32; // Stoke.
	for( Int32 iLine=0; iLine<Lines.Num(); iLine++ )
		ReqMem	+= Lines[iLine].Text.Len() * sizeof(Char);
	ReqMem	+= Lines.Num() * sizeof(Int32);

	// Write all required data into buffer.
	UInt8*	Buffer	= (UInt8*)mem::alloc(ReqMem);
	UInt8*	Walk	= Buffer;

	// Write editor stuff.
	*(Int32*)Walk	= Lines.Num();		Walk += sizeof(Int32);
	*(Int32*)Walk	= ScrollTop;		Walk += sizeof(Int32);
	*(Int32*)Walk	= CaretXBegin;		Walk += sizeof(Int32);
	*(Int32*)Walk	= CaretYBegin;		Walk += sizeof(Int32);

	// Write line by line.
	for( Int32 iLine=0; iLine<Lines.Num(); iLine++ )
	{
		TLine& Line	= Lines[iLine];

		*(Int32*)Walk	= Line.Text.Len();	Walk += sizeof(Int32);
		mem::copy( Walk, *Line.Text, Line.Text.Len()*sizeof(Char) );
		Walk	+= Line.Text.Len()*sizeof(Char);
	}

#if UNDO_TEXT_COMPRESS
	// Compress text.
	void*	Compressed;
	SizeT	ComSize;
	CLZWCompressor LZW;
	LZW.Encode
	(
		Buffer,
		ReqMem,
		Compressed,
		ComSize
	);

	// Dbg.
	//log( L"Script compression %i -> %i", ReqMem, ComSize );

	// Save to slot.
	UndoStack[iSlot]	= Compressed;
	mem::free(Buffer);
#else
	// Don't compress.
	UndoStack[iSlot]	= Buffer;
#endif
}


//
// Restore an text from byte sequence.
//
void WCodeEditor::LoadFromUndoStack( Int32 iSlot )
{
	assert(UndoStack[iSlot]);

	// Destroy old data.
	Lines.Empty();
	Pool.PopAll();

#if UNDO_TEXT_COMPRESS
	// Unpack compressed data.
	void*			Data;
	SizeT			DataSize;
	CLZWCompressor	LZW;
	LZW.Decode
	(
		UndoStack[iSlot],
		-1000,				// LZW, doesn't really need it.
		Data,
		DataSize
	);
#else
	// Not compressed.
	void*			Data;
	Data	= UndoStack[iSlot];
#endif

	// Prepare for walking.
	UInt8*	Walk	= (UInt8*)Data;

	// Read editor stuff.
	Lines.SetNum(*(Int32*)Walk);		Walk += sizeof(Int32);
	ScrollTop	= *(Int32*)Walk;		Walk += sizeof(Int32);
	CaretXBegin	= *(Int32*)Walk;		Walk += sizeof(Int32);
	CaretYBegin	= *(Int32*)Walk;		Walk += sizeof(Int32);

	// Read line by line.
	for( Int32 iLine=0; iLine<Lines.Num(); iLine++ )
	{
		TLine& Line	= Lines[iLine];
		Int32 Len	= *(Int32*)Walk;		
		Walk += sizeof(Int32);
		Line.Text	= String( (Char*)Walk, Len );
		Walk	+= Len * sizeof(Char);
	}

	// Update editor.
	HighlightAll();
	CaretXEnd			= CaretXBegin;
	CaretYEnd			= CaretYBegin;
	ScrollBar->Value	= 100*ScrollTop / Max( Lines.Num()-1, 1 );

#if UNDO_TEXT_COMPRESS
	// Free.
	mem::free( Data );
#endif
}


/*-----------------------------------------------------------------------------
    Syntax highlight.
-----------------------------------------------------------------------------*/

//
// Hash table of identifiers to highlight.
// Don't touch this mess.
//
struct TKeywordItem
{
public:
	Char*			Text;
	TKeywordItem*	Next;
	TKeywordItem( Char* InText )
		:	Text( InText ),
			Next( nullptr )
	{}
} *GKeywordHash[256] = {};
inline UInt8 HashKeyword( Char* InText )
{
	return 0xff & ( InText[0] ^ InText[1]*21 );
}
void InitKeywordHash()
{
	static Bool bInit = false;
	if( !bInit )
	{
#define KEYWORD( word )\
{ \
	static TKeywordItem Key( L#word ); \
	Int32 iHash = HashKeyword(Key.Text); \
	Key.Next = GKeywordHash[iHash];\
	GKeywordHash[iHash] = &Key;\
}\

#include "../Compiler/FrKeyword.h"
#undef KEYWORD
		bInit	= true;
	}
}


//
// Highlight the entire code.
// Computes spans for each line of the
// source text.
//
void WCodeEditor::HighlightAll()
{
	// Initialize the hash.
	InitKeywordHash();

	// Prepare.
	Bool bInComment = false;
	Pool.PopAll();

	// Cleanup old refs.
	for( Int32 iLine=0; iLine<Lines.Num(); iLine++ )
		Lines[iLine].First	= nullptr;

	// For each line.
	for( Int32 iLine=0; iLine<Lines.Num(); iLine++ )
	{
		TLine& Line = Lines[iLine];
		Char ThisChar = 0, PrevChar = 0;
		UInt8 Buffer[2048];

		// Don't highlight too long line.
		if( Line.Text.Len() > arr_len(Buffer) )
			continue;

		// For each symbol.
		for( Int32 i=0; i<Line.Text.Len();  )
		{
			ThisChar = Line.Text(i);

			if( bInComment )
			{
				// This text is inside multi-line comment.
				if( PrevChar=='*' && ThisChar=='/' )
				{
					// End of multiline comment detected.
					bInComment	= false;
				}
				Buffer[i]	= HIGH_Comment;
				i++;
			}
			else if( PrevChar=='/' && ThisChar=='*' )
			{
				// Start of multi-line comment.
				Buffer[i]		= HIGH_Comment;
				Buffer[i-1]		= HIGH_Comment;
				bInComment		= true;
				i++;
			}
			else if( PrevChar=='/' && ThisChar=='/' )
			{
				// Comment to the end of the line.
				Buffer[i]		= HIGH_Comment;
				Buffer[i-1]		= HIGH_Comment;
				i++;
				while( i<Line.Text.Len() )
					Buffer[i++] = HIGH_Comment;
				i++;
			}
			else if( ThisChar=='"' )
			{
				// Quote.
				Buffer[i++]	= HIGH_Quote;
				for( ; i<Line.Text.Len(); i++ )
				{
					ThisChar	 = Line.Text(i);
					Buffer[i]	 = HIGH_Quote;
					if( ThisChar=='"' )
						break;
				}
				i++;
			}
			else if( ThisChar=='@' )
			{
				// Thread label.
				Buffer[i++]	= HIGH_Label;
				for( ; i<Line.Text.Len(); i++ )
				{
					ThisChar = Line.Text(i);
					if( IsDigit(ThisChar) || IsLetter(ThisChar) )
						Buffer[i]	= HIGH_Label;
					else
						break;
				}
			}
			else if( ThisChar=='#' )
			{
				// Resource constant.
				Buffer[i++]	= HIGH_Resource;
				for( ; i<Line.Text.Len(); i++ )
				{
					ThisChar = Line.Text(i);
					if( IsDigit(ThisChar) || IsLetter(ThisChar) )
						Buffer[i]	= HIGH_Resource;
					else
						break;
				}
			}
			else if( IsLetter(ThisChar) )
			{
				// Parse the word, maybe its a keyword.
				Char Word[64] = {};
				Int32 iLetter = 0;

				for( ; i<Line.Text.Len() && iLetter<arr_len(Word); i++ )
				{
					ThisChar = Line.Text(i);

					if( IsLetter(ThisChar) )
						Word[iLetter++] = ThisChar;
					else
						break;
				}

				// Try to find just parsed word in the hash.
				UInt8 Type = HIGH_Text;
				for( TKeywordItem* K=GKeywordHash[HashKeyword(Word)]; K; K=K->Next )
					if( wcscmp( Word, K->Text ) == 0 )
					{
						Type = HIGH_Keyword;
						break;
					}

				// Set word type.
				for( Int32 j=0; j<iLetter; j++ )
					Buffer[i-j-1] = Type;
			}
			else
			{
				// Just a symbol.
				Buffer[i]	= HIGH_Text;
				i++;
			}

			PrevChar = Line.Text(i-1);
		}


		// Convert buffer to the linked list of the spans.
		TSpan** Dest = &Line.First;
		Line.First	= nullptr;

		for( Int32 i=0; i<Line.Text.Len();  )
		{
			// Maybe text too large to highlight.
			if( !Pool.CanPush(sizeof(TSpan)) )
				return;

			// Allocate new span and add to list.
			TSpan* Span			= Pool.PushT<TSpan>();		
			Span->Type			= Buffer[i]; 
			*Dest				= Span;
			Dest				= &Span->Next;

			// Compute the length of the span.
			while	(	
						i<Line.Text.Len() && 
						Buffer[i]==Span->Type 
					)
			{
				Span->Length++;
				i++;
			}
		}
	}	
}


/*-----------------------------------------------------------------------------
    WFindDialog implementation.
-----------------------------------------------------------------------------*/

//
// Button 'Find' has been clicked.
//
void WFindDialog::ButtonFindClick( WWidget* Sender )
{
	if( TextEdit->Text )
	{
		if( !CodeEditor->FindText( TextEdit->Text, CaseCheckBox->bChecked ) )
			Root->ShowMessage
			(
				*String::Format( L"Text '%s' not found", *TextEdit->Text ),
				L"Message",
				true
			);
	}
}


/*-----------------------------------------------------------------------------
    WAutoComplete implementation.
-----------------------------------------------------------------------------*/

//
// Autocomplete panel constructor.
//
WCodeEditor::WAutoComplete::WAutoComplete( WCodeEditor* InEditor, WWindow* InRoot )
	:	WListBox( InEditor, InRoot ),	
		Editor( InEditor ),
		iX( -1 )
{
	// Set own variables.
	SetSize( 360, 110 );
}


//
// Autocomplete panel destructor.
//
WCodeEditor::WAutoComplete::~WAutoComplete()
{
}


//
// Add a new entry to list.
//
void WCodeEditor::WAutoComplete::AddEntry( EAutoType InType, String InLabel, String InText )
{
	TEntry	E;
	E.Type	= InType;
	E.Label	= InLabel;
	E.Text	= InText;
	Entries.Push(E);

	// Add to list of items.
	this->AddItem( InText, (void*)(Entries.Num()-1) );
}


//
// Insert hint to code.
//
void WCodeEditor::WAutoComplete::Accept()
{
	if( ItemIndex != -1 )
	{
		assert(Editor->CaretYBegin == Editor->CaretYEnd);
		TEntry&	Entry		= Entries[(UInt32)(Items[ItemIndex].Data)];
		String& ThisLine	= Editor->Lines[Editor->CaretYBegin].Text;

		String Chunk		=	(Entry.Type!=AT_Method && Entry.Type!=AT_Function) ?
									Entry.Text :
									Entry.Text + L"()";

		Editor->BeginTransaction();
		{
			ThisLine	=	String::Delete( ThisLine, iX, Editor->CaretXEnd-iX  );
			ThisLine	=	String::Copy
									( 
										ThisLine, 
										0, 
										iX 
									)
					+ Chunk +
							String::Copy
									( 
										ThisLine, 
										iX, 
										ThisLine.Len()-iX 
									);	

			// Update caret location.
			if( Entry.Type!=AT_Method && Entry.Type!=AT_Function )
				Editor->CaretXBegin	= Editor->CaretXEnd	= iX+Chunk.Len();
			else
				Editor->CaretXBegin	= Editor->CaretXEnd	= iX+Chunk.Len()-1;
		}
		Editor->EndTransaction();
	}

	// Suicide.
	Editor->AutoDialog	= nullptr;
	delete this;
}


//
// Test string.
//
static Bool TestSubstring( String Needle, String HayStack )
{
	if( Needle.Len() > HayStack.Len() )
		return false;
	for( Int32 i=0; i<Needle.Len(); i++ )
		if( toupper(Needle(i)) != toupper(HayStack(i)) )
			return false;

	return true;
}


//
// Double click.
//
void WCodeEditor::WAutoComplete::OnDblClick( EMouseButton Button, Int32 X, Int32 Y )
{
	WListBox::OnDblClick( Button, X, Y );
	Accept();
}


//
// Key has been pressed on dialog.
//
void WCodeEditor::WAutoComplete::OnKeyDown( Int32 Key )
{
	WListBox::OnKeyDown( Key );

	// <Enter>
	if( Key == 0xd )
		Accept();
}


//
// Apply filter for searching.
//
void WCodeEditor::WAutoComplete::Filter()
{
	String ThisLine	= Editor->Lines[Editor->CaretYBegin].Text;
	if( iX>ThisLine.Len() || Editor->CaretXBegin<iX )
		return;

	String	Test	= String::Copy( ThisLine, iX, Editor->CaretXBegin-iX );

	// Refill database.
	this->Empty();
	for( Int32 e=0; e<Entries.Num(); e++ )
	{
		TEntry& Entry = Entries[e];

		if( TestSubstring( Test, Entry.Text ) )
			this->AddItem( Entry.Text, (void*)e );
	}
	this->AlphabetSort();
}


//
// Redraw auto dialog. Override standard dialog.
//
void WCodeEditor::WAutoComplete::OnPaint( CGUIRenderBase* Render )
{
	assert(Items.Num()<=Entries.Num());
	WList::OnPaint( Render );
	TPoint Base = ClientToWindow(TPoint::Zero);

	// Show or hide the slider.
	if( Items.Num() * ItemsHeight > Size.Height   )
	{
		Slider->bVisible = true;
	}
	else
	{
		Slider->bVisible = false;
		Slider->SetValue(0);
	}

	// Draw BG.
	Render->DrawRegion
	( 
		Base, Size,
		TColor( 0x30, 0x30, 0x30, 0xff ), 
		GUI_COLOR_LIST_BORDER,
		BPAT_Solid 
	);

	// Turn on clipping.
	Render->SetClipArea
	( 
		Base, 
		TSize( Size.Width-1, Size.Height ) 
	);

	// Draw items.
	if( Items.Num() > 0 )
	{
		Int32 TextY	= (ItemsHeight - Root->Font1->Height) / 2;

		// For each item.
		for( Int32 i = 0, iItem = YToIndex(0); 
			 i < Size.Height/ItemsHeight && iItem < Items.Num(); 
			 i++, iItem++ )
		{			
			TListItem& Item = Items[iItem];

			if( iHighlight == iItem )
				Render->DrawRegion
						( 
							TPoint( Base.X + 1, Base.Y+i * ItemsHeight + 1 ),
							TSize( Size.Width - 2, ItemsHeight ),
							TColor( 0x50, 0x50, 0x50, 0xff ),
							TColor( 0x50, 0x50, 0x50, 0xff ),
							BPAT_Solid 
						);

			if( ItemIndex == iItem )
				Render->DrawRegion
						( 
							TPoint( Base.X + 1, Base.Y+i * ItemsHeight + 1 ),
							TSize( Size.Width - 2, ItemsHeight ),
							GUI_COLOR_LIST_SEL,
							GUI_COLOR_LIST_SEL,
							BPAT_Solid 
						);

			// Draw items text and it type.
			static const TColor AutoColors[AT_MAX] =
			{
				TColor( 0x39, 0x39, 0xc1, 0xff ),	// AT_Property. 
				TColor( 0xc1, 0x39, 0x39, 0xff ),	// AT_Method.
				TColor( 0xc1, 0x70, 0x39, 0xff ),	// AT_Component.
				TColor( 0xc1, 0x39, 0x39, 0xff ),	// AT_Function. 
				TColor( 0x70, 0xc1, 0x39, 0xff ),	// AT_Script. 
				TColor( 0x70, 0xc1, 0x39, 0xff ),	// AT_Resource. 
				TColor( 0x39, 0x9f, 0x70, 0xff )	// AT_Keyword.
			};
			static const String AutoNames[AT_MAX] =
			{
				L"Property",	// AT_Property. 
				L"Method",		// AT_Method.
				L"Component",	// AT_Component.
				L"Function",	// AT_Function.
				L"Script",		// AT_Script.
				L"ResType",		// AT_Resource.
				L"Keyword"		// AT_Keyword.
			};

			TPoint	TextLoc = TPoint( Base.X + 3, Base.Y + i*ItemsHeight + 1 + TextY );		
			TEntry&	Entry		= Entries[(UInt32)Item.Data];

			Render->DrawText
			( 
				TextLoc, 
				AutoNames[Entry.Type], 
				AutoColors[Entry.Type], 
				Root->Font1 
			);
			Render->DrawText
			( 
				TPoint( TextLoc.X+75, TextLoc.Y ), 
				Entry.Label, 
				GUI_COLOR_TEXT, 
				Root->Font1 
			);
		}
	}
}


//
// Move dialog to caret location.
//
void WCodeEditor::WAutoComplete::MoveToCaret()
{
	Location.X	=	Editor->ColumnToX(Editor->CaretXEnd)+1;	
	Location.Y	=	Editor->LineToY(Editor->CaretYEnd)+Editor->CharSize.Height-1;

	// If dialog at bottom.
	if( Location.Y+Size.Height > Owner->Size.Height )
		Location.Y	=	Editor->LineToY(Editor->CaretYEnd)-Size.Height+2;		
}


//
// Fill autocomplete panel with list of components.
//
void WCodeEditor::WAutoComplete::FillBy( TArray<FExtraComponent*>& InArr )
{
	ItemIndex	= -1;
	Items.Empty();
	Entries.Empty();

	for( Int32 e=0; e<InArr.Num(); e++ )
		AddEntry
		(
			AT_Component,
			String(L"$")+InArr[e]->GetName(), 
			InArr[e]->GetName()
		);

	this->AlphabetSort();
}


//
// Fill autocomplete panel with properties and methods
// being class and all it supers.
//
void WCodeEditor::WAutoComplete::FillBy( CClass* Class )
{
	ItemIndex	= -1;
	Items.Empty();
	Entries.Empty();

	while( Class )
	{
		for( Int32 p=0; p<Class->Properties.Num(); p++ )
		{
			CProperty* Prop	= Class->Properties[p];
			AddEntry
			(
				AT_Property,
				String::Format( L"%s %s", *Prop->TypeName(), *Prop->Name ),
				Prop->Name
			);
		}
		for( Int32 m=0; m<Class->Methods.Num(); m++ )
		{
			CNativeFunction* Func	= Class->Methods[m];
			AddEntry
			(
				AT_Method,
				Func->GetSignature(),
				Func->Name
			);
		}

		Class	= Class->Super;
	}

	this->AlphabetSort();
}


//
// Fill by every thing that possible.
//
void WCodeEditor::WAutoComplete::FillBy( FScript* Script )
{
	ItemIndex	= -1;
	Items.Empty();
	Entries.Empty();

	// Native functions.
	for( Int32 f=0; f<CClassDatabase::GFuncs.Num(); f++ )
	{
		CNativeFunction* Func	= CClassDatabase::GFuncs[f];
		if( !(Func->Flags & (NFUN_UnaryOp|NFUN_BinaryOp|NFUN_SuffixOp|NFUN_AssignOp|NFUN_Method)) )
			AddEntry
			(
				AT_Function,
				Func->GetSignature(),
				Func->Name
			);
	}

	// Base properties and methods.
	if( !Script->IsStatic() )
	{
		CClass*	Class	= Script->Base->GetClass();
		while( Class )
		{
			for( Int32 p=0; p<Class->Properties.Num(); p++ )
			{
				CProperty* Prop	= Class->Properties[p];
				AddEntry
				(
					AT_Property,
					String::Format( L"%s %s", *Prop->TypeName(), *Prop->Name ),
					Prop->Name
				);
			}
			for( Int32 m=0; m<Class->Methods.Num(); m++ )
			{
				CNativeFunction* Func	= Class->Methods[m];
				AddEntry
				(
					AT_Method,
					Func->GetSignature(),
					Func->Name
				);
			}

			Class	= Class->Super;
		}
	}

	// Instance buffer.
	if( Script->IsScriptable() && Script->InstanceBuffer )
		for( Int32 p=0; p<Script->Properties.Num(); p++ )
		{
			CProperty* Prop	= Script->Properties[p];
			AddEntry
			(
				AT_Property,
				String::Format( L"%s %s", *Prop->TypeName(), *Prop->Name ),
				Prop->Name
			);
		}

	// Script functions.
	for( Int32 f=0; f<Script->Methods.Num(); f++ )
	{
		CFunction*	Func	= Script->Methods[f];
		AddEntry
		(
			AT_Method,
			Func->GetSignature(),
			Func->Name
		);
	}

	// Static properties.
	for( Int32 p=0; p<Script->Statics.Num(); p++ )
	{
		CProperty* Prop	= Script->Statics[p];
		AddEntry
		(
			AT_Property,
			String::Format( L"[s] %s %s", *Prop->TypeName(), *Prop->Name ),
			Prop->Name
		);
	}

	// Static functions.
	for( Int32 f=0; f<Script->StaticFunctions.Num(); f++ )
	{
		CFunction*	Func	= Script->StaticFunctions[f];
		AddEntry
		(
			AT_Method,
			Func->GetSignature(),
			String(L"[%s]") + Func->Name
		);
	}

	// Resource classes.
	for( Int32 c=0; c<CClassDatabase::GClasses.Num(); c++ )
	{
		CClass* C	= CClassDatabase::GClasses[c];
		if( C->IsA(FResource::MetaClass) && !(C->Flags & CLASS_Abstract) )
			AddEntry
			(
				AT_Resource,
				C->GetAltName(), C->GetAltName()
			);
	}

	// Scripts.
	for( Int32 i=0; i<GObjectDatabase->GObjects.Num(); i++ )
	{
		FObject* Obj	= GObjectDatabase->GObjects[i];
		if( Obj && Obj->IsA(FScript::MetaClass) )
			AddEntry
			(
				AT_Script,
				Obj->GetName(),
				Obj->GetName()
			);
	}

	// Available components.
	for( Int32 i=0; i<Script->Components.Num(); i++ )
	{
		FComponent* C	= Script->Components[i];
		AddEntry
		(
			AT_Component,
			String(L"$")+C->GetName(),
			String(L"$")+C->GetName()
		);
	}

	// All keywords.
#if AUTO_KEYWORD
#define KEYWORD(word)	AddEntry( AT_Keyword, L#word, L#word );
#include "../Compiler/FrKeyword.h"
#undef KEYWORD
#endif

	this->AlphabetSort();
}


/*-----------------------------------------------------------------------------
    WCompilerOutput implementation.
-----------------------------------------------------------------------------*/

//
// Compiler output constructor.
//
WCompilerOutput::WCompilerOutput( WContainer* InOwner, WWindow* InRoot )
	:	WContainer( InOwner, InRoot ),
		Messages()
{
	// Initialize own fields.
	SetSize( 300, 200 );
	Padding	= TArea( FORM_HEADER_SIZE, 0, 0, 0 );
	Caption	= L"Output";

	// Allocate log.
	Log			= new WLog( this, Root );
	Log->Align	= AL_Client;
	Log->EventGoto	= WIDGET_INDEX_EVENT(WCompilerOutput::GotoMessage);
}


//
// Compiler output destructor.
//
WCompilerOutput::~WCompilerOutput()
{
	Messages.Empty();
	// Scroll bar will be destroyed
	// in WContainer::~WContainer()
}


//
// Empty records list.
//
void WCompilerOutput::Clear()
{
	Messages.Empty();
	Log->Clear();
}


//
// Add a new record.
//
void WCompilerOutput::AddMessage( String InText, FScript* Script, Int32 iLine, TColor InColor )
{
	TMessage Msg;
	Msg.Script = Script;
	Msg.iLine = iLine;

	Log->AddLine( InText, nullptr, InColor );
	Messages.Push(Msg);
	assert(Messages.Num() == Log->Lines.Num());
}


//
// Redraw compiler output panel.
//
void WCompilerOutput::OnPaint( CGUIRenderBase* Render )
{
	TPoint Base = ClientToWindow(TPoint::Zero);
	Render->SetClipArea( Base, TSize(Size.Width, Size.Height+3) );

	// Draw frame.
	Render->DrawRegion
	(
		Base,
		Size,
		TColor( 0x45, 0x45, 0x45, 0xff ),
		GUI_COLOR_FORM_BORDER,
		BPAT_Solid
	);
	
	// Draw header.
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
// Follow the white rabbit.
//
void WCompilerOutput::GotoMessage( WWidget* Sender, Int32 iMessage )
{
	TMessage& Msg = Messages[iMessage];

	if( Msg.Script && Msg.iLine != -1 )
	{
		// Open script page.
		WScriptPage* Page = (WScriptPage*)GEditor->OpenPageWith(Msg.Script);
		assert(Page);
		GEditor->EditorPages->ActivateTabPage(Page);
		
		// Goto line.
		Page->CodeEditor->ScrollToLine(Msg.iLine-1);
	}
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/