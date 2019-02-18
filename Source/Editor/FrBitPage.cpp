/*=============================================================================
	FrBitPage.cpp: Bitmap page.
	Created by Vlad Gordienko, Jul. 2016.
	Material support by Vlad, Feb. 2018.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    WTexturePage implementation.
-----------------------------------------------------------------------------*/

//
// Texture page constructor.
//
WTexturePage::WTexturePage( FTexture* InTexture, WContainer* InOwner, WWindow* InRoot )
	:	WEditorPage( InOwner, InRoot ),
		Scale( 1.f ),
		Pan( 0, 0 ),
		DragMode( DRAG_None ),
		DragFrom( 0, 0 ),
		bMouseMove( false ),
		SidePanel( nullptr )

{
	// Initialize own fields.
	Padding		= TArea( 0, 0, 0, 0 );
	Texture		= InTexture;
	Caption		= InTexture->GetName();
	TabWidth	= Root->Font1->TextWidth( *Caption ) + 30;
	PageType	= PAGE_Texture;
	Color		= PAGE_COLOR_TEXTURE;

	// Create toolbar and buttons.
	ToolBar = new WToolBar( this, Root );
	ToolBar->SetSize( 3000, 28 );

	EditButton				= new WButton( ToolBar, Root );
	EditButton->Caption		= L"Edit";
	EditButton->Tooltip		= InTexture->IsA(FMaterial::MetaClass) ? L"Open Layers Editor" : L"Open Effects Editor";
	EditButton->bEnabled	= false;
	EditButton->bToggle		= true;
	EditButton->bDown		= false;
	EditButton->EventClick	= WIDGET_EVENT(WTexturePage::ButtonEditClick);
	EditButton->SetSize( 70, 22 );
	ToolBar->AddElement( EditButton );
	ToolBar->AddElement( nullptr );

	EraseButton				= new WButton( ToolBar, Root );
	EraseButton->Caption	= L"Erase";
	EraseButton->Tooltip	= L"Erase Effects";
	EraseButton->bEnabled	= false;
	EraseButton->EventClick = WIDGET_EVENT(WTexturePage::ButtonEraseClick);
	EraseButton->SetSize( 70, 22 );
	ToolBar->AddElement( EraseButton );
	ToolBar->AddElement( nullptr );

	ZoomInButton				= new WButton( ToolBar, Root );
	ZoomInButton->Caption		= L"+";
	ZoomInButton->Tooltip		= L"Zoom In";
	ZoomInButton->EventClick	= WIDGET_EVENT(WTexturePage::ButtonZoomInClick);	
	ZoomInButton->SetSize( 25, 22 );
	ToolBar->AddElement( ZoomInButton );

	ZoomOutButton				= new WButton( ToolBar, Root );
	ZoomOutButton->Caption		= L"-";
	ZoomOutButton->Tooltip		= L"Zoom Out";
	ZoomOutButton->EventClick	= WIDGET_EVENT(WTexturePage::ButtonZoomOutClick);
	ZoomOutButton->SetSize( 25, 22 );
	ToolBar->AddElement( ZoomOutButton );

	// Create the parameters side panel.
	if( Texture->IsA(FMaterial::MetaClass) )
	{
		// Material panel.
		SidePanel	= new WMaterialPanel
		(
			As<FMaterial>( Texture ),
			this,
			Root
		);

		// Turn on widgets.
		EraseButton->bEnabled	= false;
		EditButton->bEnabled	= true;
		EditButton->bDown		= true;
	}		
	else if	( 
				Texture->IsA(FFireBitmap::MetaClass) ||
				Texture->IsA(FTechBitmap::MetaClass) ||
				Texture->IsA(FWaterBitmap::MetaClass) 
			)
	{
		// Editable demo-effect.
		SidePanel	= new WDemoEffectPanel
		(
			As<FDemoBitmap>( Texture ),
			this,
			Root
		);

		// Turn on widgets.
		EraseButton->bEnabled	= true;
		EditButton->bEnabled	= true;
		EditButton->bDown		= true;
	}
}


