/*=============================================================================
    FrEdit.h: Text/Numbers edit classes.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    WEdit.
-----------------------------------------------------------------------------*/

//
// An edit text type.
//
enum EEditType
{
	EDIT_String,
	EDIT_Integer,
	EDIT_Float
};


//
// A box with editable text.
//
class WEdit: public WWidget
{
public:
	// Variables.
	TNotifyEvent	EventAccept;
	TNotifyEvent	EventChange;
	Bool			bReadOnly;
	EEditType		EditType;
	String			Text;

	// WEdit interface.
	WEdit( WContainer* InOwner, WWindow* InRoot );
	void SetText( String NewText, Bool bNotify = true );

	// WWidget interface.
	void OnDeactivate();
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y );
	void OnDblClick( EMouseButton Button, Int32 X, Int32 Y );
	void OnKeyDown( Int32 Key );
	void OnCharType( Char TypedChar );
	void OnPaint( CGUIRenderBase* Render );

	// Notifications.
	virtual void OnChange();
	virtual void OnAccept();

protected:
	// Internal.
	Int32		ScrollX;
	Int32		CaretBegin;
	Int32		CaretEnd;
	TSize		CharSize;
	Float		OldFloat;
	Int32		OldInt32;
	Bool		bDrawSelection;

	Int32 CaretToPixel( Int32 C );
	Int32 PixelToCaret( Int32 X );
	void ScrollToCaret();
	void SelectAll();
	void ClearSelected();
	void Store();
};


/*-----------------------------------------------------------------------------
    WSpinner.
-----------------------------------------------------------------------------*/

//
// Widget to edit numberic values.
//
class WSpinner: public WEdit
{
public:
	// Variables.
	union
	{
		struct{ Float FMin; Float FMax; Float FScale; };
		struct{ Int32 IMin; Int32 IMax; Int32 IScale; };
	};

	// WSpinner interface.
	WSpinner( WContainer* InOwner, WWindow* InRoot );
	void SetRange( Int32 InMin, Int32 InMax, Int32 InScale=1 );
	void SetRange( Float InMin, Float InMax, Float InScale=1.f );
	void SetValue( Int32 InValue, Bool bNotify = true );
	void SetValue( Float InValue, Bool bNotify = true );
	Int32 GetIntValue() const;
	Float GetFloatValue() const;

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y );
	void OnDblClick( EMouseButton Button, Int32 X, Int32 Y );
	void OnDeactivate();

	// WEdit interface.
	void OnChange();
	void OnAccept();

private:
	// Spinner internal.
	Int32			iSpinButton;
	Int32			LastCursorY;

	void Increment( bool bNegative, Int32 Multiplier );
	void FixValue();
};

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/