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
	void HighlightError( Int32 iLine );

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
	void ScrollToLine( Int32 iLine );
	Bool FindText( String S, Bool bMatchCase );
	void Undo();
	void Redo();

	// WWidget interface.
	void OnActivate();
	void OnDeactivate();
	void OnDblClick( EMouseButton Button, Int32 X, Int32 Y );
	void OnPaint( CGUIRenderBase* Render );
	void OnMouseScroll( Int32 Delta );
	void OnDragOver( void* Data, Int32 X, Int32 Y, Bool& bAccept );
	void OnDragDrop( void* Data, Int32 X, Int32 Y );		
	void OnResize();
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y );
	void OnCharType( Char TypedChar );
	void OnKeyUp( Int32 Key );
	void OnKeyDown( Int32 Key );
	void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y );

private:
	// A highlight span.
	struct TSpan
	{
		Int32		Type	: 4;
		Int32		Length	: 28;
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
		Int32				iX;

		WAutoComplete( WCodeEditor* InCodeEditor, WWindow* InRoot );
		~WAutoComplete();
		void MoveToCaret();
		void Accept();
		void Filter();
		void AddEntry( EAutoType InType, String InLabel, String InText );
		void OnPaint( CGUIRenderBase* Render );
		void OnDblClick( EMouseButton Button, Int32 X, Int32 Y );   
		void OnKeyDown( Int32 Key );
		void FillBy( CClass* Class );
		void FillBy( TArray<FExtraComponent*>& InArr );
		void FillBy( FScript* Script );
	};

	// Text variables.
	TArray<TLine>			Lines;
	CDynamicPool			Pool;

	// Internal variables.
	Int32					ScrollTop;
	TSize					CharSize;
	Float					LastHighlightTime;
	Float					LastTypeTime;

	// Caret info.
	Bool					bDrag;
	Bool					bReadyDrag;
	Int32					DragX;
	Int32					DragY;
	Int32					CaretXBegin;
	Int32					CaretXEnd;
	Int32					CaretYBegin;
	Int32					CaretYEnd;
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
	Bool IsInSelection( Int32 X, Int32 Y );
	Int32 XToColumn( Int32 X, Int32 iLine );
	Int32 ColumnToX( Int32 iColumn );
	Int32 YToLine( Int32 Y );
	Int32 LineToY( Int32 iLine );
	void HighlightBrackets( Bool bUnmark = false );

	// Undo/Redo management.
	enum{ HISTORY_LIMIT	= 20 };
	TArray<void*>		UndoStack;
	Int32				UndoTop;
	Bool				bUndoLock;
	void BeginTransaction();
	void EndTransaction();
	void SaveToUndoStack( Int32 iSlot );
	void LoadFromUndoStack( Int32 iSlot );

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
		Int32 iLine = -1, 
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
		Int32 iLine;
	};
	TArray<TMessage>	Messages;
	WLog*				Log;

	void GotoMessage( WWidget* Sender, Int32 iMessage );
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/