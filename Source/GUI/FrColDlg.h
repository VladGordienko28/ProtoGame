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
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y );

	// WForm interface.
	void Hide();
	void OnClose();

	// Static color instance.
	static	math::Color		SharedColor;

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
	math::Color		Selected;

	// Internal interface.
	void UpdateFromRGB( WWidget* Sender );
	void UpdateFromHSL( WWidget* Sender );
	void UpdateFromA( WWidget* Sender );
	void RefreshSL();
	void SetAlphaBlend( UInt8 Alpha );

	// Notifications.
	void ButtonOkClick( WWidget* Sender );
	void ButtonCancelClick( WWidget* Sender );
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/