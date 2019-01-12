/*=============================================================================
    FrGuiMenu.cpp: Menu classes implementation.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#include "GUI.h"

/*-----------------------------------------------------------------------------
    WMenu implementation.
-----------------------------------------------------------------------------*/

//
// Menu constructor.
//
WMenu::WMenu( WContainer* InOwner, WWindow* InRoot )
	:	WWidget( InOwner, InRoot ),
		Items(),
		Parent( nullptr ),
		iSelected( -1 ),
		Popped( nullptr )

{	
	Align		= AL_None;
	bVisible	= false;
	bStayOnTop	= true;
	SetSize( 50, 50 );	
}


//
// Menu destructor.
//
WMenu::~WMenu()
{
	// No way! Submenus will be killed by
	// Owner in WContainer::~WContainer!
#if 0
	// Kill own sub menus.
	for( Integer i=0; i<Items.Num(); i++ )
		if( Items[i].SubMenu  )
			delete Items[i].SubMenu;
#endif
}


//
// Set a menu parent.
//
void WMenu::SetParent( WMenu* Menu )
{
	assert(Menu);
	assert(!Menu->Parent);	
	assert(Menu != this);
	Parent	= Menu;
}


//
// Add a new item, with sub-menu.
//
Integer WMenu::AddSubMenu( String Title, WMenu* SubMenu )		
{
	TMenuItem Item;
	Item.Event		= TNotifyEvent();
	Item.SubMenu	= SubMenu;
	Item.Text		= Title;
	Item.Y			= Items.Num() ? Items.Last().Y+Items.Last().Height : 1;
	Item.Height		= Title.Len() ? MENU_ITEM_HEIGHT : 3;
	Item.bEnabled	= true;
	Item.bToggle	= false;
	Item.bChecked	= false;

	// Resize to fit the item.
	SetSize	(	
				Max( Size.Width, Root->Font1->TextWidth(*Title)+64 ), 
				Item.Y + Item.Height + 1 
			);

	return Items.Push( Item );
}


//
// Add a new clickable item to this menu.
//
Integer WMenu::AddItem( String Title, TNotifyEvent InEvent, Bool InbToggle )
{
	TMenuItem Item;
	Item.Event		= InEvent;
	Item.SubMenu	= nullptr;
	Item.Text		= Title;
	Item.Y			= Items.Num() ? Items.Last().Y+Items.Last().Height : 1;
	Item.Height		= Title.Len() ? MENU_ITEM_HEIGHT : 3;
	Item.bEnabled	= true;
	Item.bToggle	= InbToggle;
	Item.bChecked	= false;

	// Resize to fit the item.
	SetSize	(	
				Max( Size.Width, Root->Font1->TextWidth(*Title)+64 ), 
				Item.Y + Item.Height + 1 
			);

	return Items.Push( Item );
}


//
// Hide menu and its sub-menus.
//
void WMenu::Hide()
{
	bVisible	= false;
	if( Popped )
		Popped->Hide();
}


//
// Show this menu at specified location.
//
void WMenu::Show( TPoint P )
{
	// Figure out proper show location of menu
	// to avoid leave owner area.
	if( P.X+Size.Width > Owner->Size.Width )
		P.X -= Size.Width;

	if( P.Y+Size.Height > Owner->Size.Height )
		P.Y -= Size.Height;

	// Show it.
	Location	= P;
	bVisible	= true;
	iSelected	= -1;
	Popped		= nullptr;
}


