/*=============================================================================
    FrList.cpp: List based widgets.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#include "GUI.h"

/*-----------------------------------------------------------------------------
    WComboBox implementation.
-----------------------------------------------------------------------------*/

//
// ComboBox constructor.
//
WComboBox::WComboBox( WContainer* InOwner, WWindow* InRoot )
	:	WList( InOwner, InRoot ),
		bHighlight( false )
{
	// Create dropdown list.
	DropList				= new WListBox( Root, Root );	
	DropList->bStayOnTop	= true;
	DropList->EventChange	= TNotifyEvent( this, (TNotifyEvent::TEvent)&WComboBox::DropListChanged );

	// Set drop list really on top, sort of hack, but
	// works well. Because DropList should stay on the top of top!
	for( Int32 i=Root->Children.find(DropList); i<Root->Children.size()-1; i++ )
		Root->Children.swap( i, i+1 );

	HideDropList();
	SetSize( 150, 18 );
}


//
// ComboBox destructor.
//
WComboBox::~WComboBox()
{
	// Figure out, is DropList still valid?
	if( DropList && (Root->Children.find(DropList) != -1) )
		delete DropList;
}


//
// ComboBox paint.
//
void WComboBox::OnPaint( CGUIRenderBase* Render )
{
	WList::OnPaint( Render );

	TPoint Base = ClientToWindow(TPoint::Zero);

	// Draw pad.
	Render->DrawRegion
				(
					Base,
					Size,
					GUI_COLOR_LIST,
					GUI_COLOR_LIST_BORDER,
					BPAT_Solid 
				);

	// Highlight button.
	if( bHighlight || IsExpanded() )
		Render->DrawRegion
					( 
						TPoint( Base.X + Size.Width - 18, Base.Y ),
						TSize(18, Size.Height), 
						IsExpanded() ? GUI_COLOR_LIST_SEL : GUI_COLOR_LIST_HIGHLIGHT, 
						GUI_COLOR_LIST_BORDER,
						BPAT_Solid 
					);

	// Draw a little triangle.
	Render->DrawPicture
				( 
					TPoint( Base.X+Size.Width-12, Base.Y+Size.Height/2-2 ), 
					TSize(6, 3), 
					TPoint(15, 0), 
					TSize(6, 3), 
					Root->Icons 
				);

	// Draw selected item text.
	if( ItemIndex != -1 )
	{
		TListItem& Item	= Items[ItemIndex];
		TPoint TextPos	= TPoint( Base.X + 3, Base.Y + (Size.Height-Root->Font1->Height) / 2 );
		Render->SetClipArea( Base, TSize( Size.Width-19, Size.Height ) );
		Render->DrawText( TextPos, Item.Name, bEnabled ? GUI_COLOR_TEXT : GUI_COLOR_TEXT_OFF, Root->Font1 );
	}	
}


//
// Dbl clicked - increment index.
//
void WComboBox::OnDblClick( EMouseButton Button, Int32 X, Int32 Y )
{
	WList::OnDblClick( Button, X, Y );

	if( Items.size() && X < Size.Width-18 )
	{
		ItemIndex = ( ItemIndex + 1 ) % Items.size();	  
		OnChange();
	}
}


//
// Mouse press comboBox.
//
void WComboBox::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y )
{
	WList::OnMouseDown( Button, X, Y );

	if( Button == MB_Left && X > Size.Width-18 )
	{
		// Toggle.
		if( IsExpanded() )		
			HideDropList();
		 else
			ShowDropList();
	}
}


//
// Cursor enter area.
//
void WComboBox::OnMouseEnter()
{
	WList::OnMouseEnter();
	bHighlight	= true;
}


//
// Cursor leave area.
//
void WComboBox::OnMouseLeave()
{
	WList::OnMouseLeave();
	bHighlight	= false;
}


