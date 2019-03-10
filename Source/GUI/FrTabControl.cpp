/*=============================================================================
    FrTabControl.cpp: Tab control widget.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#include "GUI.h"

/*-----------------------------------------------------------------------------
    WTabPage implementation.
-----------------------------------------------------------------------------*/

// Whether fade tab, if it not in focus.
#define FADE_TAB		0


//
// Tab page constructor.
//
WTabPage::WTabPage( WContainer* InOwner, WWindow* InRoot )
	:	WContainer( InOwner, InRoot ),
		Color( math::colors::STEEL_BLUE ),
		TabWidth( 70 ),
		TabControl( nullptr ),
		bCanClose( true )
{
	Align	= AL_Client;
	Padding	= TArea( 0, 0, 0, 0 );
}


//
// Tab page repaint.
//
void WTabPage::OnPaint( CGUIRenderBase* Render ) 
{
	WContainer::OnPaint( Render );

#if 0
	// Debug draw.
	Render->DrawRegion
			( 
				ClientToWindow( TPoint( 0, 0 ) ),
				Size,
				Color * COLOR_CornflowerBlue,
				Color * COLOR_CornflowerBlue,
				BPAT_Diagonal 
			);
#endif
}


//
// Close the tab page.
//
void WTabPage::Close( Bool bForce )
{
	assert(TabControl != nullptr);
	assert(bCanClose == true);
	TabControl->CloseTabPage( TabControl->Pages.find(this), bForce );
}


/*-----------------------------------------------------------------------------
    WTabControl implementation.
-----------------------------------------------------------------------------*/

//
// Tabs control constructor.
//
WTabControl::WTabControl( WContainer* InOwner, WWindow* InRoot )
	:	WContainer( InOwner, InRoot ),
		Pages(),
		iActive( -1 ),
		iHighlight( -1 ),
		iCross( -1 ),
		DragPage( nullptr ),
		bWasDrag( false ),
		bOverflow( false )
{
	// Grab some place for header.
	SetHeaderSide(ETH_Top);
	Align		= AL_Client;	

	// Overflow popup menu.
	Popup		= new WPopupMenu( this, InRoot );
}


//
// Set header side.
//
void WTabControl::SetHeaderSide( ETabHeaderSide InSide )
{
	HeaderSide = InSide;
	if( InSide == ETH_Bottom )
	{
		// Down.
		Padding		= TArea( 0, 21, 0, 0 );
	}
	else
	{
		// Up.
		Padding		= TArea( 21, 0, 0, 0 );
	}

	AlignChildren();
}


