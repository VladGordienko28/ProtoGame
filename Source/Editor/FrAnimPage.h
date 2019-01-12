/*=============================================================================
    FrAnimPage.h: Animation page.
    Copyright Oct.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    WAnimationPage.
-----------------------------------------------------------------------------*/

// 
// Animation editor page.
//
class WAnimationPage: public WEditorPage, public CRefsHolder
{
public:
	// Variables.
	FAnimation*			Animation;

	// WAnimationPage interface.
	WAnimationPage( FAnimation* InAnimation, WContainer* InOwner, WWindow* InRoot );

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y );
	void OnDragOver( void* Data, Int32 X, Int32 Y, Bool& bAccept );
	void OnDragDrop( void* Data, Int32 X, Int32 Y );

	// WTabPage interface.
	Bool OnQueryClose();
	void OnOpen();

	// WEditorPage interface.
	void TickPage( Float Delta );
	FResource* GetResource()
	{ 
		return Animation; 
	}

private:
	// Internal widgets.
	WToolBar*			ToolBar;
	WPanel*				LeftPanel;
	WPanel*				PreviewPanel;
	WPanel*				SeqPanel;
	WButton*			PlayButton;
	WButton*			StopButton;
	WListBox*			SeqsList;
	WEdit*				SeqNameEdit;
	WButton*			NewSeqButton;
	WButton*			DeleteSeqButton;
	WEdit*				StartFrmEdit;
	WEdit*				CountFrmsEdit;
	WLabel*				StartLabel;
	WLabel*				CountLabel;
	WPanel*				PlayerPanel;
	WButton*			ZoomInButton;
	WButton*			ZoomOutButton;

	// Internal.
	Bool				bPreview;
	Float				Scale;
	TPoint				Pan;
	Int32				iFrame;

	// MVC stuff.
	void UpdateControls();

	// Widgets notifications.
	void ButtonPlayClick( WWidget* Sender );
	void ButtonStopClick( WWidget* Sender );
	void ListSequencesChange( WWidget* Sender );
	void EditNameChange( WWidget* Sender );
	void ButtonNewClick( WWidget* Sender );
	void ButtonDeleteClick( WWidget* Sender );
	void EditStartChange( WWidget* Sender );
	void EditCountChange( WWidget* Sender );
	void ButtonZoomInClick( WWidget* Sender );
	void ButtonZoomOutClick( WWidget* Sender );

public:
	// CRefsHolder interface.
	void CountRefs( CSerializer& S )
	{
		Serialize( S, Animation );
		if( !Animation )
			this->Close( true );
	}
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/