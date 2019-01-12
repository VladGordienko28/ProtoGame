/*=============================================================================
    FrTask.h: Task processing dialog.
    Copyright Dec.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    WTaskDialog.
-----------------------------------------------------------------------------*/

//
// A dialog to show processing.
//
class WTaskDialog: public WForm, public IProgressIndicator
{
public:
	// WTaskDialog interface.
	WTaskDialog( WWindow* InRoot );
	~WTaskDialog();

	// IProgressIndicator interface.
	void BeginTask( String TaskName ) override;
	void EndTask() override;
	void UpdateDetails( String Details ) override;
	void SetProgress( Int32 Numerator, Int32 Denominator ) override;

	// WForm interface.
	void Hide();

	// Accessors.
	inline Bool InProgress()
	{
		return bInProgress;
	}

private:
	// Internal.
	Bool			bInProgress;
	WForm*			OldModal;
	WLabel*			Label;
	WProgressBar*	ProgressBar;

	void RedrawAll();
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/