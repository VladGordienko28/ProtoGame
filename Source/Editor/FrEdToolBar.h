/*=============================================================================
    FrEdToolBar.h: An editor main toolbar.
    Copyright Aug.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    WEditorToolBar.
-----------------------------------------------------------------------------*/

//
// An editor main toolbar.
//
class WEditorToolBar: public WToolBar
{
public:
	// Controls.
	WPictureButton*		NewProjectButton;
	WPictureButton*		OpenProjectButton;
	WPictureButton*		SaveProjectButton;
	WPictureButton*		UndoButton;
	WPictureButton*		RedoButton;
	WComboBox*			PlayModeCombo;
	WPictureButton*		PlayButton;
	WPictureButton*		PauseButton;
	WPictureButton*		StopButton;
	WPictureButton*		BuildButton;

	// WEditorToolBar interface.
	WEditorToolBar( WContainer* InOwner, WWindow* InRoot );

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );

	// Notifications.
	void ButtonNewProjectClick( WWidget* Sender );
	void ButtonOpenProjectClick( WWidget* Sender );
	void ButtonSaveProjectClick( WWidget* Sender );
	void ButtonUndoClick( WWidget* Sender );
	void ButtonRedoClick( WWidget* Sender );
	void ButtonPlayClick( WWidget* Sender );
	void ButtonPauseClick( WWidget* Sender );
	void ButtonStopClick( WWidget* Sender );
	void ButtonBuildClick( WWidget* Sender );
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/