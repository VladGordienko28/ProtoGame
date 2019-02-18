/*=============================================================================
	FrTree.cpp: WTreeView implementation.
	Created by Vlad Gordienko, Apr. 2018.
=============================================================================*/

#include "GUI.h"

//
// TreeView constants.
//
#define TREEVIEW_NODES_INTERVAL	3


/*-----------------------------------------------------------------------------
	WTreeView implementation.
-----------------------------------------------------------------------------*/

//
// Tree constructor.
//
WTreeView::WTreeView( WContainer* InOwner, WWindow* InRoot )
	:	WContainer( InOwner, InRoot ),
		Nodes(),
		iSelected( -1 ),
		RenderOrder(),
		ScrollTop( 0 ),
		CharHeight( Root->Font1->Height )
{
	// Allocate scrollbar.
	ScrollBar				= new WSlider( this, InRoot );
	ScrollBar->Align		= AL_Right;
	ScrollBar->EventChange	= WIDGET_EVENT(WTreeView::ScrollBarChange);
	ScrollBar->SetSize( 12, 50 );
	ScrollBar->SetOrientation( SLIDER_Vertical );

	// Initialize own fields.
	SetSize( 300, 400 );
}


//
// Add a new node to the tree.
//
Int32 WTreeView::AddNode( String InName, Int32 IniParent, void* InData )
{
	assert(IniParent==-1 || (IniParent>=0 && IniParent<Nodes.size()));

	Int32 Level = IniParent != -1 ? Nodes[IniParent].Level+1 : 1;
	Int32 iThis = Nodes.push(TNode( InName, Level, IniParent, InData ));

	// Sort items for rendering.
	ComputeOrder();

	return iThis;
}


//
// Select a next node.
//
void WTreeView::SelectNext()
{
	if( RenderOrder.size() == 0 )
		return;
	
	Int32 iOrdered = RenderOrder.find(iSelected);
	if( iOrdered == -1 )
	{
		iSelected = 0;
	}
	else
	{
		if( iOrdered < RenderOrder.size()-1 )
			iSelected = RenderOrder[iOrdered+1];
	}

	OnChange();
	ScrollToNode(iSelected);
}


//
// Select a prev node.
//
void WTreeView::SelectPrev()
{
	if( RenderOrder.size() == 0 )
		return;

	Int32 iOrdered = RenderOrder.find(iSelected);
	if( iOrdered == -1 )
	{
		iSelected = RenderOrder.last();
	}
	else
	{
		if( iOrdered > 0 )
			iSelected = RenderOrder[iOrdered-1];
	}

	OnChange();
	ScrollToNode(iSelected);
}


//
// User click on treeview.
//
void WTreeView::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y )
{ 
	WContainer::OnMouseDown( Button, X, Y );

	if( Button != MB_Left )
		return;

	// Hit something.
	Bool bAtSign = false;
	Int32 iHit = XYToIndex( X, Y, &bAtSign );

	if( iHit == -1 )
		return;

	TNode& HitNode = Nodes[iHit];

	if( HitNode.NumChildren != 0 && bAtSign )
	{
		// Toggle expand/collapse.
		HitNode.bExpanded ^= 1;
		ComputeOrder();
	}

	// Select this node.
	iSelected = iHit;
}


//
// Find a node by name and parent.
//
Int32 WTreeView::FindNode( String TestName, Int32 iParent )
{
	for( Int32 i=0; i<Nodes.size(); i++ )
		if( TestName == Nodes[i].Name )
		{
			if( iParent != -1 && !IsParentNode(i, iParent) )
				continue;

			return i;
		}

	// Nothing found.
	return -1;
}


//
// On resized.
//
void WTreeView::OnResize()
{
	WContainer::OnResize();
	ScrollToNode(iSelected);
}


//
// When user release button.
//
void WTreeView::OnMouseUp( EMouseButton Button, Int32 X, Int32 Y )
{
	WContainer::OnMouseUp( Button, X, Y );
}


//
// Scroll tree to show iNode.
//
void WTreeView::ScrollToNode( Int32 iNode )
{
	if( iNode == -1 )
		return;

	Int32 iOrdered = RenderOrder.find(iNode);
	if( iOrdered == -1 )
		return;

	Int32 NumVis	= Size.Height / (CharHeight+TREEVIEW_NODES_INTERVAL);

	// Scroll from the current location.
	while( iOrdered < ScrollTop )				ScrollTop--;
	while( iOrdered >= ScrollTop+NumVis )		ScrollTop++;

	// Update scroll bar.
	ScrollBar->Value	= 100*ScrollTop / Max( RenderOrder.size()-1, 1 );
}


