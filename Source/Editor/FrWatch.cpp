/*=============================================================================
    FrWatch.cpp: Runtime properties watch dialog.
    Copyright Dec.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    WWatchListDialog implementation.
-----------------------------------------------------------------------------*/

//
// Watch dialog constructor.
//
WWatchListDialog::WWatchListDialog( WPlayPage* InPage, WWindow* InRoot )
	:	WForm( InRoot, InRoot ),
		Page( InPage ),
		Entity( nullptr ),
		Divider( 144 ),
		Watches()
{
	// Initialize own variables.
	Caption				= String::Format( L"Watch List [%s]", *Page->SourceLevel->GetName() );
	Padding				= TArea( FORM_HEADER_SIZE+1, 12, 0, 0 );
	bSizeableW			= false;
	bSizeableH			= true;
	bCanClose			= true;
	MinHeight			= 200;
	SetSize( 300, 450 );

	// Allocate controls.
	TopPanel					= new WPanel( this, Root );
	TopPanel->Align				= AL_Top;
	TopPanel->SetSize( 300, 76 );

	ScriptLabel					= new WLabel( TopPanel, Root );
	ScriptLabel->Caption		= L"Script: ";
	ScriptLabel->Location		= TPoint( 8, 8 );

	ScriptCombo					= new WComboBox( TopPanel, Root );
	ScriptCombo->Location		= TPoint( 56, 7 );
	ScriptCombo->EventChange	= WIDGET_EVENT(WWatchListDialog::ComboScriptChange);
	ScriptCombo->SetSize( 236, 18 );

	EntityLabel					= new WLabel( TopPanel, Root );
	EntityLabel->Caption		= L"Entity: ";
	EntityLabel->Location		= TPoint( 8, 32 );

	EntityCombo					= new WComboBox( TopPanel, Root );
	EntityCombo->Location		= TPoint( 56, 31 );
	EntityCombo->EventChange	= WIDGET_EVENT(WWatchListDialog::ComboEntityChange);
	EntityCombo->SetSize( 236, 18 );

	PublicOnlyCheck				= new WCheckBox( TopPanel, Root );
	PublicOnlyCheck->Caption	= L"Public Only?";
	PublicOnlyCheck->Tooltip	= L"Whether Show Only Public Properties?";
	PublicOnlyCheck->Location	= TPoint( 8, 56 );
	PublicOnlyCheck->bChecked	= true;
	PublicOnlyCheck->EventClick	= WIDGET_EVENT(WWatchListDialog::CheckPublicOnlyClick);

	ScrollBar					= new WSlider( this, Root );
	ScrollBar->Align			= AL_Right;
	ScrollBar->Margin			= TArea( -1, 0, 0, 0 );
	ScrollBar->SetOrientation(SLIDER_Vertical);
	ScrollBar->SetSize( 12, 50 );

	// Fill lists.
	FLevel* Lev = Page->PlayLevel;
	TArray<FScript*> Used;
	for( Integer i=0; i<Lev->Entities.Num(); i++ )
		Used.AddUnique( Lev->Entities[i]->Script );
	for( Integer i=0; i<Used.Num(); i++ )
		ScriptCombo->AddItem( Used[i]->GetName(), Used[i] );
	ScriptCombo->AlphabetSort();

	// Update list.
	UpdateWatches();
}


//
// Watcher destructor.
//
WWatchListDialog::~WWatchListDialog()
{
}


//
// Count references in watcher.
//
void WWatchListDialog::CountRefs( CSerializer& S )
{
	Serialize( S, Entity );

	// Don't check script list, since it never
	// changed during playing.

	Bool bNeedRebuild = false;
	for( Integer i=0; i<EntityCombo->Items.Num(); i++ )
	{
		FEntity* After = (FEntity*)EntityCombo->Items[i].Data;
		Serialize( S, After );
		if( After != EntityCombo->Items[i].Data )
		{
			bNeedRebuild	= true;
			break;
		}
	}

	// Refresh list of entity since one of
	// them are destroyed.
	ComboScriptChange( this );
}