//
// Draw a tab control.
//
void WTabControl::OnPaint( CGUIRenderBase* Render )
{
	// Call parent.
	WContainer::OnPaint( Render );
		
	// Precompute.
	TPoint Base		= ClientToWindow(TPoint::Zero);
	Int32	TextY	= HeaderSide == ETH_Top ? 
						Base.Y + (19-Root->Font1->Height) / 2 : 
						Base.Y+Size.Height - Root->Font1->Height - (19-Root->Font1->Height) / 2;
	Int32 XWalk	= Base.X;

	// Draw a master bar.
	Render->DrawRegion
	( 
		HeaderSide==ETH_Top ? 
			Base : 
			TPoint(Base.X, Base.Y+Size.Height-20),
		TSize( Size.Width, 20 ),
		GUI_COLOR_PANEL,
		/*GUI_COLOR_PANEL_BORDER*/GUI_COLOR_PANEL, 
		BPAT_Solid 
	);	


	// Draw headers.
	math::Color	ActiveColor;
	Int32 iPage;
	for( iPage=0; iPage<Pages.size(); iPage++ )
	{
		WTabPage* Page = Pages[iPage];

		// Test, maybe list overflow.
		if( XWalk+Page->TabWidth >= Base.X+Size.Width-15 )
			break;

		// Special highlight required?
		if( iPage == iActive || iPage == iHighlight )
		{
			math::Color DrawColor	= iPage == iActive ? Page->Color : math::Color( 0x1d, 0x1d, 0x1d, 0xff ) + Page->Color;
			math::Color DarkColor	= DrawColor * math::Color( 0xdd, 0xdd, 0xdd, 0xff );

			// Fade color, if not in focus.
			if( iPage == iActive )
			{
#if FADE_TAB
				if( !IsChildFocused() )
					DrawColor	= TColor( 0x50, 0x50, 0x50, 0xff )+DrawColor*0.35f;
#endif
				ActiveColor	= DrawColor;
			}

			Render->DrawRegion
			( 
				TPoint( XWalk,  HeaderSide == ETH_Top ? Base.Y : Base.Y+Size.Height-19 ), 
				TSize( Page->TabWidth, 19 ), 
				DrawColor, 
				DrawColor, 
				BPAT_Solid 
			);

			if( Page->bCanClose )
			{
				// Highlight selected cross.
				if( iCross == iPage )
					Render->DrawRegion
					( 
						TPoint( XWalk+Page->TabWidth - 21, HeaderSide == ETH_Top ? 
							Base.Y + 3 : Base.Y+Size.Height-16 ), 
						TSize( 13, 13 ), 
						DarkColor, 
						DarkColor,
						BPAT_Solid  
					);

				// Draw cross.
				Render->DrawPicture
				( 
					TPoint( XWalk+Page->TabWidth - 20, HeaderSide == ETH_Top ? 
						Base.Y + 4 : Base.Y+Size.Height-15 ), 
					TSize( 11, 11 ), 
					TPoint( 0, 11 ), 
					TSize( 11, 11 ), 
					Root->Icons 
				);
			}
		}

		// Draw tab title.
		Render->DrawText
		( 
			TPoint( XWalk + 5, TextY ), 
			Page->Caption,  
			GUI_COLOR_TEXT, 
			//COLOR_White,
			Root->Font1 
		);

		// To next.
		XWalk+= Page->TabWidth;
	}

	// Draw a little marker if list overflow.
	if( bOverflow = iPage < Pages.size() )
	{
		TPoint Cursor = Root->MousePos;
		TPoint Start  = TPoint( Base.X+Size.Width-14, HeaderSide == ETH_Top ? Base.Y+3 : Base.Y+Size.Height-17 );

		if	( 
				Cursor.X>=Start.X && Cursor.Y>=Start.Y && 
				Cursor.X<=Start.X+14 && Cursor.Y<=Start.Y+14 
			)
				Render->DrawRegion
				(
					Start,
					TSize( 14, 14 ),
					GUI_COLOR_BUTTON_HIGHLIGHT,
					GUI_COLOR_BUTTON_HIGHLIGHT,
					BPAT_Solid
				);

		Render->DrawPicture
		(
			TPoint( Base.X+Size.Width-10, Start.Y+6 ),
			TSize( 7, 4 ), 
			TPoint( 0, 32 ),
			TSize( 7, 4 ),
			Root->Icons
		);
	}

	// Draw a tiny color bar according to active page.
	if( iActive != -1 )
	{
		WTabPage* Active = Pages[iActive];

		Render->DrawRegion
		(
			TPoint( Base.X, HeaderSide == ETH_Top ? Base.Y + 19 : Base.Y + Size.Height - 21 ), 
			TSize( Size.Width, 2 ), 
			Active->Color, 
			Active->Color, 
			BPAT_Solid 
		);
	}
}


//
// Return the active page, if
// no page active return nullptr.
//
WTabPage* WTabControl::GetActivePage()
{
	return iActive!=-1 && iActive<Pages.size() ? Pages[iActive] : nullptr;
}


//
// Add a new tab and activate it.
//
Int32 WTabControl::AddTabPage( WTabPage* Page )
{
	Int32 iNew		= Pages.push(Page);
	Page->TabControl	= this;
	
	ActivateTabPage( iNew );

	return iNew;
}


