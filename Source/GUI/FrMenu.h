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
		Integer			Y;
		Integer			Height;	
		WMenu*			SubMenu;
		TNotifyEvent	Event;
	};
	TArray<TMenuItem>		Items;

	// WMenu interface.
	WMenu( WContainer* InOwner, WWindow* InRoot );
	~WMenu();
	virtual void SetParent( WMenu* Menu );
	virtual void Hide();
	virtual void Show( TPoint P );
	Integer AddSubMenu( String Title, WMenu* SubMenu );
	Integer AddItem( String Title, TNotifyEvent InEvent = TNotifyEvent(), Bool InbToggle = false );

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );
	void OnMouseUp( EMouseButton Button, Integer X, Integer Y );
	void OnMouseMove( EMouseButton Button, Integer X, Integer Y );

private:
	// Internal.
	WMenu*		Parent;	
	WMenu*		Popped;
	Integer		iSelected;

	Integer YToIndex( Integer InY );
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
		Integer	X;
		Integer	Width;
		WMenu*	SubMenu;
	};
	TArray<TMainMenuItem>	Items;

	// WMainMenu interface.
	WMainMenu( WContainer* InOwner, WWindow* InRoot );
	~WMainMenu();
	Integer AddSubMenu( String Title, WMenu* SubMenu );

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );
	void OnMouseDown( EMouseButton Button, Integer X, Integer Y );
	void OnMouseMove( EMouseButton Button, Integer X, Integer Y );
	void OnDeactivate();
	void OnMouseLeave();

private:	
	// Internal.
	Bool		bPopping;
	Integer		iSelected;
	WMenu*		Active;

	Integer XToIndex( Integer InX );
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