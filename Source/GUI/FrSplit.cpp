/*=============================================================================
	FrSplit.cpp: Splitter family classes.
	Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#include "GUI.h"

/*-----------------------------------------------------------------------------
	WHSplitBox implementation.
-----------------------------------------------------------------------------*/

//
// Splitter constructor.
//
WHSplitBox::WHSplitBox( WContainer* InOwner, WWindow* InRoot )
	:	WContainer( InOwner, InRoot ),
		LeftMin( 30 ),
		RightMin( 30 ),
		Separator( 50 ),
		bMoveSeparator( false ),
		HoldOffset( 0 ),
		RatioRule( HRR_KeepAspect ),
		LeftMax( 0 ),
		RightMax( 0 )
{
	bStayOnTop = false;
	Cursor = CR_SizeWE;
	Size = TSize( 100, 100 );
	OldXSize = Size.Width;
}


//
// Mouse move on splitter.
//
void WHSplitBox::OnMouseMove( EMouseButton Button, Int32 X, Int32 Y )
{
	WContainer::OnMouseMove( Button, X, Y );

	if( Button == MB_Left && bMoveSeparator )
	{
		Separator = X - HoldOffset;
		UpdateSubWidgets();
	}
}


//
// Mouse release button on splitter.
//
void WHSplitBox::OnMouseUp( EMouseButton Button, Int32 X, Int32 Y )
{
	WContainer::OnMouseUp( Button, X, Y );

	if( Button == MB_Left )
	{
		HoldOffset = 0;
		bMoveSeparator = false;
	}
}


//
// Mouse down on splitter.
// 
void WHSplitBox::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y )
{
	WContainer::OnMouseDown( Button, X, Y );

	if( Button == MB_Left )
	{
		HoldOffset = X-Separator;
		
		if( abs(HoldOffset) <= HSPLIT_THICKNESS/2 )
		{
			bMoveSeparator = true;
		}
	}
}


//
// Split Box rendering.
//
void WHSplitBox::OnPaint( CGUIRenderBase* Render )
{
	TPoint Base = ClientToWindow(TPoint::Zero);
	TColor FillColor = bMoveSeparator ? GUI_COLOR_SPLITTER_MOVE : GUI_COLOR_SPLITTER;

	Render->DrawRegion
	(
		TPoint( Base.X + Separator - HSPLIT_THICKNESS/2, Base.Y ),
		TSize( HSPLIT_THICKNESS, Size.Height ),
		FillColor,
		FillColor,
		BPAT_Solid
	);
}


//
// Splitter resize.
//
void WHSplitBox::OnResize()
{
	WContainer::OnResize();
	
	if( RatioRule == HRR_PreferRight )
	{
		Separator = Separator - (OldXSize - Size.Width);
	}
	if( RatioRule == HRR_KeepAspect )
	{
		Float Aspect = Float(Separator) / max(Float(OldXSize), 0.1f );
		Separator = Size.Width * Aspect;
	}

	UpdateSubWidgets();

	OldXSize = Size.Width;
}


//
// Locate sub-widgets.
//
Bool WHSplitBox::UpdateSubWidgets()
{
	// Clip separator.
	if( LeftMax != 0 )
		Separator = min( Separator, LeftMax );

	if( RightMax != 0 )
		Separator = max( Separator, Size.Width-RightMax );

	Separator = clamp
	(
		Separator,
		LeftMin + HSPLIT_THICKNESS/2,
		Size.Width - RightMin - HSPLIT_THICKNESS/2
	);

	if( Children.size() < 2 )
		return false;

	WWidget* Left = Children[0];
	WWidget* Right = Children[1];

	Left->Location.X = 0;
	Left->Location.Y = 0;
	Left->Size.Width = Separator - HSPLIT_THICKNESS/2;
	Left->Size.Height = Size.Height;
	Left->WidgetProc( WPE_Resize, TWidProcParms() );

	Right->Location.X = Separator + HSPLIT_THICKNESS/2;
	Right->Location.Y = 0;
	Right->Size.Width = Size.Width - Right->Location.X;
	Right->Size.Height = Size.Height;
	Right->WidgetProc( WPE_Resize, TWidProcParms() );

	return true;
}


