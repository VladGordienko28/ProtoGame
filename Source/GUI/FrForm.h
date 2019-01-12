/*=============================================================================
    FrForm.h: Float window.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    WForm.
-----------------------------------------------------------------------------*/

//
// Form header size.
//
#define FORM_HEADER_SIZE	20


//
// Float sub window.
//
class WForm: public WContainer
{
public:
	// Variables.
	TNotifyEvent	EventOpen;
	TNotifyEvent	EventClose;
	Bool			bSizeableW;
	Bool			bSizeableH;
	Bool			bCanClose;
	Integer			MinWindth;
	Integer			MinHeight;

	// WForm interface.
	WForm(  WContainer* InOwner, WWindow* InRoot  );

	// WWidget interface.
	void OnMouseDown( EMouseButton Button, Integer X, Integer Y );
	void OnMouseUp( EMouseButton Button, Integer X, Integer Y );
	void OnMouseMove( EMouseButton Button, Integer X, Integer Y );
	void OnPaint( CGUIRenderBase* Render );

	// WForm interface.
	virtual void Show( Integer X = 0, Integer Y = 0 );
	virtual void Hide();

	// New WForm events.
	virtual void OnClose()
	{
		EventClose( this );
	}
	virtual void OnOpen()
	{
		EventOpen( this );
	}

private:
	// Form internal.
	Bool		bIsMove;
	Bool		bIsSize;
	TPoint		HoldOffset;
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/