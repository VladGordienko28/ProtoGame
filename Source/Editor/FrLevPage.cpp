/*=============================================================================
    FrViewport.cpp: Viewport page.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    WEntitySearchDialog.
-----------------------------------------------------------------------------*/

//
// Entity search dialog.
//
class WEntitySearchDialog: public WForm
{
public:
	// Variables.
	WLevelPage*		Page;
	WPanel*			TopPanel;
	WPanel*			BottomPanel;
	WLabel*			NameLabel;
	WEdit*			NameEdit;
	WListBox*		EntitiesList;

	// WEntitySearchDialog interface.
	WEntitySearchDialog( WLevelPage* InPage, WWindow* InRoot )
		:	WForm( InPage, InRoot ),
			Page( InPage )
	{
		// Initialize form.
		Caption						= L"Entity Search";
		bCanClose					= true;
		bSizeableH					= false;
		bSizeableW					= false;
		SetSize( 216, 321 );

		TopPanel					= new WPanel( this, InRoot );
		TopPanel->Location			= TPoint( 8, 28 );
		TopPanel->SetSize( 200, 240 );	

		BottomPanel					= new WPanel( this, InRoot );
		BottomPanel->Location		= TPoint( 8, 274 );
		BottomPanel->SetSize( 200, 40 );	

		EntitiesList				= new WListBox( TopPanel, InRoot );
		EntitiesList->Location		= TPoint( 8, 10 );

		EntitiesList->ItemIndex		= -1;	
		EntitiesList->EventDblClick	= WIDGET_EVENT(WEntitySearchDialog::ListEntDblClick);
		EntitiesList->SetSize( 185, 220 );

		NameLabel					= new WLabel( BottomPanel, InRoot );
		NameLabel->Caption			= L"Name: ";
		NameLabel->Location			= TPoint( 10, 12 );

		NameEdit					= new WEdit( BottomPanel, InRoot );
		NameEdit->Location			= TPoint( 55, 11 );
		NameEdit->EditType			= EDIT_String;
		NameEdit->EventChange		= WIDGET_EVENT(WEntitySearchDialog::EditNameChange);
		NameEdit->SetSize( 135, 18 );

		Hide();
	}
	void RefreshList()
	{
		EntitiesList->Empty();

		for( Int32 iEnt=0; iEnt<Page->Level->Entities.size(); iEnt++ )
		{
			FEntity* Entity = Page->Level->Entities[iEnt];

			if( NameEdit->Text )
				if( String::Pos
							( 
								String::UpperCase( NameEdit->Text ), 
								String::UpperCase( Entity->GetName() )
							) == -1 )
					continue;

			EntitiesList->AddItem( Entity->GetName(), nullptr );
		}
		EntitiesList->AlphabetSort();
	}

	// WForm interface.
	void Show( Int32 X = 0, Int32 Y = 0 )
	{
		Location	= TPoint( X, Y );
		bVisible	= true;
		RefreshList();
	}
	void OnClose()
	{
		WForm::OnClose();
		Hide();
	}

	// Controls notification.
	void EditNameChange( WWidget* Sender )
	{
		RefreshList();
	}
	void ListEntDblClick( WWidget* Sender )
	{
		if( EntitiesList->ItemIndex != -1 )
		{
			FEntity* Entity = Page->Level->FindEntity( EntitiesList->Items[EntitiesList->ItemIndex].Name );
			if( Entity )
			{
				Page->Level->Camera.Location	= Entity->Base->Location;

				Page->Selector.UnselectAll();
				Page->Selector.SelectEntity( Entity, true );
				Page->UpdateInspector();
			}
		}
	}
};


/*-----------------------------------------------------------------------------
    WLevelPage implementation.
-----------------------------------------------------------------------------*/

//
// Global internal entity clipboard.
// Trans level objects.
//
// Onto Transmigration!
//
static Array<UInt8> GEntityClipboard;


//
// Level page constructor.
//
WLevelPage::WLevelPage( FLevel* InLevel, WContainer* InOwner, WWindow* InRoot )
	:	WEditorPage( InOwner, InRoot ), 
		Selector( InLevel ), 
		Level( InLevel ), 
		Tool( LEV_Edit )
{
	// Initialize own fields.
	Padding		= TArea( 0, 0, 0, 0 );
	Caption		= Level->GetName();
	PageType	= PAGE_Level;
	Color		= PAGE_COLOR_LEVEL;
	TabWidth	= Root->Font1->TextWidth( *Caption ) + 30;

	// Set initial snap.
	TranslationSnap	= 0.5f;
	RotationSnap	= 65536 / 24;   // about 15 deg.

	// Undo/Redo transactor.
	Transactor	= new CLevelTransactor(Level);

	// Toolbar and buttons.
	ToolBar		= new WToolBar( this, Root );
	ToolBar->SetSize( 3000, 28 );

	EditButton				= new WPictureButton( ToolBar, Root );
	EditButton->Tooltip		= L"Edit Tool";
	EditButton->bDown		= true;
	EditButton->Scale		= TSize( 16, 16 );
	EditButton->Offset		= TPoint( 64, 32 );
	EditButton->Picture		= Root->Icons;
	EditButton->EventClick	= WIDGET_EVENT(WLevelPage::ButtonEditClick); 
	EditButton->SetSize( 22, 22 );
	ToolBar->AddElement( EditButton );

	PaintButton				= new WPictureButton( ToolBar, Root );
	PaintButton->Tooltip	= L"Model Paint Tool";
	PaintButton->Scale		= TSize( 16, 16 );
	PaintButton->Offset		= TPoint( 96, 32 );
	PaintButton->Picture	= Root->Icons;
	PaintButton->EventClick = WIDGET_EVENT(WLevelPage::ButtonPaintClick);
	PaintButton->SetSize( 22, 22 );
	ToolBar->AddElement( PaintButton );
	
	KeyButton				= new WPictureButton( ToolBar, Root );
	KeyButton->Tooltip		= L"Keyframe Edit Tool";
	KeyButton->Scale		= TSize( 16, 16 );
	KeyButton->Offset		= TPoint( 80, 32 );
	KeyButton->Picture		= Root->Icons;
	KeyButton->EventClick	= WIDGET_EVENT(WLevelPage::ButtonKeyClick);
	KeyButton->SetSize( 22, 22 );
	ToolBar->AddElement( KeyButton );
	ToolBar->AddElement(nullptr);

	SearchDialogButton				= new WPictureButton( ToolBar, Root );
	SearchDialogButton->Tooltip		= L"Open Search Dialog";
	SearchDialogButton->Scale		= TSize( 16, 16 );
	SearchDialogButton->Offset		= TPoint( 80, 0 );
	SearchDialogButton->Picture		= Root->Icons;
	SearchDialogButton->EventClick	= WIDGET_EVENT(WLevelPage::ButtonSearchDialogClick);
	SearchDialogButton->SetSize( 22, 22 );
	ToolBar->AddElement( SearchDialogButton );	

	RndFlagsButton					= new WPictureButton( ToolBar, Root );
	RndFlagsButton->Tooltip			= L"Render Flags";
	RndFlagsButton->Scale			= TSize( 16, 16 );
	RndFlagsButton->Offset			= TPoint( 128, 32 );
	RndFlagsButton->Picture			= Root->Icons;
	RndFlagsButton->EventClick		= WIDGET_EVENT(WLevelPage::ButtonRndFlagsClick);
	RndFlagsButton->SetSize( 22, 22 );
	ToolBar->AddElement( RndFlagsButton );	
	ToolBar->AddElement( nullptr );

	DragSnapCombo					= new WComboBox( ToolBar, Root );
	DragSnapCombo->Tooltip			= L"Drag Grid Snap";
	DragSnapCombo->EventChange		= WIDGET_EVENT(WLevelPage::ComboDragSnapChange);
	DragSnapCombo->SetSize( 64, 22 );
	ToolBar->AddElement( DragSnapCombo );
	for( Float Scale=0.25f; Scale<=4.f; Scale *= 2.f )
		DragSnapCombo->AddItem( String::Format( L"%.2f", Scale ), nullptr );
	DragSnapCombo->SetItemIndex( 1, false );
	ToolBar->AddElement( nullptr );

	BuildPathsButton				= new WPictureButton( ToolBar, Root );
	BuildPathsButton->Tooltip		= L"Build AI Paths..";
	BuildPathsButton->Scale			= TSize( 16, 16 );
	BuildPathsButton->Offset		= TPoint( 112, 32 );
	BuildPathsButton->Picture		= Root->Icons;
	BuildPathsButton->EventClick	= WIDGET_EVENT(WLevelPage::ButtonBuildPathsClick);
	BuildPathsButton->SetSize( 22, 22 );
	ToolBar->AddElement( BuildPathsButton );	

	DestroyPathsButton				= new WPictureButton( ToolBar, Root );
	DestroyPathsButton->Tooltip		= L"Destroy AI Paths";
	DestroyPathsButton->Scale		= TSize( 16, 16 );
	DestroyPathsButton->Offset		= TPoint( 144, 32 );
	DestroyPathsButton->Picture		= Root->Icons;
	DestroyPathsButton->EventClick	= WIDGET_EVENT(WLevelPage::ButtonDestroyPathsClick);
	DestroyPathsButton->SetSize( 22, 22 );
	ToolBar->AddElement( DestroyPathsButton );	

	// Forms.
	TileEditor		= new WTileEditor( this, InRoot);
	KeyframeEditor	= new WKeyframeEditor( this, InRoot );
	EntitySearch	= new WEntitySearchDialog( this, InRoot );

	// Backdrop popup.
	BackdropPopup	= new WPopupMenu( this, Root );
	BackdropPopup->AddItem( L"Add Entity Here", WIDGET_EVENT(WLevelPage::PopAddEntityClick) );
	BackdropPopup->AddItem( L"" );
	BackdropPopup->AddItem( L"Paste", WIDGET_EVENT(WLevelPage::PopPasteClick) );
	BackdropPopup->AddItem( L"" );
	BackdropPopup->AddItem( L"Unfreeze All", WIDGET_EVENT(WLevelPage::PopUnfreezeAllClick) );

	// Vertex popup.
	VertexPopup	= new WPopupMenu( this, Root );
	VertexPopup->AddItem( L"Insert Vertex", WIDGET_EVENT(WLevelPage::PopInsertVertexClick) );
	VertexPopup->AddItem( L"" );
	VertexPopup->AddItem( L"Remove Vertex", WIDGET_EVENT(WLevelPage::PopRemoveVertexClick) );

	// Submenus.
	WMenu* CSGMenu	= new WMenu( this, Root );
	CSGMenu->AddItem( L"Union", WIDGET_EVENT(WLevelPage::PopCSGUnionClick ) );
	CSGMenu->AddItem( L"Difference", WIDGET_EVENT(WLevelPage::PopCSGDifferenceClick) );
	CSGMenu->AddItem( L"Intersection", WIDGET_EVENT(WLevelPage::PopCSGIntersectionClick) );

	// Entity popup.
	EntityPopup	= new WPopupMenu( this, Root );
	EntityPopup->AddItem( L"Select All ...", WIDGET_EVENT(WLevelPage::PopSelectAllClick) );
	EntityPopup->AddItem( L"" );
	EntityPopup->AddItem( L"Duplicate", WIDGET_EVENT(WLevelPage::PopDuplicateClick) );
	EntityPopup->AddItem( L"Copy", WIDGET_EVENT(WLevelPage::PopCopyClick) );
	EntityPopup->AddItem( L"Cut", WIDGET_EVENT(WLevelPage::PopCutClick) );
	EntityPopup->AddItem( L"" );
	EntityPopup->AddItem( L"Delete", WIDGET_EVENT(WLevelPage::PopDeleteClick) );
	EntityPopup->AddItem( L"" );
	EntityPopup->AddSubMenu( L"CSG", CSGMenu );
	EntityPopup->AddItem( L"" );
	EntityPopup->AddItem( L"Freeze Selected", WIDGET_EVENT(WLevelPage::PopFreezeSelectedClick) );
	EntityPopup->AddItem( L"" );
	EntityPopup->AddItem( L"Edit Script", WIDGET_EVENT(WLevelPage::PopEditScriptClick) );
	EntityPopup->AddItem( L"" );

	// Render pop-up.
	RndFlagsPopup	= new WPopupMenu( this, Root );
	RndFlagsPopup->AddItem( L"Show Grid",		WIDGET_EVENT(WLevelPage::PopRndFlagClick), true );
	RndFlagsPopup->AddItem( L"Show Backdrop",	WIDGET_EVENT(WLevelPage::PopRndFlagClick), true );
	RndFlagsPopup->AddItem( L"Show Hidden",		WIDGET_EVENT(WLevelPage::PopRndFlagClick), true );
	RndFlagsPopup->AddItem( L"Show Logic",		WIDGET_EVENT(WLevelPage::PopRndFlagClick), true );
	RndFlagsPopup->AddItem( L"Show Particles",	WIDGET_EVENT(WLevelPage::PopRndFlagClick), true );
	RndFlagsPopup->AddItem( L"Show Portals",	WIDGET_EVENT(WLevelPage::PopRndFlagClick), true );
	RndFlagsPopup->AddItem( L"Show Lighting",	WIDGET_EVENT(WLevelPage::PopRndFlagClick), true );
	RndFlagsPopup->AddItem( L"Show Model",		WIDGET_EVENT(WLevelPage::PopRndFlagClick), true );
	RndFlagsPopup->AddItem( L"Show Stats",		WIDGET_EVENT(WLevelPage::PopRndFlagClick), true );
	RndFlagsPopup->AddItem( L"Show Misc",		WIDGET_EVENT(WLevelPage::PopRndFlagClick), true );
	RndFlagsPopup->AddItem( L"Show HUD",		WIDGET_EVENT(WLevelPage::PopRndFlagClick), true );
	RndFlagsPopup->AddItem( L"Show Effects",	WIDGET_EVENT(WLevelPage::PopRndFlagClick), true );
	RndFlagsPopup->AddItem( L"Show Paths",		WIDGET_EVENT(WLevelPage::PopRndFlagClick), true );
	for( Int32 i=0; i<RndFlagsPopup->Items.size(); i++ )
		RndFlagsPopup->Items[i].bChecked	= Level->RndFlags & (1 << i);
}