/*-----------------------------------------------------------------------------
	WVSplitBox implementation.
-----------------------------------------------------------------------------*/

//
// Splitter constructor.
//
WVSplitBox::WVSplitBox( WContainer* InOwner, WWindow* InRoot )
	:	WContainer( InOwner, InRoot ),
		TopMin( 30 ),
		BottomMin( 30 ),
		Separator( 50 ),
		bMoveSeparator( false ),
		HoldOffset( 0 ),
		RatioRule( VRR_KeepAspect ),
		TopMax( 0 ),
		BottomMax( 0 )
{
	bStayOnTop = false;
	Cursor = CR_SizeNS;
	Size = TSize( 100, 100 );
	OldYSize = Size.Height;
}


//
// Mouse move on splitter.
//
void WVSplitBox::OnMouseMove( EMouseButton Button, Int32 X, Int32 Y )
{
	WContainer::OnMouseMove( Button, X, Y );

	if( Button == MB_Left && bMoveSeparator )
	{
		Separator = Y - HoldOffset;
		UpdateSubWidgets();
	}
}


//
// Mouse release button on splitter.
//
void WVSplitBox::OnMouseUp( EMouseButton Button, Int32 X, Int32 Y )
{
	WContainer::OnMouseUp( Button, X, Y );

	if( Button == MB_Left )
	{
		HoldOffset = 0;
		bMoveSeparator = false;
	}
}


//
// Mouse down on splitter.
// 
void WVSplitBox::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y )
{
	WContainer::OnMouseDown( Button, X, Y );

	if( Button == MB_Left )
	{
		HoldOffset = Y-Separator;
		
		if( abs(HoldOffset) <= VSPLIT_THICKNESS/2 )
		{
			bMoveSeparator = true;
		}
	}
}


//
// Split Box rendering.
//
void WVSplitBox::OnPaint( CGUIRenderBase* Render )
{
	TPoint Base = ClientToWindow(TPoint::Zero);
	TColor FillColor = bMoveSeparator ? GUI_COLOR_SPLITTER_MOVE : GUI_COLOR_SPLITTER;

	Render->DrawRegion
	(
		TPoint( Base.X, Base.Y + Separator - VSPLIT_THICKNESS/2 ),
		TSize( Size.Width, VSPLIT_THICKNESS ),
		FillColor,
		FillColor,
		BPAT_Solid
	);
}


//
// Splitter resize.
//
void WVSplitBox::OnResize()
{
	WContainer::OnResize();
	
	if( RatioRule == VRR_PreferBottom )
	{
		Separator = Separator - (OldYSize - Size.Height);
	}
	if( RatioRule == VRR_KeepAspect )
	{
		Float Aspect = Float(Separator) / max(Float(OldYSize), 0.1f );
		Separator = Size.Width * Aspect;
	}

	UpdateSubWidgets();

	OldYSize = Size.Height;
}


//
// Locate sub-widgets.
//
Bool WVSplitBox::UpdateSubWidgets()
{
	// Clip separator.
	if( TopMax != 0 )
		Separator = min( Separator, TopMax );

	if( BottomMax != 0 )
		Separator = max( Separator, Size.Height-BottomMax );

	Separator = clamp
	(
		Separator,
		TopMin + VSPLIT_THICKNESS/2,
		Size.Height - BottomMin - VSPLIT_THICKNESS/2
	);

	if( Children.size() < 2 )
		return false;

	WWidget* Top = Children[0];
	WWidget* Bottom = Children[1];

	Top->Location.X = 0;
	Top->Location.Y = 0;
	Top->Size.Width = Size.Width;
	Top->Size.Height = Separator - VSPLIT_THICKNESS/2;
	Top->WidgetProc( WPE_Resize, TWidProcParms() );

	Bottom->Location.X = 0;
	Bottom->Location.Y = Separator + VSPLIT_THICKNESS/2;
	Bottom->Size.Width = Size.Width;
	Bottom->Size.Height = Size.Height - Bottom->Location.Y;
	Bottom->WidgetProc( WPE_Resize, TWidProcParms() );

	return true;
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/