/*=============================================================================
    FrTileEd.cpp: Model editor.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

// Is should save tiles location after adding, removing tile.
#define SAVE_TILE_LOC	1

/*-----------------------------------------------------------------------------
    WTilesGrid implementation.
-----------------------------------------------------------------------------*/

//
// Tiles grid constructor.
//
WTileEditor::WTilesGrid::WTilesGrid( WWindow* InRoot, WTileEditor* InEditor )
	:	WWidget( InEditor, InRoot ),
		Editor( InEditor ),
		bCapture( false )
{
	// Set default size.
	SetSize( 256, 256 );
}


//
// Mouse press tile editor.
//
void WTileEditor::WTilesGrid::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y )
{
	WWidget::OnMouseDown( Button, X, Y );

	if( !Editor->Model )
		return;

	if( Button == MB_Left )
	{
		// L click - prepare for collection.
		Editor->Model->Selected.Empty();
		bCapture = true;
		OnMouseMove( Button, X, Y );
	}
	else if( Button == MB_Right )
	{
		// R click - unselect all.
		Editor->Model->Selected.Empty();
		bCapture = false;
	}
}


//
// Mouse release button.
//
void WTileEditor::WTilesGrid::OnMouseUp( EMouseButton Button, Int32 X, Int32 Y )
{
	WWidget::OnMouseUp( Button, X, Y );

	if( Button == MB_Left )
		bCapture = false;
}


//
// Mouse move on tiles grid.
//
void WTileEditor::WTilesGrid::OnMouseMove( EMouseButton Button, Int32 X, Int32 Y )
{
	WWidget::OnMouseMove( Button, X, Y );

	if( !Editor->Model )
		return;

	// Check bounds.
	if( X < 0 || Y < 0 || X >= Size.Width || Y >= Size.Height )
		return;

	if( bCapture )
	{
		FModelComponent* Model = Editor->Model;

		// Compute tile index below cursor.
		Int32 NewX = Trunc( X * Model->TilesPerU / 256.f );
		Int32 NewY = Trunc( Y * Model->TilesPerV / 256.f );
		Int32 iTile = NewX + NewY * Model->TilesPerU;

		if( Model->Selected.Num() == 0 )
		{
			// Add first tile.
			Model->Selected.Push( iTile );
		}
		else
		{
			// Add extra tiles.
			if( Model->Selected[0] == 0 )
			{
				// Don't add tiles after eraser tile.
			}
			else
			{
				// Collect all tiles, except eraser tile.
				if( iTile != 0 )
					Model->Selected.AddUnique( iTile );
			}
		}
	}
}


//
// When user drag something above tiles grid.
//
void WTileEditor::WTilesGrid::OnDragOver( void* Data, Int32 X, Int32 Y, Bool& bAccept )
{
	bAccept = Data && ((FObject*)Data)->IsA(FTexture::MetaClass) && Editor->Model;
}


//
// When user drop bitmap on tiles grid.
//
void WTileEditor::WTilesGrid::OnDragDrop( void* Data, Int32 X, Int32 Y )
{
	Editor->Model->Texture = (FTexture*)Data;
}