//
// Open up drop list.
//
void WComboBox::ShowDropList()
{
	// Update items list.
	DropList->Empty();
	for( Int32 i=0; i<Items.size();i++ )
		DropList->AddItem( Items[i].Name, Items[i].Data );

	// Compute show location and size.
	TPoint P = ClientToWindow(TPoint::Zero);
	//DropList->SetSize( Size.Width, DropList->ItemsHeight*Items.Num()+3 );
	DropList->Size.Width	= Size.Width;
	DropList->Size.Height	= DropList->ItemsHeight*Items.size()+3;

	// Fuck! Fucked Visual Studio crashes when debug step-by-step
	// if without variable! Crap!
	Int32 MaxY = Root->Size.Height;

	// Show it upward or downward.
	if( P.Y+Size.Height+DropList->Size.Height > MaxY )
		P.Y -= DropList->Size.Height-1;	
	else
		P.Y += Size.Height-1;

	DropList->Location = P;
	
	// Show.log( L"%i", Root->Size.Height );
	DropList->bVisible = true;

	// Set drop list really on top, sort of hack, but
	// works well. Because DropList should stay on the top of top!
	for( Int32 i=Root->Children.find(DropList); i<Root->Children.size()-1; i++ )
		Root->Children.swap( i, i+1 );
}


//
// Shut down drop list.
//
void WComboBox::HideDropList()
{
	DropList->bVisible = false;
}


//
// Return true, if drop list are visible.
//
Bool WComboBox::IsExpanded()
{
	return DropList->bVisible;
}


//
// Notification from drop list.
//
void WComboBox::DropListChanged( WWidget* Sender )
{
	ItemIndex = DropList->ItemIndex;
	OnChange();
	HideDropList();
}


//
// ComboBox lost focus.
//
void WComboBox::OnDeactivate()
{
	WList::OnDeactivate();
	HideDropList();
}


/*-----------------------------------------------------------------------------
    WListBox implementation.
-----------------------------------------------------------------------------*/

//
// ListBox constructor.
//
WListBox::WListBox( WContainer* InOwner, WWindow* InRoot )
	: WList( InOwner, InRoot ),
	  iHighlight( -1 ),
	  ItemsHeight( 12 )
{
	// Allocate slider.
	Slider = new WSlider( this, Root );
	Slider->SetOrientation( SLIDER_Vertical );
	Slider->SetValue( 0 );
	SetSize( 150, 110 );		
}


//
// When listbox resized.
//
void WListBox::OnResize()
{
	WList::OnResize();
	Slider->SetLocation( Size.Width-12, 0 );
	Slider->SetSize( 12, Size.Height );
}


//
// When mouse leave listbox.
//
void WListBox::OnMouseLeave()
{
	WList::OnMouseLeave();	
	iHighlight = -1;
}


//
// Mouse hover list box.
//
void WListBox::OnMouseMove( EMouseButton Button, Int32 X, Int32 Y )
{
	WList::OnMouseMove( Button, X, Y );
	iHighlight = YToIndex( Y );

	if( iHighlight != -1 && !Items[iHighlight].bEnabled )
		iHighlight = -1;
}


//
// On item click.
//
void WListBox::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y ) 
{
	WList::OnMouseDown( Button, X, Y );   
	ItemIndex = YToIndex( Y );
	iHighlight = -1;

	if( ItemIndex != -1 && !Items[ItemIndex].bEnabled )
		ItemIndex = -1;

	OnChange();
}


//
// Dbl click on item.
//
void WListBox::OnDblClick( EMouseButton Button, Int32 X, Int32 Y )
{
	WList::OnDblClick( Button, X, Y );
	OnDoubleClick();
}


//
// When user press some key.
//
void WListBox::OnKeyDown( Int32 Key )
{
	WList::OnKeyDown(Key);
	if( Key == KEY_Up )		// <Up>
		SelectPrev();
	if( Key == KEY_Down )		// <Down>
		SelectNext();

	// Scroll to show selected.
	if( Slider->bVisible && Items.size()>1 )
		Slider->Value	= Clamp( ItemIndex*100/(Items.size()-1), 0, 100 );
}


//
// Mouse scroll over list box.
//
void WListBox::OnMouseScroll( Int32 Delta )
{
	WList::OnMouseScroll(Delta);
	if( Slider->bVisible )
	{
		Slider->Value	= Clamp
		( 
			Slider->Value-Delta/120, 
			0, 
			100 
		);
	}
}


