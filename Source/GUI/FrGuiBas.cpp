/*=============================================================================
    FrWidBas.cpp: Base widget system implementation.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

#include "GUI.h"

/*-----------------------------------------------------------------------------
    WWidget implementation.
-----------------------------------------------------------------------------*/

//
// An Identity point.
//
const TPoint TPoint::Zero = TPoint( 0, 0 );


//
// Widget constructor.
//
WWidget::WWidget( WContainer* InOwner, WWindow* InRoot )
	: bVisible( true ),
	  bEnabled( true ),
	  bStayOnTop( false ),
	  Align( AL_None ),
	  Cursor( CR_Arrow ),
	  Caption( L"" ),
	  Tooltip( L"" ),
	  Location( 0, 0 ),
	  Size( 64, 64 ),
	  Root( InRoot ),
	  Owner( nullptr ),
	  Margin( 0, 0, 0, 0 )
{
	SetOwner( InOwner );
}


//
// Widget destructor.
//
WWidget::~WWidget()
{
	if( Root->Focused == this )
		Root->SetFocused( Owner );

	if( Root->Highlight == this )
		Root->Highlight = nullptr;

	SetOwner( nullptr );
}


//
// Transform point from widget coords into
// windows.
//
TPoint WWidget::ClientToWindow( TPoint p ) const
{
	TPoint R = p;

	for( const WWidget* W = this; W; W = W->Owner )
		R += W->Location; 

	return R;
}


//
// Transform point from windows coords into
// local widget's.
//
TPoint WWidget::WindowToClient( TPoint p ) const
{
	TPoint R = p;

	for( const WWidget* W = this; W; W = W->Owner )
		R -= W->Location; 

	return R;
}


//
// Return true, if point is inside this widget.
//
Bool WWidget::IsInWidget( TPoint P ) const
{
	return ( P.X >= Location.X )&&
		   ( P.Y >= Location.Y )&&
		   ( P.X <= Location.X + Size.Width )&&
		   ( P.Y <= Location.Y + Size.Height );
}


//
// Set a new owner for widget.
//
void WWidget::SetOwner( WContainer* InOwner )
{
	if( Owner != nullptr )
	{
		Owner->Children.RemoveUnique( this );
		Owner->ShuffleChildren();
		Owner->AlignChildren();
	}

	Owner = InOwner;
	if( InOwner )
	{
		Owner->Children.Push( this );
		Owner->ShuffleChildren();
		Owner->AlignChildren();
	}
}


//
// Set a new component size.
//
void WWidget::SetSize( Integer NewWidth, Integer NewHeight )
{
	Size.Width = NewWidth;
	Size.Height = NewHeight;
	OnResize();

	if( Owner )
		Owner->AlignChildren();
}


//
// Set a widget location.
//
void WWidget::SetLocation( Integer NewX, Integer NewY )
{
	Location.X	= NewX;
	Location.Y	= NewY;
}


//
// Start drag cargo.
//
void WWidget::BeginDrag( void* Data )
{
	Root->Drag.bDrag	= true;
	Root->Drag.bAccept	= false;
	Root->Drag.Data		= Data;
	Root->Drag.Source	= this;
}


//
// Return true, if widget is in focus.
//
Bool WWidget::IsFocused() const
{
	return Root->Focused == this;
}


//
// Return true, if widget or child are
// in focus. Otherwise return false.
//
Bool WWidget::IsChildFocused() const
{
	WWidget* F = Root->Focused;

	while( F )
	{
		if( F == this )
			return true;
		F	= F->Owner;
	}
	return false;
}


//
// Return true, if this widget is child of
// given owner.
//
Bool WWidget::IsChildOf( WWidget* TestOwner ) const
{
	for( const WWidget* W=this; W; W=W->Owner )
		if( W == TestOwner )
			return true;

	return false;
}