//
// Draw grid of tiles.
//
void WTileEditor::WTilesGrid::OnPaint( CGUIRenderBase* Render )
{
	WWidget::OnPaint( Render );

	if( Editor->Model && Editor->Model->Texture )
	{
		// With model.
		FModelComponent* Model	= Editor->Model;
		TPoint Base = ClientToWindow(TPoint::Zero);

		// Compute deltas.
		Float Dx = 256.f / Model->TilesPerU;
		Float Dy = 256.f / Model->TilesPerV;
		Float X = 0.f, Y = 0.f;

		// Tileset.
		Render->DrawPicture
					( 
						Base, 
						Size, 
						TPoint( 0, 0 ), 
						TSize( Model->Texture->USize, Editor->Model->Texture->VSize ), 
						Model->Texture 
					);

		// Draw selection marks.
		for( Int32 i=0; i<Model->Selected.Num(); i++ )
		{
			TColor MarkColor = Model->Selected[0] == 0 ? COLOR_DeepPink : COLOR_DeepSkyBlue;

			Int32 X = Model->Selected[i] % Model->TilesPerU;
			Int32 Y = Model->Selected[i] / Model->TilesPerU;

			Render->DrawRegion( TPoint( Base.X + X*Dx + 1, Base.Y + Y*Dy + 1 ), TSize( Dx-2, Dy-2 ), MarkColor, MarkColor, BPAT_Diagonal );
			Render->DrawRegion( TPoint( Base.X + X*Dx + 1, Base.Y + Y*Dy + 1 ), TSize( Dx-2, Dy-2 ), MarkColor, MarkColor, BPAT_PolkaDot );
		}

		// Draw grid.
		for( Int32 i=0; i<Model->TilesPerU; i+=2 )
		{
			Render->DrawRegion( TPoint( Base.X+X, Base.Y ), TSize( Dx, Size.Height ), COLOR_White, TColor( 0x31, 0x31, 0x31, 0xff ), BPAT_None );
			X += Dx * 2;
		}
		for( Int32 i=0; i<Model->TilesPerV; i+=2 )
		{
			Render->DrawRegion( TPoint( Base.X, Base.Y+Y ), TSize( Size.Width, Dy ), COLOR_White, TColor( 0x31, 0x31, 0x31, 0xff ), BPAT_None );
			Y += Dy * 2;
		}

		// Draw gray frame.
		Render->DrawRegion		
					( 
						Base, 
						Size, 
						COLOR_White, 
						TColor( 0x31, 0x31, 0x31, 0xff ), 
						BPAT_None 
					);
	}
	else
	{
		// Without selected model.
		TPoint Base = ClientToWindow(TPoint(0, 0));

		// Just gray rectangle.
		Render->DrawRegion
					( 
						Base, 
						Size, 
						TColor( 0x53, 0x53, 0x53, 0xff ), 
						TColor( 0x31, 0x31, 0x31, 0xff ), 
						BPAT_Solid 
					);
	}
}


/*-----------------------------------------------------------------------------
    WTileEditor implementation.
-----------------------------------------------------------------------------*/

//
// Tiles editor constructor.
//
WTileEditor::WTileEditor( WContainer* InOwner, WWindow* InRoot )
	:	WForm( InOwner, InRoot ),
		Model( nullptr )
{
	// Setup form.
	SetSize( 276, 385 );		
	bCanClose			= false;
	bSizeableH			= false;
	bSizeableW			= false;

	// Create grid.
	TilesGrid			= new WTilesGrid( Root, this );
	TilesGrid->Location	= TPoint( Padding.Left+10, Padding.Top+10 );

	// Create buttons.
	AddUpButton							= new WButton( this, Root );
	AddUpButton->SetSize( 24, 24 );
	AddUpButton->Caption				= L"+";
	AddUpButton->Location				= TPoint( 112, 319 );
	AddUpButton->EventClick				= TNotifyEvent( this, (TNotifyEvent::TEvent)&WTileEditor::ButtonAddUpClick );

	RemoveUpButton						= new WButton( this, Root );
	RemoveUpButton->SetSize( 24, 24 );
	RemoveUpButton->Caption				= L"-";
	RemoveUpButton->Location			= TPoint( 140, 319 );
	RemoveUpButton->EventClick			= TNotifyEvent( this, (TNotifyEvent::TEvent)&WTileEditor::ButtonRemoveUpClick );

	AddDownButton						= new WButton( this, Root );
	AddDownButton->SetSize( 24, 24 );
	AddDownButton->Caption				= L"+";
	AddDownButton->Location				= TPoint( 112, 351 );
	AddDownButton->EventClick			= TNotifyEvent( this, (TNotifyEvent::TEvent)&WTileEditor::ButtonAddDownClick );

	RemoveDownButton					= new WButton( this, Root );
	RemoveDownButton->SetSize( 24, 24 );
	RemoveDownButton->Caption			= L"-";
	RemoveDownButton->Location			= TPoint( 140, 351 );
	RemoveDownButton->EventClick		= TNotifyEvent( this, (TNotifyEvent::TEvent)&WTileEditor::ButtonRemoveDownClick );

	AddLeftButton						= new WButton( this, Root );
	AddLeftButton->SetSize( 24, 24 );
	AddLeftButton->Caption				= L"+";
	AddLeftButton->Location				= TPoint( 10, 335 );
	AddLeftButton->EventClick			= TNotifyEvent( this, (TNotifyEvent::TEvent)&WTileEditor::ButtonAddLeftClick );

	RemoveLeftButton					= new WButton( this, Root );
	RemoveLeftButton->SetSize( 24, 24 );
	RemoveLeftButton->Caption			= L"-";
	RemoveLeftButton->Location			= TPoint( 38, 335 );
	RemoveLeftButton->EventClick		= TNotifyEvent( this, (TNotifyEvent::TEvent)&WTileEditor::ButtonRemoveLeftClick );

	AddRightButton						= new WButton( this, Root );
	AddRightButton->SetSize( 24, 24 );
	AddRightButton->Caption				= L"+";
	AddRightButton->Location			= TPoint( 214, 335 );
	AddRightButton->EventClick			= TNotifyEvent( this, (TNotifyEvent::TEvent)&WTileEditor::ButtonAddRightClick );

	RemoveRightButton					= new WButton( this, Root );
	RemoveRightButton->SetSize( 24, 24 );
	RemoveRightButton->Caption			= L"-";
	RemoveRightButton->Location			= TPoint( 242, 335 );
	RemoveRightButton->EventClick		= TNotifyEvent( this, (TNotifyEvent::TEvent)&WTileEditor::ButtonRemoveRightClick );

	LayerCombo							= new WComboBox( this, Root );
	LayerCombo->SetLocation( 10, 294 );
	LayerCombo->SetSize( 256, 18 );
	LayerCombo->AddItem( L"Layer #1", nullptr );
	LayerCombo->AddItem( L"Layer #2", nullptr );
	LayerCombo->ItemIndex				= 0;

	// Hide by default.
	Hide();
}


