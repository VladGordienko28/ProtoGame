/*=============================================================================
    FrGuiBase.h: Flu widgets base functions & classes.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    Declarations.
-----------------------------------------------------------------------------*/

//
// A mouse button.
//
enum EMouseButton
{
	MB_None,
	MB_Left,
	MB_Right,
	MB_Middle
};
      

//
// Widget alignment.
//
enum EWidgetAlign
{
	AL_None,
	AL_Top,
	AL_Bottom,
	AL_Left,
	AL_Right,
	AL_Client
};


//
// Cursor style.
//
enum ECursorStyle
{
	CR_Arrow,
	CR_Cross,
	CR_HandPoint,
	CR_IBeam,
	CR_SizeAll,
	CR_SizeNS,
	CR_SizeWE,
	CR_SizeNESW,
	CR_SizeNWSE,
	CR_HSplit,
	CR_VSplit,
	CR_Drag,
	CR_MultiDrag,
	CR_NoDrop,
	CR_HourGlass,
	CR_No,
	CR_MAX
};


// 
// A cursor mode.
//
enum ECursorMode
{
	CM_Clamp,
	CM_Wrap
};


//
// WidgetProc event.
//
enum EWidgetProcEvent
{
	WPE_None			= 0,
	WPE_MouseLeave		= 1,
	WPE_MouseEnter		= 2,
	WPE_Activate		= 3,
	WPE_Deactivate		= 4,
	WPE_DragOver		= 5,
	WPE_DragDrop		= 6,

	// Window mandatory events.
	WPE_Paint			= 7,
	WPE_KeyDown			= 8,
	WPE_KeyUp			= 9,
	WPE_MouseMove		= 10,
	WPE_MouseUp			= 11,
	WPE_MouseDown		= 12,
	WPE_DblClick		= 13,
	WPE_Resize			= 14,
	WPE_CharType		= 15,
	WPE_MouseScroll		= 16
};


//
// Basic keys codes.
//
#define VK_Alt		0x12
#define VK_Ctrl		0x11
#define VK_Shift	0x10


//
// A WidgetProc parameters.
//
union TWidProcParms
{
public:
	// All possible events.
	struct{ EMouseButton Button; Int32 X; Int32 Y; };
	struct{ CGUIRenderBase* Render; };
	struct{ Int32 Key; };
	struct{ Char TypedChar; };
	struct{ Int32 Delta; };

	// Constructors.
	TWidProcParms()
	{}
	TWidProcParms( EMouseButton InButton, Int32 InX, Int32 InY )
		:	Button(InButton),
			X(InX),
			Y(InY)
	{}
	TWidProcParms( Int32 InInt32 )
		:	Key(InInt32)
	{}
	TWidProcParms( CGUIRenderBase* InRender )
		:	Render(InRender)
	{}
	TWidProcParms( Char InTypedChar )
		:	TypedChar(InTypedChar)
	{}
};


//
// An information about drag
// object.
//
struct TDragInfo
{
public:
	// Variables.
	Bool		bDrag;
	Bool		bAccept;
	WWidget*	Source;
	void*		Data;

	TDragInfo()
		:	bDrag( false ),
			bAccept( false ),
			Source( nullptr  ),
			Data( nullptr )
	{}
};


//
// TArea.
//
struct TArea
{
public:
	// Variables.
	Int32		Top;
	Int32		Bottom;
	Int32		Left;
	Int32		Right;

	// Constructors.
	TArea()
		:	Top(0), Bottom(0), Left(0), Right(0)
	{}
	TArea( Int32 InTop, Int32 InBottom, Int32 InLeft, Int32 InRight )
		:	Top(InTop), Bottom(InBottom), Left(InLeft), Right(InRight)
	{}
};


//
// Widget event constructors.
//
#define WIDGET_EVENT( name ) TNotifyEvent( this, (TNotifyEvent::TEvent)&##name ) 
#define WIDGET_INDEX_EVENT( name ) TNotifyIndexEvent( this, (TNotifyIndexEvent::TEvent)&##name ) 