//
// Level page destructor.
//
WLevelPage::~WLevelPage()
{
	// Unselect all.
	Selector.UnselectAll();

	// Destroy transactor.
	delete Transactor;
}


//
// Tick the level.
//
void WLevelPage::TickPage( Float Delta )
{
	Level->Tick(Delta);
}


//
// When page has been opened.
//
void WLevelPage::OnOpen()
{
	UpdateInspector();
}


//
// Switch the page tool.
//
void WLevelPage::SetTool( ELevelTool NewTool )
{
	if( Tool == NewTool )
		return;

	Tool = NewTool;

	// Shutdown support editors.
	TileEditor->Hide();
	KeyframeEditor->Hide();

	switch( Tool )
	{
		case LEV_Edit:
		{
			// Regular editor tool.
			Selector.UnselectAll();
			break;
		}
		case LEV_PaintModel:
		{
			// Tile graphics paint tool.
			Selector.UnselectAll();
			if( TileEditor->Location == TPoint::Zero )
				TileEditor->Show
							( 
								(Size.Width - TileEditor->Size.Width) / 2, 
								(Size.Height - TileEditor->Size.Height) / 2 
							);
			else
				TileEditor->Show
							( 
								TileEditor->Location.X, 
								TileEditor->Location.Y 
							);
			break;
		}
		case LEV_PickEntity:
		{
			// Special object inspector tool.
			break;
		}
		case LEV_Keyframe:
		{
			// Keyframe edit tool.
			Selector.UnselectAll();
			if( KeyframeEditor->Location == TPoint::Zero )
				KeyframeEditor->Show
								( 
									(Size.Width - KeyframeEditor->Size.Width) / 2, 
									(Size.Height - KeyframeEditor->Size.Height) / 2 
								);
			else
				KeyframeEditor->Show
								( 
									KeyframeEditor->Location.X, 
									KeyframeEditor->Location.Y 
								);
			break;
		}
	}

	// Reset roller.
	Roller.Update( Selector );
}


//
// Update inspector, open in inspector
// selected objects.
//
void WLevelPage::UpdateInspector()
{
	GEditor->Inspector->SetEditObjects( *(Array<FObject*>*)&Selector.Selected );
}


//
// When user drag something above viewport.
//
void WLevelPage::OnDragOver( void* Data, Int32 X, Int32 Y, Bool& bAccept )
{
	bAccept	= false;

	if( Data )
	{
		FObject* Object	= (FObject*)Data;

		// Script drop.
		if( Object->IsA(FScript::MetaClass) && Tool == LEV_Edit )
			bAccept	= !As<FScript>(Object)->IsStatic();

		// Music drop.
		if( Object->IsA(FMusic::MetaClass) )
			bAccept	= true;

		// Sound FX drop.
		if( Object->IsA(FSound::MetaClass) )
			bAccept	= true;

		// Texture drop.
		if( Object->IsA(FTexture::MetaClass) )
		{
			FEntity* Entity = GetEntityAt( X, Y, false );
			if( Entity )
			{
				CProperty* Prop = Entity->Base->GetClass()->FindProperty(L"Texture");
				bAccept = Prop && TYPE_TEXTURE.MatchWith(*Prop);
			}
		}
	}
}


//
// When user drop something.
//
void WLevelPage::OnDragDrop( void* Data, Int32 X, Int32 Y )
{
	FObject*	Obj = (FObject*)Data;

	if( Obj->IsA(FScript::MetaClass) )
	{
		// Create a new entity.
		AddEntityTo( As<FScript>(Obj), X, Y );
	}
	else if( Obj->IsA(FMusic::MetaClass) )
	{
		// Set level's soundtrack.
		Level->Soundtrack	= As<FMusic>(Obj);
	}
	else if( Obj->IsA(FSound::MetaClass) )
	{
		// Spawn ambient sound emitter.
		FScript* AmbScript = As<FScript>(GObjectDatabase->FindObject( L"AmbSound", FScript::MetaClass, nullptr ));
		if( AmbScript )
		{
			FEntity* Source = AddEntityTo( AmbScript, X, Y );

			// Mess about instance buffer, it's not really good idea,
			// but this works fine.
			if( AmbScript->Properties.size()>0 )
			{
				CProperty* Prop = AmbScript->Properties[0];
				if( Prop->Type==TYPE_Resource && Prop->Name == L"Sound" )
					*(FSound**)&Source->InstanceBuffer->Data[Prop->Offset] = As<FSound>(Obj);
			}
		}
	}
	else if( Obj->IsA(FTexture::MetaClass) )
	{
		// Put texture on brush/model or something.
			FEntity* Entity = GetEntityAt( X, Y, false );
			if( Entity )
			{
				CProperty* Prop = Entity->Base->GetClass()->FindProperty(L"Texture");
				if( Prop && TYPE_TEXTURE.MatchWith(*Prop) )
				{
					Transactor->TrackEnter();
						*(FTexture**)(((UInt8*)Entity->Base)+Prop->Offset) = As<FTexture>(Obj);
					Transactor->TrackLeave();
				}
			}
	}
}


//
// Ask page, is possible to close it right now?
//
Bool WLevelPage::OnQueryClose()
{ 
	// Yes, sure.
	return true; 
}


//
// Redraw level page.
//
void WLevelPage::OnPaint( CGUIRenderBase* Render )
{
	WEditorPage::OnPaint( Render );

	// Turn on or turn off toolbar buttons.
	DestroyPathsButton->bEnabled	= Level->Navigator != nullptr;
}


//
// When user press keyboard button.
//
void WLevelPage::OnKeyDown( Int32 Key )
{  
	if( Key == VK_DELETE )
	{
		// <Del> button.
		PopDeleteClick( this );
	}
	else if( Key == VK_F10 )
	{
		// <F10> button.
		GEditor->PlayLevel(Level);
	}
	else if( Key=='C' && Selector.Selected.size() && Root->bCtrl )
	{
		// <Ctrl>+<C>.
		PopCopyClick( this );
	}
	else if( Key=='X' && Selector.Selected.size() && Root->bCtrl  )
	{
		// <Ctrl>+<X>.
		PopCutClick( this );
	}
	else if( Key=='Z' && Root->bCtrl && Root->bShift )
	{
		// <Ctrl>+<Shift>+<Z>.
		Redo();
	}
	else if( Key=='Z' && Root->bCtrl )
	{
		// <Ctrl>+<Z>.
		Undo();
	}
}


//
// When user release keyboard button.
//
void WLevelPage::OnKeyUp( Int32 Key )
{ 
	WEditorPage::OnKeyUp( Key );
}


//
// Undo.
//
void WLevelPage::Undo()
{
	Selector.UnselectAll();
	Transactor->Undo();
	UpdateInspector();
	Roller.Update( Selector );
}


//
// Redo.
//
void WLevelPage::Redo()
{
	Selector.UnselectAll();
	Transactor->Redo();
	UpdateInspector();
	Roller.Update( Selector );
}


/*-----------------------------------------------------------------------------
    Basic mouse functions.
-----------------------------------------------------------------------------*/

// Mouse capture internal.
// Two mouse buttons are independ.
static Bool		bLeftPressed	= false;
static Bool		bLWasMouse		= false;
static TPoint	LDwdPos			= TPoint( 0, 0 );
static TPoint	LLastPos		= TPoint( 0, 0 );
static Bool		bRightPressed	= false;
static Bool		bRWasMouse		= false;
static TPoint	RDwdPos			= TPoint( 0, 0 );
static TPoint	RLastPos		= TPoint( 0, 0 );


//
// On mouse up on viewport.
//
void WLevelPage::OnMouseUp( EMouseButton Button, Int32 X, Int32 Y )
{
	if( (Button == MB_Left) && (bLeftPressed) )
	{
		// Left mouse button has been released.
		if( bLWasMouse )
			OnMouseEndDrag( MB_Left, X, Y );
		else if( !bRightPressed )
			OnMouseClick( MB_Left, X, Y );

		bLeftPressed	= false;
		bLWasMouse		= false;	
	}
	else if( (Button == MB_Right) && (bRightPressed) )
	{
		// Right mouse button has been released.
		if( bRWasMouse )
			OnMouseEndDrag( MB_Right, X, Y );
		else if( !bLeftPressed )
			OnMouseClick( MB_Right, X, Y );

		bRightPressed	= false;
		bRWasMouse		= false;
	}
}