//
// Setup editor for model.
//
void WTileEditor::SetModel( FModelComponent* InModel )
{
	// Modify old model.
	if( Model )
		Model->PenIndex = -1;

	Model = InModel;

	// Change form caption according to model.
	if( Model && Model->Entity )
		Caption = String::Format( L"Model Editor [%s]", *Model->Entity->GetName() );
	else
		Caption = String::Format( L"Model Editor" );

	SetButtonsEnabled( Model != nullptr ); 
}


//
// Change buttons state.
//
void WTileEditor::SetButtonsEnabled( Bool InbEnabled )
{
	AddUpButton->bEnabled		= InbEnabled;
	AddLeftButton->bEnabled		= InbEnabled;
	AddRightButton->bEnabled	= InbEnabled;
	AddDownButton->bEnabled		= InbEnabled;
	RemoveUpButton->bEnabled	= InbEnabled;
	RemoveLeftButton->bEnabled	= InbEnabled;
	RemoveRightButton->bEnabled	= InbEnabled;
	RemoveDownButton->bEnabled	= InbEnabled;
}


//
// Hide the tile editor.
//
void WTileEditor::Hide()
{
	// Hide form.
	WForm::Hide();
	SetModel( nullptr );
}


//
// Show the tile editor.
//
void WTileEditor::Show( Int32 X, Int32 Y )
{
	// Show form.
	WForm::Show( X, Y );

	// Update info.
	SetModel( nullptr );
}


/*-----------------------------------------------------------------------------
    Model reallocation functions.
-----------------------------------------------------------------------------*/

//
// Remove the right column from the model.
//
void WTileEditor::ButtonRemoveRightClick( WWidget* Sender )
{
	if( Model && Model->MapXSize > 1 )
	{
		UInt16* Buffer = new UInt16[Model->MapXSize*Model->MapYSize]();
		MemCopy( Buffer, &Model->Map[0], Model->MapXSize*Model->MapYSize*sizeof(UInt16) );

		Model->MapXSize = Max( Model->MapXSize - 1, 1 );
		Model->Map.SetNum( Model->MapXSize * Model->MapYSize );
		MemZero( &Model->Map[0], Model->MapXSize*Model->MapYSize*sizeof(UInt16) );

		for( Int32 y=0; y<Model->MapYSize; y++ )
			MemCopy
					(
						&Model->Map[y * Model->MapXSize],
						&Buffer[y * (Model->MapXSize+1)],
						Model->MapXSize * sizeof(UInt16)
					);

		delete[] Buffer;
	}
}


//
// Add a new column to the right of the model.
//
void WTileEditor::ButtonAddRightClick( WWidget* Sender )
{
	if( Model )
	{
		UInt16* Buffer = new UInt16[Model->MapXSize*Model->MapYSize]();
		MemCopy( Buffer, &Model->Map[0], Model->MapXSize*Model->MapYSize*sizeof(UInt16) );

		Model->MapXSize = Min<Int32>( Model->MapXSize + 1, FModelComponent::MAX_TILES_SIDE );
		Model->Map.SetNum( Model->MapXSize * Model->MapYSize );
		MemZero( &Model->Map[0], Model->MapXSize*Model->MapYSize*sizeof(UInt16) );

		for( Int32 y=0; y<Model->MapYSize; y++ )
			MemCopy
					( 
						&Model->Map[y*Model->MapXSize], 
						&Buffer[y * (Model->MapXSize-1)], 
						(Model->MapXSize-1) * sizeof(UInt16) 
					);

		delete[] Buffer;
	}
}


