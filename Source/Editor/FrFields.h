/*=============================================================================
    FrFields.h: WObjectInspector.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    Declarations.
-----------------------------------------------------------------------------*/

//
// Forward declaration.
//
class CInspectorItemBase;
class WObjectInspector;


/*-----------------------------------------------------------------------------
    CInspectorItemBase.
-----------------------------------------------------------------------------*/

// Height of inspector's item.
#define INSPECTOR_ITEM_HEIGHT	20 
#define INSPECTOR_HEADER_SIZE	FORM_HEADER_SIZE


//
// An abstract inspector item.
//
class CInspectorItemBase
{
public:
	// Variables.
	Bool						bHidden;
	Bool						bExpanded;
	String						Caption;
	WObjectInspector*			Inspector;
	TArray<CInspectorItemBase*>	Children;
	TArray<FObject*>			Objects;			
	UInt32						Depth;
	Int32						Top;

	// CInspectorItemBase interface.
	CInspectorItemBase( WObjectInspector* InInspector, UInt32 InDepth );
	virtual ~CInspectorItemBase();
	void CollapseAll();
	void ExpandAll();

	// Events from Object Inspector.
	virtual void Paint( TPoint Base, CGUIRenderBase* Render );
	virtual void MouseDown( EMouseButton Button, Int32 X, Int32 Y );
	virtual void MouseUp( EMouseButton Button, Int32 X, Int32 Y );
	virtual void MouseMove( EMouseButton Button, Int32 X, Int32 Y );
	virtual void DragOver( void* Data, Int32 X, Int32 Y, Bool& bAccept );
	virtual void DragDrop( void* Data, Int32 X, Int32 Y );
	virtual void Unselect();
};


/*-----------------------------------------------------------------------------
    WObjectInspector.
-----------------------------------------------------------------------------*/

//
// Object inspector class.
//
class WObjectInspector: public WContainer, public CRefsHolder
{
public:
	// Controls.
	WSlider*			ScrollBar;
	Int32				Separator;
	Bool				bMoveSep;

	// Items.
	TArray<CInspectorItemBase*>	Children;
	TArray<FObject*>			Objects;
	CInspectorItemBase*			Selected;
	CClass*						TopClass;
	TArray<TNotifyEvent>		CustomHandlers;

	// Entity pick processing.
	WLevelPage*				LevelPage;
	Bool					bWaitForPick;
	CInspectorItemBase*		WaitItem;

	// Color selection wait.
	CInspectorItemBase*		WaitColor;

	// WObjectInspector interface.
	WObjectInspector( WContainer* InOwner, WWindow* InRoot );
	~WObjectInspector();
	void SetEditObjects( TArray<FObject*>& Objs );
	void SetEditObject( FObject* Obj );
	void UnselectAll();
	CInspectorItemBase* GetItemAt( Int32 ParentY, Int32& LocalY );
	void UpdateChildren();
	void Empty();

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );
	void OnResize();
	void OnDragOver( void* Data, Int32 X, Int32 Y, Bool& bAccept );
	void OnDragDrop( void* Data, Int32 X, Int32 Y );
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseScroll( Int32 Delta );

	// Entity pick functions.
	void BeginWaitForPick( CInspectorItemBase* Waiter );
	void ObjectPicked( FEntity* Picked );

	// Controls notifications.
	void ScrollChange( WWidget* Sender );
	void SomethingChange( WWidget* Sender );
	void PickClick( WWidget* Sender );
	void ColorSelected( WWidget* Sender );
	void DynArrayAddClick( WWidget* Sender );
	void DynArrayRemoveClick( WWidget* Sender );

	// CRefsHolder interface.
	void CountRefs( CSerializer& S );

	// Custom manipulation.
	void SetCustomCaption( String NewCaption );
	void AddCustomProperty( String PropName, const CTypeInfo& Type, void* Addr );
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/