//
// Bitmap page destructor.
//
WTexturePage::~WTexturePage()
{ 
}


//
// Tick the page.
//
void WTexturePage::TickPage( Float Delta )
{
}


//
// Ask user for page closing.
//
Bool WTexturePage::OnQueryClose()
{
	// Ok, let user close the page without
	// any asking.
	return true;
}


//
// Open an effect editor form.
//
void WTexturePage::ButtonEditClick( WWidget* Sender )
{
	// Toggle side panel.
	if( SidePanel )
		SidePanel->bVisible = EditButton->bDown;
}


//
// Erase temporal bitmap effects.
//
void WTexturePage::ButtonEraseClick( WWidget* Sender )
{
	FBitmap* Bitmap = As<FBitmap>(Texture);
	if( Bitmap )
		Bitmap->Erase();
}


//
// User has down on bitmap page.
//
void WTexturePage::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y )
{
	WEditorPage::OnMouseDown( Button, X, Y );

	// Figure out drag mode.
	if( Texture->IsA(FDemoBitmap::MetaClass) )
	{
		// Drawing bitmap.
		TPoint BitPos	= TPoint
							(	
								(Pan.X + X - (Size.Width-Texture->USize*Scale)/2)/Scale, 
								(Pan.Y + Y - (Size.Height-Texture->VSize*Scale)/2)/Scale  
							);

		if	(	
				BitPos.X >= 0 && 
				BitPos.X < Texture->USize && 
				BitPos.Y >= 0 && 
				BitPos.Y < Texture->VSize 
			)
		{
			// Cursor inside bitmap.
			DragMode	= DRAG_Drawing;
		}
		else
		{
			// Cursor outside bitmap.
			if( Button == MB_Left )
			{
				DragMode	= DRAG_Panning;
				DragFrom	= TPoint( X, Y );
			}
		}
	}
	else
	{
		// Simple bitmap, always panning it.
		if( Button == MB_Left )
		{
			DragMode	= DRAG_Panning;
			DragFrom	= TPoint( X, Y );
		}
	}

	// Start track for click.
	bMouseMove	= false;
}


//
// User has release mouse button.
//
void WTexturePage::OnMouseUp( EMouseButton Button, Int32 X, Int32 Y )
{
	WEditorPage::OnMouseUp( Button, X, Y );

	// Notify about click?
	if( !bMouseMove && Texture->IsA(FDemoBitmap::MetaClass) && DragMode == DRAG_Drawing )
	{
		// Figure out mouse location onto bitmap.
		FDemoBitmap* DemoBitmap = As<FDemoBitmap>(Texture);

		TPoint BitPos	= TPoint
							(	
								(Pan.X + X - (Size.Width-DemoBitmap->USize*Scale)/2)/Scale, 
								(Pan.Y + Y - (Size.Height-DemoBitmap->VSize*Scale)/2)/Scale  
							);

		// Test bounds and notify.
		if	(	BitPos.X >= 0 && 
				BitPos.X < DemoBitmap->USize && 
				BitPos.Y >= 0 && 
				BitPos.Y < DemoBitmap->VSize 
			)
				DemoBitmap->MouseClick( Button, BitPos.X, BitPos.Y );
	}

	// Cleanup.
	DragMode	= DRAG_None;
	bMouseMove	= false;
}


