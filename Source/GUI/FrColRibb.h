/*=============================================================================
	FrColRibb.h: WColorRibbon widget.
	Created by Vlad Gordienko, Mar. 2018.
=============================================================================*/

#define COLOR_RIBBON_ENABLED 0


#if COLOR_RIBBON_ENABLED
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
	void SetCurve( math::InterpCurve<math::Color>* InCurve );
	virtual void OnChange();

	// WWidget interface.
	void OnDblClick( EMouseButton Button, Int32 X, Int32 Y );    
	void OnPaint( CGUIRenderBase* Render );
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseLeave();

private:
	struct Bitmap
	{
		Array<math::Color> data;
		rend::Texture2DHandle handle;
	};

	// Internal.
	math::InterpCurve<math::Color>*	Curve;
	Bitmap							Ribbon;
	Int32							iSelected;

	// Helper functions.
	void UpdateRibbon();
	Int32 GetMarkerAt( Int32 X, Int32 Y );
	void ColorSelected( WWidget* Sender );
};

#endif

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/