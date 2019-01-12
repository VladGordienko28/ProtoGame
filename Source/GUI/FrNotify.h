/*=============================================================================
    FrWidNotify.h: Event notification delegates.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    TNotifyEvent.
-----------------------------------------------------------------------------*/

//
// A pointer to event method.
//
struct TNotifyEvent
{
public:
	// Signature of event.
	typedef void(WWidget::*TEvent)( WWidget* Sender );

	// Variables.
	WWidget*	Widget;
	TEvent		Event;
	
	// Constructor.
	TNotifyEvent( WWidget* InWidget = nullptr, TEvent InEvent = nullptr )
		: Widget( InWidget ),
		  Event( InEvent )
	{}

	// Invoke operator.
	void operator()( WWidget* Sender )
	{
		if( Widget )
			(Widget->*Event)( Sender );
	}

	// Testing operator.
	operator Bool() const
	{
		return Widget && Event;
	}
};


/*-----------------------------------------------------------------------------
	TNotifyIndexEvent.
-----------------------------------------------------------------------------*/

//
// A pointer to indexed event method.
//
struct TNotifyIndexEvent
{
public:
	// Signature of event.
	typedef void(WWidget::*TEvent)( WWidget* Sender, Integer Index );

	// Variables.
	WWidget*	Widget;
	TEvent		Event;

	// Constructor.
	TNotifyIndexEvent( WWidget* InWidget = nullptr, TEvent InEvent = nullptr )
		: Widget( InWidget ),
		  Event( InEvent )
	{}

	// Invoke operator.
	void operator()( WWidget* Sender, Integer Index )
	{
		if( Widget )
			(Widget->*Event)( Sender, Index );
	}

	// Testing operator.
	operator Bool() const
	{
		return Widget && Event;
	}
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/