//
// On mouse press on viewport.
//
void WLevelPage::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y )
{
	if( Button == MB_Left )
	{
		// Left button pressed.
		bLeftPressed	= true;
		bLWasMouse		= false;
		LDwdPos			= TPoint( X, Y );
		LLastPos		= TPoint( X, Y );
	}
	else if( Button == MB_Right )
	{
		// Right button pressed.
		bRightPressed	= true;
		bRWasMouse		= false;
		RDwdPos			= TPoint( X, Y );
		RLastPos		= TPoint( X, Y );
	}
}


//
// User scroll mouse wheel.
//
void WLevelPage::OnMouseScroll( Int32 Delta )
{
	if( Selector.Selected.size() != 0 && Tool == LEV_Edit )
	{
		// Change the entity layer.
		for( Int32 i=0; i<Selector.Selected.size(); i++ )
		{
			FBaseComponent* Base = Selector.Selected[i]->Base;
			Base->Layer -= Delta / 8400.f;
			Base->Layer = Clamp( Base->Layer, 0.014f, 1.f );
		}
	}
	else
	{
		// Change camera zoom.
		TCamera& Camera = Level->Camera;
		
		if( Delta > 0 )
		{
			// Zoom out.
			while( Delta > 0 )
			{
				Camera.Zoom	*= 1.05f;
				Delta		-= 120;
			}
		}
		else
		{
			// Zoom in.
			while( Delta < 0 )
			{
				Camera.Zoom	*= 0.95f;
				Delta		+= 120;
			}
		}

		// Snap & Clamp zoom!
		Camera.Zoom	= Round(Camera.Zoom*50.f)*0.02f;
		Camera.Zoom	= Clamp( Camera.Zoom, 0.2f, 5.f );
	}
}


//
// User double click on viewport.
//
void WLevelPage::OnDblClick( EMouseButton Button, Int32 X, Int32 Y )
{
	WEditorPage::OnDblClick( Button, X, Y );

#if 0
	// If double click on item - open it script.
	if( Selector.Selected.Num() == 1 )
	{
		FScript* Script	= Selector.Selected[0]->Script;
		if( Script->IsScriptable() )
			GEditor->OpenPageWith( Script );
	}
#endif
}


/*-----------------------------------------------------------------------------
    Top level mouse functions.
-----------------------------------------------------------------------------*/

//
// Drag kind.
//
enum EDragType
{
	DRAG_None,
	DRAG_Observer,
	DRAG_Translate,
	DRAG_Rotate,
	DRAG_Scale,
	DRAG_Vertex,
	DRAG_ModelPaint,
	DRAG_LogicLink,
	DRAG_TexAlign

};


//
// The information about dragging.
//
struct TMouseDragInfo
{
public:
	EDragType	DragType;

	union
	{
		struct{ int dummy; };
		struct{ FBrushComponent* VBrush; Int32 ViVert; };						// DRAG_Vertex.
		struct{ FBaseComponent* SBase; WLevelPage::EStretchHandle SHandle; };	// DRAG_Scale.
		struct{ FLogicComponent* LSource; Int32 LPlug; };						// DRAG_LogicLink.
		struct{ TVector AScale; };												// DRAG_TexAlign;
	};

	TMouseDragInfo()
	{}
} DragInfo;


//
// Process mouse drag.
//
void WLevelPage::OnMouseDrag( EMouseButton Button, Int32 X, Int32 Y, Int32 DeltaX, Int32 DeltaY )
{
	// Transform widget delta to world vector.
	TVector Delta;
	TCamera& Camera = Level->Camera;
	TVector FOV	= Camera.GetFitFOV( Size.Width, Size.Height );
	Delta.X		= +DeltaX * (FOV.X / Size.Width ) * Camera.Zoom;
	Delta.Y		= -DeltaY * (FOV.Y / Size.Height) * Camera.Zoom;
	Delta		= TransformVectorBy( Delta, TCoords(Camera.Location, Camera.Rotation).Transpose() );

	// Process drag type.
	switch( DragInfo.DragType )		
	{
		case DRAG_Observer:
		{
			// Move editor camera.
			Camera.Location -= Delta;
			break;
		}
		case DRAG_Vertex:
		{
			// Move vertex.
			FBrushComponent* B	= DragInfo.VBrush;
			Int32 i	= DragInfo.ViVert;
			B->Vertices[i]	+= Delta;
			break;	
		}
		case DRAG_Translate:
		{
			// Translate selected.
			for( Int32 i=0; i<Selector.Selected.size(); i++ )
			{
				FBaseComponent* Base = Selector.Selected[i]->Base;
				Base->Location += Delta;
			}
			Roller.Update( Selector );

			// Handle keyframe.
			if	(	
					Tool == LEV_Keyframe &&
					Selector.Selected.size() == 1 &&
					KeyframeEditor->Entity == Selector.Selected[0] &&
					KeyframeEditor->iFrame > -1
				)
			{
				FKeyframeComponent* Keyframe = KeyframeEditor->Keyframe;
				Keyframe->Points[KeyframeEditor->iFrame].Location	= Keyframe->Base->Location;
				Keyframe->Points[KeyframeEditor->iFrame].Location.Snap( TranslationSnap );
			}
			break;
		}
		case DRAG_Rotate:
		{
			// Rotate entities.
			Int32 RotDelta = -DeltaX*32 - DeltaY*8;

			for( Int32 i=0; i<Selector.Selected.size(); i++ )
				if( !Selector.Selected[i]->Base->bFixedAngle )
				{
					FBaseComponent* Base = Selector.Selected[i]->Base;
					Base->Rotation += RotDelta;
				}

			Roller.Angle += RotDelta;

			// Handle keyframe.
			if	(	
					Tool == LEV_Keyframe &&
					Selector.Selected.size() == 1 &&
					KeyframeEditor->Entity == Selector.Selected[0] &&
					KeyframeEditor->iFrame > -1
				)
			{
				FKeyframeComponent* Keyframe = KeyframeEditor->Keyframe;
				Keyframe->Points[KeyframeEditor->iFrame].Rotation	= Keyframe->Base->Rotation;
				Keyframe->Points[KeyframeEditor->iFrame].Rotation.Snap( RotationSnap );
				Keyframe->Points[KeyframeEditor->iFrame].bCCW		= RotDelta > 0;
			}
			break;
		}
		case DRAG_Scale:
		{
			// Scale entity.
			TVector DeltaS, DeltaL;
			FBaseComponent* Base = DragInfo.SBase;
			Delta	= TransformVectorBy( Delta, Base->ToLocal() );

			switch( DragInfo.SHandle )
			{
				case STH_NW:
				case STH_SE:
					DeltaL = TVector( +Delta.X*0.5f, +Delta.Y*0.5 );
					DeltaS = TVector( -Delta.X, +Delta.Y ) * (DragInfo.SHandle == STH_NW ? 1.f : -1.f);
					break;

				case STH_NE:
				case STH_SW:
					DeltaL = TVector( +Delta.X*0.5f, +Delta.Y*0.5 );
					DeltaS = TVector( +Delta.X, +Delta.Y ) * (DragInfo.SHandle == STH_NE ? 1.f : -1.f);
					break;

				case STH_N:
				case STH_S:
					DeltaL = TVector( 0.f, +Delta.Y*0.5 );
					DeltaS = TVector( 0.f, +Delta.Y ) * (DragInfo.SHandle == STH_N ? 1.f : -1.f);
					break;

				case STH_E:
				case STH_W:
					DeltaL = TVector( +Delta.X*0.5f, 0.f );
					DeltaS = TVector( +Delta.X, 0.f ) * (DragInfo.SHandle == STH_E ? 1.f : -1.f);
					break;
			}

			// Apply deltas.
			DeltaL	= TransformVectorBy( DeltaL, Base->ToWorld() );

			if( ( Base->Size.X + DeltaS.X >= 0.5f )&&
				( Base->Size.Y + DeltaS.Y >= 0.5f ) )
			{
				Base->Location += DeltaL;
				Base->Size += DeltaS;
			}

			Base->Size.X = Clamp( Base->Size.X, 0.5f, 1000.f );
			Base->Size.Y = Clamp( Base->Size.Y, 0.5f, 1000.f );

			Roller.Update(Selector);
			break;
		}
		case DRAG_ModelPaint:
		{
			// Paint model.
			if( TileEditor->Model && TileEditor->Model->bSelected )
				PaintModelAt( TileEditor->Model, TileEditor->LayerCombo->ItemIndex, X, Y );
			break;
		}
		case DRAG_TexAlign:
		{
			// Align textures on brushes.
			for( Int32 i=0; i<Selector.Selected.size(); i++ )
			{
				if( Selector.Selected[i]->Base->IsA(FBrushComponent::MetaClass) )
				{
					FBrushComponent* Brush = (FBrushComponent*)Selector.Selected[i]->Base;

					if( Button == MB_Left && Root->bAlt )
					{
						// Rotate texture.
						Int32 RotDelta = -DeltaX*32 - DeltaY*8;
						Brush->TexCoords = Brush->TexCoords << RotDelta;
					}
					else if( Button == MB_Left )
					{
						// Translate texture.
						Brush->TexCoords = Brush->TexCoords >> Delta;
					}
					else if( Button == MB_Right )
					{
						// Scale texture.
						DragInfo.AScale	+= TVector( DeltaX, DeltaY ) * 0.03125f;		
						Int32 dX	= Trunc(DragInfo.AScale.X);
						Int32 dY	= Trunc(DragInfo.AScale.Y);

						DragInfo.AScale.X	-= dX;
						DragInfo.AScale.Y	-= dY;

						for( ; dX>0; dX-- )		Brush->TexCoords.XAxis	*= 0.95f;
						for( ; dX<0; dX++ )		Brush->TexCoords.XAxis	*= 1.05f;

						for( ; dY>0; dY-- )		Brush->TexCoords.YAxis	*= 1.05f;
						for( ; dY<0; dY++ )		Brush->TexCoords.YAxis	*= 0.95f;
					}
				}
			}
			break;
		}
	}
}


