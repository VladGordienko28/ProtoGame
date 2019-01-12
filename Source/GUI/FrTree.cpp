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
Integer WTreeView::AddNode( String InName, Integer IniParent, void* InData )
{
	assert(IniParent==-1 || (IniParent>=0 && IniParent<Nodes.Num()));

	Integer Level = IniParent != -1 ? Nodes[IniParent].Level+1 : 1;
	Integer iThis = Nodes.Push(TNode( InName, Level, IniParent, InData ));

	// Sort items for rendering.
	ComputeOrder();

	return iThis;
}


//
// Select a next node.
//
void WTreeView::SelectNext()
{
	if( RenderOrder.Num() == 0 )
		return;
	
	Integer iOrdered = RenderOrder.FindItem(iSelected);
	if( iOrdered == -1 )
	{
		iSelected = 0;
	}
	else
	{
		if( iOrdered < RenderOrder.Num()-1 )
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
	if( RenderOrder.Num() == 0 )
		return;

	Integer iOrdered = RenderOrder.FindItem(iSelected);
	if( iOrdered == -1 )
	{
		iSelected = RenderOrder.Last();
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
void WTreeView::OnMouseDown( EMouseButton Button, Integer X, Integer Y )
{ 
	WContainer::OnMouseDown( Button, X, Y );

	if( Button != MB_Left )
		return;

	// Hit something.
	Bool bAtSign = false;
	Integer iHit = XYToIndex( X, Y, &bAtSign );

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
Integer WTreeView::FindNode( String TestName, Integer iParent )
{
	for( Integer i=0; i<Nodes.Num(); i++ )
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
void WTreeView::OnMouseUp( EMouseButton Button, Integer X, Integer Y )
{
	WContainer::OnMouseUp( Button, X, Y );
}


//
// Scroll tree to show iNode.
//
void WTreeView::ScrollToNode( Integer iNode )
{
	if( iNode == -1 )
		return;

	Integer iOrdered = RenderOrder.FindItem(iNode);
	if( iOrdered == -1 )
		return;

	Integer NumVis	= Size.Height / (CharHeight+TREEVIEW_NODES_INTERVAL);

	// Scroll from the current location.
	while( iOrdered < ScrollTop )				ScrollTop--;
	while( iOrdered >= ScrollTop+NumVis )		ScrollTop++;

	// Update scroll bar.
	ScrollBar->Value	= 100*ScrollTop / Max( RenderOrder.Num()-1, 1 );
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
	Integer iVisFirst	= ScrollTop;
	Integer iVisLast	= Min( ScrollTop + Size.Height/(TREEVIEW_NODES_INTERVAL+CharHeight), RenderOrder.Num()-1 );

	// Draw root line.
	if( RenderOrder.Num() != 0 )
		Render->DrawRegion
		(
			TPoint( Base.X + 8, Base.Y + 1 ),
			TSize( 1, (FindLastChildren(-1)+1-ScrollTop)*(TREEVIEW_NODES_INTERVAL+CharHeight) - CharHeight/2 ),
			GUI_COLOR_TREEVIEW_LINES,
			GUI_COLOR_TREEVIEW_LINES,
			BPAT_Solid
		);

	// Draw all vertical lines.
	for( Integer i=0; i<RenderOrder.Num(); i++ )
	{
		Integer iNode = RenderOrder[i];
		TNode& Node = Nodes[iNode];

		// Draw vertical line, if required.
		if( Node.NumChildren != 0 && Node.bExpanded )
		{
			Integer LineX = Base.X + 20 * Node.Level;
			Integer LineYBegin = (i-ScrollTop) * (TREEVIEW_NODES_INTERVAL + CharHeight) + TREEVIEW_NODES_INTERVAL;
			Integer LineYEnd = (FindLastChildren(iNode)-ScrollTop)*(TREEVIEW_NODES_INTERVAL+CharHeight) - TREEVIEW_NODES_INTERVAL;

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
	Integer YWalk = TREEVIEW_NODES_INTERVAL;

	for( Integer i=iVisFirst; i<=iVisLast; i++ )
	{
		Integer iNode = RenderOrder[i];
		TNode& Node = Nodes[iNode];
		Integer LabelX = Base.X + 20 * Node.Level;
		Integer LabelY = Base.Y + YWalk;

		// Selection mark, if required.
		if( iNode == iSelected )
		{
			Integer TextWidth = Root->Font1->TextWidth(*Node.Name);

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
Integer WTreeView::XYToIndex( Integer X, Integer Y, Bool* AtSign )
{
	Integer YWalk = TREEVIEW_NODES_INTERVAL;

	if( AtSign )
		*AtSign = false;

	for( Integer i=ScrollTop; i<RenderOrder.Num(); i++ )
	{
		// Precompute.
		Integer iNode = RenderOrder[i];
		TNode& Node = Nodes[iNode];
		Integer LabelX = 20 * Node.Level - 1;
		Integer LabelY = YWalk;

		// Advance Y.
		YWalk += TREEVIEW_NODES_INTERVAL + CharHeight;

		// Test Y.
		if( InRange( Y, LabelY, YWalk ) )
		{
			// Test X.
			Integer TextWidth = Root->Font1->TextWidth(*Node.Name) + 2;

			if( InRange( X, LabelX, LabelX+TextWidth ) )
			{
				return iNode;
			}
			else if( AtSign && InRange( X, LabelX-20, LabelX ) )
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
void ComputeOrderMinion( Integer iParent, TArray<Integer>& RenderOrder, TArray<WTreeView::TNode>& Nodes )
{
	assert(iParent != -1);

	RenderOrder.Push(iParent);
	Nodes[iParent].NumChildren = 0;
	Bool bExpanded = Nodes[iParent].bExpanded;

	for( Integer i=0; i<Nodes.Num(); i++ )
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
	RenderOrder.Empty();

	for( Integer i=0; i<Nodes.Num(); i++ )
		if( Nodes[i].iParent == -1 )
			ComputeOrderMinion( i, RenderOrder, Nodes );
}


//
// A keyboard button had been pressed.
//
void WTreeView::OnKeyDown( Integer Key )
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
void WTreeView::OnDblClick( EMouseButton Button, Integer X, Integer Y )
{
	WContainer::OnDblClick( Button, X, Y );
	OnDoubleClick();
}


//
// Erase the tree.
//
void WTreeView::Empty()
{
	Nodes.Empty();
	ComputeOrder();
}


//
// Return true if TestParent is parent node of iNode.
//
Bool WTreeView::IsParentNode( Integer iNode, Integer TestParent ) const
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
	for( Integer i=0; i<Nodes.Num(); i++ )
		Nodes[i].bExpanded = true;

	ComputeOrder();
	OnChange();
}


//
// Collapse all nodes.
//
void WTreeView::CollapseAll()
{
	for( Integer i=0; i<Nodes.Num(); i++ )
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
	Nodes.Sort([]( const TNode& A, const TNode& B )->Bool
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
void WTreeView::OnMouseScroll( Integer Delta )
{
	WContainer::OnMouseScroll(Delta);

	// Scroll text in aspect 1:3.
	ScrollTop	-= Delta / 40;
	ScrollTop	= Clamp( ScrollTop, 0, RenderOrder.Num()-1 );

	// Update scroll bar.
	ScrollBar->Value	= 100*ScrollTop / Max( RenderOrder.Num()-1, 1 );
}


//
// Scroll text via left scroll bar.
//
void WTreeView::ScrollBarChange( WWidget* Sender )
{
	ScrollTop	= ScrollBar->Value * (RenderOrder.Num()-1) / 100;
	ScrollTop	= Clamp( ScrollTop, 0, RenderOrder.Num()-1 );
}


//
// Return data of i'th node.
//
void* WTreeView::DataOf( Integer iNode )
{
	return iNode != -1 ? Nodes[iNode].Data : nullptr;
}


//
// Find a last render item with parent.
//
Integer WTreeView::FindLastChildren( Integer iParent )
{
	for( Integer i=RenderOrder.Num()-1; i>=0; i-- )
		if( Nodes[RenderOrder[i]].iParent == iParent )
			return i;

	return -1;
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/