//
// Widget main proc.
//
void WWidget::WidgetProc( EWidgetProcEvent Event, TWidProcParms Parms )
{
	switch( Event )
	{
		case WPE_Paint:
			OnPaint( Parms.Render );
			break;

		case WPE_KeyDown:
			OnKeyDown( Parms.Key );
			break;

		case WPE_KeyUp:
			OnKeyUp( Parms.Key );
			break;

		case WPE_CharType:
			OnCharType( Parms.TypedChar );
			break;

		case WPE_Resize:
			OnResize();
			break;

		case WPE_DblClick:
			OnDblClick( Parms.Button, Parms.X, Parms.Y );
			break;

		case WPE_MouseLeave:
			OnMouseLeave();
			break;

		case WPE_MouseEnter:
			OnMouseEnter();
			break;

		case WPE_Activate:
			OnActivate();
			break;

		case WPE_Deactivate:
			OnDeactivate();
			break;

		case WPE_MouseScroll:
			OnMouseScroll( Parms.Delta );
			break;

		case WPE_DragDrop:
			OnDragDrop( Root->Drag.Data, Parms.X, Parms.Y );
			break;

		case WPE_DragOver:
			OnDragOver( Root->Drag.Data, Parms.X, Parms.Y, Root->Drag.bAccept );
			break;

		case WPE_MouseDown:
			Root->SetFocused( this );
			OnMouseDown( Parms.Button, Parms.X, Parms.Y );
			break;

		case WPE_MouseUp:
			if( Root->Drag.bDrag )
			{
				WidgetProc( WPE_DragOver, Parms );
				if( Root->Drag.bAccept )
					WidgetProc( WPE_DragDrop, Parms );
				Root->Drag.bDrag = false;
				Root->Drag.Source = nullptr;
			}
			else
				OnMouseUp( Parms.Button, Parms.X, Parms.Y );
			break;

		case WPE_MouseMove:
			if( Root->Drag.bDrag && Root->Highlight == this )
				WidgetProc( WPE_DragOver, Parms );

			if( !Root->Drag.bDrag && !Root->IsCapture() && Root->Highlight != this )
			{
				if( Root->Highlight )
					Root->Highlight->WidgetProc( WPE_MouseLeave, Parms );

				WidgetProc( WPE_MouseEnter, Parms );	
			}

			Root->SetHighlight( this );
			if( !Root->Drag.bDrag )
				OnMouseMove( Parms.Button, Parms.X, Parms.Y );
			break;
	}
}


//
// Widget paint.
//
void WWidget::OnPaint( CGUIRenderBase* Render )
{
	if( Owner )
	{
		Render->SetClipArea
						( 
							Owner->ClientToWindow( TPoint::Zero ),
							Owner->Size 
						);
	}
}


//
// Drag something.
//
void WWidget::OnDragOver( void* Data, Integer X, Integer Y, Bool& bAccept )
{
	bAccept	= false;
}


//
// Widget events.
//
void WWidget::OnActivate()
{}
void WWidget::OnDeactivate()
{}
void WWidget::OnResize()
{}
void WWidget::OnMouseEnter()
{}
void WWidget::OnMouseLeave()
{}
void WWidget::OnMouseDown( EMouseButton Button, Integer X, Integer Y )
{}
void WWidget::OnMouseUp( EMouseButton Button, Integer X, Integer Y )
{}
void WWidget::OnMouseMove( EMouseButton Button, Integer X, Integer Y )
{}
void WWidget::OnDblClick( EMouseButton Button, Integer X, Integer Y )
{}
void WWidget::OnKeyDown( Integer Key )
{}
void WWidget::OnKeyUp( Integer Key )
{}
void WWidget::OnCharType( Char TypedChar )
{}
void WWidget::OnMouseScroll( Integer Delta )
{}
void WWidget::OnDragDrop( void* Data, Integer X, Integer Y )
{}


/*-----------------------------------------------------------------------------
    WContainer implementation.
-----------------------------------------------------------------------------*/

//
// Container constructor.
//
WContainer::WContainer( WContainer* InOwner, WWindow* InRoot )
	:	WWidget( InOwner, InRoot ),
		Children(),
		Padding( 0, 0, 0, 0 )
{}


//
// Container destructor.
//
WContainer::~WContainer()
{
	// Kill children by children this way,
	// since they remove self itself.
	while( Children.Num() > 0 )
		delete Children[0];
}


//
// Return children widget below the point,
// if no widget, return nullptr.
//
WWidget* WContainer::GetWidgetAt( TPoint P ) const
{
	for( Integer i=Children.Num()-1; i >= 0; i-- )
		if	( 
				Children[i]->bEnabled && 
				Children[i]->bVisible && 
				Children[i]->IsInWidget( P ) 
			)
				return Children[i];

	return nullptr;
}


