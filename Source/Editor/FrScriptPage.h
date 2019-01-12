/*=============================================================================
    FrScriptPage.h: Script edit page.
    Copyright Aug.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    WScriptPage.
-----------------------------------------------------------------------------*/

//
// Script editor page.
//
class WScriptPage: public WEditorPage, public CRefsHolder
{
public:
	// Variables.
	FScript*			Script;
	WCompilerOutput*	Output;
	WCodeEditor*		CodeEditor;

	// WScriptPage interface.
	WScriptPage( FScript* InScript, WContainer* InOwner, WWindow* InRoot );
	~WScriptPage();	
	void SaveScriptText( Bool bAsk );
	void HighlightError( Integer iLine );

	// WTabPage interface.
	Bool OnQueryClose();
	void OnOpen(); 

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );

	// WEditorPage interface.
	void TickPage( Float Delta );
	void Undo();
	void Redo();
	FResource* GetResource()
	{ 
		return Script; 
	}

private:
	// Internal widgets.
	WToolBar*			ToolBar;
	WForm*				FindDialog;
	WForm*				GotoLineDialog;
	WForm*				AlterDialog;
	WPictureButton*		SaveButton;
	WPictureButton*		CompileAllButton;
	WPictureButton*		FindDialogButton;
	WPictureButton*		AlterButton;

	// Widgets notifications.
	void ButtonSaveClick( WWidget* Sender );
	void ButtonCompileAllClick( WWidget* Sender );
	void ButtonFindDialogClick( WWidget* Sender );
	void ButtonAlterClick( WWidget* Sender );

public:
	// CRefsHolder interface.
	void CountRefs( CSerializer& S )
	{
		Serialize( S, Script );

		if( !Script )
			this->Close( true );
	}

	// Friends.
	friend class WCodeEditor;
};


/*-----------------------------------------------------------------------------
    WCodeEditor.
-----------------------------------------------------------------------------*/

//
// Script code editor.
//
class WCodeEditor: public WContainer
{
public:
	// Variables.
	Bool				bDirty;
	Bool				bFlashy;
	WScriptPage*		Page;
	FScript*			Script;

	// Friends.
	friend class WScriptPage;
	friend class WAutoComplete;
	friend class WEditorMainMenu;

	// WCodeEditor interface.
	WCodeEditor( WScriptPage* InPage, WContainer* InOwner, WWindow* InRoot );
	~WCodeEditor();
	void SetDirty( Bool InbDirty );
	void ScrollToLine( Integer iLine );
	Bool FindText( String S, Bool bMatchCase );
	void Undo();
	void Redo();

	// WWidget interface.
	void OnActivate();
	void OnDeactivate();
	void OnDblClick( EMouseButton Button, Integer X, Integer Y );
	void OnPaint( CGUIRenderBase* Render );
	void OnMouseScroll( Integer Delta );
	void OnDragOver( void* Data, Integer X, Integer Y, Bool& bAccept );
	void OnDragDrop( void* Data, Integer X, Integer Y );		
	void OnResize();
	void OnMouseDown( EMouseButton Button, Integer X, Integer Y );
	void OnCharType( Char TypedChar );
	void OnKeyUp( Integer Key );
	void OnKeyDown( Integer Key );
	void OnMouseMove( EMouseButton Button, Integer X, Integer Y );
	void OnMouseUp( EMouseButton Button, Integer X, Integer Y );

private:
	// A highlight span.
	struct TSpan
	{
		Integer		Type	: 4;
		Integer		Length	: 28;
		TSpan*		Next;
	};
	
	// An editor line of the text.
	struct TLine
	{
	public:
		String		Text;
		TSpan*		First;
	};

	// An Autocomplete panel.
	class WAutoComplete: public WListBox
	{
	public:
		enum EAutoType
		{ 
			AT_Property, 
			AT_Method, 
			AT_Component, 
			AT_Function,
			AT_Script,
			AT_Resource,
			AT_Keyword,
			AT_MAX 
		};
		struct TEntry
		{
			EAutoType	Type;
			String		Text;
			String		Label;
		};

		WCodeEditor*		Editor;
		TArray<TEntry>		Entries;
		Integer				iX;

		WAutoComplete( WCodeEditor* InCodeEditor, WWindow* InRoot );
		~WAutoComplete();
		void MoveToCaret();
		void Accept();
		void Filter();
		void AddEntry( EAutoType InType, String InLabel, String InText );
		void OnPaint( CGUIRenderBase* Render );
		void OnDblClick( EMouseButton Button, Integer X, Integer Y );   
		void OnKeyDown( Integer Key );
		void FillBy( CClass* Class );
		void FillBy( TArray<FExtraComponent*>& InArr );
		void FillBy( FScript* Script );
	};

	// Text variables.
	TArray<TLine>			Lines;
	CDynamicPool			Pool;

	// Internal variables.
	Integer					ScrollTop;
	TSize					CharSize;
	Float					LastHighlightTime;
	Float					LastTypeTime;

	// Caret info.
	Bool					bDrag;
	Bool					bReadyDrag;
	Integer					DragX;
	Integer					DragY;
	Integer					CaretXBegin;
	Integer					CaretXEnd;
	Integer					CaretYBegin;
	Integer					CaretYEnd;
	TPoint					EnclosingBrackets[2];

	// Widgets.
	WAutoComplete*			AutoDialog;
	WSlider*				ScrollBar;
	WPopupMenu*				PopUp;

	// WCodeEditor events.
	void OnChange();

	// Widgets notification.
	void ScrollBarChange( WWidget* Sender );
	void PopCopyClick( WWidget* Sender );
	void PopCutClick( WWidget* Sender );
	void PopPasteClick( WWidget* Sender );			

	// Support functions.
	void ScrollToCaret();
	void HighlightAll();
	void ClearSelected();
	void SelectAll();
	Bool IsInSelection( Integer X, Integer Y );
	Integer XToColumn( Integer X, Integer iLine );
	Integer ColumnToX( Integer iColumn );
	Integer YToLine( Integer Y );
	Integer LineToY( Integer iLine );
	void HighlightBrackets( Bool bUnmark = false );

	// Undo/Redo management.
	enum{ HISTORY_LIMIT	= 20 };
	TArray<void*>		UndoStack;
	Integer				UndoTop;
	Bool				bUndoLock;
	void BeginTransaction();
	void EndTransaction();
	void SaveToUndoStack( Integer iSlot );
	void LoadFromUndoStack( Integer iSlot );

	// Friends.
	friend class WGotoLineDialog;
	friend class WFindDialog;
};


/*-----------------------------------------------------------------------------
    WCompilerOutput.
-----------------------------------------------------------------------------*/

//
// An compiler output information panel.
//
class WCompilerOutput: public WContainer
{
public:
	// WCompilerOutput interface.
	WCompilerOutput( WContainer* InOwner, WWindow* InRoot );
	~WCompilerOutput();
	void AddMessage
	( 
		String InText, 
		FScript* Script = nullptr, 
		Integer iLine = -1, 
		TColor InColor = COLOR_White
	);
	void Clear();

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );

private:
	// Variables.
	struct TMessage
	{
		FScript* Script;
		Integer iLine;
	};
	TArray<TMessage>	Messages;
	WLog*				Log;

	void GotoMessage( WWidget* Sender, Integer iMessage );
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/