//
// When user finish drag.
//
void WLevelPage::OnMouseEndDrag( EMouseButton Button, Int32 X, Int32 Y )
{
	switch( DragInfo.DragType )	
	{
		case DRAG_Translate:
		{
			// Snap location.
			for( Int32 i=0; i<Selector.Selected.size(); i++ )
			{
				FBaseComponent* Base = Selector.Selected[i]->Base;
				Base->Location.Snap(TranslationSnap);
			}
			Roller.Update( Selector );
			Transactor->TrackLeave();
			break;
		}
		case DRAG_Vertex:
		{
			// Snap the vertex.
			FBrushComponent* B = DragInfo.VBrush;
			Int32 i = DragInfo.ViVert;
			B->Vertices[i].Snap(TranslationSnap);
	
			// Merge overlap vertices.
			for( Int32 v=0; v<B->NumVerts && B->NumVerts>3; v++ )
			{
				TVector&	V1 = B->Vertices[v],
							V2 = B->Vertices[(v+1) % B->NumVerts];
				if( (V1-V2).SizeSquared() < EPSILON )
				{
					for( Int32 m=v; m<B->NumVerts-1; m++ )
						B->Vertices[m] = B->Vertices[m+1];
					B->NumVerts--;
					v--;
				}
			}
			Transactor->TrackLeave();
			break;
		}
		case DRAG_Scale:
		{
			// Snap scale.
			FBaseComponent* Base = DragInfo.SBase;
			Base->Location.Snap( TranslationSnap * 0.5f );
			Base->Size.Snap( TranslationSnap );
			Transactor->TrackLeave();
			break;
		}
		case DRAG_Rotate:
		{
			// Snap rotation.
			for( Int32 i=0; i<Selector.Selected.size(); i++ )
				if( !Selector.Selected[i]->Base->bFixedAngle )
				{
					FBaseComponent* Base = Selector.Selected[i]->Base;
					Base->Rotation.Snap(RotationSnap);
				}
			Transactor->TrackLeave();
			break;
		}
		case DRAG_LogicLink:
		{
			// Try to connect.
			ELogicSocket SocType = LOGSOC_Jack;
			FLogicComponent* L	= nullptr;
			Int32 iJack = GetSocketAt( X, Y, SocType, L );
			if( iJack != -1 )
			{
				// Link 'em.
				assert(DragInfo.LSource != nullptr);
				DragInfo.LSource->AddConnector( L, DragInfo.LPlug, iJack );
			}
			Transactor->TrackLeave();
			break;
		}
		case DRAG_ModelPaint:
		{
			// Finish paint.
			Transactor->TrackLeave();
			break;
		}
		case DRAG_TexAlign:
		{
			// Snap texture matrix.
			for( Int32 i=0; i<Selector.Selected.size(); i++ )
			{
				if( Selector.Selected[i]->Base->IsA(FBrushComponent::MetaClass) )
				{
					FBrushComponent* Brush = (FBrushComponent*)Selector.Selected[i]->Base;

					// Translation snap.
					Brush->TexCoords.Origin.Snap( TranslationSnap );

					// Rotation snap.
					Float	XLen	= Brush->TexCoords.XAxis.Size();
					Float	YLen	= Brush->TexCoords.YAxis.Size();
					TAngle	Rot		= VectorToAngle( Brush->TexCoords.XAxis );

					Rot.Snap( RotationSnap );
					Brush->TexCoords.XAxis	= AngleToVector(Rot) * XLen;
					Brush->TexCoords.YAxis	= -AngleToVector(Rot).Cross() * YLen;
				}
			}
			Transactor->TrackLeave();
			break;
		}
	}

	// No more drag.
	DragInfo.DragType = DRAG_None;
}


//
// On mouse move in viewport.
//
void WLevelPage::OnMouseMove( EMouseButton Button, Int32 X, Int32 Y )
{
	// Is moved with hold button.
	if( bLeftPressed )
	{
		// Left button hold.
		if( !bLWasMouse )
			OnMouseBeginDrag( MB_Left, X, Y );

		OnMouseDrag( MB_Left, X, Y, X-LLastPos.X, Y-LLastPos.Y );
		bLWasMouse	= true;
		LLastPos	= TPoint( X, Y );
	}
	if( bRightPressed )
	{
		// Right button hold.
		if( !bRWasMouse )
			OnMouseBeginDrag( MB_Right, X, Y );

		OnMouseDrag( MB_Right, X, Y, X-RLastPos.X, Y-RLastPos.Y );
		bRWasMouse	= true;
		RLastPos	= TPoint( X, Y );
	}

	// Change cursor style according to tool and object below cursor.
	switch( Tool )
	{
		case LEV_Edit:
		{
			//
			// Normal edit tool.
			//
			FBrushComponent* Brush;
			EStretchHandle Handle;			
			FBaseComponent*	Base;

			if( DragInfo.DragType == DRAG_LogicLink )
			{
				// Logic connection.
				Cursor	= CR_Arrow;
			}
			else if( GetRollerAt( X, Y ) || DragInfo.DragType == DRAG_Rotate )
			{
				// Cursor for roller.
				Cursor	= CR_SizeWE;
			}
			else if( GetVertexAt( X, Y, Brush ) != -1 )
			{
				// Cursor for vertex.
				Cursor	= CR_SizeAll;
			}
			else if (	
						(DragInfo.DragType==DRAG_Scale && (Handle=DragInfo.SHandle)!=STH_None)||
						(Handle = GetStretchHandleAt( X, Y, Base )) != STH_None 	
					)
			{
				// Cursor for stretch handle.
				switch( Handle )
				{
					case STH_NW:
					case STH_SE:
						Cursor	= CR_SizeNWSE;
						break;

					case STH_N:
					case STH_S:
						Cursor	= CR_SizeNS;
						break;

					case STH_NE:
					case STH_SW:
						Cursor	= CR_SizeNESW;
						break;

					case STH_E:
					case STH_W:
						Cursor	= CR_SizeWE;
						break;
				}
			}
			else if( GetEntityAt( X, Y, true ) )
			{
				// Cursor for entity.
				Cursor	= CR_Cross;
			}
			else
			{
				// Regular.
				Cursor	= CR_Arrow; 
			}
			break;
		}
		case LEV_PaintModel:
		{
			//
			// Model paint tool.
			//
			if( TileEditor->Model )
			{
				TVector P = ScreenToWorld( X, Y );
				TileEditor->Model->PenIndex = TileEditor->Model->WorldToMapIndex( P.X, P.Y );
			}
			break;
		}
		case LEV_PickEntity:
		{
			//
			// Entity pick tool.
			//
			Cursor = GetEntityAt( X, Y, true ) ? CR_HandPoint : CR_Arrow;
			break;
		}
	}

	// Put information into status bar.
	if( !(GFrameStamp & 7) )
	{
		FEntity* Entity = GetEntityAt( X, Y, false );
		TVector Pen	= ScreenToWorld( X, Y );
		GEditor->StatusBar->Panels[0].Text = Entity ? String::Format
																( 
																	L"%s [%s]", 
																	*Entity->GetName(), 
																	*Entity->Script->GetName() 
																) 
																	: L"None";
		GEditor->StatusBar->Panels[1].Text = String::Format
														( 
															L"[X=%.2f, Y=%.2f]", 
															Pen.X, 
															Pen.Y 
														);
	}
}	 


//
// Process mouse click.
//
void WLevelPage::OnMouseClick( EMouseButton Button, Int32 X, Int32 Y )
{
	switch( Tool )
	{
		case LEV_Edit:
		{
			//
			// Normal edit tool.
			//

			// Test with vertex.
			{
				FBrushComponent* Brush;
				Int32 iVert = GetVertexAt( X, Y, Brush );
				if( iVert != -1 )
				{
					ClickVertex( Brush, iVert, Button, X, Y );
					return;
				}
			}

			// Test with logic socket.
			{
				FLogicComponent* L;
				ELogicSocket	Sock	= LOGSOC_Plug;
				Int32 iPlug			= GetSocketAt( X, Y, Sock, L );
				if( (iPlug != -1) && (Button == MB_Right) )
				{
					// Unplug all.
					Transactor->TrackEnter();
					{
						L->RemoveConnectors( iPlug );
					}
					Transactor->TrackLeave();
					return;
				}
			}

			// Test with entity.
			FEntity* Entity = GetEntityAt( X, Y );

			if( Entity )
				ClickEntity( Entity, Button, X, Y );
			else
				ClickBackdrop( Button, X, Y );

			Roller.Update(Selector);
			break;
		}
		case LEV_PaintModel:
		{
			//
			// Model paint tool.
			//
			FEntity* Entity = GetEntityAt( X, Y );

			if( Entity && Entity->Base->IsA(FModelComponent::MetaClass) )
			{
				// Click model.
				FModelComponent* Model = As<FModelComponent>(Entity->Base);

				if( Model->bSelected )
				{
					// Click on selected model.
					if( Button == MB_Left )
					{
						// L click on selected model.
						Transactor->TrackEnter();
						{
							PaintModelAt( Model, TileEditor->LayerCombo->ItemIndex, X, Y );
						}
						Transactor->TrackLeave();
					}
					else if( Button == MB_Right )
					{
						// R click on selected model.
						TileEditor->SetModel( nullptr );
						Selector.UnselectAll();
						UpdateInspector();
					}
				}
				else
				{
					// Click on not selected model.
					Selector.UnselectAll();
					Selector.SelectEntity( Model->Entity, true );
					TileEditor->SetModel( Model );
					UpdateInspector();
				}
			}
			else
			{
				// Click outside model.
				TileEditor->SetModel( nullptr );
				Selector.UnselectAll();
				UpdateInspector();
			}
			break;
		}
		case LEV_PickEntity:
		{
			//
			// Entity pick tool.
			//
			FEntity* Entity = GetEntityAt( X, Y, false );
			GEditor->Inspector->ObjectPicked( (Entity && Button==MB_Left) ? Entity : nullptr );
			SetTool( LEV_Edit );
			break;
		}
		case LEV_Keyframe:
		{
			//
			// Keyframe tool.
			//
			FEntity* Entity = GetEntityAt( X, Y );

			if( Entity )
			{
				if( Entity->Base->bSelected )
				{
					// Click on selected.
					KeyframeEditor->SetEntity( nullptr );
					Selector.UnselectAll();
				}
				else
				{
					// Click on non selected.
					Selector.UnselectAll();
					Selector.SelectEntity( Entity, true );
					KeyframeEditor->SetEntity( Entity );
				}
			}
			else
			{
				// Click on backdrop.
				KeyframeEditor->SetEntity( nullptr );
				Selector.UnselectAll();
			}
			Roller.Update(Selector);
			break;
		}
	}
}


//
// When user click brush vertex.
//
void WLevelPage::ClickVertex( FBrushComponent* Brush, Int32 iVert, EMouseButton Button, Int32 X, Int32 Y )
{
	if( Button == MB_Right )
	{
		assert(Brush);
		assert(iVert != -1);

		DragInfo.DragType	= DRAG_Vertex;
		DragInfo.VBrush		= Brush;
		DragInfo.ViVert		= iVert;

		VertexPopup->Show( TPoint( X, Y ) );
	}
}


//
// User click entity.
//
void WLevelPage::ClickEntity( FEntity* Entity, EMouseButton Button, Int32 X, Int32 Y )
{
	if( Button == MB_Left )
	{
		// Left click at entity.
		if( Root->bCtrl )
		{
			// LMouse + Ctrl.
			Selector.SelectEntity( Entity, true );
			UpdateInspector();
		}
		else if( Root->bAlt )
		{
			// LMouse + Alt.
			Selector.SelectEntity( Entity, false );
			UpdateInspector();
		}
		else
		{
			// Simple LClick.
			Bool bWasSelected = Entity->Base->bSelected;
			Selector.UnselectAll();
			Selector.SelectEntity( Entity, !bWasSelected );
			UpdateInspector();
		}
	}
	else if( Button == MB_Right )
	{
		if( Entity->Base->bSelected )
		{
			// R click on the entity.
			String Info		= Selector.GetSelectionInfo();
			FScript* Script	= Selector.Selected[0]->Script;

			EntityPopup->Items[0].Text	= String::Format( L"Select All %s", *Script->GetName() );

			// Disable some items.
			EntityPopup->Items[8].SubMenu->Items[0].bEnabled	=
			EntityPopup->Items[8].SubMenu->Items[1].bEnabled	=
			EntityPopup->Items[8].SubMenu->Items[2].bEnabled	= Script->Base->IsA(FBrushComponent::MetaClass);
			EntityPopup->Items[8].SubMenu->Items[0].bEnabled	= false; // CSG_Union.
			EntityPopup->Items[12].bEnabled	= Selector.Selected.size()==1 && Selector.Selected[0]->Script->IsScriptable();
			EntityPopup->Size.Width			= Max( 60+Root->Font1->TextWidth(*EntityPopup->Items[0].Text), 138 );
			EntityPopup->Show( TPoint( X, Y ) );
		}
		else
		{
			// Click on non selected.
			ClickBackdrop( Button, X, Y );
		}
	}
}


