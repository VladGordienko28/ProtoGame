/*=============================================================================
    FrTabControl.h: Tab control.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    WTabPage.
-----------------------------------------------------------------------------*/

//
// A single page of tab control.
//
class WTabPage: public WContainer
{
public:
	// Variables.
	TColor			Color;
	Int32			TabWidth;
	WTabControl*	TabControl;
	Bool			bCanClose;

	// WTabPage interface.
	WTabPage( WContainer* InOwner, WWindow* InRoot );
	virtual Bool OnQueryClose()
	{
		return true;
	}
	virtual void OnOpen()
	{}
	virtual void Close( Bool bForce = false );

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );
};


/*-----------------------------------------------------------------------------
    WTabControl.
-----------------------------------------------------------------------------*/

// An tab label side.
enum ETabHeaderSide
{
	ETH_Top,
	ETH_Bottom
};


//
// A tab control.
//
class WTabControl: public WContainer
{
public:
	// Variables.
	TArray<WTabPage*>	Pages;
	Int32				iActive;

	// WTabControl interface.	
	WTabControl( WContainer* InOwner, WWindow* InRoot );
	void ActivateTabPage( Int32 iPage );
	void ActivateTabPage( WTabPage* Page );
	void CloseTabPage( Int32 iPage, Bool bForce = false );
	Int32 AddTabPage( WTabPage* Page );
	WTabPage* GetActivePage();
	void SetHeaderSide( ETabHeaderSide InSide );

	// WWidget interface.
	void OnActivate();
	void OnPaint( CGUIRenderBase* Render );
	void OnMouseLeave();
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y );

private:
	// Internal.
	WPopupMenu*			Popup;
	Int32				iHighlight;
	Int32				iCross;
	WTabPage*			DragPage;
	Bool				bWasDrag;
	Bool				bOverflow;
	ETabHeaderSide		HeaderSide;

	Int32 XToIndex( Int32 InX );
	Int32 XYToCross( Int32 InX, Int32 InY );
	void PopupPageClick( WWidget* Sender );
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/