//
// ListBox paint.
//
void WListBox::OnPaint( CGUIRenderBase* Render )
{
	WList::OnPaint( Render );
	TPoint Base = ClientToWindow(TPoint::Zero);
	
	// Show or hide the slider.
	if( Items.size() * ItemsHeight > Size.Height   )
	{
		Slider->bVisible = true;
	}
	else
	{
		Slider->bVisible = false;
		Slider->SetValue( 0 );
	}

	// Draw pad.
	Render->DrawRegion
			( 
				Base,
				Size,
				GUI_COLOR_LIST,
				GUI_COLOR_LIST_BORDER,
				BPAT_Solid 
			);

	// Turn on clipping.
	Render->SetClipArea
					( 
						Base, 
						TSize( Size.Width-1, Size.Height ) 
					);

	// Draw items.
	if( Items.size() > 0 )
	{
		Int32 TextY	= (ItemsHeight - Root->Font1->Height) / 2;

		// For each item.
		for( Int32 i = 0, iItem = YToIndex(0); 
			 i < Size.Height/ItemsHeight && iItem < Items.size(); 
			 i++, iItem++ )
		{			
			TListItem& Item = Items[iItem];

			if( iHighlight == iItem )
				Render->DrawRegion
						( 
							TPoint( Base.X + 1, Base.Y+i * ItemsHeight + 1 ),
							TSize( Size.Width - 2, ItemsHeight ),
							GUI_COLOR_LIST_HIGHLIGHT,
							GUI_COLOR_LIST_HIGHLIGHT,
							BPAT_Solid 
						);

			if( ItemIndex == iItem )
				Render->DrawRegion
						( 
							TPoint( Base.X + 1, Base.Y+i * ItemsHeight + 1 ),
							TSize( Size.Width - 2, ItemsHeight ),
							GUI_COLOR_LIST_SEL,
							GUI_COLOR_LIST_SEL,
							BPAT_Solid 
						);

			// Draw picture if has.
			Int32 XOffset = 3;
			if( Item.Picture )
			{
				Render->DrawPicture
				(
					TPoint( Base.X+5, Base.Y + i*ItemsHeight + (ItemsHeight-Item.PicSize.Height)/2+1 ),
					Item.PicSize,
					Item.PicOffset,
					Item.PicSize,
					Item.Picture
				);

				XOffset += Item.PicSize.Width + 5;
			}

			// Draw item text.
			TPoint TextLoc = TPoint( Base.X + XOffset, Base.Y + i*ItemsHeight + 1 + TextY );		
			Render->DrawText
					( 
						TextLoc, 
						Item.Name, 
						Item.bEnabled && bEnabled ? GUI_COLOR_TEXT : GUI_COLOR_TEXT_OFF, 
						Root->Font1 
					);
		}
	}
}


//
// convert Y value to index, also handle
// slider offset. If no item found return -1.
//
Int32 WListBox::YToIndex( Int32 Y ) const
{
	Int32 NumVis = Size.Height / ItemsHeight;
	Int32 Offset = Round((Items.size() - NumVis) * Slider->Value / 100.f);
	Int32 Index  = Offset + Y / ItemsHeight;
	return Index < 0 ? -1 : Index >= Items.size() ? -1 : Index;
}


/*-----------------------------------------------------------------------------
    WList implementation.
-----------------------------------------------------------------------------*/

//
// List constructor.
//
WList::WList( WContainer* InOwner, WWindow* InRoot )
	:	WContainer( InOwner, InRoot ),
		Items(),
		ItemIndex( -1 )
{
}


//
// Add a new item to the list.
//
Int32 WList::AddItem( String InName, void* InData )			
{
	return Items.push(TListItem( InName, InData ));
}


//
// Add a new picture item to the list.
//
Int32 WList::AddPictureItem( String InName, FTexture* Picture, TPoint PicOffset, TSize PicSize, void* Data )
{
	return Items.push(TListItem( InName, Picture, PicOffset, PicSize, Data ));
}


//
// Remove an item from list, if invalid index,
// nothing will happened.
//
void WList::Remove( Int32 iItem )	
{
	if( iItem >= 0 && iItem < Items.size() )
	{
		Items.removeShift(iItem);
		ItemIndex = Clamp( ItemIndex, -1, Items.size()-1 );	
	}
}