//
// Mouse move bitmap.
//
void WTexturePage::OnMouseMove( EMouseButton Button, Int32 X, Int32 Y )
{
	WEditorPage::OnMouseMove( Button, X, Y );

	// Mark it.
	bMouseMove	= true;

	// Are we inside bitmap?
	TPoint BitPos	= TPoint
						(	
							(Pan.X + X - (Size.Width-Texture->USize*Scale)/2)/Scale, 
							(Pan.Y + Y - (Size.Height-Texture->VSize*Scale)/2)/Scale  
						);

	if( DragMode == DRAG_Panning )
	{
		// Panning the bitmap.
		Pan.X		-= X - DragFrom.X;
		Pan.Y		-= Y - DragFrom.Y;
		DragFrom	= TPoint( X, Y );
	}

	if	(	
			BitPos.X >= 0 && 
			BitPos.X < Texture->USize && 
			BitPos.Y >= 0 && 
			BitPos.Y < Texture->VSize 
		)
	{
		// We are inside bitmap, but we can draw?
		if( DragMode == DRAG_Drawing || Button == MB_None )
			if( Texture->IsA(FBitmap::MetaClass) )
				((FBitmap*)Texture)->MouseMove( Button, BitPos.X, BitPos.Y );

		// Update status bar.
		GEditor->StatusBar->Panels[0].Text	= String::Format( L"U: %d", BitPos.X );
		GEditor->StatusBar->Panels[1].Text	= String::Format( L"V: %d", BitPos.Y );
	}
	else
	{
		// Update status bar.
		GEditor->StatusBar->Panels[0].Text	= L"U: n/a";
		GEditor->StatusBar->Panels[1].Text	= L"V: n/a";
	}
}


//
// When page has been opened/reopened.
//
void WTexturePage::OnOpen()
{
	// Let inspector show bitmap's properties.
	if( Texture )
		GEditor->Inspector->SetEditObject( Texture );
}


//
// Zoom in bitmap.
//
void WTexturePage::ButtonZoomInClick( WWidget* Sender )
{
	Scale = Min( 4.f, Scale * 2.f );
}


//
// Zoom out bitmap.
//
void WTexturePage::ButtonZoomOutClick( WWidget* Sender )
{
	Scale = Max( 0.125f, Scale * 0.5f );
}


//
// Draw the bitmap page.
//
void WTexturePage::OnPaint( CGUIRenderBase* Render )
{
	TPoint Base = ClientToWindow(TPoint::Zero);

	// Clip to page.
	Render->SetClipArea( Base, Size );

	// Draw cool backdrop, with pattern.
	Render->DrawRegion
				( 
					Base, 
					TSize( Size.Width, Size.Height ), 
					TColor( 0x30, 0x30, 0x30, 0xff ), 
					TColor( 0x30, 0x30, 0x30, 0xff ), 
					BPAT_Solid 
				);

	Render->DrawRegion
				( 
					Base, 
					TSize( Size.Width, Size.Height ), 
					TColor( 0x3f, 0x3f, 0x3f, 0xff ), 
					TColor( 0x3f, 0x3f, 0x3f, 0xff ), 
					BPAT_PolkaDot 
				);

	// Draw bitmap.
	{
		Int32 X = Base.X - Pan.X + ( Size.Width - Texture->USize * Scale ) / 2;
		Int32 Y = Base.Y - Pan.Y + ( Size.Height - Texture->VSize * Scale ) / 2;
		Int32 W = Texture->USize * Scale;
		Int32 H = Texture->VSize * Scale;

		// Draw border.
		Render->DrawRegion
					( 
						TPoint( X, Y-1 ), 
						TSize( W+1, H+1 ), 
						COLOR_Black, 
						COLOR_Black, 
						BPAT_None 
					);

		// Draw image.
		Render->DrawPicture
						( 
							TPoint( X, Y ), 
							TSize( W, H ), 
							TPoint( 0, 0 ), 
							TSize( Texture->USize, Texture->VSize ), 
							Texture  
						);
	}

	// Draw bitmap's zoom.
	Render->DrawText
				( 
					TPoint( Base.X + 10, Base.Y + 38 ), 
					String::Format( L"x%.2f", Scale ), 
					COLOR_White, 
					Root->Font1 
				);
}


void WTexturePage::OnResize()
{
	WEditorPage::OnResize();

	if( SidePanel )
	{
		SidePanel->SetLocation
		(
			Size.Width - SidePanel->Size.Width,
			ToolBar->Size.Height
		);
	}
}


/*-----------------------------------------------------------------------------
	WDemoEffectPanel implementation.
-----------------------------------------------------------------------------*/