//
// Redraw menu.
//
void WMenu::OnPaint( CGUIRenderBase* Render )
{
	WWidget::OnPaint( Render );

	TPoint Base = ClientToWindow(TPoint::Zero);
	Integer  TextY = Base.Y +(MENU_ITEM_HEIGHT-Root->Font1->Height) / 2;	

	// Draw pad.
	Render->DrawRegion( Base, Size, GUI_COLOR_MENU, GUI_COLOR_MENU_SELECTED, BPAT_Solid );

	// Draw each item.
	for( Integer i=0; i<Items.Num(); i++ )
	{
		TMenuItem& Item = Items[i];

		// Draw selection.
		if( iSelected == i && Item.Text.Len() )
			Render->DrawRegion
					( 
						TPoint( Base.X + 2, Base.Y + Item.Y + 1 ), 
						TSize( Size.Width - 4, Item.Height - 2 ), 
						Item.bEnabled ? GUI_COLOR_MENU_HIGH : GUI_COLOR_MENU_HIGH - TColor( 0x23, 0x23, 0x23, 0x00 ), 
						Item.bEnabled ? GUI_COLOR_MENU_HIGH : GUI_COLOR_MENU_HIGH - TColor( 0x23, 0x23, 0x23, 0x00 ), 
						BPAT_Solid  
					);

		// Draw caption or divider.
		if( Item.Text.Len() )
			Render->DrawText
					( 
						TPoint( Base.X + 18, Item.Y + TextY ), 
						Item.Text, 
						Item.bEnabled ? GUI_COLOR_TEXT : GUI_COLOR_TEXT_OFF, 
						Root->Font1 
					);
		else
			Render->DrawRegion
					( 
						TPoint( Base.X + 3,Base.Y + Item.Y + 1 ), 
						TSize( Size.Width - 6, 1 ), 
						GUI_COLOR_MENU_SELECTED, 
						GUI_COLOR_MENU_SELECTED,
						BPAT_Solid 
					);

		// If checked - draw mark.
		if( Item.bChecked && Item.Text.Len() )
			Render->DrawPicture
					( 
						TPoint( Base.X + 3, Base.Y + Item.Y + 7 ), 
						TSize( 10, 10 ), 
						TPoint( 0, 22 ), 
						TSize( 10, 10 ), 
						Root->Icons 
					);

		// If have submenu draw little triangle.
		if( Item.SubMenu )
			Render->DrawPicture
					( 
						TPoint( Base.X + Size.Width - 16, Base.Y + Item.Y + 7 ), 
						TSize( 4, 7 ), 
						TPoint(0, 0), 
						TSize(4, 7), 
						Root->Icons 
					);
	}
}


//
// When click happened on a menu item.
//
void WMenu::OnMouseUp( EMouseButton Button, Integer X, Integer Y )
{
	WWidget::OnMouseUp( Button, X, Y );

	Integer iItem =	YToIndex(Y);

	// Call event.
	if( iItem != -1 && Items[iItem].bEnabled )
	{
		if( Items[iItem].bToggle )
			Items[iItem].bChecked	^= 1;

		Items[iItem].Event( this );
		Hide();
	}
}


//
// On Mouse hover above menu.
//
void WMenu::OnMouseMove( EMouseButton Button, Integer X, Integer Y )
{
	WWidget::OnMouseMove( Button, X, Y );

	// Find selected item.
	iSelected = YToIndex(Y);

	// Pop new menu and remove old if any.
	if( iSelected != -1 )
	{
		TMenuItem& Item = Items[iSelected];

#if 0
		if( !Item.bEnabled )
			iSelected	= -1;
#endif

		if( Item.bEnabled && Item.SubMenu )
		{
			// New item has submenu.
			if( Item.SubMenu != Popped )
			{
				// Replace popped.
				if( Popped )	Popped->Hide();

				Popped	= Item.SubMenu;
				Popped->Show(TPoint( Location.X + Size.Width, Location.Y + Item.Y ));
			}
		}
		else
		{
			// New item has no submenu, just shutdown old.
			if( Popped )
				Popped->Hide();
			Popped = nullptr;
		}
	}
	else
	{
		// Nothing selected, close if something pop.
		if( Popped )
			Popped->Hide();
		Popped = nullptr;
	}
}


//
// Convert Y value to the index of item, if no
// item found - return -1.
//
Integer WMenu::YToIndex( Integer InY )
{
	for( Integer i=0; i<Items.Num(); i++ )
	{
		TMenuItem& Item = Items[i];

		if( ( InY > Item.Y ) &&
			( InY < Item.Y+Item.Height ) )
				return i;
	}
	return -1;
}


/*-----------------------------------------------------------------------------
    WMainMenu implementation.
-----------------------------------------------------------------------------*/

//
// Main menu constructor.
//
WMainMenu::WMainMenu( WContainer* InOwner, WWindow* InRoot )
	:	WWidget( InOwner, InRoot ),
		Items(),
		Active( nullptr ),
		bPopping( false ),
		iSelected( -1 )
{
	SetSize( 100, 21 );
	Align	= AL_Top;
}