//
// Redraw watching list.
//
void WWatchListDialog::OnPaint( CGUIRenderBase* Render )
{ 
	WForm::OnPaint(Render); 	
	TPoint Base	= ClientToWindow(TPoint::Zero);

	// Draw BG.
	Render->DrawRegion
	(
		TPoint( Base.X, Base.Y+TopPanel->Size.Height+FORM_HEADER_SIZE ),
		TSize( Size.Width-11, ScrollBar->Size.Height ),
		TColor( 0x33, 0x33, 0x33, 0xff ),
		TColor( 0x66, 0x66, 0x66, 0xff ),
		BPAT_Solid
	);

	Integer YWalk	= Base.Y+TopPanel->Size.Height+FORM_HEADER_SIZE+1;
	Integer YSize	= Size.Height-12-TopPanel->Size.Height-FORM_HEADER_SIZE-4;

	// Clip to area.
	Render->SetClipArea
	(
		TPoint( Base.X, YWalk ),
		TSize( Size.Width, YSize )
	);

	// Render items.
	if( Watches.Num() > 0 )
	{
		Integer NumVis	= YSize / 17;
		Integer	Offset	= Round((Float)(Watches.Num()-NumVis) * ScrollBar->Value / 100.f);
		
		Offset	= Clamp( Offset, 0, Watches.Num()-1 );
		if( NumVis > Watches.Num() )	
			ScrollBar->Value	= 0;

		for( Integer i=Offset, j=0; i<Watches.Num(); i++, j++ )
		{
			TWatch&	W	= Watches[i];

			Render->DrawRegion
			(
				TPoint( Base.X+1, YWalk ),
				TSize( Size.Width-11, 18 ),
				TColor( 0x44, 0x44, 0x44, 0xff ),
				TColor( 0x33, 0x33, 0x33, 0xff ),
				BPAT_Solid
			);

			Render->DrawText
			(
				TPoint( Base.X+4, YWalk+1 ),
				W.Caption,
				GUI_COLOR_TEXT,
				WWindow::Font1
			);

			Render->DrawText
			(
				TPoint( Base.X+4+Divider, YWalk+1 ),
				W.Property->ToString( W.Address ),
				GUI_COLOR_TEXT,
				WWindow::Font1
			);

			YWalk	+= 17;
			if( j >= NumVis )
				break;
		}
	}

	// Draw divider.
	Render->DrawRegion
	(
		TPoint( Base.X+Divider, Base.Y+TopPanel->Size.Height+FORM_HEADER_SIZE+2 ),
		TSize( 0, Size.Height-12-TopPanel->Size.Height-FORM_HEADER_SIZE-3 ),
		TColor( 0x33, 0x33, 0x33, 0xff ),
		TColor( 0x33, 0x33, 0x33, 0xff ),
		BPAT_None
	);
}


// Drag divider now?
static Bool	GDivMove	= false;

//
// User press mouse button.
//
void WWatchListDialog::OnMouseDown( EMouseButton Button, Integer X, Integer Y )
{ 
	WForm::OnMouseDown( Button, X, Y );
	if( Button == MB_Left && Abs(X-Divider)<=3 && Y<Size.Height-12 && Y>20 )
		GDivMove	= true;
}


//
// User unpress mouse button.
//
void WWatchListDialog::OnMouseUp( EMouseButton Button, Integer X, Integer Y )
{
	WForm::OnMouseUp( Button, X, Y );
	GDivMove	= false;
}


