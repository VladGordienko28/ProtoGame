/*=============================================================================
    FrPlayPage.h: Level test page.
    Copyright Aug.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    WPlayPage.
-----------------------------------------------------------------------------*/

//
// Playing mode.
//
enum EPlayMode
{
	PLAY_Debug,			// Play level with a debugging tools.
	PLAY_Release		// Play as in final game.
};


//
// Level play page.
//
class WPlayPage: public WEditorPage
{
public:
	// Variables.
	EPlayMode		PlayMode;

	// WPlayPage interface.
	WPlayPage( FLevel* InOrigianl, EPlayMode InPlayMode, WContainer* InOwner, WWindow* InRoot );
	~WPlayPage();
	void AddScriptMessage( ESeverity Severity, String Message );

	// WTabPage interface.
	void OnOpen();

	// WWidget interface.
	void OnKeyDown( Integer Key );
	void OnMouseMove( EMouseButton Button, Integer X, Integer Y );
	void OnDblClick( EMouseButton Button, Integer X, Integer Y );   
	void OnMouseDown( EMouseButton Button, Integer X, Integer Y );
	void OnMouseUp( EMouseButton Button, Integer X, Integer Y );
	void OnMouseScroll( Integer Delta );

	// WEditorPage interface.
	void RenderPageContent( CCanvas* Canvas );
	void TickPage( Float Delta );
	FResource* GetResource()
	{ 
		return PlayLevel; 
	}

	// Friends.
	friend CEditor;
	friend WWatchListDialog;

private:
	// Variables.
	WWatchListDialog*	WatchList;
	FLevel*				SourceLevel;
	FLevel*				PlayLevel;
	Float				PlayTime;

	// Runtime script errors.
	enum { MAX_SCRIPT_MSG_LIST = 8 };
	TArray<String>	Messages;
	Double			LastPushTime;

	// Internal.
	void RunLevel();
	void ShutdownLevel();
};


/*-----------------------------------------------------------------------------
    WWatchListDialog.
-----------------------------------------------------------------------------*/

//
// Dialog to watch in-play entities properties.
//
class WWatchListDialog: public WForm, public CRefsHolder
{
public:
	// WWatchListDialog interface.
	WWatchListDialog( WPlayPage* InPage, WWindow* InRoot );
	~WWatchListDialog();
	void UpdateWatches();

	// WWidget interface.
	void OnResize();
	void OnPaint( CGUIRenderBase* Render );
	void OnMouseDown( EMouseButton Button, Integer X, Integer Y );
	void OnMouseUp( EMouseButton Button, Integer X, Integer Y );
	void OnMouseMove( EMouseButton Button, Integer X, Integer Y );
	void OnMouseScroll( Integer Delta );

	// WForm interface.
	void OnClose()
	{
		WForm::OnClose();
		Hide();
	}

	// Friends.
	friend WPlayPage;

private:
	// An information about property to watch.
	class TWatch
	{
	public:
		String			Caption;
		CProperty*		Property;
		void*			Address;

		TWatch( String InCaption, CProperty* InProp, void* InAddr )
			: Caption( InCaption ), Property(InProp), Address(InAddr)
		{}
	};	
	
	// Internal.
	WPlayPage*			Page;
	WPanel*				TopPanel;
	WLabel*				ScriptLabel;
	WLabel*				EntityLabel;
	WComboBox*			ScriptCombo;
	WComboBox*			EntityCombo;
	WCheckBox*			PublicOnlyCheck;
	WSlider*			ScrollBar;
	TArray<TWatch>		Watches;
	FEntity*			Entity;
	Integer				Divider;

	// Notifications.
	void CheckPublicOnlyClick( WWidget* Sender );
	void ComboScriptChange( WWidget* Sender );
	void ComboEntityChange( WWidget* Sender );

	// CRefsHolder interface.
	void CountRefs( CSerializer& S );
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/