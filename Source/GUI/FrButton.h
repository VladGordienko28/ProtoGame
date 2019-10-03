/*=============================================================================
    FrButton.h: A clickable widgets.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    WButton.
-----------------------------------------------------------------------------*/

//
// A simple button.
//
class WButton: public WWidget
{
public:
	// Variables.
	TNotifyEvent	EventClick;
	Bool			bDown;
	Bool			bToggle;

	// WWidget interface.
	WButton( WContainer* InOwner, WWindow* InRoot );
	void OnPaint( CGUIRenderBase* Render ) ;
	void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y );

	// WButton interface.
	virtual void OnClick();

private:
	// Internal.
	Bool	bHold;
};


/*-----------------------------------------------------------------------------
    WCheckBox.
-----------------------------------------------------------------------------*/

// CheckBox square side.
#define CHECKBOX_SIDE		13

//
// A check box.
//
class WCheckBox: public WWidget
{
public:
	// Variables.
	Bool			bChecked;
	TNotifyEvent	EventClick;	

	// WCheckBox interface.
	WCheckBox( WContainer* InOwner, WWindow* InRoot );
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y );
	void OnPaint( CGUIRenderBase* Render );

	// WCheckBox interface.
	void SetChecked( Bool InbChecked, Bool bNotify=true );
	virtual void OnClick();

private:
	// Internal.
	Bool		bHold;
};


/*-----------------------------------------------------------------------------
	WRadioButton.
-----------------------------------------------------------------------------*/

// RadioButton square side.
#define RADIOBUTTON_SIDE		11

//
// A radio button.
//
class WRadioButton: public WWidget
{
public:
	// Variables.
	Bool			bChecked;
	TNotifyEvent	EventClick;	

	// WCheckBox interface.
	WRadioButton( WContainer* InOwner, WWindow* InRoot );
	~WRadioButton();
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y );
	void OnPaint( CGUIRenderBase* Render );

	// WRadioButton interface.
	void AddToList( WRadioButton* InPrev );
	virtual void OnClick();

private:
	// Internal.
	Bool			bHold;
	WRadioButton*	NextButton;
	WRadioButton*	PrevButton;
};


/*-----------------------------------------------------------------------------
    WLinkLabel.
-----------------------------------------------------------------------------*/

//
// A clickable blue label.
//
class WLinkLabel: public WWidget
{
public:
	// Variables.
	TNotifyEvent        EventClick;
	math::Color			Color;

	// WWidget interface.
	WLinkLabel( WContainer* InOwner, WWindow* InRoot );
	void OnPaint( CGUIRenderBase* Render );
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y );

	// WLinkLabel interface.
	virtual void OnClick();
};


/*-----------------------------------------------------------------------------
    WPictureButton.
-----------------------------------------------------------------------------*/

//
// Button with a picture.
//
class WPictureButton: public WButton
{
public:
	// Variables.
	img::Image::Ptr	Picture;
	TPoint			Offset;
	TSize			Scale;

	// WPictureButton interface.
	WPictureButton( WContainer* InOwner, WWindow* InRoot );
	void OnPaint( CGUIRenderBase* Render );
};


/*-----------------------------------------------------------------------------
    WToolBar.
-----------------------------------------------------------------------------*/

//
// A list of buttons.
//
class WToolBar: public WContainer
{
public:
	// WToolBar interface.
	WToolBar( WContainer* InOwner, WWindow* InRoot );
	void AddElement( WWidget* InElem );

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );

private:
	// Internal.
	Array<WWidget*>	Elements;
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/