//
// Cleanup the list.
//
void WList::Empty()	
{
	Items.empty();
	ItemIndex = -1;
}


//
// Update selected item.
//
void WList::SetItemIndex( Int32 NewIdx, Bool bNotify )	
{
	ItemIndex = Clamp( NewIdx, -1, Items.size()-1 );

	if( bNotify )
		OnChange();
}


//
// Selected item changed notification.
//
void WList::OnChange()	
{
	EventChange( this );
}


//
// Double click on item notification.
//
void WList::OnDoubleClick()	
{
	EventDblClick( this );
}


//
// Sort list alphabet order.
//
void WList::AlphabetSort()
{
	Items.sort([]( const TListItem& A, const TListItem& B )->Bool
		{
			return String::CompareText( A.Name, B.Name ) < 0;
		});
}


//
// Select next item.
//
void WList::SelectNext()
{
	if( ItemIndex < Items.size()-1 )
	{
		ItemIndex++;
		while( ItemIndex <= Items.size()-1 && !Items[ItemIndex].bEnabled )
			ItemIndex++;
		if( ItemIndex >= Items.size() )
			ItemIndex = -1;

		OnChange();
	}
}


//
// Select prev item.
//
void WList::SelectPrev()
{
	if( ItemIndex > 0 )
	{
		ItemIndex--;
		while( ItemIndex > 0 && !Items[ItemIndex].bEnabled )
			ItemIndex--;

		OnChange();
	}
}


/*-----------------------------------------------------------------------------
	WLog implementation.
-----------------------------------------------------------------------------*/

//
// Log constructor.
//
WLog::WLog( WContainer* InOwner, WWindow* InRoot )
	:	WContainer( InOwner, InRoot ),
		iFirst(-1),
		iLast(-1),
		ScrollTop(0)
{
	// Allocate scrollbar.
	ScrollBar				= new WSlider( this, InRoot );
	ScrollBar->Align		= AL_Right;
	ScrollBar->EventChange	= WIDGET_EVENT(WLog::ScrollBarChange);
	ScrollBar->SetSize( 12, 50 );
	ScrollBar->SetOrientation( SLIDER_Vertical );

	// Allocate popup menu.
	PopUp				= new WPopupMenu( InRoot, InRoot );
	PopUp->AddItem( L"Copy", WIDGET_EVENT(WLog::PopCopyClick) );
	PopUp->AddItem( L"" );
	PopUp->AddItem( L"Select All", WIDGET_EVENT(WLog::PopSelectAllClick) );	
	PopUp->AddItem( L"" );
	PopUp->AddItem( L"Delete", WIDGET_EVENT(WLog::PopDeleteClick) );	
	PopUp->AddItem( L"" );
	PopUp->AddItem( L"Goto...", WIDGET_EVENT(WLog::PopGotoClick) );	
	PopUp->AddItem( L"" );
	PopUp->AddItem( L"To Next", WIDGET_EVENT(WLog::PopToNextClick) );	
	PopUp->AddItem( L"To Previous", WIDGET_EVENT(WLog::PopToPrevClick) );	

	// Initialize own fields.
	SetSize( 300, 200 );
}


//
// Log stream destructor.
//
WLog::~WLog()
{
	// Since Root owns Popup.
	delete PopUp;
}


//
// User release cursor button.
//
void WLog::OnMouseUp( EMouseButton Button, Int32 X, Int32 Y )
{
	if( Button == MB_Left )
	{
		// Sort for copying.
		if( iLast < iFirst )
			Exchange( iFirst, iLast );

		// Unselect if out of list.
		if( iLast == -1 || iFirst == -1 )
		{
			iLast = iFirst = -1;
		}
	}
}


//
// Goto clicked.
//
void WLog::PopGotoClick( WWidget* Sender )
{
	if( iFirst != -1 && iLast == iFirst )
	{
		OnGoto(iFirst);
	}
}


