/*=============================================================================
    FrMenu.h: Menu widgets.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    WMenu.
-----------------------------------------------------------------------------*/

// A menu item height.
#define MENU_ITEM_HEIGHT 21


//
// A menu.
//
class WMenu: public WWidget
{
public:
	// Variables.
	struct TMenuItem
	{
	public:
		String			Text;
		Bool			bEnabled;
		Bool			bToggle;
		Bool			bChecked;
		Int32			Y;
		Int32			Height;	
		WMenu*			SubMenu;
		TNotifyEvent	Event;
	};
	Array<TMenuItem>		Items;

	// WMenu interface.
	WMenu( WContainer* InOwner, WWindow* InRoot );
	~WMenu();
	virtual void SetParent( WMenu* Menu );
	virtual void Hide();
	virtual void Show( TPoint P );
	Int32 AddSubMenu( String Title, WMenu* SubMenu );
	Int32 AddItem( String Title, TNotifyEvent InEvent = TNotifyEvent(), Bool InbToggle = false );

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y );

private:
	// Internal.
	WMenu*		Parent;	
	WMenu*		Popped;
	Int32		iSelected;

	Int32 YToIndex( Int32 InY );
};


/*-----------------------------------------------------------------------------
    WMainMenu.
-----------------------------------------------------------------------------*/

//
// A main menu stripe.
//
class WMainMenu: public WWidget
{
public:
	// Variables.
	struct TMainMenuItem
	{
	public:
		String	Text;
		Int32	X;
		Int32	Width;
		WMenu*	SubMenu;
	};
	Array<TMainMenuItem>	Items;

	// WMainMenu interface.
	WMainMenu( WContainer* InOwner, WWindow* InRoot );
	~WMainMenu();
	Int32 AddSubMenu( String Title, WMenu* SubMenu );

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y );
	void OnDeactivate();
	void OnMouseLeave();

private:	
	// Internal.
	Bool		bPopping;
	Int32		iSelected;
	WMenu*		Active;

	Int32 XToIndex( Int32 InX );
};


/*-----------------------------------------------------------------------------
    WPopupMenu.
-----------------------------------------------------------------------------*/

//
// A popup menu.
//
class WPopupMenu: public WMenu
{
public:
	// WPopupMenu interface.
	WPopupMenu( WContainer* InOwner, WWindow* InRoot )
		:	WMenu( InOwner, InRoot )
	{}

	// WMenu interface.
	void Show( TPoint P )
	{
		WMenu::Show( P );
		Root->SetFocused( this );
	}

	// WWidget interface.
	void OnDeactivate()
	{
		WWidget::OnDeactivate();
		Hide();
	}
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/