//
// Return true, if given widget is my 
// children.
//
Bool WContainer::IsChild( WWidget* Widget ) const
{
	return Children.FindItem( Widget ) != -1;
}


//
// Container main procedure.
//
void WContainer::WidgetProc( EWidgetProcEvent Event, TWidProcParms Parms )
{
	switch ( Event )
	{
		case WPE_Paint:
			WWidget::WidgetProc( Event, Parms );
			for( Integer i=0; i<Children.Num(); i++ )
				if( Children[i]->bVisible )
					Children[i]->WidgetProc( Event, Parms );
			break;

		case WPE_MouseMove:
		case WPE_MouseDown:	
		case WPE_MouseUp:
		case WPE_DblClick:
		{
			WWidget* Below = GetWidgetAt( TPoint( Parms.X, Parms.Y ) );

			if( Event == WPE_MouseDown || Event == WPE_DblClick )
			{
				if( Below )
					Below->WidgetProc( Event, TWidProcParms
														( 
															Parms.Button, 
													        Parms.X - Below->Location.X, 
															Parms.Y - Below->Location.Y ) 
														);
				else
					WWidget::WidgetProc( Event, Parms );
			}
			else if( Event == WPE_MouseUp || Event == WPE_MouseMove )
			{
				if( Below && !(Root->IsCapture() && Root->Focused == this) )
					Below->WidgetProc( Event, TWidProcParms
														( 
															Parms.Button, 
													        Parms.X - Below->Location.X, 
															Parms.Y - Below->Location.Y ) 
														);
				else
					WWidget::WidgetProc( Event, Parms );
			}

		/*
		// I'm not sure about condition, it's messy a little.
		// Not all cases are tested :(
		//if( Below && !((Event == WPE_MouseMove) && !Root->Drag.bDrag && (Root->IsCapture() && Root->Focused==this)) )	
		if( Below && !(Root->IsCapture() && Root->Focused == this && (Event == WPE_MouseMove || Event == WPE_MouseUp)) )
			Below->WidgetProc( Event, TWidProcParms( Parms.Button, 
			                                         Parms.X - Below->Location.X, 
													 Parms.Y - Below->Location.Y ) );
		else
			WWidget::WidgetProc( Event, Parms );
		*/

			break;
		}

		case WPE_Resize:
			AlignChildren();
			for( Integer i=0; i<Children.Num(); i++ )
				Children[i]->WidgetProc( Event, Parms );

			WWidget::WidgetProc( Event, Parms );
			break;

		default:
			WWidget::WidgetProc( Event, Parms );
	}
}


//
// Align all children, according align type.
//
void WContainer::AlignChildren()
{
	Integer	MinX	= Padding.Left, 
			MinY	= Padding.Top, 
			MaxX	= Size.Width-Padding.Right, 
			MaxY	= Size.Height-Padding.Bottom;

	// Top widgets.
	for( Integer i=0; i<Children.Num(); i++ )
		if( Children[i]->Align == AL_Top )
		{
			WWidget* W	= Children[i];
			if( !W->bVisible )
				continue;

			W->Location.X	= Padding.Left + W->Margin.Left;
			W->Location.Y	= MinY + W->Margin.Top;
			W->Size.Width	= MaxX - MinX - W->Margin.Left - W->Margin.Right;

			MinY	+= W->Size.Height + W->Margin.Top;
		}

	// Bottom.
	for( Integer i=0; i<Children.Num(); i++ )
		if( Children[i]->Align == AL_Bottom )
		{
			WWidget* W	= Children[i];
			if( !W->bVisible )
				continue;

			MaxY	-= W->Size.Height + W->Margin.Bottom;

			W->Location.X	= Padding.Left + W->Margin.Left;
			W->Location.Y	= MaxY;
			W->Size.Width	= MaxX - MinX - W->Margin.Left - W->Margin.Right;
		}

		
	// Left widgets.
	for( Integer i=0; i<Children.Num(); i++ )
		if( Children[i]->Align == AL_Left )
		{
			WWidget* W	= Children[i];
			if( !W->bVisible )
				continue;

			W->Location.X	= MinX + W->Margin.Left;
			W->Location.Y	= MinY + W->Margin.Top;
			W->Size.Height	= MaxY - MinY - W->Margin.Top - W->Margin.Bottom;

			MinX	+= W->Size.Width + W->Margin.Left;
		}
		
	// Right widgets.
	for( Integer i=0; i<Children.Num(); i++ )
		if( Children[i]->Align == AL_Right )
		{
			WWidget* W	= Children[i];
			if( !W->bVisible )
				continue;

			MaxX	-= W->Size.Width;

			W->Location.X	= MaxX - W->Margin.Right;
			W->Location.Y	= MinY + W->Margin.Top;
			W->Size.Height	= MaxY - MinY - W->Margin.Top - W->Margin.Bottom;
		}

	// Client widget, just first one.
	for( Integer i=0; i<Children.Num(); i++ )
		if( Children[i]->Align == AL_Client )
		{
			WWidget* W	= Children[i];
			if( !W->bVisible )
				continue;

			W->Location.X	= MinX + W->Margin.Left;
			W->Location.Y	= MinY + W->Margin.Top;
			W->Size.Width	= MaxX - MinX - W->Margin.Left - W->Margin.Right;
			W->Size.Height	= MaxY - MinY - W->Margin.Top - W->Margin.Bottom;

			break;
		}

/*
	// Notify all children.
	for( Integer i=0; i<Children.Num(); i++ )
		Children[i]->WidgetProc( WPE_Resize, TWidProcParms() );
*/
}