//
// Remove the left column from the model.
//
void WTileEditor::ButtonRemoveLeftClick( WWidget* Sender )
{
	if( Model && Model->MapXSize > 1 )
	{
		UInt16* Buffer = new UInt16[Model->MapXSize*Model->MapYSize]();
		MemCopy( Buffer, &Model->Map[0], Model->MapXSize*Model->MapYSize*sizeof(UInt16) );

		Model->MapXSize = Max( Model->MapXSize - 1, 1 );
		Model->Map.SetNum( Model->MapXSize * Model->MapYSize );
		MemZero( &Model->Map[0], Model->MapXSize*Model->MapYSize*sizeof(UInt16) );

		for( Int32 y=0; y<Model->MapYSize; y++ )
			MemCopy
					(
						&Model->Map[y * Model->MapXSize],
						&Buffer[y * (Model->MapXSize+1) + 1],
						Model->MapXSize * sizeof(UInt16)
					);

		delete[] Buffer;

#if SAVE_TILE_LOC
		Model->Location.X += Model->TileSize.X;
#endif
	}
}


//
// Add a new column to the left of the model.
//
void WTileEditor::ButtonAddLeftClick( WWidget* Sender )
{
	if( Model && Model->MapXSize < FModelComponent::MAX_TILES_SIDE )
	{
		UInt16* Buffer = new UInt16[Model->MapXSize*Model->MapYSize]();
		MemCopy( Buffer, &Model->Map[0], Model->MapXSize*Model->MapYSize*sizeof(UInt16) );

		Model->MapXSize = Min<Int32>( Model->MapXSize + 1, FModelComponent::MAX_TILES_SIDE );
		Model->Map.SetNum( Model->MapXSize * Model->MapYSize );
		MemZero( &Model->Map[0], Model->MapXSize*Model->MapYSize*sizeof(UInt16) );

		for( Int32 y=0; y<Model->MapYSize; y++ )
			MemCopy
					( 
						&Model->Map[1 + y*Model->MapXSize], 
						&Buffer[y * (Model->MapXSize-1)], 
						(Model->MapXSize-1) * sizeof(UInt16) 
					);

		delete[] Buffer;

#if SAVE_TILE_LOC
		Model->Location.X -= Model->TileSize.X;
#endif
	}
}


//
// Remove the down row from the model.
//
void WTileEditor::ButtonRemoveDownClick( WWidget* Sender )
{
	if( Model && Model->MapYSize > 1 )
	{
		MemCopy
				(
					&Model->Map[0],
					&Model->Map[Model->MapXSize],
					Model->MapXSize * Model->MapYSize * sizeof(UInt16)
				);

		Model->MapYSize = Max( Model->MapYSize - 1, 1 );
		Model->Map.SetNum( Model->MapXSize * Model->MapYSize );

#if SAVE_TILE_LOC
		Model->Location.Y += Model->TileSize.Y;
#endif
	}
}


//
// Add a new row to the down of the model.
//
void WTileEditor::ButtonAddDownClick( WWidget* Sender )
{
	if( Model && Model->MapYSize < FModelComponent::MAX_TILES_SIDE )
	{
		Model->MapYSize = Min<Int32>( Model->MapYSize + 1, FModelComponent::MAX_TILES_SIDE );
		Model->Map.SetNum( Model->MapXSize * Model->MapYSize );

		MemCopy
				( 
					&Model->Map[Model->MapXSize],
					&Model->Map[0],
					Model->MapXSize * (Model->MapYSize-1) * sizeof(UInt16)
				);

		MemZero
				( 
					&Model->Map[0],
					Model->MapXSize * sizeof(UInt16)
				);

#if SAVE_TILE_LOC
		Model->Location.Y -= Model->TileSize.Y;
#endif
	}
}


//
// Remove the up row from the model.
//
void WTileEditor::ButtonRemoveUpClick( WWidget* Sender )
{
	if( Model )
	{
		Model->MapYSize = Max( Model->MapYSize - 1, 1 );
		Model->Map.SetNum( Model->MapXSize * Model->MapYSize );
	}
}


//
// Add a new row to the up of the model.
//
void WTileEditor::ButtonAddUpClick( WWidget* Sender )
{
	if( Model )
	{
		Model->MapYSize = Min<Int32>( Model->MapYSize + 1, FModelComponent::MAX_TILES_SIDE );
		Model->Map.SetNum( Model->MapXSize * Model->MapYSize );
	}
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/