//
// Tree redraw.
//
void WTreeView::OnPaint( CGUIRenderBase* Render )
{
	WContainer::OnPaint(Render);
	TPoint Base = ClientToWindow(TPoint::Zero);

	// Draw background.
	Render->DrawRegion
	(
		Base,
		Size,
		GUI_COLOR_TREEVIEW_BACKGROUND,
		GUI_COLOR_TREEVIEW_BORDER,
		BPAT_Solid
	);

	// Visible lines bounds.
	Int32 iVisFirst	= ScrollTop;
	Int32 iVisLast	= Min( ScrollTop + Size.Height/(TREEVIEW_NODES_INTERVAL+CharHeight), RenderOrder.size()-1 );

	// Draw root line.
	if( RenderOrder.size() != 0 )
		Render->DrawRegion
		(
			TPoint( Base.X + 8, Base.Y + 1 ),
			TSize( 1, (FindLastChildren(-1)+1-ScrollTop)*(TREEVIEW_NODES_INTERVAL+CharHeight) - CharHeight/2 ),
			GUI_COLOR_TREEVIEW_LINES,
			GUI_COLOR_TREEVIEW_LINES,
			BPAT_Solid
		);

	// Draw all vertical lines.
	for( Int32 i=0; i<RenderOrder.size(); i++ )
	{
		Int32 iNode = RenderOrder[i];
		TNode& Node = Nodes[iNode];

		// Draw vertical line, if required.
		if( Node.NumChildren != 0 && Node.bExpanded )
		{
			Int32 LineX = Base.X + 20 * Node.Level;
			Int32 LineYBegin = (i-ScrollTop) * (TREEVIEW_NODES_INTERVAL + CharHeight) + TREEVIEW_NODES_INTERVAL;
			Int32 LineYEnd = (FindLastChildren(iNode)-ScrollTop)*(TREEVIEW_NODES_INTERVAL+CharHeight) - TREEVIEW_NODES_INTERVAL;

			LineYBegin = Clamp( LineYBegin, 1, Size.Height-1 );
			LineYEnd = Clamp( LineYEnd, 1, Size.Height-1 );

			if( (LineYBegin >= Size.Height) )
				break;

			if( LineYBegin != LineYEnd )
				Render->DrawRegion
				(
					TPoint( LineX+8, Base.Y + LineYBegin + CharHeight ),
					TSize( 1, LineYEnd - LineYBegin-1 ),
					GUI_COLOR_TREEVIEW_LINES,
					GUI_COLOR_TREEVIEW_LINES,
					BPAT_Solid
				);
		}
	}

	// Draw nodes.
	Int32 YWalk = TREEVIEW_NODES_INTERVAL;

	for( Int32 i=iVisFirst; i<=iVisLast; i++ )
	{
		Int32 iNode = RenderOrder[i];
		TNode& Node = Nodes[iNode];
		Int32 LabelX = Base.X + 20 * Node.Level;
		Int32 LabelY = Base.Y + YWalk;

		// Selection mark, if required.
		if( iNode == iSelected )
		{
			Int32 TextWidth = Root->Font1->TextWidth(*Node.Name);

			Render->DrawRegion
			(
				TPoint( LabelX-1, LabelY ),
				TSize( TextWidth+2, CharHeight ),
				GUI_COLOR_TREEVIEW_SELECTED,
				GUI_COLOR_TREEVIEW_SELECTED,
				BPAT_Solid
			);
		}

		// Draw text.
		Render->DrawText
		( 
			TPoint(LabelX, LabelY),
			Node.Name,
			GUI_COLOR_TEXT,
			Root->Font1
		);

		// Draw horizontal line.
		Render->DrawRegion
		(
			TPoint( LabelX-12, LabelY + CharHeight/2 ),
			TSize( 10, 1 ),
			GUI_COLOR_TREEVIEW_LINES,
			GUI_COLOR_TREEVIEW_LINES,
			BPAT_Solid
		);

		// Draw sign if can.
		if( Node.NumChildren != 0 )
			Render->DrawPicture
			( 
				TPoint( LabelX-16, LabelY+3 ), 
				TSize( 9, 9 ), 
				TPoint( Node.bExpanded ? 30 : 21, 0 ), 
				TSize( 9, 9 ), 
				Root->Icons 
			);

		// Advance.
		YWalk += TREEVIEW_NODES_INTERVAL + CharHeight;
	}
}


//
// Conver pixels point to index of node.
//
Int32 WTreeView::XYToIndex( Int32 X, Int32 Y, Bool* AtSign )
{
	Int32 YWalk = TREEVIEW_NODES_INTERVAL;

	if( AtSign )
		*AtSign = false;

	for( Int32 i=ScrollTop; i<RenderOrder.size(); i++ )
	{
		// Precompute.
		Int32 iNode = RenderOrder[i];
		TNode& Node = Nodes[iNode];
		Int32 LabelX = 20 * Node.Level - 1;
		Int32 LabelY = YWalk;

		// Advance Y.
		YWalk += TREEVIEW_NODES_INTERVAL + CharHeight;

		// Test Y.
		if( inRange( Y, LabelY, YWalk ) )
		{
			// Test X.
			Int32 TextWidth = Root->Font1->TextWidth(*Node.Name) + 2;

			if( inRange( X, LabelX, LabelX+TextWidth ) )
			{
				return iNode;
			}
			else if( AtSign && inRange( X, LabelX-20, LabelX ) )
			{
				*AtSign = true;
				return iNode;
			}
			else
				return -1;
		}
	}

	// Nothing found.
	return -1;
}