//
// Bitmap edit form constructor.
//
WDemoEffectPanel::WDemoEffectPanel( FDemoBitmap* InBitmap, WContainer* InOwner, WWindow* InRoot )
	:	WPanel( InOwner, InRoot ),
		Bitmap( InBitmap )
{
	// Initialize form.
	Caption = String::Format( L"%s Editor", *Bitmap->GetClass()->GetAltName() );
	SetSize( 355, 145 );
	SetLocation( 0, 0 );

	// ComboBox.
	DrawType = new WComboBox( this, Root );
	DrawType->Location = TPoint( 10, 28 );
	DrawType->SetSize( 280+55, 18 );
	DrawType->EventChange = WIDGET_EVENT(WDemoEffectPanel::DrawTypeChange);

	if( Bitmap->IsA(FFireBitmap::MetaClass) )
	{
		// Fire draw types.
		CEnum*	Enum = CClassDatabase::StaticFindEnum( L"ESparkType" );
		assert(Enum != nullptr);

		for( Int32 i=0; i<Enum->Elements.size(); i++ )
			DrawType->AddItem( Enum->GetAliasOf(i), nullptr );

		// Set default params.
		FireParams				= &((FFireBitmap*)InBitmap)->DrawParams;
		FireParams->Area		= 240;
		FireParams->Direction	= 0;
		FireParams->DrawType	= FFireBitmap::SPARK_Fireball;
		FireParams->Frequency	= 16;
		FireParams->Heat		= 255;
		FireParams->Life		= 100;
		FireParams->Size		= 64;
		FireParams->Speed		= 2;

		DrawType->ItemIndex		= FireParams->DrawType;
	}
	else if( Bitmap->IsA(FWaterBitmap::MetaClass) )
	{
		// Water draw types.
		CEnum*	Enum = CClassDatabase::StaticFindEnum( L"EDropType" );
		assert(Enum != nullptr);

		for( Int32 i=0; i<Enum->Elements.size(); i++ )
			DrawType->AddItem( Enum->GetAliasOf(i), nullptr );

		// Set default params.
		WaterParams				= &((FWaterBitmap*)InBitmap)->DrawParams;
		WaterParams->Amplitude	= 255;
		WaterParams->Depth		= 255;
		WaterParams->DrawType	= FWaterBitmap::DROP_Oscillator;
		WaterParams->Frequency	= 8;
		WaterParams->Size		= 64;
		WaterParams->Speed		= 128;

		DrawType->ItemIndex		= WaterParams->DrawType;
	}
	else if( Bitmap->IsA(FTechBitmap::MetaClass) )
	{
		// Tech draw types.
		CEnum*	Enum = CClassDatabase::StaticFindEnum( L"EPanelType" );
		assert(Enum != nullptr);

		for( Int32 i=0; i<Enum->Elements.size(); i++ )
			DrawType->AddItem( Enum->GetAliasOf(i), nullptr );

		// Set default params.
		TechParams				= &((FTechBitmap*)InBitmap)->DrawParams;
		TechParams->DrawType	= FTechBitmap::TECH_Circle;
		TechParams->Depth		= 255;
		TechParams->Size		= 30;
		TechParams->Time		= 22;

		DrawType->ItemIndex		= TechParams->DrawType;
	}
	else
	{
		// Unsupported type.
		assert(false && "Unknown demo-effect");
	}

	// Allocate controls.
	for( Int32 i=0; i<MAX_EFFECT_PARAMS; i++ )
	{
		// Label.
		Labels[i] = new WLabel( this, Root );
		Labels[i]->Location = TPoint( 5, 55+i*20 );

		// Slider.
		Sliders[i] = new WSlider( this, Root );
		Sliders[i]->SetSize( 255, 12 );
		Sliders[i]->Location = TPoint( 90, 55+i*20 );
		Sliders[i]->SetOrientation(SLIDER_Horizontal);
		Sliders[i]->EventChange = WIDGET_EVENT(WDemoEffectPanel::AnySliderChange);

		// Params.
		Params[i].Address = nullptr;
		Params[i].Name = L"";
	}

	// Setup default params.
	DrawType->OnChange();
}


