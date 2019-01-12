/*=============================================================================
    FrGamBld.h: A game builder dialog.
    Copyright Dec.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    WGameBuilderDialog.
-----------------------------------------------------------------------------*/

//
// A builder dialog.
//
class WGameBuilderDialog: public WForm
{
public:
	// WGameBuilderDialog interface.
	WGameBuilderDialog( WWindow* InRoot );
	~WGameBuilderDialog();
	void BuildGame();
	void RebuildGame();

	// WForm interface.
	void Show( Int32 X = 0, Int32 Y = 0 );
	void Hide();
	void OnClose();

private:
	// Controls.
	WLabel*				TargetLabel;
	WLabel*				ConfigLabel;
	WLabel*				CmdLabel;
	WComboBox*			TargetCombo;
	WComboBox*			ConfigCombo;
	WCheckBox*			LaunchCheck;
	WEdit*				ParmsEdit;
	WButton*			BuildButton;
	WButton*			CancelButton;

	// Notifications.
	void ButtonBuildClick( WWidget* Sender );
	void ButtonCancelClick( WWidget* Sender );
	void CheckLaunchClick( WWidget* Sender );
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/