/*=============================================================================
    FrList.h: List based widgets.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    Definitions.
-----------------------------------------------------------------------------*/

//
// Common list item.
//
struct TListItem
{
public:
	// Variables.
	String		Name;
	void*		Data;
	FTexture*	Picture;
	TPoint		PicOffset;
	TSize		PicSize;
	Bool		bEnabled;

	// Constructors.
	TListItem()
		:	Name(),
			Data( nullptr ),
			bEnabled( true ),
			Picture( nullptr ),
			PicOffset( 0, 0 ),
			PicSize( 0, 0 )
	{}
	TListItem( String InName, void* InData, Bool InbEnabled=true )
		: Name( InName ),
		  Data( InData ),
		  bEnabled( InbEnabled ),
		  Picture( nullptr ),
		  PicOffset( 0, 0 ),
		  PicSize( 0, 0 )
	{}
	TListItem( String InName, FTexture* Picture, TPoint PicOffset, TSize PicSize, void* InData, Bool InbEnabled=true)
		: Name( InName ),
		  Data( InData ),
		  bEnabled( InbEnabled ),
		  Picture( Picture ),
		  PicOffset( PicOffset ),
		  PicSize( PicSize )
	{}

};


/*-----------------------------------------------------------------------------
    WList.
-----------------------------------------------------------------------------*/

//
// An abstract list widget.
//
class WList: public WContainer
{
public:
	// Variables.
	TNotifyEvent		EventChange;
	TNotifyEvent		EventDblClick;
	TArray<TListItem>	Items;
	Integer				ItemIndex;

	// WListWidget interface.
	WList( WContainer* InOwner, WWindow* InRoot );
	virtual Integer AddItem( String InName, void* InData );
	virtual Integer AddPictureItem( String InName, FTexture* Picture, TPoint PicOffset, TSize PicSize, void* Data );
	virtual void Remove( Integer iItem );
	virtual void Empty();
	virtual void SetItemIndex( Integer NewIdx, Bool bNotify = true );
	virtual void SelectNext();
	virtual void SelectPrev();
	virtual void OnChange();
	virtual void OnDoubleClick();

	// Utility.
	void AlphabetSort();
};


/*-----------------------------------------------------------------------------
    WListBox.
-----------------------------------------------------------------------------*/

//
// A Simple listbox.
//
class WListBox: public WList
{
public:
	// Variables.
	Integer			ItemsHeight;

	// WWidget interface.
	WListBox( WContainer* InOwner, WWindow* InRoot );
	void OnResize();
	void OnMouseLeave();
	void OnMouseMove( EMouseButton Button, Integer X, Integer Y );
	void OnMouseDown( EMouseButton Button, Integer X, Integer Y ) ;
	void OnDblClick( EMouseButton Button, Integer X, Integer Y );
	void OnPaint( CGUIRenderBase* Render );
	void OnKeyDown( Integer Key );
	void OnMouseScroll( Integer Delta );

protected:
	// Internal.
	WSlider*	Slider;
	Integer		iHighlight;

	Integer YToIndex( Integer Y ) const;
};


/*-----------------------------------------------------------------------------
    WComboBox.
-----------------------------------------------------------------------------*/

//
// A combination list box.
//
class WComboBox: public WList
{
public:
	// WComboBox interface.
	WComboBox( WContainer* InOwner, WWindow* InRoot );
	~WComboBox();

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );
	void OnDblClick( EMouseButton Button, Integer X, Integer Y );
	void OnMouseDown( EMouseButton Button, Integer X, Integer Y );
	void OnMouseEnter();
	void OnMouseLeave();
	void OnDeactivate();

private:
	// Internal.
	WListBox*	DropList;		
	Bool		bHighlight;

	void ShowDropList();
	void HideDropList();
	Bool IsExpanded();
	void DropListChanged( WWidget* Sender );
};


/*-----------------------------------------------------------------------------
	WLog.
-----------------------------------------------------------------------------*/

//
// A list of clickable colored lines.
//
class WLog: public WContainer
{
public:
	// An single line of log.
	struct TLine
	{
	public:
		String		Text;
		TColor		Color;
		void*		Data;
	};

	// Variables.
	TArray<TLine>		Lines;
	TNotifyEvent		EventChange;
	TNotifyIndexEvent	EventGoto;
	Integer				iFirst, iLast;

	// WLog interface.
	WLog( WContainer* InOwner, WWindow* InRoot );
	~WLog();
	void Clear();
	Integer AddLine( String InText, void* InData=nullptr, TColor InColor=COLOR_White );
	void* DataOf( Integer i );
	virtual void OnChange();
	virtual void OnGoto( Integer i );

	// WWidget interface.
	void OnDblClick( EMouseButton Button, Integer X, Integer Y );    
	void OnPaint( CGUIRenderBase* Render );
	void OnMouseDown( EMouseButton Button, Integer X, Integer Y );
	void OnMouseUp( EMouseButton Button, Integer X, Integer Y );
	void OnMouseMove( EMouseButton Button, Integer X, Integer Y );
	void OnKeyDown( Integer Key );
	void OnMouseScroll( Integer Delta );

private:
	// Internal.
	WSlider*	ScrollBar;
	WPopupMenu*	PopUp;
	Integer		ScrollTop;

	// Internal events.
	void PopCopyClick( WWidget* Sender );
	void PopSelectAllClick( WWidget* Sender );
	void PopDeleteClick( WWidget* Sender );
	void PopGotoClick( WWidget* Sender );
	void PopToNextClick( WWidget* Sender );
	void PopToPrevClick( WWidget* Sender );
	void ScrollBarChange( WWidget* Sender );
	void ScrollToLast();

	// Internal.
	Integer YToIndex( Integer Y ) const;
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/