//
// Close tab by its index.
//
void WTabControl::CloseTabPage( Int32 iPage, Bool bForce )
{
	assert(iPage>=0 && iPage<Pages.size());

	WTabPage* P = Pages[iPage];

	// Is tab prepared for DIE?
	if( P->bCanClose && (bForce || P->OnQueryClose()) )
	{
		Pages.removeShift(iPage);

		if( Pages.size() )
		{
			// Open new item or just close.
			if( iActive == iPage )
			{
				// Open new one.
				ActivateTabPage( Pages.size()-1 );
				iHighlight	= -1;
			}
			else
			{
				// Save selection.
				iActive = iActive > iPage ? iActive-1 : iActive;
			}
		}
		else
		{
			// Nothing to open, all tabs are closed.
			iHighlight	= iActive = -1;
		}

		// Destroy widget.
		delete P;
	}
}


//
// Mouse click on tabs bar.
//
void WTabControl::OnMouseUp( EMouseButton Button, Int32 X, Int32 Y )
{
	WContainer::OnMouseUp( Button, X, Y );

	iCross	= XYToCross( X, Y );		

	// Close tab if cross pressed.
	if( !bWasDrag && 
		Button == MB_Left && 
		iCross != -1 )
	{
		CloseTabPage(iCross);
	}

	// Close tab via middle button.
	if( Button == MB_Middle )
	{
		Int32 iPage = XToIndex(X);

		if( iPage != -1 )
			CloseTabPage(iPage);
	}

	// No more drag.
	DragPage	= nullptr;
	bWasDrag	= false;
}


//
// When mouse hover tabs bar.
//
void WTabControl::OnMouseMove( EMouseButton Button, Int32 X, Int32 Y )
{
	WContainer::OnMouseMove( Button, X, Y );

	if( !DragPage )
	{
		// No drag, just highlight.
		iHighlight	= XToIndex( X );
		iCross		= XYToCross( X, Y );
	}
	else
	{
		// Process tab drag.
		Int32 XWalk = 0;
		for( Int32 i=0; i<Pages.size(); i++  )
			if( Pages[i] != DragPage )
				XWalk += Pages[i]->TabWidth;
			else
				break;

		while( X < XWalk )
		{
			Int32 iDrag = Pages.find( DragPage );
			if( iDrag <= 0 ) break;
			XWalk -= Pages[iDrag-1]->TabWidth;
			Pages.swap( iDrag, iDrag-1 );
		}
		while( X > XWalk+DragPage->TabWidth )
		{
			Int32 iDrag = Pages.find( DragPage );
			if( iDrag >= Pages.size()-1 ) break;
			XWalk += Pages[iDrag+1]->TabWidth;
			Pages.swap( iDrag, iDrag+1 );
		}

		// Refresh highlight.
		iActive		= iHighlight = Pages.find( DragPage );
		bWasDrag	= true;
	}
}


//
// When mouse press tabs bar.
//
void WTabControl::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y )
{
	WContainer::OnMouseDown( Button, X, Y );

	// Test maybe clicked on little triangle.
	if( bOverflow && Button==MB_Left && X > Size.Width-14 )
	{
		// Setup popup and show it.
		Popup->Items.empty();
		for( Int32 i=0; i<Pages.size(); i++ )
			Popup->AddItem( Pages[i]->Caption, WIDGET_EVENT(WTabControl::PopupPageClick), true );

		Popup->Show( TPoint( Size.Width, HeaderSide==ETH_Top ? 20 : Size.Height-20 ) );
		return;
	}

	Int32 iClicked	= XToIndex(X);
	iCross				= XYToCross( X, Y );

	// Activate pressed.
	if( iCross == -1 && 
		iClicked != -1 && 
		iClicked != iActive &&
		!DragPage &&
		Button != MB_Middle )
	{
		ActivateTabPage( iClicked );
	}

	// Prepare to drag active page.
	bWasDrag	= false;
	if( Button == MB_Left && iActive != -1 )
		DragPage = Pages[iActive];
}


