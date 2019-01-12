/*=============================================================================
    FrTileEd.h: Model tile editor.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    WTileEditor.
-----------------------------------------------------------------------------*/

//
// Tiles editor.
//
class WTileEditor: public WForm, public CRefsHolder
{
public:
	// Grid of tiles to pick some.
	class WTilesGrid: public WWidget
	{
	public:
		// Variables.
		WTileEditor*		Editor;
		Bool				bCapture;

		// WWidget interface.
		WTilesGrid( WWindow* InRoot, WTileEditor* InEditor );
		void OnPaint( CGUIRenderBase* Render );
		void OnMouseDown( EMouseButton Button, Integer X, Integer Y );
		void OnMouseUp( EMouseButton Button, Integer X, Integer Y );
		void OnMouseMove( EMouseButton Button, Integer X, Integer Y );
		void OnDragOver( void* Data, Integer X, Integer Y, Bool& bAccept );
		void OnDragDrop( void* Data, Integer X, Integer Y );
	};

	// Variables.
	FModelComponent*	Model;
	WButton*			AddUpButton;
	WButton*			AddLeftButton;
	WButton*			AddRightButton;
	WButton*			AddDownButton;
	WButton*			RemoveUpButton;
	WButton*			RemoveLeftButton;
	WButton*			RemoveRightButton;
	WButton*			RemoveDownButton;
	WTilesGrid*			TilesGrid;
	WComboBox*			LayerCombo;

	// WTileEditor interface.
	WTileEditor( WContainer* InOwner, WWindow* InRoot );
	void SetModel( FModelComponent* InModel );
	void SetButtonsEnabled( Bool InbEnabled );

	// Controls notifications.
	void ButtonAddUpClick( WWidget* Sender );
	void ButtonAddLeftClick( WWidget* Sender );
	void ButtonAddRightClick( WWidget* Sender );
	void ButtonAddDownClick( WWidget* Sender );
	void ButtonRemoveUpClick( WWidget* Sender );
	void ButtonRemoveLeftClick( WWidget* Sender );
	void ButtonRemoveRightClick( WWidget* Sender );
	void ButtonRemoveDownClick( WWidget* Sender );

	// WForm interface.
	void Show( Integer X = 0, Integer Y = 0 );
	void Hide();

	// CRefsHolder interface.
	void CountRefs( CSerializer& S )
	{
		Serialize( S, Model );
		SetModel( Model );
	}
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/