//
// Place widget layer to proper location.
// according to bStayOnTop flag.
//
void WContainer::ShuffleChildren()
{
	TArray<WWidget*> Stored = Children;
	Children.Empty();

	for( Integer i=0; i<Stored.Num(); i++ )
		if( !Stored[i]->bStayOnTop )
			Children.Push( Stored[i] );

	for( Integer i=0; i<Stored.Num(); i++ )
		if( Stored[i]->bStayOnTop )
			Children.Push( Stored[i] );

	assert(Children.Num() == Stored.Num());
}


/*-----------------------------------------------------------------------------
    WWindow implementation.
-----------------------------------------------------------------------------*/

//
// GUI Drawing objects.
//
TStaticFont*	WWindow::Font1	= nullptr;
TStaticFont*	WWindow::Font2	= nullptr;
TStaticBitmap*	WWindow::Icons	= nullptr;


//
// Window constructor.
//
WWindow::WWindow()
	:	WContainer( nullptr, this ),
		Focused(nullptr),
		Highlight(nullptr),
		bLMouse(false),
		bRMouse(false),
		bMMouse(false),
		bAlt(false),
		bShift(false),
		bCtrl(false),
		LastHighlightTime(0.0),
		OldHighlight(nullptr),
		Modal( nullptr ),
		CursorMode(CM_Clamp)
{
	// Initialize own variables.
	Padding	= TArea( 0, 0, 0, 0 );
}


//
// Window destructor.
//
WWindow::~WWindow()
{
#if 0
	// Kill resources.
	freeandnil(Font1);
	freeandnil(Font2);
	freeandnil(Icons);
#endif
}


//
// Set widget focused.
//
void WWindow::SetFocused( WWidget* Widget )
{
	if( Focused != Widget )
	{
		if( Focused )
			Focused->WidgetProc( WPE_Deactivate, TWidProcParms() );

		Focused = Widget;
		if( Widget )
			Widget->WidgetProc( WPE_Activate, TWidProcParms() );
	}
}


//
// Set a highlight object.
//
void WWindow::SetHighlight( WWidget* Widget )
{
	Highlight = Widget;

	if( Highlight != OldHighlight )
	{
		LastHighlightTime	= GPlat->Now();
		OldHighlight		= Highlight;
	}
}


