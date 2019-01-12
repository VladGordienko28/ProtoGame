/*=============================================================================
    FrColDlg.h: Color chooser dialog.
    Copyright Dec.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    WColorChooser.
-----------------------------------------------------------------------------*/

//
// A dialog to choose color.
//
class WColorChooser: public WForm
{
public:
	// WColorChooser interface.
	WColorChooser( WWindow* InRoot, Bool InbUseAlpha=false, TNotifyEvent InOk = TNotifyEvent() );
	~WColorChooser();

	// WWidget interface.  
	void OnPaint( CGUIRenderBase* Render );
	void OnMouseDown( EMouseButton Button, Integer X, Integer Y );
	void OnMouseUp( EMouseButton Button, Integer X, Integer Y );
	void OnMouseMove( EMouseButton Button, Integer X, Integer Y );

	// WForm interface.
	void Hide();
	void OnClose();

	// Static color instance.
	static	TColor		SharedColor;

private:
	// Controls.
	WButton*		OkButton;
	WButton*		CancelButton;
	WSpinner*		RSpinner, *GSpinner, *BSpinner;
	WSpinner*		ASpinner;
	WSpinner*		HSpinner, *SSpinner, *LSpinner;

	// Internal.
	Bool			bUseAlpha;
	TNotifyEvent	EventOk;
	WForm*			OldModal;
	Bool			bMoveSL, bMoveH;
	TColor			Selected;

	// Internal interface.
	void UpdateFromRGB( WWidget* Sender );
	void UpdateFromHSL( WWidget* Sender );
	void UpdateFromA( WWidget* Sender );
	void RefreshSL();
	void SetAlphaBlend( Byte Alpha );

	// Notifications.
	void ButtonOkClick( WWidget* Sender );
	void ButtonCancelClick( WWidget* Sender );
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/