//
// User click backdrop.
//
void WLevelPage::ClickBackdrop( EMouseButton Button, Int32 X, Int32 Y )
{
	if( Button == MB_Left )
	{
		// L click backdrop.
		if( Root->bAlt || Root->bCtrl )
			return;	// Multi selection.

		Selector.UnselectAll();
		UpdateInspector();
	}
	else if( Button == MB_Right )
	{
		// R click backdrop.
		FResource* Res	= GEditor->Browser->GetSelected();
		FScript* Script = Res && Res->IsA(FScript::MetaClass) ? (FScript*)Res : nullptr;

		BackdropPopup->Items[0].Text		= String::Format( L"Add %s here", Script ? *Script->GetName() : L"<Entity>" );
		BackdropPopup->Items[0].bEnabled	= Script != nullptr;
		BackdropPopup->Items[2].bEnabled	= GEntityClipboard.size()!=0 && GPlat->ClipboardPaste()==L"Entity";
		BackdropPopup->Size.Width			= Max( 60+Root->Font1->TextWidth(*BackdropPopup->Items[0].Text), 165 );
		BackdropPopup->Show(TPoint( X, Y ));		
		Selector.UnselectAll();
		UpdateInspector();
	}
}


//
// Start mouse drag.
//
void WLevelPage::OnMouseBeginDrag( EMouseButton Button, Int32 X, Int32 Y )
{ 
	switch( Tool )
	{
		case LEV_Edit:
		{
			//
			// Normal edit tool.
			//

			// Test with roller.
			if( GetRollerAt( X, Y ) )
			{
				Transactor->TrackEnter();
				DragInfo.DragType	= DRAG_Rotate;
				return;
			}

			// Test with logic sockets.
			if( Level->RndFlags & RND_Logic )
			{
				ELogicSocket Sock	= LOGSOC_Plug;
				FLogicComponent* L	= nullptr;
				Int32 iPlug		= GetSocketAt( X, Y, Sock, L );

				if( iPlug != -1 )
				{
					Transactor->TrackEnter();
					DragInfo.DragType	= DRAG_LogicLink;
					DragInfo.LSource	= L;
					DragInfo.LPlug		= iPlug;
					return;
				}
			}

			// Test with stretch handle.
			{
				FBaseComponent* Base;
				EStretchHandle Handle = GetStretchHandleAt( X, Y, Base );
				if( Handle != STH_None )
				{
					Transactor->TrackEnter();
					DragInfo.DragType	= DRAG_Scale;
					DragInfo.SBase		= Base;
					DragInfo.SHandle	= Handle;
					return;
				}
			}

			// Test with vertex.
			{
				FBrushComponent* Brush;
				Int32 iVert = GetVertexAt( X, Y, Brush );
				if( iVert != -1 )
				{
					Transactor->TrackEnter();
					DragInfo.DragType	= DRAG_Vertex;
					DragInfo.VBrush		= Brush;
					DragInfo.ViVert		= iVert;
					return;
				}
			}

			FEntity* Entity = GetEntityAt( X, Y );
			if( Entity && Entity->Base->bSelected )
			{
				if( Root->bShift )
				{
					// Shift texture.
					Transactor->TrackEnter();
					DragInfo.AScale		= TVector( 0.f, 0.f );
					DragInfo.DragType	= DRAG_TexAlign;
				}
				else
				{
					// Translate entities.
					Transactor->TrackEnter();
					DragInfo.DragType	= DRAG_Translate;
				}
				return;
			}
			else
			{
				// Move observer.
				if( Button == MB_Left )
					DragInfo.DragType	= DRAG_Observer;
				return;
			}
			break;
		}
		case LEV_PaintModel:
		{
			//
			// Model paint.
			//
			FEntity* Entity = GetEntityAt( X, Y );
	
			if	( 
					Entity && 
					Entity->Base->bSelected && 
					Entity->Base->IsA(FModelComponent::MetaClass) && 
					Button == MB_Left 
				)
			{
				Transactor->TrackEnter();
				DragInfo.DragType = DRAG_ModelPaint;
			}
			else
					DragInfo.DragType = DRAG_Observer;
			break;
		}
		case LEV_Keyframe:
		{
			//
			// Key Edit tool.
			//
			if( GetRollerAt( X, Y ) )
			{
				Transactor->TrackEnter();
				DragInfo.DragType	= DRAG_Rotate;
				return;
			}
			FEntity* Entity = GetEntityAt( X, Y );
			DragInfo.DragType = Entity && Entity->Base->bSelected ? DRAG_Translate : DRAG_Observer;
			if( DragInfo.DragType == DRAG_Translate )
				Transactor->TrackEnter();
			break;
		}
		case LEV_PickEntity:
		{
			//
			// Entity pick tool.
			//
			DragInfo.DragType = DRAG_Observer;
			break;
		}
	}
}


/*-----------------------------------------------------------------------------
    Helper functions.
-----------------------------------------------------------------------------*/

//
// Return entity in specified location, if no entity hit, return nil.
// By the way, its take care about layer, priority to
// the top. If bFast don't taking into account layer.
//
FEntity* WLevelPage::GetEntityAt( Int32 X, Int32 Y, Bool bFast )
{
	FEntity*	Result		= nullptr;
	Float		BestLayer	= -9999.9f;
	TVector		V			= ScreenToWorld( X, Y );

	for( Int32 i=0; i<Level->Entities.size(); i++ )
	{
		FEntity*		Entity = Level->Entities[i];
		FBaseComponent* Base	= Entity->Base;

		// Don't test at bottom.
		if( Base->Layer <= BestLayer )
			continue;

		// Don't pick frozen.
		if( Base->bFrozen )
			continue;

		if( Base->IsA(FBrushComponent::MetaClass) )
		{
			// Test hit with brush.
			FBrushComponent* Brush = (FBrushComponent*)Base;

			if( IsConvexPoly( Brush->Vertices, Brush->NumVerts ) )
			{
				// Convex poly, test for real hit.
				if( IsPointInsidePoly
								( 
									TransformPointBy( V, Brush->ToLocal() ), 
									Brush->Vertices, 
									Brush->NumVerts ) 
								)
				{
					// Hit brush!
					Result		= Entity;
					BestLayer	= Base->Layer;
				}
			}
			else
			{
				// Not convex brush, test for aabb hit.
				if( Brush->GetAABB().IsInside(V) )
				{
					// Hit brush!
					Result		= Entity;
					BestLayer	= Base->Layer;
				}
			}
		}
		else if( Base->IsA(FZoneComponent::MetaClass) )
		{
			// Test hit with zone.
			FZoneComponent* Zone = (FZoneComponent*)Base;
			TRect ScreenRect;

			// Flip Y axis.
			WorldToScreen( Zone->GetAABB().Min, ScreenRect.Min.X, ScreenRect.Max.Y );
			WorldToScreen( Zone->GetAABB().Max, ScreenRect.Max.X, ScreenRect.Min.Y );

			if( ScreenRect.AtBorder(TVector(X, Y), 3.f) )
			{
				Result		= Entity;
				BestLayer	= Base->Layer;
			}
		}
		else if( Base->IsA(FModelComponent::MetaClass) )
		{
			// Test hit with model.
			TRect R	= Base->GetAABB();

			if( R.IsInside( V ) )
			{
				// Hit entity.
				Result		= Entity;
				BestLayer	= Base->Layer;
			}
		}
		else if( Base->IsA(FPortalComponent::MetaClass) )
		{
			// Test hit with the portal.
			FPortalComponent* Portal = (FPortalComponent*)Base;
			TCoords C = Portal->ToWorld();
			TVector V1 = TransformPointBy( TVector( 0.f, -Portal->Width*0.5f ), C );
			TVector V2 = TransformPointBy( TVector( 0.f, +Portal->Width*0.5f ), C );
			WorldToScreen( V1, V1.X, V1.Y );
			WorldToScreen( V2, V2.X, V2.Y );

			if( PointOnSegment( TVector(X, Y), V1, V2, 2.9f ) )
			{
				// Hit portal.
				Result		= Entity;
				BestLayer	= Base->Layer;
			}
		}
		else
		{
			// Most probably FRectComponent.
			TRect R		= TRect( TVector( 0.f, 0.f ), Base->Size );

			if( R.IsInside( TransformPointBy( V, Base->ToLocal() ) ) )
			{
				// Hit entity.
				Result		= Entity;
				BestLayer	= Base->Layer;
			}
		}

		// Fast or correct search?
		if( bFast && Result )
			break;
	}

	return Result;
}


//
// Return stretch handle at cursor [X, Y] location.
// If hit handle return OutBase, it own this handle.
// Note: Looking for control points only for selected actors.
//
WLevelPage::EStretchHandle WLevelPage::GetStretchHandleAt( Int32 X, Int32 Y, FBaseComponent*& OutBase )
{
	for( Int32 iEntity=0; iEntity<Selector.Selected.size(); iEntity++ )
	{
		FBaseComponent* Base = Selector.Selected[iEntity]->Base;

		// Never stretch some bases.
		if( !Base->IsA(FRectComponent::MetaClass) || Base->bFixedSize )	
			continue;

		// Compute handles.
		TVector Size2 = Base->Size * 0.5f;
		TCoords	Coords	= TCoords( Base->Location, Base->Rotation );
		TVector XAxis = Coords.XAxis * Size2.X, 
				YAxis = Coords.YAxis * Size2.Y;

		TVector Handles[STH_MAX] = 
		{
			TVector( 0.f, 0.f ),
			Coords.Origin - XAxis + YAxis,
			Coords.Origin + YAxis,
			Coords.Origin + XAxis + YAxis,
			Coords.Origin + XAxis,
			Coords.Origin + XAxis - YAxis,
			Coords.Origin - YAxis,
			Coords.Origin - XAxis - YAxis,
			Coords.Origin - XAxis,
		};

		// Transform handles.
		for( Int32 i=1; i<STH_MAX; i++ )
			WorldToScreen( Handles[i], Handles[i].X, Handles[i].Y );

		// Test for hit.
		for( Int32 i=1; i<STH_MAX; i++ )
			if( Abs(X-Handles[i].X) <= 8.f && Abs(Y-Handles[i].Y) <= 8.f )
			{
				// Found.
				OutBase	= Base;
				return (EStretchHandle)i;
			}
	}

	// Nothing found.
	OutBase = nullptr;
	return STH_None;
}


