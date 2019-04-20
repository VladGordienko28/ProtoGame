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
class WPlayPage: public WEditorPage, public ILogCallback
{
public:
	// Variables.
	EPlayMode		PlayMode;

	// WPlayPage interface.
	WPlayPage( FLevel* InOrigianl, EPlayMode InPlayMode, WContainer* InOwner, WWindow* InRoot );
	~WPlayPage();
	void AddScriptMessage( Bool bImportant, String Message, math::Color Color );

	// WTabPage interface.
	void OnOpen();

	// WWidget interface.
	void OnKeyDown( Int32 Key );
	void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y );
	void OnDblClick( EMouseButton Button, Int32 X, Int32 Y );   
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseScroll( Int32 Delta );

	// WEditorPage interface.
	void RenderPageContent( CCanvas* Canvas );
	void TickPage( Float Delta );
	FResource* GetResource()
	{ 
		return PlayLevel; 
	}

	// ILogCallback interface
	void handleMessage( ELogLevel level, const Char* message ) override;
	void handleScriptMessage( ELogLevel level, const Char* message ) override;
	void handleFatalMessage( const Char* message ) override;
	void handleFatalScriptMessage( const Char* message ) override;

	// Friends.
	friend CEditor;
	friend WWatchListDialog;

private:
	// Variables.
	WWatchListDialog*	WatchList;
	FLevel*				SourceLevel;
	FLevel*				PlayLevel;
	Float				PlayTime;

	// Runtime script messages.
	struct ScriptMessage
	{
		String message;
		math::Color color;
	};

	enum { MAX_SCRIPT_MSG_LIST = 8 };
	Array<ScriptMessage>	Messages;
	Double					LastPushTime;

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
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseScroll( Int32 Delta );

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
	Array<TWatch>		Watches;
	FEntity*			Entity;
	Int32				Divider;

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