//
// Window main.
//
void WWindow::WidgetProc( EWidgetProcEvent Event, TWidProcParms Parms )
{
	if( !Modal )
	{
		//
		// Regular window processing.
		//
		switch (Event)
		{
			case WPE_KeyDown:
			case WPE_KeyUp:
			case WPE_CharType:
				if( Focused && Focused != this )
					Focused->WidgetProc( Event, Parms );
				WContainer::WidgetProc( Event, Parms );

				// System keys.
				if( Event == WPE_KeyDown || Event == WPE_KeyUp )
				{
					if( Parms.Key == VK_Alt )		bAlt	= Event == WPE_KeyDown;
					if( Parms.Key == VK_Ctrl )		bCtrl	= Event == WPE_KeyDown;
					if( Parms.Key == VK_Shift )		bShift	= Event == WPE_KeyDown;
				}
				break;

			case WPE_MouseScroll:
				if( Highlight )
					Highlight->WidgetProc( WPE_MouseScroll, Parms );
				break;

			case WPE_MouseMove:
			case WPE_DblClick:  
				if( !Drag.bDrag && IsCapture() && Focused && Focused != this )
				{
					TPoint Client = Focused->WindowToClient( TPoint( Parms.X, Parms.Y ) );
					Focused->WidgetProc( Event, TWidProcParms( Parms.Button, Client.X, Client.Y ) );
				}
				else
					WContainer::WidgetProc( Event, Parms );

				MousePos	= TPoint( Parms.X, Parms.Y );
				break;

			case WPE_MouseDown:
				if( Parms.Button == MB_Left )	bLMouse = true;
				if( Parms.Button == MB_Right )	bRMouse = true;
				if( Parms.Button == MB_Middle )	bMMouse = true;

				WContainer::WidgetProc( Event, Parms );
				break;

			case WPE_MouseUp:
				if( !Drag.bDrag && Focused && Focused != this )
				{
					TPoint Client = Focused->WindowToClient( TPoint( Parms.X, Parms.Y ) );
					Focused->WidgetProc( Event, TWidProcParms( Parms.Button, Client.X, Client.Y ) );
				}
				else
					WContainer::WidgetProc( Event, Parms );
		
				if( Parms.Button == MB_Left )	bLMouse = false;
				if( Parms.Button == MB_Right )	bRMouse = false;
				if( Parms.Button == MB_Middle )	bMMouse = false;
				break;
	
			case WPE_Paint:
				Parms.Render->SetBrightness( 1.f );
				WContainer::WidgetProc( Event, Parms );
				DrawTooltip( Parms.Render );
				break;

			default:
				WContainer::WidgetProc( Event, Parms );
				break;
		}
	}
	else
	{
		//
		// Modal dialog processing.
		//

		// Make sure modal dialog are ALWAYS last in the children list.
		// Modal should be at top of the top.
		if( Modal != Children.Last() )
		{
			Integer iModal = Children.FindItem(Modal);
			assert(iModal != -1);
			while( iModal < Children.Num()-1 )
			{
				Children.Swap( iModal, iModal+1 );
				iModal++;
			}
		}

		switch (Event)
		{
			case WPE_KeyDown:
			case WPE_KeyUp:
			case WPE_CharType:
				if( Focused && Focused != this && Focused->IsChildOf(Modal) )
					Focused->WidgetProc( Event, Parms );

				Modal->WidgetProc( Event, Parms );

				// System keys.
				if( Event == WPE_KeyDown || Event == WPE_KeyUp )
				{
					if( Parms.Key == VK_Alt )		bAlt	= Event == WPE_KeyDown;
					if( Parms.Key == VK_Ctrl )		bCtrl	= Event == WPE_KeyDown;
					if( Parms.Key == VK_Shift )		bShift	= Event == WPE_KeyDown;
				}
				break;

			case WPE_MouseScroll:
				if( Highlight && Highlight->IsChildOf(Modal) )
					Highlight->WidgetProc( WPE_MouseScroll, Parms );
				break;

			case WPE_MouseMove:
			case WPE_DblClick:
				if( !Drag.bDrag && IsCapture() && Focused && Focused!=this && Focused->IsChildOf(Modal) )
				{
					TPoint Client = Focused->WindowToClient( TPoint( Parms.X, Parms.Y ) );
					Focused->WidgetProc( Event, TWidProcParms( Parms.Button, Client.X, Client.Y ) );
				}
				else
				{
					WWidget* Below = GetWidgetAt(TPoint(Parms.X, Parms.Y));
					if( Below && Below->IsChildOf(Modal) )
					{
						TPoint Client = Below->WindowToClient( TPoint( Parms.X, Parms.Y ) );
						Below->WidgetProc( Event, TWidProcParms( Parms.Button, Client.X, Client.Y ) );
					}
				}
				MousePos	= TPoint( Parms.X, Parms.Y );
				break;

			case WPE_MouseDown:
				if( Parms.Button == MB_Left )	bLMouse = true;
				if( Parms.Button == MB_Right )	bRMouse = true;		
				if( Parms.Button == MB_Middle )	bMMouse = true;
				{
					WWidget* Below = GetWidgetAt(TPoint(Parms.X, Parms.Y));
					if( Below && Below->IsChildOf(Modal) )
					{
						TPoint Client = Below->WindowToClient( TPoint( Parms.X, Parms.Y ) );
						Below->WidgetProc( Event, TWidProcParms( Parms.Button, Client.X, Client.Y ) );
					}
				}
				break;

			case WPE_MouseUp:
				if( !Drag.bDrag && Focused && Focused != this && Focused->IsChildOf(Modal) )
				{
					TPoint Client = Focused->WindowToClient( TPoint( Parms.X, Parms.Y ) );
					Focused->WidgetProc( Event, TWidProcParms( Parms.Button, Client.X, Client.Y ) );
				}

				if( Parms.Button == MB_Left )	bLMouse = false;
				if( Parms.Button == MB_Right )	bRMouse = false;
				if( Parms.Button == MB_Middle )	bMMouse = false;
				break;
	
			case WPE_Paint:
				for( Integer i=0; i<Children.Num(); i++ )
					if( Children[i]->bVisible )
					{
						Parms.Render->SetBrightness( Children[i] == Modal ? 1.f : 0.5f );
						Children[i]->WidgetProc( Event, Parms );
					}

				if( Highlight && Highlight->IsChildOf(Modal) )
					DrawTooltip( Parms.Render );

				break;

			default:
				WContainer::WidgetProc( Event, Parms );
				break;
		}
	}
}