//
// Return socket at specific widget location. Here's two ways of use:
//  A. SType == LOGSOC_None: Find any type of socket, SType return gotten type.
//  B. SType == LOGSOC_Plug | SType == LOGSOC_Jack: Find by type only.
// Return socket index.
//
Int32 WLevelPage::GetSocketAt( Int32 X, Int32 Y, ELogicSocket& SType, FLogicComponent*& L )
{
	L = nullptr;

	for( FLogicComponent* Test=Level->FirstLogicElement; Test; Test=Test->NextLogicElement )
	{
		// Don't test if element frozen.
		if( Test->Base->bFrozen )
			continue;

		if( SType != LOGSOC_Jack )
			for( Int32 j=0; j<Test->NumPlugs; j++ )
			{
				TVector S = Test->GetPlugPos(j);
				WorldToScreen( S, S.X, S.Y );

				if( Abs(X-S.X) <= 7.f && Abs(Y-S.Y) <= 7.f )
				{
					SType	= LOGSOC_Plug;
					L		= Test;
					return j;
				}
			}

		if( SType != LOGSOC_Plug )
			for( Int32 j=0; j<Test->NumJacks; j++ )
			{
				TVector S = Test->GetJackPos(j);
				WorldToScreen( S, S.X, S.Y );

				if( Abs(X-S.X) <= 7.f && Abs(Y-S.Y) <= 7.f )
				{
					SType	= LOGSOC_Jack;
					L		= Test;
					return j;
				}
			}
	}

	// No found.
	return -1;
}


//
// Return brush vertex at widget location, if found return its index and brush in
// OutBrush, if not found, return -1 and OutBrush is nullptr.
//
Int32 WLevelPage::GetVertexAt( Int32 X, Int32 Y, FBrushComponent*& OutBrush )
{
	TVector Pix( X, Y );
	OutBrush	= nullptr;

	for( Int32 k=0; k<Selector.Selected.size(); k++ )
	{
		FBaseComponent* Base = Selector.Selected[k]->Base;

		if( !Base->IsA(FBrushComponent::MetaClass) )
			continue;

		FBrushComponent* Brush = As<FBrushComponent>(Base);
		for( Int32 iVert=0; iVert<Brush->NumVerts; iVert++ )
		{
			TVector V;
			WorldToScreen( Brush->Location + Brush->Vertices[iVert], V.X, V.Y );

			if( Abs(Pix.X-V.X) <= 8.f && Abs(Pix.Y-V.Y) <= 8.f )
			{
				OutBrush = Brush;
				return iVert;
			}
		}
	}

	return -1;
}


//
// Paint tiles on model.
//
void WLevelPage::PaintModelAt( FModelComponent* Model, Int32 iLayer, Int32 X, Int32 Y )
{
	TVector V = ScreenToWorld( X, Y ); 
	Int32 iTile = Model->WorldToMapIndex( V.X, V.Y );

	if( iTile != -1 && Model->Selected.size() > 0 )
	{
		Int32 Tx = Model->Selected[0] % Model->TilesPerU;
		Int32 Ty = Model->Selected[0] / Model->TilesPerU;

		Int32 Mx = iTile % Model->MapXSize;
		Int32 My = iTile / Model->MapXSize;

		for( Int32 i=0; i<Model->Selected.size(); i++ )
		{
			Int32 TDx = (Model->Selected[i] % Model->TilesPerU) - Tx;
			Int32 TDy = (Model->Selected[i] / Model->TilesPerU) - Ty;

			Int32 Nx = Mx + TDx;
			Int32 Ny = My - TDy;

			// Plot tile, if in valid range.
			if( Nx >= 0 && Ny >= 0 && Nx < Model->MapXSize && Ny < Model->MapYSize )
			{
				UInt16*	TilePtr = &Model->Map[Nx + Ny*Model->MapXSize];

				if( iLayer == 0 )
				{
					*TilePtr	&= 0xff00;
					*TilePtr	|= Model->Selected[i];
				}
				else
				{
					*TilePtr	&= 0x00ff;
					*TilePtr	|= ((Int32)Model->Selected[i]) << 8;
				}
			}
		}
	}
}


//
// Add entity to specified location.
//
FEntity* WLevelPage::AddEntityTo( FScript* Script, Int32 X, Int32 Y )
{
	FEntity* Entity;
	Transactor->TrackEnter();
	{
		TVector V = ScreenToWorld( X, Y );
		Entity = Level->CreateEntity( Script, L"", V );

		// Initially snap it to grid.
		Entity->Base->Location.Snap( TranslationSnap );
	}
	Transactor->TrackLeave();

	return Entity;
}


//
// Return true, if given point hits the roller.
//
Bool WLevelPage::GetRollerAt( Int32 X, Int32 Y )
{
	if( !Roller.bVisible )
		return false;

	// Distance from cursor to roller.
	Float Zoom	= Level->Camera.Zoom;
	Float Dist	= Distance( Roller.Position, ScreenToWorld( X, Y ) );

	return	( Dist >= (ROLLER_RADIUS-0.15f)*Zoom ) &&
			( Dist <= (ROLLER_RADIUS+0.15f)*Zoom );
}


//
// Transform point in widget coords to the level worlds.
//
TVector WLevelPage::ScreenToWorld( Int32 X, Int32 Y )
{
	TPoint P = ClientToWindow(TPoint::Zero);
	TViewInfo View
				(
					Level->Camera.Location,
					Level->Camera.Rotation,
					Level->Camera.GetFitFOV( Size.Width, Size.Height ),
					Level->Camera.Zoom,
					false,
					P.X,
					P.Y,
					Size.Width,
					Size.Height
				);

	return View.Deproject( X, Y );
}


//
// Transform point in the level world coords to the widgets.
//
void WLevelPage::WorldToScreen( TVector V, Float& OutX, Float& OutY )
{
	TPoint P = ClientToWindow(TPoint::Zero);
	TViewInfo View
				(
					Level->Camera.Location,
					Level->Camera.Rotation,
					Level->Camera.GetFitFOV( Size.Width, Size.Height ),
					Level->Camera.Zoom,
					false,
					P.X,
					P.Y,
					Size.Width,
					Size.Height
				);

	View.Project( V, OutX, OutY );
}


/*-----------------------------------------------------------------------------
    Editor rendering.
-----------------------------------------------------------------------------*/

//
// Draw a paths network in the level.
//
void WLevelPage::DrawPathsNetwork( CCanvas* Canvas, CNavigator* Navigator )
{
	// Draw all nodes.
	for( Int32 iNode=0; iNode<Navigator->Nodes.size(); iNode++ )
	{
		TPathNode& Node = Navigator->Nodes[iNode];

		Canvas->DrawPoint
						(	 
							Node.Location,
							10.f,
							Node.GetDrawColor()
						);
	}

	// Draw all edges.
	TVector Bias( 0.f, (5.f*Canvas->View.FOV.Y*Canvas->View.Zoom)/(Canvas->View.Height) );

	for( Int32 iEdge=0; iEdge<Navigator->Edges.size(); iEdge++ )
	{
		TPathEdge& Edge = Navigator->Edges[iEdge];

		TPathNode& NodeA = Navigator->Nodes[Edge.iStart];
		TPathNode& NodeB = Navigator->Nodes[Edge.iFinish];

		Canvas->DrawLine
					(
						NodeA.Location,
						NodeB.Location + Bias,
						Edge.GetDrawColor(),
						false
					);
	}
}


//
// Draw entity keyframe trajectory.
//
void WLevelPage::DrawKeyframe( CCanvas* Canvas, FEntity* Entity )
{
	// Find keyframe component in entity.
	FKeyframeComponent* Keyframe = nullptr;
	for( Int32 i=0; i<Entity->Components.size(); i++ )
		if( Entity->Components[i]->IsA(FKeyframeComponent::MetaClass) )
		{
			Keyframe	= (FKeyframeComponent*)Entity->Components[i];
			break;
		}

	if( !Keyframe )
		return;

	// Draw trajectory.
	for( Int32 i=1; i<Keyframe->Points.size(); i++ )
		Canvas->DrawLine
					( 
						Keyframe->Points[i-1].Location,
						Keyframe->Points[i].Location,
						COLOR_Pink,
						false
					);

	// Draw control points.
	for( Int32 i=0; i<Keyframe->Points.size(); i++ )
	{
		Canvas->DrawPoint
						(
							Keyframe->Points[i].Location,
							5.f,
							COLOR_Pink
						);

		Canvas->DrawLineStar
						(
							Keyframe->Points[i].Location,
							Keyframe->Points[i].Rotation,
							1.f,
							COLOR_Pink,
							false
						);
	}

	// Draw keys numbers in viewport space.
	TPoint P = ClientToWindow(TPoint::Zero);
	Canvas->PushTransform(TViewInfo( P.X, P.Y, Size.Width, Size.Height ));
	for( Int32 i=0; i<Keyframe->Points.size(); i++ )
	{
		Float X, Y;
		WorldToScreen( Keyframe->Points[i].Location, X, Y );

		Canvas->DrawText
					(	
						String::Format( L"%d", i+1 ),
						Root->Font2,
						COLOR_White,
						TVector( X-10, Y-25 )
					);
	}
	Canvas->PopTransform();
}


//
// Draw all logic links and labels for
// each plug and jack.
//
void WLevelPage::DrawLogicCircuit( CCanvas* Canvas )
{
	TPoint P = ClientToWindow(TPoint::Zero);

	for( FLogicComponent* Logic=Level->FirstLogicElement; Logic; Logic=Logic->NextLogicElement )
	{
		FBaseComponent*		Base = Logic->Base;

		// Is visible?
		if( !Canvas->View.Bounds.IsOverlap(Base->GetAABB()) )
			continue;

		Canvas->PushTransform(TViewInfo( P.X, P.Y, Size.Width, Size.Height ));
		{
			// Draw plugs.
			for( Int32 i=0; i<Logic->NumPlugs; i++ )
			{
				Float X, Y;
				WorldToScreen( Logic->GetPlugPos(i), X, Y );
				
				Canvas->DrawText
				(
					Logic->PlugsName[i],
					Root->Font2,
					COLOR_White,
					TVector( X-Root->Font2->TextWidth(*Logic->PlugsName[i])-4, Y-8 )
				);
			}

			// Draw jacks.
			for( Int32 i=0; i<Logic->NumJacks; i++ )
			{
				Float X, Y;
				WorldToScreen( Logic->GetJackPos(i), X, Y );
				
				Canvas->DrawText
				(
					Logic->JacksName[i],
					Root->Font2,
					COLOR_White,
					TVector( X+4, Y-8 )
				);
			}
		}
		Canvas->PopTransform();
	}
}


//
// Draw level's scroll bounds.
//
void WLevelPage::DrawScrollClamp( CCanvas* Canvas )
{
	Canvas->DrawLineRect
	(
		Level->Camera.ScrollBound.Center(),
		Level->Camera.ScrollBound.Size(),
		0,
		COLOR_LimeGreen,
		false
	);
}


