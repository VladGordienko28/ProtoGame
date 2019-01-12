/*=============================================================================
	FrBitPage.h: Bitmap page.
	Created by Vlad Gordienko, Jul. 2016.
	Material support and refactoring by Vlad, Feb. 2018.
=============================================================================*/

/*-----------------------------------------------------------------------------
	WDemoEffectPanel.
-----------------------------------------------------------------------------*/

//
// Panel with demo-effect parameters.
//
class WDemoEffectPanel: public WPanel
{
public:
	// Maximum parameters per effect type.
	enum{ MAX_EFFECT_PARAMS = 4 };

	// Variables.
	FDemoBitmap*	Bitmap;

	// WDemoEffectPanel interface.
	WDemoEffectPanel( FDemoBitmap* InBitmap, WContainer* InOwner, WWindow* InRoot );
	~WDemoEffectPanel();

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );

private:
	// An information about effect parameter.
	struct TParam
	{
		String	Name;
		Byte*	Address;
	};

	// Internal.
	WSlider*		Sliders[MAX_EFFECT_PARAMS];
	WLabel*			Labels[MAX_EFFECT_PARAMS];
	TParam			Params[MAX_EFFECT_PARAMS];

	WComboBox*		DrawType;

	// Fast access parameters.
	union
	{
		FFireBitmap::TFireDrawParams*	FireParams;
		FWaterBitmap::TWaterDrawParams*	WaterParams;
		FTechBitmap::TTechDrawParams*	TechParams;
	};

	// Notifications.
	void DrawTypeChange( WWidget* Sender );
	void AnySliderChange( WWidget* Sender );
};


/*-----------------------------------------------------------------------------
	WMaterialPanel.
-----------------------------------------------------------------------------*/

//
// Panel with material layers.
//
class WMaterialPanel: public WPanel
{
public:
	// Maximum layers in material.
	enum{ MAX_LAYERS = 4 };

	// Variables.
	FMaterial*	Material;

	// WMaterialPanel interface.
	WMaterialPanel( FMaterial* InMaterial, WContainer* InOwner, WWindow* InRoot );
	~WMaterialPanel();

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );
	void OnDblClick( EMouseButton Button, Integer X, Integer Y );   
	void OnMouseDown( EMouseButton Button, Integer X, Integer Y );

private:
	// Internal.
	WToolBar*			ToolBar;
	WPictureButton*		AddLayerButton;
	WPictureButton*		RemoveLayerButton;
	WPictureButton*		ToUpButton;
	WPictureButton*		ToDownButton;
	WPopupMenu*			TypePopup;

	// Variables.
	Integer			iSelected;

	// Notifications.
	void ButtonAddLayerClick( WWidget* Sender );
	void ButtonRemoveLayerClick( WWidget* Sender );
	void ButtonToUpClick( WWidget* Sender );
	void ButtonToDownClick( WWidget* Sender );
	void PopupTypeClick( WWidget* Sender );

	// Helpers.
	void UpdateButtons();
};


/*-----------------------------------------------------------------------------
	WTexturePage.
-----------------------------------------------------------------------------*/

//
// Texture editor page.
//
class WTexturePage: public WEditorPage, public CRefsHolder
{
public:
	// Variables.
	FTexture*			Texture;
	WPanel*				SidePanel;

	// WBitmapPage interface.
	WTexturePage( FTexture* InTexture, WContainer* InOwner, WWindow* InRoot );
	~WTexturePage();

	// WTabPage interface.
	Bool OnQueryClose();
	void OnOpen();

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );				// Render page content!!!!! better !!!!
	void OnMouseDown( EMouseButton Button, Integer X, Integer Y );
	void OnMouseUp( EMouseButton Button, Integer X, Integer Y );
	void OnMouseMove( EMouseButton Button, Integer X, Integer Y );
	void OnResize();

	// WEditorPage interface.
	void TickPage( Float Delta );
	FResource* GetResource()
	{ 
		return Texture; 
	}

private:
	// Internal.
	enum EDragMode
	{
		DRAG_None,
		DRAG_Panning,
		DRAG_Drawing
	};

	EDragMode			DragMode;
	TPoint				DragFrom;
	Float				Scale;
	TPoint				Pan;				// TViewInfo!!!
	Bool				bMouseMove;

	// Internal widgets.
	WToolBar*			ToolBar;
	WButton*			EditButton;			// Toggle!!!!!!!!!!!!!!!!!!!!!!!!
	WButton*			EraseButton;
	WButton*			ZoomInButton;		
	WButton*			ZoomOutButton;		

	// Widgets notifications.
	void ButtonEditClick( WWidget* Sender );
	void ButtonEraseClick( WWidget* Sender );
	void ButtonZoomInClick( WWidget* Sender );
	void ButtonZoomOutClick( WWidget* Sender );

public:
	// CRefsHolder interface.
	void CountRefs( CSerializer& S )
	{
		Serialize( S, Texture );

		if( !Texture )
			this->Close( true );
	}
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/