//
// Draw a little tooltip box.
//
void WWindow::DrawTooltip( CGUIRenderBase* Render )
{
	if	( 
			Highlight && 
			Highlight->Tooltip &&
			(GPlat->Now()-LastHighlightTime) > 0.5f 
		)
	{
		TSize HintSize = TSize( Font1->TextWidth(*Highlight->Tooltip), Font1->Height );
		TPoint DrawPos = TPoint( MousePos.X+16, MousePos.Y+8 );
		Render->SetClipArea( TPoint(0, 0), Size );		

		// Avoid out of window.
		if( DrawPos.X+HintSize.Width > Size.Width )		DrawPos.X	-= HintSize.Width + 32;
		if( DrawPos.Y+HintSize.Height > Size.Height )	DrawPos.Y	-= HintSize.Height + 16;

		Render->DrawRegion
						(
							DrawPos, 
							TSize( HintSize.Width+8, HintSize.Height+2 ),
							GUI_COLOR_TOOLTIP,
							GUI_COLOR_TOOLTIP,
							BPAT_Solid
						);

		Render->DrawText
					(	
						TPoint( DrawPos.X+4, DrawPos.Y+1 ),
						Highlight->Tooltip,
						GUI_COLOR_TEXT,
						Font1
					);
	}
}


//
// Paint the window.
//
void WWindow::OnPaint( CGUIRenderBase* Render )
{
	WContainer::OnPaint( Render );
/*
	Render->SetClipArea( Location, Size );
	Render->DrawRegion( Location, 
		                Size,
						GUI_COLOR_WINDOW,
						GUI_COLOR_WINDOW,
						BPAT_Solid );	
*/
}


//
// Return cursor style to draw.
//
ECursorStyle WWindow::GetDrawCursor() const
{
	if( Drag.bDrag )
	{
		// Drag something, spit WWidget::Cursor.
		return Drag.bAccept ? CR_Drag : CR_NoDrop;
	}
	else
	{
		// Regular.
		if( !Modal )
			return Highlight ? Highlight->Cursor : Cursor;
		else
			return (Highlight && Highlight->IsChildOf(Modal)) ? Highlight->Cursor : Cursor;
	}
}


Bool WWindow::IsLPressed() const
{
	return bLMouse;
}


Bool WWindow::IsRPressed() const
{
	return bRMouse;
}


Bool WWindow::IsMPressed() const
{
	return bMMouse;
}


Bool WWindow::IsCapture() const
{
	return IsLPressed() || IsRPressed();
}

ECursorMode WWindow::GetCursorMode() const
{
	return CursorMode;
}