//
// Render the page.
//
void WLevelPage::RenderPageContent( CCanvas* Canvas )
{
	TPoint Base = ClientToWindow(TPoint::Zero);
									  
	// Render level.
	GEditor->GRender->RenderLevel
							( 
								Canvas, 
								Level, 
								Base.X, 
								Base.Y, 
								Size.Width, 
								Size.Height 
							);

	// Set level transform, for editor stuff rendering.
	Canvas->SetTransform
					( 
						TViewInfo
							( 
								Level->Camera.Location, 
								Level->Camera.Rotation, 
								Level->Camera.GetFitFOV(Size.Width, Size.Height), 
								Level->Camera.Zoom, 
								false, 
								Base.X, 
								Base.Y, 
								Size.Width, 
								Size.Height
							) 
					);

	// Draw scroll bounds.
	DrawScrollClamp( Canvas );

	// Draw navigator.
	if( (Level->RndFlags & RND_Paths) && Level->Navigator )
		DrawPathsNetwork( Canvas, Level->Navigator );

	// Draw roller.
	Roller.Draw( Canvas, DragInfo.DragType == DRAG_Rotate );
	
	// Draw trajectories for all selected objects.
	for( Int32 i=0; i<Selector.Selected.size(); i++ )
		DrawKeyframe( Canvas, Selector.Selected[i] );

	// Draws logic.
	if( Level->RndFlags & RND_Logic )
		DrawLogicCircuit( Canvas );

	// Draw current drag relative.
	switch( DragInfo.DragType )
	{
		case DRAG_Vertex:
		{
			// Highlight drag vertex.
			FBrushComponent* B = DragInfo.VBrush;
			Int32 i = DragInfo.ViVert;
			Canvas->DrawCoolPoint( B->Vertices[i] + B->Location, 10.f, COLOR_DeepPink );
			break;
		}
		case DRAG_LogicLink:
		{
			// Draw active link line.
			TVector Pen = ScreenToWorld( LLastPos.X, LLastPos.Y );
			Canvas->DrawSmoothLine( DragInfo.LSource->GetPlugPos(DragInfo.LPlug), Pen, COLOR_LightBlue, false );
		}
	}

	// Set screen coords transform.
	Canvas->SetTransform
					(
						TViewInfo
							(
								Base.X, 
								Base.Y, 
								Size.Width, 
								Size.Height
							)
					);
	// Draw stats.
	Canvas->DrawText
				( 
					String::Format( L"FPS: %d", GEditor->FPS ), 
					Root->Font1, 
					COLOR_White, 
					TVector( 10.f, 38.f ) 
				);

#if FLU_PROFILE_MEMORY
	Canvas->DrawText
				( 
					String::Format( L"Mem: %.2f kB", Double(mem::stats().totalAllocatedBytes) / 1024 ), 
					Root->Font1, 
					COLOR_White, 
					TVector( 10.f, 55.f ) 
				);
#endif
}


/*-----------------------------------------------------------------------------
    Entity clipboard functions.
-----------------------------------------------------------------------------*/

//
// Buffer writer.
//
class CEntityBufferWriter: public CSerializer
{
public:
	// Variables.
	Array<UInt8>	Buffer;

	// CEntityBufferWriter interface.
	CEntityBufferWriter()
		:	Buffer()
	{
		Mode	= SM_Save;
	}

	// CSerializer interface.
	void SerializeData( void* Mem, SizeT Count )
	{
		Int32 OldNum = Buffer.size();
		Buffer.setSize( OldNum+Count );
		mem::copy( &Buffer[OldNum], Mem, Count );
	}
	void SerializeRef( FObject*& Obj )
	{
		String ObjName = Obj ? Obj->GetName() : L"null";
			Serialize( *this, ObjName );
		if( Obj )
		{
			String ClassName = Obj->GetClass()->Name;
			String OwnerName = Obj->GetOwner() ? Obj->GetOwner()->GetName() : L"null";
			Serialize( *this, ClassName );
			Serialize( *this, OwnerName );
		}
	}
	SizeT TotalSize()
	{
		return Buffer.size();
	}
	void Seek( SizeT NewPos )
	{}
	SizeT Tell()
	{
		return Buffer.size();
	}
};


//
// Buffer reader.
//
class CEntityBufferReader: public CSerializer
{
public:
	// Variables.
	Array<UInt8>	Buffer;
	Int32			iPos;

	// CEntityBufferReader interface.
	CEntityBufferReader( Array<UInt8>& InBuffer )
		:	Buffer( InBuffer ),
			iPos( 0 )
	{
		Mode	= SM_Load;
	}

	// CSerializer interface.
	void SerializeData( void* Mem, SizeT Count )
	{
		mem::copy( Mem, &Buffer[iPos], Count );
		iPos	+= Count;
	}
	void SerializeRef( FObject*& Obj )
	{
		String ObjName;
		Serialize( *this, ObjName );
		if( ObjName != L"null" )
		{
			String ClassName;
			String OwnerName;
			Serialize( *this, ClassName );
			Serialize( *this, OwnerName );
			CClass*	 ReqClass = CClassDatabase::StaticFindClass(*ClassName);
			FObject* Owner = OwnerName != L"null" ? GObjectDatabase->FindObject(OwnerName) : nullptr;
			Obj	= GObjectDatabase->FindObject( ObjName, ReqClass ? ReqClass : FObject::MetaClass, Owner );
		}
		else
			Obj	= nullptr;
	}
	SizeT TotalSize()
	{
		return Buffer.size();
	}
	void Seek( SizeT NewPos )
	{
		iPos	= NewPos;
	}
	SizeT Tell()
	{
		return iPos;
	}
};


//
// Copy entity to clipboard.
//
void WLevelPage::PopCopyClick( WWidget* Sender )
{
	FEntity* Entity = Selector.Selected[0];
	assert(Entity);

	CEntityBufferWriter Writer;

	// Lead stuff.
	Serialize( Writer, Entity->Script );

	// Components.
	Entity->Base->SerializeThis( Writer );
	for( Int32 e=0; e<Entity->Components.size(); e++ )
		Entity->Components[e]->SerializeThis( Writer );

	// Instance buffer.
	if( Entity->InstanceBuffer )
	{
		FScript* Script = Entity->Script;
		Int32 NumProps = Script->Properties.size();
		Serialize( Writer, NumProps );
		for( Int32 iProp=0; iProp<Script->Properties.size(); iProp++ )
		{
			CProperty* Prop = Script->Properties[iProp];

			UInt8 Type	= (UInt8)Prop->Type;
			UInt8 ArrDim = Prop->ArrayDim;
			Serialize( Writer, Type );
			Serialize( Writer, ArrDim );
			Prop->SerializeValue( &Entity->InstanceBuffer->Data[Prop->Offset], Writer );
		}
	}

	// Actually copy to clipboard.
	GEntityClipboard	= Writer.Buffer;
	GPlat->ClipboardCopy( L"Entity" );
}


//
// Paste entity from clipboard.
//
void WLevelPage::PopPasteClick( WWidget* Sender )
{
	// If nothing in clipboard.
	if( GEntityClipboard.size() == 0 || GPlat->ClipboardPaste() != L"Entity" )
		return;

	Transactor->TrackEnter();
	{
		Selector.UnselectAll();
		
		FEntity* Entity = nullptr;
		CEntityBufferReader Reader(GEntityClipboard);

		String EntityName;
		FScript* Script;
		Serialize( Reader, Script );
		if( !Script )
			goto Leave;

		// Generate new unique name.
		
		for( Int32 iUniq=0; ; iUniq++ )
		{
			String TestName = String::Format( L"%s%d", *Script->GetName(), iUniq );
			if( !GObjectDatabase->FindObject( TestName, FEntity::MetaClass, Level ) )
			{
					EntityName = TestName;
					break;
			}
		}

		// Allocate entity and components.
		Entity	= NewObject<FEntity>( EntityName, Level );

		// Set main props.
		Entity->Level	= Level;
		Entity->Script	= Script;

		// Copy components.
		FBaseComponent* Base = CopyObject( Script->Base, Script->Base->GetName(), Entity );
		Base->SerializeThis( Reader );
		Base->InitForEntity( Entity );
		for( Int32 e=0; e<Script->Components.size(); e++ )
		{
			FExtraComponent* Extra = CopyObject( Script->Components[e], Script->Components[e]->GetName(), Entity );
			Extra->SerializeThis( Reader );
			Extra->InitForEntity( Entity );
		}

		// Instance buffer.
		if( Entity->Script->InstanceBuffer )
		{
			Entity->InstanceBuffer = new CInstanceBuffer(Script->Properties);
			Entity->InstanceBuffer->Data.setSize(Script->InstanceSize);

			Int32 NumProps;
			Serialize( Reader, NumProps );
			if( NumProps == Script->Properties.size() )
				for( Int32 iProp=0; iProp<Script->Properties.size(); iProp++ )
				{
					CProperty* Prop = Script->Properties[iProp];
					UInt8 Type;
					UInt8 ArrDim;
					Serialize( Reader, Type );
					Serialize( Reader, ArrDim );

					if( Type != Prop->Type || ArrDim != Prop->ArrayDim )
						break;

					Prop->SerializeValue( &Entity->InstanceBuffer->Data[Prop->Offset], Reader );
				}
		}

		// Add entity to level db.
		Level->Entities.push( Entity );

		// Place entity to proper location in world.
		Entity->Base->Location	= ScreenToWorld( RDwdPos.X, RDwdPos.Y );
		Entity->Base->Location.Snap(TranslationSnap);

Leave:;
	}
	Transactor->TrackLeave();
}


//
// Cut selected object.
//
void WLevelPage::PopCutClick( WWidget* Sender )
{
	Transactor->TrackEnter();
	{
		PopCopyClick( Sender );
		Level->DestroyEntity( Selector.Selected[0] );
	}
	Transactor->TrackLeave();
}


/*-----------------------------------------------------------------------------
    Popup events.
-----------------------------------------------------------------------------*/

//
// Freeze all selected entities.
//
void WLevelPage::PopFreezeSelectedClick( WWidget* Sender )
{
	for( Int32 i=0; i<Selector.Selected.size(); i++ )
		Selector.Selected[i]->Base->bFrozen	= true;
	Selector.UnselectAll();
}


//
// Unfreeze all entities.
//
void WLevelPage::PopUnfreezeAllClick( WWidget* Sender )
{
	for( Int32 i=0; i<Level->Entities.size(); i++ )
		Level->Entities[i]->Base->bFrozen	= false;
}


//
// Set render flags.
//
void WLevelPage::PopRndFlagClick( WWidget* Sender )
{
	// Zero all flags.
	Level->RndFlags	= 0;

	// Copy from popup.
	for( Int32 i=0; i<RndFlagsPopup->Items.size(); i++ )
		if( RndFlagsPopup->Items[i].bChecked )
			Level->RndFlags	|= 1 << i;
}


//
// Add a new entity.
//
void WLevelPage::PopAddEntityClick( WWidget* Sender )
{
	// Get script.
	FResource* Res	= GEditor->Browser->GetSelected();
	FScript* Script = Res && Res->IsA(FScript::MetaClass) ? (FScript*)Res : nullptr;

	if( Script )
		AddEntityTo( Script, RDwdPos.X, RDwdPos.Y );
}


//
// Delete selected entities.
//
void WLevelPage::PopDeleteClick( WWidget* Sender )
{
	Transactor->TrackEnter();
	{
		for( Int32 i=0; i<Selector.Selected.size(); i++ )
			Level->DestroyEntity( Selector.Selected[i] );

		Selector.UnselectAll();
		Roller.Update( Selector );
		UpdateInspector();
	}
	Transactor->TrackLeave();
}


//
// Select all entities with the same script.
//
void WLevelPage::PopSelectAllClick( WWidget* Sender )
{
	assert(Selector.Selected.size()>0);
	FScript* Script = Selector.Selected[0]->Script;
	Selector.SelectByScript( Script );
	UpdateInspector();
}


