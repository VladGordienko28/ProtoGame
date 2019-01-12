/*=============================================================================
    FrLevPage.h: Level page.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    Declarations.
-----------------------------------------------------------------------------*/

// Roller size.
#define ROLLER_RADIUS	4.f


//
// An level tool.
//
enum ELevelTool
{
	LEV_Edit,
	LEV_PaintModel,
	LEV_Keyframe,
	LEV_PickEntity
};


//
// Helper class to select objects.
//
class TSelector
{
public:
	// Variables.
	FLevel*				Level;	
	TArray<FEntity*>	Selected;

	// TSelector interface.
	TSelector( FLevel* InLevel );
	~TSelector();
	void SelectEntity( FEntity* Entity, Bool bSelect );
	void UnselectAll();
	void SelectAll();
	void SelectByScript( FScript* InScript );
	String GetSelectionInfo() const;
};


/*-----------------------------------------------------------------------------
    WLevelPage.
-----------------------------------------------------------------------------*/

//
// Level editor page.
//
class WLevelPage: public WEditorPage, public CRefsHolder
{
public:
	// Variables.
	FLevel*				Level;
	CLevelTransactor*	Transactor;

	// Friends.
	friend WEntitySearchDialog;
	friend WObjectInspector;
	friend TSelector;
	friend TMouseDragInfo;
	friend class WEditorMainMenu;

	// WLevelPage interface.
	WLevelPage( FLevel* InLevel, WContainer* InOwner, WWindow* InRoot );
	~WLevelPage();
	void SetTool( ELevelTool NewTool );

	// WTabPage interface.
	Bool OnQueryClose();
	void OnOpen();

	// WWidget interface.
	void OnDblClick( EMouseButton Button, Int32 X, Int32 Y );
	void OnPaint( CGUIRenderBase* Render );
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseScroll( Int32 Delta );
	void OnDragOver( void* Data, Int32 X, Int32 Y, Bool& bAccept );
	void OnDragDrop( void* Data, Int32 X, Int32 Y );
	void OnKeyDown( Int32 Key );
	void OnKeyUp( Int32 Key );

	// WLevelPage events.
	virtual void OnMouseDrag( EMouseButton Button, Int32 X, Int32 Y, Int32 DeltaX, Int32 DeltaY );
	virtual void OnMouseBeginDrag( EMouseButton Button, Int32 X, Int32 Y );
	virtual void OnMouseEndDrag( EMouseButton Button, Int32 X, Int32 Y );
	virtual void OnMouseClick( EMouseButton Button, Int32 X, Int32 Y );

	// WEditorPage interface.
	void RenderPageContent( CCanvas* Canvas );
	void TickPage( Float Delta );
	void Undo();
	void Redo();
	FResource* GetResource()
	{ 
		return Level; 
	}

private:
	// A handle to stretch entity.
	enum EStretchHandle
	{
		STH_None,
		STH_NW,
		STH_N,
		STH_NE,
		STH_E,
		STH_SE,
		STH_S,
		STH_SW,
		STH_W,
		STH_MAX
	};

	// A logic element socket.
	enum ELogicSocket
	{
		LOGSOC_None,
		LOGSOC_Jack,
		LOGSOC_Plug
	};

	// A roller to rotate objects in level.
	struct TRoller
	{
	public:
		// Variables.
		Bool			bVisible;
		TVector			Position;
		TAngle			Angle;

		// Functions.
		TRoller();
		void Update( TSelector& Selector );
		void Draw( CCanvas* Canvas, Bool bHighlight );
	};

	// Internal.
	TSelector			Selector;
	TRoller				Roller;
	ELevelTool			Tool;
	Float				TranslationSnap;
	Int32				RotationSnap;

	// Internal widgets.
	WToolBar*			ToolBar;
	WPictureButton*		EditButton;
	WPictureButton*		PaintButton;
	WPictureButton*		KeyButton;
	WPictureButton*		RndFlagsButton;
	WPictureButton*		SearchDialogButton;
	WPictureButton*		BuildPathsButton;
	WPictureButton*		DestroyPathsButton;
	WComboBox*			DragSnapCombo;
	WPopupMenu*			BackdropPopup;
	WPopupMenu*			VertexPopup;
	WPopupMenu*			EntityPopup;
	WPopupMenu*			RndFlagsPopup;
	WTileEditor*		TileEditor;
	WKeyframeEditor*	KeyframeEditor;
	WForm*				EntitySearch;

	// Internal widgets events.
	void PopAddEntityClick( WWidget* Sender );
	void PopPasteClick( WWidget* Sender );
	void PopCopyClick( WWidget* Sender );
	void PopCutClick( WWidget* Sender );
	void PopSelectAllClick( WWidget* Sender );
	void PopDeleteClick( WWidget* Sender );
	void PopDuplicateClick( WWidget* Sender );
	void PopEditScriptClick( WWidget* Sender );
	void PopInsertVertexClick( WWidget* Sender );
	void PopRemoveVertexClick( WWidget* Sender );
	void PopCSGUnionClick( WWidget* Sender );
	void PopCSGIntersectionClick( WWidget* Sender );
	void PopCSGDifferenceClick( WWidget* Sender );
	void PopRndFlagClick( WWidget* Sender );
	void PopFreezeSelectedClick( WWidget* Sender );
	void PopUnfreezeAllClick( WWidget* Sender );
	void ButtonEditClick( WWidget* Sender );
	void ButtonKeyClick( WWidget* Sender );
	void ButtonPaintClick( WWidget* Sender );
	void ButtonSearchDialogClick( WWidget* Sender );
	void ButtonRndFlagsClick( WWidget* Sender );
	void ComboDragSnapChange( WWidget* Sender );
	void ButtonBuildPathsClick( WWidget* Sender );
	void ButtonDestroyPathsClick( WWidget* Sender );

	// Helper functions.
	TVector ScreenToWorld( Int32 X, Int32 Y );
	void WorldToScreen( TVector V, Float& OutX, Float& OutY );
	void UpdateInspector();
	void PaintModelAt( FModelComponent* Model, Int32 iLayer, Int32 X, Int32 Y );
	FEntity* GetEntityAt( Int32 X, Int32 Y, Bool bFast = false );
	Bool GetRollerAt( Int32 X, Int32 Y );
	Int32 GetVertexAt( Int32 X, Int32 Y, FBrushComponent*& OutBrush );
	EStretchHandle GetStretchHandleAt( Int32 X, Int32 Y, FBaseComponent*& OutBase );
	Int32 GetSocketAt( Int32 X, Int32 Y, ELogicSocket& SType, FLogicComponent*& L );
	FEntity* AddEntityTo( FScript* Script, Int32 X, Int32 Y );
	void DrawKeyframe( CCanvas* Canvas, FEntity* Entity );
	void DrawPathsNetwork( CCanvas* Canvas, CNavigator* Navigator );
	void DrawLogicCircuit( CCanvas* Canvas );
	void DrawScrollClamp( CCanvas* Canvas );

	// Click notifications.
	void ClickBackdrop( EMouseButton Button, Int32 X, Int32 Y );
	void ClickEntity( FEntity* Entity, EMouseButton Button, Int32 X, Int32 Y );
	void ClickVertex( FBrushComponent* Brush, Int32 iVert, EMouseButton Button, Int32 X, Int32 Y );

public:
	// CRefsHolder interface.
	void CountRefs( CSerializer& S )
	{
		Serialize( S, Level );
		Serialize( S, Selector.Level );
		Serialize( S, Selector.Selected );
		CLEANUP_ARR_NULL( Selector.Selected );
		if( !Level )
			this->Close( true );
	}
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/