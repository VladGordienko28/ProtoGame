/*=============================================================================
	FrSplit.h: Widgets to split area.
	Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
	WHSplitBox.
-----------------------------------------------------------------------------*/

#define HSPLIT_THICKNESS 7

//
// HSplitter Ratio-Rule.
//
enum EHSplitRatio
{
	HRR_KeepAspect,
	HRR_PreferLeft,
	HRR_PreferRight
};


//
// A horizontal split box.
//
class WHSplitBox: public WContainer
{
public:
	// Variables.
	Int32 LeftMin;
	Int32 LeftMax;
	Int32 RightMin;
	Int32 RightMax;
	EHSplitRatio RatioRule;

	// WHSplitBox interface.
	WHSplitBox( WContainer* InOwner, WWindow* InRoot );

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render ) override;
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y ) override;
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y ) override;
	void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y ) override;
	void OnResize() override;

private:
	// Internal.
	Int32		Separator;
	Int32		HoldOffset;
	Int32		OldXSize;
	Bool		bMoveSeparator;

	Bool UpdateSubWidgets();
};


/*-----------------------------------------------------------------------------
	WVSplitBox.
-----------------------------------------------------------------------------*/

#define VSPLIT_THICKNESS 7

//
// VSplitter Ratio-Rule.
//
enum EVSplitRatio
{
	VRR_KeepAspect,
	VRR_PreferTop,
	VRR_PreferBottom
};


//
// A vertical split box.
//
class WVSplitBox: public WContainer
{
public:
	// Variables.
	Int32 TopMin;
	Int32 TopMax;
	Int32 BottomMin;
	Int32 BottomMax;
	EVSplitRatio RatioRule;

	// WVSplitBox interface.
	WVSplitBox( WContainer* InOwner, WWindow* InRoot );

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render ) override;
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y ) override;
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y ) override;
	void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y ) override;
	void OnResize() override;

private:
	// Internal.
	Int32		Separator;
	Int32		HoldOffset;
	Int32		OldYSize;
	Bool		bMoveSeparator;

	Bool UpdateSubWidgets();
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/