//
// Remove a selected vertex from the Brush.
//
void WLevelPage::PopRemoveVertexClick( WWidget* Sender )
{
	FBrushComponent* C = DragInfo.VBrush;
	Int32 iVert = DragInfo.ViVert;
	DragInfo.DragType = DRAG_None;

	if( C->NumVerts <= 3 )
	{
		Root->ShowMessage( L"Brush should have at least 3 vertices.", L"CSG", true );
		return;
	}

	Transactor->TrackEnter();
	{
		// Shift vertices to replace.
		for( Int32 i=iVert; i<C->NumVerts-1; i++ )
			C->Vertices[i] = C->Vertices[i+1];

		C->NumVerts--;
	}
	Transactor->TrackLeave();
}


//
// Insert a new vertex to Brush.
//
void WLevelPage::PopInsertVertexClick( WWidget* Sender )
{
	FBrushComponent* C = DragInfo.VBrush;
	Int32 iVert = DragInfo.ViVert;
	DragInfo.DragType = DRAG_None;

	if( C->NumVerts >= FBrushComponent::MAX_BRUSH_VERTS )
	{
		Root->ShowMessage( L"Vertices per brush are exceed.", L"CSG", true );
		return;
	}

	Transactor->TrackEnter();
	{
		// Shift vertices to exempt space.
		for( Int32 i=C->NumVerts; i>=iVert+1; i-- )
			C->Vertices[i] = C->Vertices[i-1];

		C->NumVerts++;
		C->Vertices[iVert+1] = ( C->Vertices[iVert] + C->Vertices[(iVert+2) % C->NumVerts] ) * 0.5f;
	}
	Transactor->TrackLeave();
}


//
// Apply CSG union operation to all selected
// Brushes.
//
void WLevelPage::PopCSGUnionClick( WWidget* Sender )
{
	Transactor->TrackEnter();
	{
		for( Int32 i=0; i<Selector.Selected.size(); i++ )
			if( Selector.Selected[i]->Base->IsA(FBrushComponent::MetaClass) )
			{
				FBrushComponent* B = (FBrushComponent*)Selector.Selected[i]->Base;
				CSGUnion( B, Level );
			}

		UpdateInspector();
	}
	Transactor->TrackLeave();
}


//
// Apply CSG intersection operation to all selected
// brushes.
//
void WLevelPage::PopCSGIntersectionClick( WWidget* Sender )
{
	Transactor->TrackEnter();
	{
		for( Int32 i=0; i<Selector.Selected.size(); i++ )
			if( Selector.Selected[i]->Base->IsA(FBrushComponent::MetaClass) )
			{
				FBrushComponent* B = (FBrushComponent*)Selector.Selected[i]->Base;
				CSGIntersection( B, Level );
			}

		UpdateInspector();
	}
	Transactor->TrackLeave();
}


//
// Apply CSG difference operation to all selected
// brushes.
//
void WLevelPage::PopCSGDifferenceClick( WWidget* Sender )
{
	Transactor->TrackEnter();
	{
		for( Int32 i=0; i<Selector.Selected.size(); i++ )
			if( Selector.Selected[i]->Base->IsA(FBrushComponent::MetaClass) )
			{
				FBrushComponent* B = (FBrushComponent*)Selector.Selected[i]->Base;
				CSGDifference( B, Level );
			}

		UpdateInspector();
	}
	Transactor->TrackLeave();
}


//
// Edit script being selected entity.
//
void WLevelPage::PopEditScriptClick( WWidget* Sender )
{
	if( Selector.Selected.size() == 1 )
	{
		FScript* Script	= Selector.Selected[0]->Script;
		if( Script->IsScriptable() )
			GEditor->OpenPageWith( Script );
	}
}


//
// Duplicate selected entities.
//
void WLevelPage::PopDuplicateClick( WWidget* Sender )
{
	Transactor->TrackEnter();
	{
		for( Int32 iSource=0; iSource<Selector.Selected.size(); iSource++ )
		{
			FEntity* Source	= Selector.Selected[iSource];

			// Generate new unique name.
			String EntityName;
			for( Int32 iUniq=0; ; iUniq++ )
			{
				String TestName = String::Format( L"%s%d", *Source->Script->GetName(), iUniq );
				if( !GObjectDatabase->FindObject( TestName, FEntity::MetaClass, Level ) )
				{
					EntityName = TestName;
					break;
				}
			}

			// Create entity.
			FEntity* New	= NewObject<FEntity>( EntityName, Level );

			// Set main props.
			New->Level	= Level;
			New->Script	= Source->Script;

			// Copy components.
			FBaseComponent* Base = CopyObject( Source->Base, Source->Base->GetName(), New );
			Base->InitForEntity( New );
			for( Int32 e=0; e<Source->Components.size(); e++ )
			{
				FExtraComponent* Extra = CopyObject( Source->Components[e], Source->Components[e]->GetName(), New );
				Extra->InitForEntity( New );
			}

			// Instance buffer.
			if( New->Script->InstanceBuffer )
			{
				New->InstanceBuffer = new CInstanceBuffer( New->Script->Properties );
				New->InstanceBuffer->Data.setSize( New->Script->InstanceSize );

				// Copy data from the source entity.
				if( New->Script->Properties.size() )
					New->InstanceBuffer->CopyValues( &Source->InstanceBuffer->Data[0] );
			}

			// Add entity to level db.
			Level->Entities.push( New );

			// Place entity to proper location in world.
			New->Base->Location	+= Source->Base->GetAABB().Size()*1.17f;
			New->Base->Location.Snap(TranslationSnap);
			New->Base->Layer	+= 0.01f;
		}
	}
	Transactor->TrackLeave();
}


//
// Change editor drag snap.
//
void WLevelPage::ComboDragSnapChange( WWidget* Sender )
{
	TranslationSnap	= Pow( 2.f, DragSnapCombo->ItemIndex-2.f );
}


/*-----------------------------------------------------------------------------
    Toolbar buttons notifications.
-----------------------------------------------------------------------------*/

//
// Open entity search dialog.
//
void WLevelPage::ButtonSearchDialogClick( WWidget* Sender )
{
	if( !EntitySearch->bVisible )
		EntitySearch->Show( Size.Width/3, Size.Height/3 );
}


//
// Button render flags clicked.
//
void WLevelPage::ButtonRndFlagsClick( WWidget* Sender )
{
	RndFlagsPopup->Show(WindowToClient(Root->MousePos));
}


//
// Set an edit tool.
//
void WLevelPage::ButtonEditClick( WWidget* Sender )
{
	// Update buttons status.
	EditButton->bDown	= true;
	KeyButton->bDown	= false;
	PaintButton->bDown	= false;

	SetTool( LEV_Edit );
}


//
// Set keyframe master tool.
//
void WLevelPage::ButtonKeyClick( WWidget* Sender )
{
	// Update buttons status.
	EditButton->bDown	= false;
	KeyButton->bDown	= true;
	PaintButton->bDown	= false;

	SetTool( LEV_Keyframe );
}


//
// Set paint tool.
//
void WLevelPage::ButtonPaintClick( WWidget* Sender )
{
	// Update buttons status.
	EditButton->bDown	= false;
	KeyButton->bDown	= false;
	PaintButton->bDown	= true;

	SetTool( LEV_PaintModel );
}


//
// Button build paths clicked.
//
void WLevelPage::ButtonBuildPathsClick( WWidget* Sender )
{
	GEditor->BuildPaths( Level );
}


//
// Destroy a level navigation network.
//
void WLevelPage::ButtonDestroyPathsClick( WWidget* Sender )
{
	GEditor->DestroyPaths( Level );
}


/*-----------------------------------------------------------------------------
    TSelector implementation.
-----------------------------------------------------------------------------*/

//
// Selector constructor.
//
TSelector::TSelector( FLevel* InLevel )
	:	Level( InLevel ),
		Selected()
{
	assert(Level);
	assert(!Level->IsTemporal());
}


//
// Selector destructor.
//
TSelector::~TSelector()
{
	Selected.empty();
}


//
// Select or unselect the entity.
// Warning: do no change ..Base->bSelected manually, let
// this function handle it.
//
void TSelector::SelectEntity( FEntity* Entity, Bool bSelect )
{
	if( bSelect )
	{
		// Select the entity, even if it already selected,
		// doesn't matter.
		Entity->Base->bSelected	= true;
		Selected.addUnique( Entity );					
	}
	else
	{
		// Unselect the entity, even if it already unselected,
		// doesn't matter.
		Entity->Base->bSelected	= false;
		Selected.removeUnique( Entity );
	}
}


//
// Unselect all actors in the level.
//
void TSelector::UnselectAll()
{
	// Unselect even if incidentally set bSelected to non selected.
	if( Level )
		for( Int32 i=0; i<Level->Entities.size(); i++ )
			Level->Entities[i]->Base->bSelected = false;

	Selected.empty();
}


//
// Return information about select entities
// in friendly format.
//
String TSelector::GetSelectionInfo() const
{
	if( Selected.size() == 0 )
		return L"Nothing";

	FScript* Shared = Selected[0]->Script;
	for( Int32 i=1; i<Selected.size(); i++ )
		if( Selected[i]->Script != Shared )
		{
			Shared	= nullptr;
			break;
		}

	return String::Format( L"%i %s", Selected.size(), Shared ? *Shared->GetName() : L"Entity" );
}


//
// Select all objects.
//
void TSelector::SelectAll()
{
	UnselectAll();

	for( Int32 i=0; i<Level->Entities.size(); i++ )
		SelectEntity( Level->Entities[i], true );
}


//
// Select all objects being script.
//
void TSelector::SelectByScript( FScript* InScript )
{
	UnselectAll();

	for( Int32 i=0; i<Level->Entities.size(); i++ )
		if( Level->Entities[i]->Script == InScript )
			SelectEntity( Level->Entities[i], true );
}


/*-----------------------------------------------------------------------------
    TRoller implementation.
-----------------------------------------------------------------------------*/

//
// Roller constructor.
//
WLevelPage::TRoller::TRoller()
	:	bVisible( false ),
		Position( 9999.9f, 9999.9f ),
		Angle( 0 )
{
}


//
// Update the roller.
//
void WLevelPage::TRoller::Update( TSelector& Sel )
{
	Angle	= 0;

	// Find master entity for roller.
	// Should be last selected.
	FEntity* Master = nullptr;
	for( Int32 i=Sel.Selected.size()-1; i>=0; i-- )
		if( !Sel.Selected[i]->Base->bFixedAngle )
		{
			Master	= Sel.Selected[i];
			break;
		}

	if( Master )
	{
		// Roller has at least one master.
		bVisible	= true;
		Position	= Master->Base->Location;
	}
	else
	{
		// No master, mean no roller.
		bVisible	= false;
		Position	= TVector( -9999.9f, -9999.9f );
	}
}


//
// Draw roller gizmo.
//
void WLevelPage::TRoller::Draw( CCanvas* Canvas, Bool bHighlight )
{
	if( bVisible )
	{
		if( bHighlight )
		{
			// Draw highlighted.
			Canvas->DrawCircle
							( 
								Position, 
								ROLLER_RADIUS * Canvas->View.Zoom, 
								COLOR_Gold, 
								false 
							);

			Canvas->DrawLineStar
							(
								Position,
								Angle,
								ROLLER_RADIUS * Canvas->View.Zoom * 0.75f,
								COLOR_Gold,
								false
							);
		}
		else
		{
			// Draw as unused.
			Canvas->DrawCircle
							( 
								Position, 
								ROLLER_RADIUS * Canvas->View.Zoom, 
								COLOR_Blue, 
								false 
							);
		}	
	}
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/