//
// Mouse hower over items.
//
void WWatchListDialog::OnMouseMove( EMouseButton Button, Integer X, Integer Y )
{
	WForm::OnMouseMove( Button, X, Y );

	if( GDivMove )
	{
		Divider	= Clamp( X, 100, Size.Width-100 );
		//log( L"Divider at %i", Divider );
	}

	// Switch cursor accroding to style.
	if( Y<Size.Height-12 && Y>20 )
		Cursor	= GDivMove || Abs(X-Divider)<=3	?	CR_SizeWE:
													CR_Arrow;
}


//
// Scroll.
//
void WWatchListDialog::OnMouseScroll( Integer Delta )
{ 
	WForm::OnMouseScroll(Delta); 

	ScrollBar->Value	= Clamp
	( 
		ScrollBar->Value-Delta/120, 
		0, 
		100 
	);
}


//
// Update list of watcher.
//
void WWatchListDialog::UpdateWatches()
{
	Watches.Empty();
	if( Entity )
	{
		// Add properties being base.
		for( CClass* C = Entity->Base->GetClass(); C; C = C->Super )
		{
			for( Integer i=0; i<C->Properties.Num(); i++ )
			{
				CProperty* P = C->Properties[i];
				if( P->ArrayDim == 1 )
					if( !PublicOnlyCheck->bChecked || (P->Flags & PROP_Editable) )
						Watches.Push(TWatch( P->Name, P, ((Byte*)Entity->Base)+P->Offset ));
			}
		}

		// Add properties being InstanceBuffer.
		if( Entity->Script->IsScriptable() )
			for( Integer i=0; i<Entity->Script->Properties.Num(); i++ )
			{
				CProperty* P = Entity->Script->Properties[i];
				if( P->ArrayDim == 1 )
					if( !PublicOnlyCheck->bChecked || (P->Flags & PROP_Editable) )
						Watches.Push(TWatch( P->Name, P, &Entity->InstanceBuffer->Data[P->Offset] ));
			}

		// Add properties being extra components.
		for( Integer e=0; e<Entity->Components.Num(); e++ )
		{
			FExtraComponent* Extra	= Entity->Components[e];
			for( CClass* C=Extra->GetClass(); C; C = C->Super )
			{
				for( Integer i=0; i<C->Properties.Num(); i++ )
				{
					CProperty*	P	= C->Properties[i];
					if( P->ArrayDim == 1 )
						if( !PublicOnlyCheck->bChecked || (P->Flags & PROP_Editable) )
							Watches.Push(TWatch( String::Format( L"$%s.%s", *Extra->GetName(), *P->Name ), P, ((Byte*)Extra)+P->Offset ));
				}
			}
		}
	}
}


//
// When entity changed.
//
void WWatchListDialog::ComboEntityChange( WWidget* Sender )
{
	Entity	= nullptr;
	if( ScriptCombo->ItemIndex == -1 || EntityCombo->ItemIndex == -1 )
		return;

	Entity = (FEntity*)EntityCombo->Items[EntityCombo->ItemIndex].Data;
	UpdateWatches();
}


//
// When script changed.
//
void WWatchListDialog::ComboScriptChange( WWidget* Sender )
{
	Entity	= nullptr;
	EntityCombo->Empty();
	UpdateWatches();
	if( ScriptCombo->ItemIndex == -1 )
		return;

	FScript* Script = (FScript*)ScriptCombo->Items[ScriptCombo->ItemIndex].Data;

	// Update list of entities being this script.
	FLevel* Lev = Page->PlayLevel;
	for( Integer i=0; i<Lev->Entities.Num(); i++ )
		if( Lev->Entities[i]->Script == Script )
			EntityCombo->AddItem( Lev->Entities[i]->GetName(), Lev->Entities[i] );
	EntityCombo->AlphabetSort();

	UpdateWatches();
}


//
// When dialog resized.
//
void WWatchListDialog::OnResize()
{
	WForm::OnResize();
	AlignChildren();
}


//
// Toggle public only check box.
//
void WWatchListDialog::CheckPublicOnlyClick( WWidget* Sender )
{
	UpdateWatches();
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/