/*-----------------------------------------------------------------------------
    TPoint.
-----------------------------------------------------------------------------*/

//
// A point.
//
struct TPoint
{
public:
	// Variables.
	Int32 X;
	Int32 Y;

	static const TPoint Zero;

	// TPoint interface.
	TPoint()
		:	X(0), Y(0)
	{}
	TPoint( Int32 InX, Int32 InY )
		:	X( InX ), Y( InY )
	{}
	TPoint operator+=( const TPoint& P )
	{
		X += P.X;
		Y += P.Y;
		return *this;
	}
	TPoint operator+( const TPoint& P ) const
	{
		return TPoint( X + P.X, Y + P.Y );
	}
	TPoint operator-=( const TPoint& P )
	{
		X -= P.X;
		Y -= P.Y;
		return *this;
	}
	TPoint operator-( const TPoint& P ) const
	{
		return TPoint( X - P.X, Y - P.Y );
	}
	Bool operator==( const TPoint& P ) const
	{
		return X == P.X && Y == P.Y;
	}
	Bool operator!=( const TPoint& P ) const
	{
		return X != P.X || Y != P.Y;
	}
};


/*-----------------------------------------------------------------------------
    TSize.
-----------------------------------------------------------------------------*/

//
// A widget size.
//
struct TSize
{
public:
	// Variables.
	Int32		Width;
	Int32		Height;

	// TSize interface.
	TSize()
		:	Width(0),
			Height(0)
	{}
	TSize( Int32 InSide )
		:	Width( InSide ), Height( InSide ) 
	{}
	TSize( Int32 InWidth, Int32 InHeight )
		:	Width(InWidth),
			Height(InHeight)
	{}
	Bool operator==( const TSize& S ) const
	{
		return Width == S.Width && Height == S.Height;
	}
	Bool operator!=( const TSize& S ) const
	{
		return Width != S.Width || Height != S.Height;
	}
};


/*-----------------------------------------------------------------------------
    CGUIRenderBase.
-----------------------------------------------------------------------------*/

//
// Brush pattern.
//
enum EBrushPattern
{
	BPAT_None,
	BPAT_Solid,
	BPAT_PolkaDot,
	BPAT_Diagonal,
	BPAT_MAX
};


//
// An abstract interface to render the GUI.
//
class CGUIRenderBase
{
public:
	// CGUIRenderBase abstract interface.
	virtual void DrawRegion( TPoint P, TSize S, TColor Color, TColor BorderColor, EBrushPattern Pattern ) = 0;
	virtual void SetClipArea( TPoint P, TSize S ) = 0;
	virtual void DrawPicture( TPoint P, TSize S, TPoint BP, TSize BS, FTexture* Texture ) = 0;
	virtual void DrawText( TPoint P, const Char* Text, Int32 Len, TColor Color, FFont* Font ) = 0;
	virtual void SetBrightness( Float Brig ) = 0;

	// CGUIRenderBase utility.
	void DrawText( TPoint P, String Text, TColor Color, FFont* Font )
	{
		DrawText( P, *Text, Text.Len(), Color, Font );
	}
};


/*-----------------------------------------------------------------------------
    WWidget.
-----------------------------------------------------------------------------*/

//
// An abstract widget.
//
class WWidget
{
public:
	// Variables.
	Bool			bVisible;
	Bool			bEnabled;
	Bool			bStayOnTop;
	EWidgetAlign	Align;
	TArea			Margin;
	ECursorStyle	Cursor;
	String			Tooltip;
	String			Caption;
	TPoint			Location;
	TSize			Size;
	WWindow*		Root;
	WContainer*		Owner;		

	// WWidget interface.
	WWidget( WContainer* InOwner, WWindow* InRoot );
	virtual ~WWidget();
	TPoint ClientToWindow( TPoint p ) const;
	TPoint WindowToClient( TPoint p ) const;
	Bool IsInWidget( TPoint P ) const;
	Bool IsFocused() const;
	Bool IsChildFocused() const;
	Bool IsChildOf( WWidget* TestOwner ) const;
	void SetLocation( Int32 NewX, Int32 NewY );
	void SetSize( Int32 NewWidth, Int32 NewHeight );
	void BeginDrag( void* Data );
	virtual void WidgetProc( EWidgetProcEvent Event, TWidProcParms Parms );