//
// Main menu destructor.
//
WMainMenu::~WMainMenu()
{
	for( Integer i=0; i<Items.Num(); i++ )
		if( Items[i].SubMenu )
			delete Items[i].SubMenu;
}


//
// Add a new submenu to the menu.
//
Integer WMainMenu::AddSubMenu( String Title, WMenu* SubMenu )
{
	assert(SubMenu);

	TMainMenuItem Item;
	Item.SubMenu	= SubMenu;
	Item.Text		= Title;
	Item.Width		= Root->Font1->TextWidth( *Title ) + 10;
	Item.X			= Items.Num() ? Items.Last().X + Items.Last().Width : 4;

	return Items.Push(Item);
}


//
// Main menu paint.
//
void WMainMenu::OnPaint( CGUIRenderBase* Render )
{
	WWidget::OnPaint( Render );

	TPoint Base = ClientToWindow( TPoint(0, 0) );
	Integer TextY = Base.Y + (Size.Height-Root->Font1->Height) / 2;

	// Draw pad.
	Render->DrawRegion( Base, Size, GUI_COLOR_MENU_BAR, GUI_COLOR_MENU_BAR, BPAT_Solid );

	// Draw each item.
	for( Integer i=0; i<Items.Num(); i++ )
	{
		TMainMenuItem& Item = Items[i];

		if( i == iSelected )
			Render->DrawRegion
					( 
						TPoint( Item.X, Location.Y ), 
						TSize( Item.Width, Size.Height ), 
						bPopping ? GUI_COLOR_MENU_SELECTED: GUI_COLOR_MENU_HIGH, 
						bPopping ? GUI_COLOR_MENU_SELECTED: GUI_COLOR_MENU_HIGH, 
						BPAT_Solid
					);
			
			Render->DrawText
					( 
						TPoint( Item.X + 5, TextY ), 
						Item.Text, 
						GUI_COLOR_TEXT, 
						Root->Font1 
					);
	}
}


//
// When mouse leave menu bar.
//
void WMainMenu::OnMouseLeave()
{
	WWidget::OnMouseLeave();

	if( !bPopping )
		iSelected	= -1;
}


//
// Mouse press menu bar.
//
void WMainMenu::OnMouseDown( EMouseButton Button, Integer X, Integer Y )
{
	WWidget::OnMouseDown( Button, X, Y );
		
	iSelected = XToIndex(X);
	
	// Start popping.
	if( iSelected != -1 )
	{
		TMainMenuItem& Item = Items[iSelected];

		if( Active )
			Active->Hide();

		Active	= Item.SubMenu;
		Active->Show(TPoint( Item.X, Location.Y + Size.Height ));
		bPopping	= true;	
	}
}


//
// When menu bar lost focus.
//
void WMainMenu::OnDeactivate()
{
	WWidget::OnDeactivate();
	
	iSelected	= -1;
	bPopping	= false;		

	if( Active )
		Active->Hide();
}


//
// Mouse hover on the menu bar.
//
void WMainMenu::OnMouseMove( EMouseButton Button, Integer X, Integer Y )
{
	WWidget::OnMouseMove( Button, X, Y );

	// Figure out selected.
	iSelected	= XToIndex(X);

	// Clamp selection.
	if( bPopping && iSelected == -1 )
		iSelected	=  X < 10 ? 0 : Items.Num()-1;

	if( bPopping )
	{
		if( iSelected != -1 )
		{
			TMainMenuItem& Item = Items[iSelected];

			if( Active != Item.SubMenu )
			{
				if( Active )
					Active->Hide();

				Active = Item.SubMenu;
				Active->Show(TPoint( Item.X, Location.Y + Size.Height ));
			}
		}
	}
}


//
// Convert X value to the index of item, if no
// item found - return -1.
//
Integer WMainMenu::XToIndex( Integer InX )
{
	for( Integer i=0; i<Items.Num(); i++ )
	{
		TMainMenuItem& Item = Items[i];

		if( ( InX >= Item.X ) &&
			( InX < Item.X+Item.Width ) )
				return i;
	}
	return -1;
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/