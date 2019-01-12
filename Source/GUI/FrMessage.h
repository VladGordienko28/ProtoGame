/*=============================================================================
    FrMessage.h: Dialogs for showing messages.
    Copyright Nov.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    WMessageBox.
-----------------------------------------------------------------------------*/

//
// A dialog to show messages. Do not use it
// directly! Let WWindow handle it properly.
//
class WMessageBox: public WForm
{
public:
	// WMessageBox interface.
	WMessageBox( WWindow* InRoot, String InText, String InCaption, Bool InbModal=true );
	~WMessageBox();

	// Buttons set initialization.
	void SetOk( TNotifyEvent InOk = TNotifyEvent() );
	void SetOkCancel( TNotifyEvent InCancel = TNotifyEvent(), TNotifyEvent InOk = TNotifyEvent() );
	void SetYesNo( TNotifyEvent InYes = TNotifyEvent(), TNotifyEvent InNo = TNotifyEvent() );
	void SetYesNoCancel( TNotifyEvent InYes = TNotifyEvent(), TNotifyEvent InNo = TNotifyEvent(), TNotifyEvent InCancel = TNotifyEvent() );
	void SetNotification();

	// WForm interface.
	void Hide();
	void OnClose();

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );
	void OnDeactivate();

private:
	// Controls.
	WButton*		YesButton;
	WButton*		NoButton;
	WButton*		OkButton;
	WButton*		CancelButton;

	// Internal.
	Bool			bNotification;
	Bool			bModal;
	TArray<String>	Lines;
	WForm*			OldModal;

	// Notifications.
	void ButtonCloseClick( WWidget* Sender )
	{
		Hide();
	}
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/