//
// User moves cursor on log.
//
void WLog::OnMouseMove( EMouseButton Button, Int32 X, Int32 Y )
{
	if( Button == MB_Left )
	{
		// Select group of items.
		Int32 iBelow = YToIndex(Y);
		if( iBelow != -1 )
		{
			iLast = iBelow;
			
			// Slowly scroll.
			if( Y < 0 )				ScrollTop = Max( 0, ScrollTop-1 );
			if( Y > Size.Height )	ScrollTop = Min( Lines.size()-1, ScrollTop+1 );
			ScrollBar->Value = 100*ScrollTop / Max( Lines.size()-1, 1 );
		}
	}
}


//
// User pressed mouse button.
//
void WLog::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y )
{
	if( Button == MB_Left )
	{
		// Start selection.
		iFirst = iLast = YToIndex(Y);
	}
	else if( Button == MB_Right )
	{
		// Show popup menu.
		Int32 iBelow = YToIndex(Y);
		if( iBelow != -1 )
		{
			if( iBelow < iFirst || iBelow > iLast )
				iFirst = iLast = iBelow;

			PopUp->Items[6].bEnabled = iFirst == iLast;
			PopUp->Items[8].bEnabled = iFirst == iLast && iFirst < Lines.size()-1;
			PopUp->Items[9].bEnabled = iFirst == iLast && iFirst > 0;
			PopUp->Show(ClientToWindow(TPoint(X, Y)));
		}
		else
		{
			iFirst = iLast = -1;
		}
	}
}


//
// Draw log.
//
void WLog::OnPaint( CGUIRenderBase* Render )
{
	TPoint Base = ClientToWindow(TPoint::Zero);
	Render->SetClipArea( Base, Size );

	// Visible lines bounds.
	Int32 iVisFirst	= ScrollTop;
	Int32 iVisLast	= Min( ScrollTop + Size.Height/13, Lines.size()-1 );

	// Draw frame.
	Render->DrawRegion
	(
		Base,
		Size,
		GUI_COLOR_LIST,
		GUI_COLOR_LIST_BORDER,
		BPAT_Solid
	);

	// Draw selection area.
	if( iFirst != -1 && iLast != -1 )
	{
		Int32 FirstLine = Min( iFirst-ScrollTop, iLast-ScrollTop );
		Int32 LastLine = Max( iFirst-ScrollTop, iLast-ScrollTop );

		// Test out of bound selection.
		Int32 Y1 = Max( 0, FirstLine*13 ) + 1;
		Int32 Y2 = Min( Size.Height-1, (LastLine+1)*13 );

		if( Y1 < Size.Height-1 && Y2 > 1 )
			Render->DrawRegion
			(
				TPoint( Base.X+1, Base.Y + Y1 ),
				TSize( Size.Width-2, Y2-Y1 ),
				GUI_COLOR_LIST_SEL,
				GUI_COLOR_LIST_SEL,
				BPAT_Solid 
			);
	}

	// Draw items.
	for( Int32 iLine=iVisFirst; iLine<=iVisLast; iLine++ )
	{
		TLine& Line = Lines[iLine];

		Render->DrawText
		( 
			Base + TPoint(10, (iLine-ScrollTop)*13), 
			Line.Text, 
			Line.Color, 
			Root->Font2 
		);
	}
}


//
// Handle key down.
//
void WLog::OnKeyDown( Int32 Key )
{
	if( iLast != -1 && iFirst != -1 )
	{
		if( Key == KEY_Up )
		{
			if( iLast == iFirst )
				iLast = iFirst = Clamp( iFirst-1, 0, Lines.size()-1 );
			else
				iLast = iFirst;

			ScrollToLast();
		}
		if( Key == KEY_Down )
		{
			if( iLast == iFirst )
				iLast = iFirst = Clamp( iFirst+1, 0, Lines.size()-1 );
			else
				iFirst = iLast;

			ScrollToLast();
		}
		if( Key == KEY_Delete )
		{
			PopDeleteClick(this);
		}
		if( Root->bCtrl && Key == KEY_C )
		{
			PopCopyClick(this);
		}
	}

	if( Root->bCtrl && Key == KEY_A )
	{
		PopSelectAllClick(this);
	}
}


//
// Scroll text via left scroll bar.
//
void WLog::ScrollBarChange( WWidget* Sender )
{
	ScrollTop	= ScrollBar->Value * (Lines.size()-1) / 100;
	ScrollTop	= Clamp( ScrollTop, 0, Lines.size()-1 );
}