void WWindow::SetCursorMode( ECursorMode InMode )
{
	CursorMode = InMode;
}


/*-----------------------------------------------------------------------------
    Dialogs.
-----------------------------------------------------------------------------*/

//
// Show simple message box.
//
void WWindow::ShowMessage( String Text, String Caption, Bool bModal, TNotifyEvent Ok )
{
	WMessageBox* Msg = new WMessageBox( this, Text, Caption, bModal );
	Msg->SetOk( Ok );
}


//
// Show asking dialog.
//
void WWindow::AskYesNo( String Text, String Caption, Bool bModal, TNotifyEvent Yes, TNotifyEvent No )
{
	WMessageBox* Msg = new WMessageBox( this, Text, Caption, bModal );
	Msg->SetYesNo( Yes, No );
}


//
// Show asking dialog.
//
void WWindow::AskYesNoCancel( String Text, String Caption, Bool bModal, TNotifyEvent Yes, TNotifyEvent No, TNotifyEvent Cancel )
{
	WMessageBox* Msg = new WMessageBox( this, Text, Caption, bModal );
	Msg->SetYesNoCancel( Yes, No, Cancel );
}


//
// Show temporal notification dialog. It's will disappear
// when it loses focus.
//
void WWindow::ShowNotification( String Text, String Caption )
{
	WMessageBox* Msg = new WMessageBox( this, Text, Caption, false );
	Msg->SetNotification();
	SetFocused( Msg );
}


//
// This loading code outdated. But maybe useful as
// example of how to load bitmaps and fonts.
//
#if 0
/*-----------------------------------------------------------------------------
    BMP-files structs.
-----------------------------------------------------------------------------*/

//
// It's not good to store it here, but I don't want
// to include WinApi to the my GDI system.
//
#pragma pack(push,1)
struct TBitmapFileHeader
{
	Word		bfType;
	DWord		bfSize;
	Word		bfReserved1;
	Word		bfReserved2;
	DWord		bfOffBits;
};


struct TBitmapInfoHeader
{
	DWord		biSize;
	Integer		biWidth;
	Integer		biHeight;
	Word		biPlanes;
	Word		biBitCount;
	DWord		biCompression;
	DWord		biSizeImage;
	Integer		biXPelsPerMeter;
	Integer		biYPelsPerMeter;
	DWord		biClrUsed;
	DWord		biClrImportant;
};
#pragma pack(pop)


/*-----------------------------------------------------------------------------
    TGUIBitmap implementation.
-----------------------------------------------------------------------------*/

//
// GUI bitmap constructor.
//
TGUIBitmap::TGUIBitmap()
	:	Data()
{
}


//
// GUI bitmap destructor.
// 
TGUIBitmap::~TGUIBitmap()
{
	Data.Empty();
}


//
// Return GUI bitmap data.
//
void* TGUIBitmap::GetData()
{
	return &Data[0];
}