	// Own events.
	virtual void OnActivate();
	virtual void OnDeactivate();
	virtual void OnDblClick( EMouseButton Button, Int32 X, Int32 Y );    
	virtual void OnPaint( CGUIRenderBase* Render );
	virtual void OnResize();
	virtual void OnMouseEnter();
	virtual void OnMouseLeave();
	virtual void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y );
	virtual void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y );
	virtual void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y );
	virtual void OnKeyDown( Int32 Key );
	virtual void OnKeyUp( Int32 Key );
	virtual void OnCharType( Char TypedChar );
	virtual void OnMouseScroll( Int32 Delta );
	virtual void OnDragOver( void* Data, Int32 X, Int32 Y, Bool& bAccept );
	virtual void OnDragDrop( void* Data, Int32 X, Int32 Y );

private:
	// Widget internal.
	void SetOwner( WContainer* InOwner );
};


/*-----------------------------------------------------------------------------
    WContainer.
-----------------------------------------------------------------------------*/

//
// An abstract container class.
//
class WContainer: public WWidget
{
public:
	// Variables.
	Array<WWidget*>	Children;
	TArea			Padding;
		
	// WContainer interface.
	WContainer( WContainer* InOwner, WWindow* InRoot );
	~WContainer();
	WWidget* GetWidgetAt( TPoint P ) const;
	Bool IsChild( WWidget* Widget ) const;
	void AlignChildren();
	void ShuffleChildren();

	// WWidget interface.
	void WidgetProc( EWidgetProcEvent Event, TWidProcParms Parms );
};


/*-----------------------------------------------------------------------------
    WWindow.
-----------------------------------------------------------------------------*/

//
// An window.
//
class WWindow: public WContainer
{
public:
	// Variables.
	WWidget*	Focused;
	WWidget*	Highlight;
	TDragInfo	Drag;
	WForm*		Modal;

	// Mouse state.
	TPoint		MousePos;
	ECursorMode	CursorMode;
	Bool		bLMouse;
	Bool		bRMouse;
	Bool		bMMouse;

	// Sys buttons.
	Bool		bAlt;
	Bool		bShift;
	Bool		bCtrl;

	// Tooltip management.
	Double		LastHighlightTime;
	WWidget*	OldHighlight;

	// Draw objects.
	static TStaticFont*		Font1;
	static TStaticFont*		Font2;
	static TStaticBitmap*	Icons;

	// WWindow interface.
	WWindow();
	~WWindow();
	void SetFocused( WWidget* Widget );
	void SetHighlight( WWidget* Widget );
	inline Bool IsLPressed() const;
	inline Bool IsRPressed() const;
	inline Bool IsMPressed() const;
	inline Bool IsCapture() const;
	ECursorStyle GetDrawCursor() const;
	ECursorMode GetCursorMode() const;
	void SetCursorMode( ECursorMode InMode );

	// WWidget interface.
	void WidgetProc( EWidgetProcEvent Event, TWidProcParms Parms );
	void OnPaint( CGUIRenderBase* Render );

	// Dialogs.
	void ShowMessage( String Text, String Caption, Bool bModal=true, TNotifyEvent Ok=TNotifyEvent() );
	void AskYesNo( String Text, String Caption, Bool bModal=true, TNotifyEvent Yes=TNotifyEvent(), TNotifyEvent No=TNotifyEvent() );
	void ShowNotification( String Text, String Caption );
	void AskYesNoCancel( String Text, String Caption, Bool bModal=true, TNotifyEvent Yes=TNotifyEvent(), TNotifyEvent No=TNotifyEvent(), TNotifyEvent Cancel=TNotifyEvent() );

private:
	// Window internal.
	void DrawTooltip( CGUIRenderBase* Render );
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/