//
// Scroll lines.
//
void WLog::OnMouseScroll( Int32 Delta )
{
	// Scroll text in aspect 1:3.
	ScrollTop	-= Delta / 40;
	ScrollTop	= Clamp( ScrollTop, 0, Lines.size()-1 );

	// Update scroll bar.
	ScrollBar->Value	= 100*ScrollTop / Max( Lines.size()-1, 1 );
}


//
// Scroll to iLast'th line.
//
void WLog::ScrollToLast()
{
	Int32 NumVis	= Size.Height / 13;

	// Scroll from the current location.
	while( iLast < ScrollTop )				ScrollTop--;
	while( iLast >= ScrollTop+NumVis )		ScrollTop++;

	// Update scroll bar.
	ScrollBar->Value	= 100*ScrollTop / Max( Lines.size()-1, 1 );
}


//
// Clear log.
//
void WLog::Clear()
{
	iLast = iFirst = -1;
	ScrollTop = 0;
	ScrollBar->Value = 0;
	Lines.empty();
}


//
// Return i'th item data.
//
void* WLog::DataOf( Int32 i )
{
	assert(i>=0 && i<Lines.size());
	return Lines[i].Data;
}


//
// Double click on lines.
//
void WLog::OnDblClick( EMouseButton Button, Int32 X, Int32 Y )
{
	if( Button == MB_Left )
	{
		iFirst = iLast = YToIndex(Y);
		if( iFirst != -1 )
			OnGoto(iFirst);
	}
}


//
// Convert Y location to line number.
//
Int32 WLog::YToIndex( Int32 Y ) const
{ 
	if( Lines.size() == 0 )
		return -1;

	Int32 i = Max( 0, ScrollTop + Y/13 );
	return i >= Lines.size() ? -1 : i;
}


//
// Add a new item to log.
//
Int32 WLog::AddLine( String InText, void* InData, TColor InColor )
{
	TLine Line;

	Line.Text	= InText;
	Line.Color	= InColor;
	Line.Data	= InData;

	Int32 iNew = Lines.push(Line);

	// Auto-scroll if nothing selected.
	if( iLast == -1 && iFirst == -1 )
	{
		iLast = iNew;
		ScrollToLast();
		iLast = -1;
	}

	// Update scroll bar.
	ScrollBar->Value	= 100*ScrollTop / Max( Lines.size()-1, 1 );

	return iNew;
}


//
// When something changed.
//
void WLog::OnChange()
{
	EventChange( this );
}


//
// When user wants to go to line.
//
void WLog::OnGoto( Int32 i )
{
	assert(i>=-1 && i<Lines.size());
	EventGoto( this, i );
}


//
// Copy selected text.
//
void WLog::PopCopyClick( WWidget* Sender )
{
	assert(iFirst != -1 && iLast != -1);
	Int32 i1 = Min( iFirst, iLast );
	Int32 i2 = Max( iFirst, iLast );

	if( i1 != i2 )
	{
		// Multi line.
		String Text;
		for( Int32 i=i1; i<=i2; i++ )
			Text += Lines[i].Text + L"\r\n";
		GPlat->ClipboardCopy(*Text);
	}
	else
	{
		// Single line.
		assert(i1>=0 && i1<Lines.size());
		GPlat->ClipboardCopy(*Lines[i1].Text);
	}
}


//
// Select all lines.
//
void WLog::PopSelectAllClick( WWidget* Sender )
{
	if( Lines.size() == 0 )
	{
		iLast = iFirst = -1;
		return;
	}

	iFirst = 0;
	iLast = Lines.size() - 1;
	ScrollToLast();
}


//
// Delete selected lines.
//
void WLog::PopDeleteClick( WWidget* Sender )
{
	// This method is disabled, just like in Visual Studio.
}


//
// Select next line.
//
void WLog::PopToNextClick( WWidget* Sender )
{
	iLast = ++iFirst;
	ScrollToLast();
}


//
// Select prev line.
//
void WLog::PopToPrevClick( WWidget* Sender )
{
	iLast = --iFirst;
	ScrollToLast();
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/