//
// Load a GUI bitmap, its should consist < 256 colors, because
// this bitmaps are paletted, to use less memory.
//
TGUIBitmap* WWindow::LoadBitmap( String FileName )
{
	CFileLoader Loader( FileName );

	// Load metadata.
	TBitmapFileHeader BmpHeader;
	TBitmapInfoHeader BmpInfo;
	Loader.SerializeData( &BmpHeader, sizeof(TBitmapFileHeader) );
	Loader.SerializeData( &BmpInfo, sizeof(TBitmapInfoHeader) );

	// Test for BMP.
	if( BmpHeader.bfType != 0x4d42 )
		error( L"\"%s\" is not BMP file.", *FileName );

    assert(((BmpInfo.biWidth)&(BmpInfo.biWidth-1)) == 0);
    assert(((BmpInfo.biHeight)&(BmpInfo.biHeight-1)) == 0);

	// Preinitialize bitmap.
	TGUIBitmap* Bitmap = new TGUIBitmap();
	Bitmap->Format		= BF_Palette8;
	Bitmap->USize		= BmpInfo.biWidth;
	Bitmap->VSize		= BmpInfo.biHeight;
	Bitmap->UBits		= IntLog2(Bitmap->USize);
	Bitmap->VBits		= IntLog2(Bitmap->VSize);
	Bitmap->Filter		= BFILTER_Nearest;
	Bitmap->BlendMode	= BLEND_Regular;
	Bitmap->RenderInfo	= -1;
	Bitmap->PanUSpeed	= 0.f;
	Bitmap->PanVSpeed	= 0.f;
	Bitmap->Saturation	= 1.f;
	Bitmap->AnimSpeed	= 0.f;
	Bitmap->bDynamic	= false;
	Bitmap->bRedrawn	= false;
	Bitmap->Data.SetNum( Bitmap->USize * Bitmap->VSize * sizeof(Byte) );

	// Read data.
	Loader.Seek( BmpHeader.bfOffBits );				

	if( BmpInfo.biBitCount == 24 || BmpInfo.biBitCount == 32 )
	{
		// Regular 24/32 bit image.
		// Warning! Bitmaps are V-flipped, so reverse should be here.
		Integer iPix = 0;
		for( Integer v=Bitmap->VSize-1; v>=0; v-- )
		for( Integer u=0; u<Bitmap->USize; u++ )
		{
			// Load pixel.
			Byte RawPix[4];
			Loader.SerializeData( RawPix, (BmpInfo.biBitCount == 24 ? 3 : 4) * sizeof(Byte) );
			TColor Source( RawPix[2], RawPix[1], RawPix[0], 0xff );
			if( Source == MASK_COLOR )	Source.A = 0x00;

			// Add to palette and to data.
			Bitmap->Data[u + (v << Bitmap->UBits)] = Bitmap->Palette.Colors.AddUnique(Source);

			// Check if palette overflow.
			if( Bitmap->Palette.Colors.Num() > 256 )
				error( L"\"%s\" consist more than 256 colors.", *FileName );
		}
	}
	else
		error( L"Unsupported bmp format in \"%s\"", *FileName );

	return Bitmap;
}


/*-----------------------------------------------------------------------------
    TGUIFont implementation.
-----------------------------------------------------------------------------*/

//
// GUI font constructor.
//
TGUIFont::TGUIFont()
{
}


//
// GUI font destructor.
//
TGUIFont::~TGUIFont()
{
	// Kill manually all pages.
	for( Integer i=0; i<Bitmaps.Num(); i++ )
		delete Bitmaps[i];
}


//
// Load a font from using own format, make sure its
// compatible with delphi code.
//
TGUIFont* WWindow::LoadFont( String FileName )
{
	FILE* File	= _wfopen( *FileName, L"r" );
	if( !File )
		error( L"Font \"%s\" not found.", *FileName );

	TGUIFont* Font = new TGUIFont();

	// Main line.
	Char Name[64];
	Integer Height;
	fwscanf( File, L"%d %s\n", &Height, Name );
	
	// read characters.
	Font->Height = -1; 
	Integer NumPages = 0;
	while( !feof(File) )
	{
		Char C[2];
		Integer X, Y, W, H, iBitmap;
		C[0] = *fgetws( C, 2, File );
		
		if( (Word)C[0]+1 > Font->Remap.Num() )
			Font->Remap.SetNum( (Word)C[0]+1 );

		fwscanf( File, L"%d %d %d %d %d\n", &iBitmap, &X, &Y, &W, &H );
		
		NumPages = Max( NumPages, iBitmap+1 );

		TGlyph Glyph;
		Glyph.iBitmap	= iBitmap;
		Glyph.X			= X;
		Glyph.Y			= Y;
		Glyph.W			= W;
		Glyph.H			= H;	

		Font->Height			= Max( Font->Height, H );
		Font->Remap[(Word)C[0]] = Font->Glyphs.Push(Glyph);
#if 0
		log( L"Import Char: %s", C );
#endif
	}

	// Parse directory from the filename.
	String Directory;
	{
		Integer i;
		for( i=FileName.Len()-1; i>=0; i-- )
			if( FileName[i] == L'\\' )
				break;
		Directory	= String::Copy( FileName, 0, i );
	}

	// Load pages.
	for( Integer i=0; i<NumPages; i++ )
	{
		// Set style.
		FBitmap* B = LoadBitmap( String::Format( L"%s\\%s%d_%d.bmp", *Directory, Name, Height, i ) );
		B->BlendMode = BLEND_Translucent;
		Font->Bitmaps.Push( B );
	}

	fclose( File );
	return Font;
}
#endif

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/