//
// Activate an iPage.
//
void WTabControl::ActivateTabPage( Int32 iPage )
{
	assert(iPage>=0 && iPage<Pages.size());

	// Hide all.
	for( Int32 i=0; i<Pages.size(); i++ )
		Pages[i]->bVisible	= false;

	// Show and activate.
	Pages[iPage]->bVisible	= true;
	Pages[iPage]->OnOpen();
	iActive	= iPage;

	// Realign.
	AlignChildren();
	//Pages[iPage]->AlignChildren();
	Pages[iPage]->WidgetProc( WPE_Resize, TWidProcParms() );

	// Make it focused.
	Root->SetFocused( this );
}


//
// Activate an Page.
//
void WTabControl::ActivateTabPage( WTabPage* Page )
{
	Int32 iPage = Pages.find( Page );
	assert(iPage != -1);
	ActivateTabPage( iPage );
}


//
// When mouse leave tabs.
//
void WTabControl::OnMouseLeave()
{
	WContainer::OnMouseLeave();

	// No highlight any more.
	iHighlight	= -1;
	iCross		= -1;
}


//
// Convert mouse X to tab index, if mouse outside
// any tabs return -1.
//
Int32 WTabControl::XToIndex( Int32 InX )
{
	Int32 XWalk = 0;

	for( Int32 i=0; i<Pages.size(); i++ )
	{
		WTabPage* Page = Pages[i];
			
		if( XWalk+Page->TabWidth > Size.Width-15 )
			break;

		if( ( InX >= XWalk ) &&
			( InX <= XWalk + Page->TabWidth ) )
				return i;		

		XWalk += Page->TabWidth;
	}

	return -1;
}


//
// Figure out index of close cross on tab.
// It no cross around - return -1.
//
Int32 WTabControl::XYToCross( Int32 InX, Int32 InY )
{
	Int32 XWalk = 0;

	// Fast Y rejection.
	if( HeaderSide == ETH_Top )
	{
		if( InY <= 4 || InY >= 15 )
			return -1;
	}
	else
	{
		if( InY > Size.Height-4 || InY <= Size.Height-15 )
			return -1;
	}

	// Iterate through pages list.
	for( Int32 i=0; i<Pages.size(); i++ )
	{
		WTabPage* Page = Pages[i];
			
		if( Page->bCanClose )
		{
			if( XWalk+Page->TabWidth > Size.Width-15 )
				break;

			if( ( InX > XWalk+Page->TabWidth-20 ) &&
				( InX < XWalk+Page->TabWidth-5 ) )
					return i;
		}
			
		XWalk += Page->TabWidth;
	}

	return -1;
}


//
// Page selected.
//
void WTabControl::PopupPageClick( WWidget* Sender )
{
	// Figure out chosen page.
	WTabPage* Chosen = nullptr;
	for( Int32 i=0; i<Popup->Items.size(); i++ )
		if( Popup->Items[i].bChecked )
		{
			Chosen	= Pages[i];
			break;
		}
	assert(Chosen);

	// Carousel pages, to make chosen first.
	while( Pages[0] != Chosen )
	{
		WTabPage* First = Pages[0];
		for( Int32 i=0; i<Pages.size()-1; i++ )
			Pages[i] = Pages[i+1];
		Pages[Pages.size()-1]	= First;
	}

	// Make first page active now.
	ActivateTabPage( 0 );
}


//
// Tab control has been activated.
//
void WTabControl::OnActivate()
{
	WContainer::OnActivate();

	// Pages switching failure :(
#if 0
	// Activate selected page.
	WTabPage* Page = GetActivePage();
	if( Page )
		Root->SetFocused( Page );
#endif
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/