//
// Selection change notification.
//
void WTreeView::OnChange()
{
	EventClick( this );
}


//
// Double click notification.
//
void WTreeView::OnDoubleClick()
{
	if( iSelected != -1 )
	{
		// Toggle node.
		TNode& Node = Nodes[iSelected];
		Node.bExpanded ^= 1;
		ComputeOrder();
	}

	EventDblClick( this );
}


//
// Recursive minon of WTreeView::ComputeOrder.
//
void ComputeOrderMinion( Int32 iParent, Array<Int32>& RenderOrder, Array<WTreeView::TNode>& Nodes )
{
	assert(iParent != -1);

	RenderOrder.push(iParent);
	Nodes[iParent].NumChildren = 0;
	Bool bExpanded = Nodes[iParent].bExpanded;

	for( Int32 i=0; i<Nodes.size(); i++ )
		if( Nodes[i].iParent == iParent )
		{
			if( bExpanded )
				ComputeOrderMinion( i, RenderOrder, Nodes );

			Nodes[iParent].NumChildren++;
		}
}


//
// Computes proper order for Nodes.
//
void WTreeView::ComputeOrder()
{
	RenderOrder.empty();

	for( Int32 i=0; i<Nodes.size(); i++ )
		if( Nodes[i].iParent == -1 )
			ComputeOrderMinion( i, RenderOrder, Nodes );
}


//
// A keyboard button had been pressed.
//
void WTreeView::OnKeyDown( Int32 Key )
{
	WContainer::OnKeyDown(Key);

	if( Key == KEY_Up )
		SelectPrev(); 
	else if( Key == KEY_Down )
		SelectNext();
}


//
// User double clicks on tree.
//
void WTreeView::OnDblClick( EMouseButton Button, Int32 X, Int32 Y )
{
	WContainer::OnDblClick( Button, X, Y );
	OnDoubleClick();
}


//
// Erase the tree.
//
void WTreeView::Empty()
{
	Nodes.empty();
	ComputeOrder();
}


//
// Return true if TestParent is parent node of iNode.
//
Bool WTreeView::IsParentNode( Int32 iNode, Int32 TestParent ) const
{
	if( iNode == TestParent )
		return false;

	while( iNode != -1 )
	{
		const TNode& Node = Nodes[iNode];

		if( iNode == TestParent )
			return true;

		iNode = Node.iParent;
	}

	return false;
}



//
// Expand all nodes.
//
void WTreeView::ExpandAll()
{
	for( Int32 i=0; i<Nodes.size(); i++ )
		Nodes[i].bExpanded = true;

	ComputeOrder();
	OnChange();
}


//
// Collapse all nodes.
//
void WTreeView::CollapseAll()
{
	for( Int32 i=0; i<Nodes.size(); i++ )
		Nodes[i].bExpanded = false;

	ComputeOrder();
	iSelected = -1;
	OnChange();
}


//
// Alphabet sort nodes.
//
void WTreeView::AlphabetSort()
{
	Nodes.sort([]( const TNode& A, const TNode& B )->Bool
	{
		if( A.iParent == B.iParent )
		{
			return String::CompareText( A.Name, B.Name ) < 0;
		}
		else
			return false;
	});

	ComputeOrder();
	iSelected = -1;
	OnChange();
}


//
// Scroll lines.
//
void WTreeView::OnMouseScroll( Int32 Delta )
{
	WContainer::OnMouseScroll(Delta);

	// Scroll text in aspect 1:3.
	ScrollTop	-= Delta / 40;
	ScrollTop	= Clamp( ScrollTop, 0, RenderOrder.size()-1 );

	// Update scroll bar.
	ScrollBar->Value	= 100*ScrollTop / Max( RenderOrder.size()-1, 1 );
}


//
// Scroll text via left scroll bar.
//
void WTreeView::ScrollBarChange( WWidget* Sender )
{
	ScrollTop	= ScrollBar->Value * (RenderOrder.size()-1) / 100;
	ScrollTop	= Clamp( ScrollTop, 0, RenderOrder.size()-1 );
}


//
// Return data of i'th node.
//
void* WTreeView::DataOf( Int32 iNode )
{
	return iNode != -1 ? Nodes[iNode].Data : nullptr;
}


//
// Find a last render item with parent.
//
Int32 WTreeView::FindLastChildren( Int32 iParent )
{
	for( Int32 i=RenderOrder.size()-1; i>=0; i-- )
		if( Nodes[RenderOrder[i]].iParent == iParent )
			return i;

	return -1;
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/