//
// Panel destructor.
//
WDemoEffectPanel::~WDemoEffectPanel()
{
}


//
// When any of slider changed.
//
void WDemoEffectPanel::AnySliderChange( WWidget* Sender )
{
	for( Int32 i=0; i<MAX_EFFECT_PARAMS; i++ )
		if( Params[i].Address && Sliders[i] == Sender )
		{
			*Params[i].Address = math::round(Sliders[i]->Value*255.f/100.f);
			Labels[i]->Caption = String::Format( L"%s %d", *Params[i].Name, *Params[i].Address );
		}
}


//
// Redraw panel.
//
void WDemoEffectPanel::OnPaint( CGUIRenderBase* Render )
{
	WPanel::OnPaint(Render);
	TPoint Base = ClientToWindow(TPoint::Zero);

	// Draw header.
	Render->DrawRegion
	(
		TPoint( Base.X+1, Base.Y ),
		TSize( Size.Width-1, FORM_HEADER_SIZE ),
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
// Change sliders.
//
void WDemoEffectPanel::DrawTypeChange( WWidget* Sender )
{
	if( Bitmap->IsA(FFireBitmap::MetaClass) )
	{
		// Table of parameters.
		TParam DefaultFireParams[][MAX_EFFECT_PARAMS] =
		{
			{ {L"Heat", &FireParams->Heat} }, // SPARK_Point;
			{ {L"Heat", &FireParams->Heat} }, // SPARK_RandomPoint;
			{ {L"Heat", &FireParams->Heat}, {L"Freq", &FireParams->Frequency} }, // SPARK_Phase;
			{ {L"Heat", &FireParams->Heat}, {L"Size", &FireParams->Size}, {L"Freq", &FireParams->Frequency}, {L"Speed", &FireParams->Speed} }, // SPARK_Jitter;
			{ {L"Heat", &FireParams->Heat}, {L"Size", &FireParams->Size}, {L"Freq", &FireParams->Frequency}, {L"Speed", &FireParams->Speed} }, // SPARK_Twister;
			{ {L"Heat", &FireParams->Heat} }, // SPARK_Fireball;
			{ {L"Heat", &FireParams->Heat}, {L"Life", &FireParams->Life} }, // SPARK_JetUpward;
			{ {L"Heat", &FireParams->Heat}, {L"Life", &FireParams->Life} }, // SPARK_JetLeftward;
			{ {L"Heat", &FireParams->Heat}, {L"Life", &FireParams->Life} }, // SPARK_JetRightward;
			{ {L"Heat", &FireParams->Heat}, {L"Life", &FireParams->Life} }, // SPARK_JetDownward;
			{ {L"Heat", &FireParams->Heat}, {L"Life", &FireParams->Life} }, // SPARK_Spermatozoa;
			{ {L"Heat", &FireParams->Heat}, {L"Area", &FireParams->Area}, {L"Freq", &FireParams->Frequency}, {L"Life", &FireParams->Life} }, // SPARK_Whirligig;
			{ {L"Life", &FireParams->Life}, {L"Area", &FireParams->Area}, {L"Direct", &FireParams->Direction}, {L"Speed", &FireParams->Speed} }, // SPARK_Cloud;
			{ {L"Heat", &FireParams->Heat}, {L"Freq", &FireParams->Frequency} }, // SPARK_LineLighting;
			{ {L"Heat", &FireParams->Heat}, {L"Freq", &FireParams->Frequency} }, // SPARK_RampLighting;
			{ {L"Heat", &FireParams->Heat}, {L"Freq", &FireParams->Frequency} }, // SPARK_RandomLighting;
			{ {L"Heat", &FireParams->Heat}, {L"Freq", &FireParams->Frequency}, {L"Size", &FireParams->Size} }, // SPARK_BallLighting;
		};

		FireParams->DrawType = (FFireBitmap::ESparkType)DrawType->ItemIndex;
		for( Int32 i=0; i<MAX_EFFECT_PARAMS; i++ )
			Params[i] = DefaultFireParams[DrawType->ItemIndex][i];
	}
	else if( Bitmap->IsA(FWaterBitmap::MetaClass) )
	{
		// Table of parameters.
		TParam DefaultWaterParams[][MAX_EFFECT_PARAMS] =
		{
			{ {L"Depth", &WaterParams->Depth} }, // DROP_Point;
			{ {L"Ampl", &WaterParams->Amplitude} }, // DROP_RandomPoint;
			{ {L"Depth", &WaterParams->Depth}, {L"Freq", &WaterParams->Frequency} }, // DROP_Tap;
			{ {L"Depth", &WaterParams->Depth}, {L"Speed", &WaterParams->Speed} }, // DROP_Surfer;
			{ {L"Depth", &WaterParams->Depth}, {L"Freq", &WaterParams->Frequency} }, // DROP_RainDrops;
			{ {L"Ampl", &WaterParams->Amplitude}, {L"Freq", &WaterParams->Frequency} }, // DROP_Oscillator;
			{ {L"Ampl", &WaterParams->Amplitude}, {L"Freq", &WaterParams->Frequency}, {L"Size", &WaterParams->Size} }, // DROP_VertLine;
			{ {L"Ampl", &WaterParams->Amplitude}, {L"Freq", &WaterParams->Frequency}, {L"Size", &WaterParams->Size} }, // DROP_HorizLine;
		};

		WaterParams->DrawType = (FWaterBitmap::EDropType)DrawType->ItemIndex;
		for( Int32 i=0; i<MAX_EFFECT_PARAMS; i++ )
			Params[i] = DefaultWaterParams[DrawType->ItemIndex][i];
	}
	else if( Bitmap->IsA(FTechBitmap::MetaClass) )
	{
		// Table of parameters.
		TParam DefaultTechParams[][MAX_EFFECT_PARAMS] =
		{
			{ {L"Depth", &TechParams->Depth} }, // TECH_Ivy;
			{ {L"Size", &TechParams->Size} }, // TECH_Circle;
			{ {L"Depth", &TechParams->Depth}, {L"Size", &TechParams->Size} }, // TECH_Straight;
			{ {L"Depth", &TechParams->Depth}, {L"Size", &TechParams->Size} }, // TECH_Segments;
			{ {L"Time", &TechParams->Time} }, // TECH_Grinder;
			{ {L"Time", &TechParams->Time} }, // TECH_Noisy;
			{ {L"Depth", &TechParams->Depth}, {L"Size", &TechParams->Size} } // TECH_Wavy;
		};
		
		TechParams->DrawType = (FTechBitmap::EPanelType)DrawType->ItemIndex;
		for( Int32 i=0; i<MAX_EFFECT_PARAMS; i++ )
			Params[i] = DefaultTechParams[DrawType->ItemIndex][i];
	}
	// Update widgets.
	for( Int32 i=0; i<MAX_EFFECT_PARAMS; i++ )
		if( Params[i].Address )
		{
			Sliders[i]->bVisible = true;
			Labels[i]->bVisible = true;
			Sliders[i]->SetValue(math::round((Float)(*Params[i].Address)*100.f/255.f));
		}
		else
		{
			Sliders[i]->bVisible = false;
			Labels[i]->bVisible = false;
		}
}


/*-----------------------------------------------------------------------------
	WMaterialPanel implementation.
-----------------------------------------------------------------------------*/

//
// Material edit form constructor.
//
WMaterialPanel::WMaterialPanel( FMaterial* InMaterial, WContainer* InOwner, WWindow* InRoot )
	:	WPanel( InOwner, InRoot ),
		Material( InMaterial )
{
	// Initialize form.
	Caption = L"Layers Editor";
	SetSize( 250, 250 );
	SetLocation( 0, 0 );
	Padding	= TArea( FORM_HEADER_SIZE+1, 0, 1, 1 );

	// Create toolbar and buttons.
	ToolBar = new WToolBar( this, Root );
	ToolBar->SetSize( 3000, 21 );

	AddLayerButton				= new WPictureButton( ToolBar, Root );
	AddLayerButton->Scale		= TSize( 16, 16 );
	AddLayerButton->Offset		= TPoint( 0, 128 );
	AddLayerButton->Picture		= Root->Icons;
	AddLayerButton->Tooltip		= L"Add New Layer";
	AddLayerButton->EventClick	= WIDGET_EVENT(WMaterialPanel::ButtonAddLayerClick);
	AddLayerButton->SetSize( 19, 19 );
	ToolBar->AddElement( AddLayerButton );

	RemoveLayerButton				= new WPictureButton( ToolBar, Root );
	RemoveLayerButton->Scale		= TSize( 16, 16 );
	RemoveLayerButton->Offset		= TPoint( 32, 128 );
	RemoveLayerButton->Picture		= Root->Icons;
	RemoveLayerButton->Tooltip		= L"Remove Selected Layer";
	RemoveLayerButton->EventClick	= WIDGET_EVENT(WMaterialPanel::ButtonRemoveLayerClick);
	RemoveLayerButton->SetSize( 19, 19 );
	ToolBar->AddElement( RemoveLayerButton );

	ToolBar->AddElement( nullptr );

	ToUpButton						= new WPictureButton( ToolBar, Root );
	ToUpButton->Scale				= TSize( 16, 16 );
	ToUpButton->Offset				= TPoint( 48, 128 );
	ToUpButton->Picture				= Root->Icons;
	ToUpButton->Tooltip				= L"Move Layer Up";
	ToUpButton->EventClick			= WIDGET_EVENT(WMaterialPanel::ButtonToUpClick);
	ToUpButton->SetSize( 19, 19 );
	ToolBar->AddElement( ToUpButton );

	ToDownButton					= new WPictureButton( ToolBar, Root );
	ToDownButton->Scale				= TSize( 16, 16 );
	ToDownButton->Offset			= TPoint( 64, 128 );
	ToDownButton->Picture			= Root->Icons;
	ToDownButton->Tooltip			= L"Move Layer Down";
	ToDownButton->EventClick		= WIDGET_EVENT(WMaterialPanel::ButtonToDownClick);
	ToDownButton->SetSize( 19, 19 );
	ToolBar->AddElement( ToDownButton );

	// PopUp menu with layers.
	TypePopup			= new WPopupMenu( Root, Root );
	for( Int32 i=0; i<CClassDatabase::GClasses.size(); i++ )
	{
		CClass* TestClass = CClassDatabase::GClasses[i];
		if( TestClass->IsA(FMaterialLayer::MetaClass) && !(TestClass->Flags & CLASS_Abstract) )
		{
			TypePopup->AddItem( TestClass->GetAltName(), WIDGET_EVENT(WMaterialPanel::PopupTypeClick) );
		}
	}

	UpdateButtons();
}



/////////////////////////////////////////////////



WMaterialPanel::~WMaterialPanel()
{
	// Because popup being root.
	delete TypePopup;
}


void WMaterialPanel::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y )
{
	WPanel::OnMouseDown( Button, X, Y );

	if( Button == MB_Left )
	{
			Int32 NewY = Y - 42;

			iSelected = Clamp( NewY/52, -1, Material->Layers.size()-1 );

		if( iSelected != -1 )
		{
			GEditor->Inspector->SetEditObject(Material->Layers[iSelected]);

		}

		/*
		iSelected = Random(Material->Layers.Num());

*/
	UpdateButtons();
	}
}

void WMaterialPanel::ButtonRemoveLayerClick( WWidget* Sender ){}




/*
//
// Panel with material layers.
//
class WMaterialPanel:: public WPanel
{
public:
	// Maximum layers in material.
	enum{ MAX_LAYERS = 4 };

	// Variables.
	FMaterial*	Material;

	// WMaterialPanel interface.



	WPictureButton*		AddLayerButton;
	WPictureButton*		RemoveLayerButton;
	WPictureButton*		ToTopButton;
	WPictureButton*		ToBottomButton;
	WPopupMenu*			TypePopup;

	// Variables.
	Integer			iSelected;

	// Notifications.

};*/

void WMaterialPanel::PopupTypeClick( WWidget* Sender )
{
	//@broken.
	Material->Layers.push
	(
		NewObject<FDiffuseLayer>(L"", Material)
	);
	UpdateButtons();
}



//
// Material panel paint.
//
void WMaterialPanel::OnPaint( CGUIRenderBase* Render )
{
	WPanel::OnPaint(Render);
	TPoint Base = ClientToWindow(TPoint::Zero);

	// Draw header.
	Render->DrawRegion
	(
		TPoint( Base.X+1, Base.Y ),
		TSize( Size.Width-1, FORM_HEADER_SIZE ),
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

	// Render layers.
	Int32 StartY = 42;
	Int32 StartX = 3;

	for( Int32 i=0; i<Material->Layers.size(); i++ )
	{
		FBitmap* RenderBitmap = nullptr;
		FMaterialLayer* Layer = Material->Layers[i];
		String Line;

		if( Layer->IsA(FDiffuseLayer::MetaClass) )
		{
			RenderBitmap = As<FDiffuseLayer>(Layer)->Bitmap;
			Line = Layer->GetName();
		}
		if( !RenderBitmap )
			RenderBitmap = FBitmap::NullBitmap();

		// Draw selection mark.
		if( i == iSelected )
		{
			Render->DrawRegion
			(
				Base + TPoint( StartX-2, StartY + 52*i - 1 ),
				TSize( Size.Width-2, 52 ),
				GUI_COLOR_LIST_SEL,
				GUI_COLOR_LIST_SEL,
				BPAT_Solid
			);
		}

		// Draw picture.
		Render->DrawPicture
		(
			Base + TPoint( StartX, StartY + 52 * i ),
			TSize( 50, 50 ),
			TPoint( 0, 0 ),
			TSize( RenderBitmap->USize, RenderBitmap->VSize ),
			RenderBitmap
		);
		Render->DrawText
		(
			Base + TPoint( StartX + 60, StartY + 52 * i + 16 ),
			Line,
			GUI_COLOR_TEXT,
			WWindow::Font1
		);

		// Mark master layer.
		if( Layer == Material->MainLayer )
			Render->DrawPicture
			(
				Base + TPoint( 226, StartY + 52 * i + 16 ),
				TSize( 16, 16 ),
				TPoint( 80, 32 ),
				TSize( 16, 16 ),
				WWindow::Icons
			);



	}



}

///////////////////////////////////////////////////////////


//
// Mark selected layer as main.
//
void WMaterialPanel::OnDblClick( EMouseButton Button, Int32 X, Int32 Y )
{
	if( Button == MB_Left && iSelected != -1 )
	{
		Material->MainLayer = Material->Layers[iSelected];
		Material->EditChange();
	}
}


//
// Bring selected layer down.
//
void WMaterialPanel::ButtonToDownClick( WWidget* Sender )
{
	assert(iSelected>=0 && iSelected<Material->Layers.size()-1);

	Material->Layers.swap( iSelected, iSelected+1 );
	iSelected++;

	UpdateButtons();
}


//
// Bring selected layer up.
//
void WMaterialPanel::ButtonToUpClick( WWidget* Sender )
{
	assert(iSelected>0 && iSelected<Material->Layers.size());

	Material->Layers.swap( iSelected, iSelected-1 );
	iSelected--;

	UpdateButtons();
}


//
// Add a new layer clicked.
//
void WMaterialPanel::ButtonAddLayerClick( WWidget* Sender )
{
	TypePopup->Show(Root->MousePos);
}


//
// Update buttons state.
//
void WMaterialPanel::UpdateButtons()
{
	AddLayerButton->bEnabled = Material->Layers.size() < MAX_LAYERS;
	RemoveLayerButton->bEnabled = iSelected >= 0 && iSelected < Material->Layers.size();
	ToUpButton->bEnabled = RemoveLayerButton->bEnabled && iSelected > 0;
	ToDownButton->bEnabled = RemoveLayerButton->bEnabled && iSelected < Material->Layers.size()-1;
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/