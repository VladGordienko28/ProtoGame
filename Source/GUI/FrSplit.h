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
	Integer LeftMin;
	Integer LeftMax;
	Integer RightMin;
	Integer RightMax;
	EHSplitRatio RatioRule;

	// WHSplitBox interface.
	WHSplitBox( WContainer* InOwner, WWindow* InRoot );

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render ) override;
	void OnMouseDown( EMouseButton Button, Integer X, Integer Y ) override;
	void OnMouseUp( EMouseButton Button, Integer X, Integer Y ) override;
	void OnMouseMove( EMouseButton Button, Integer X, Integer Y ) override;
	void OnResize() override;

private:
	// Internal.
	Integer		Separator;
	Integer		HoldOffset;
	Integer		OldXSize;
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
	Integer TopMin;
	Integer TopMax;
	Integer BottomMin;
	Integer BottomMax;
	EVSplitRatio RatioRule;

	// WVSplitBox interface.
	WVSplitBox( WContainer* InOwner, WWindow* InRoot );

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render ) override;
	void OnMouseDown( EMouseButton Button, Integer X, Integer Y ) override;
	void OnMouseUp( EMouseButton Button, Integer X, Integer Y ) override;
	void OnMouseMove( EMouseButton Button, Integer X, Integer Y ) override;
	void OnResize() override;

private:
	// Internal.
	Integer		Separator;
	Integer		HoldOffset;
	Integer		OldYSize;
	Bool		bMoveSeparator;

	Bool UpdateSubWidgets();
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/