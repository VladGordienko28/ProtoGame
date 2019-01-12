/*=============================================================================
	FrColRibb.h: WColorRibbon widget.
	Created by Vlad Gordienko, Mar. 2018.
=============================================================================*/

/*-----------------------------------------------------------------------------
	WColorRibbon.
-----------------------------------------------------------------------------*/

//
// Colored ribbon just like in the Photoshop.
//
class WColorRibbon: public WWidget
{
public:
	// Variables.
	TNotifyEvent	EventChange;

	// WColorRibbon interface.
	WColorRibbon( WContainer* InOwner, WWindow* InRoot );
	~WColorRibbon();
	void SetCurve( TInterpCurve<TColor>* InCurve );
	virtual void OnChange();

	// WWidget interface.
	void OnDblClick( EMouseButton Button, Integer X, Integer Y );    
	void OnPaint( CGUIRenderBase* Render );
	void OnMouseDown( EMouseButton Button, Integer X, Integer Y );
	void OnMouseUp( EMouseButton Button, Integer X, Integer Y );
	void OnMouseMove( EMouseButton Button, Integer X, Integer Y );
	void OnMouseLeave();

private:
	// Internal.
	TInterpCurve<TColor>*	Curve;
	TStaticBitmap*			Ribbon;
	Integer					iSelected;

	// Helper functions.
	void UpdateRibbon();
	Integer GetMarkerAt( Integer X, Integer Y );
	void ColorSelected( WWidget* Sender );
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/