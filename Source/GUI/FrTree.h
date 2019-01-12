/*=============================================================================
	FrTree.h: Tree-based widgets.
	Created by Vlad Gordienko, Apr. 2018.
=============================================================================*/

/*-----------------------------------------------------------------------------
	WTreeView.
-----------------------------------------------------------------------------*/

//
// A tree view widget.
//
class WTreeView: public WContainer
{
public:
	// Variables.
	TNotifyEvent		EventClick;
	TNotifyEvent		EventDblClick;

	// WTreeView interface.
	WTreeView( WContainer* InOwner, WWindow* InRoot );
	Int32 AddNode( String InName, Int32 IniParent=-1, void* InData=nullptr );
	void Empty();
	void SelectNext();
	void SelectPrev();
	void OnChange();
	void OnDoubleClick();
	Int32 FindNode( String TestName, Int32 iParent=-1 );

	// Utility.
	void AlphabetSort();
	void ExpandAll();
	void CollapseAll();
	void* DataOf( Int32 iNode );

	// WWidget interface.
	void OnDblClick( EMouseButton Button, Int32 X, Int32 Y ) override;  
	void OnPaint( CGUIRenderBase* Render ) override;
	void OnResize() override;
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y ) override;
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y ) override;
	void OnKeyDown( Int32 Key ) override;
	void OnMouseScroll( Int32 Delta ) override;

	// Accessors.
	inline void SetSelected( Int32 i, Bool bNotify=true )
	{
		iSelected = i;
		ScrollToNode( i );
		if( bNotify )
			OnChange();
	};
	inline Int32 GetSelected() const
	{
		return iSelected;
	}

	// A gui tree node.
	class TNode
	{
	public:
		// Variables.
		String		Name;
		Int32		Level;
		Int32		iParent;
		Int32		NumChildren;
		void*		Data;
		Bool		bExpanded;

		// TNode interface.
		TNode( String InName, Int32 InLevel, Int32 IniParent, void* InData )
			:	Name( InName ), Level( InLevel ),
				iParent( IniParent ), Data( InData ),
				bExpanded( true ), NumChildren(0)
		{}
	};
	TArray<TNode>		Nodes;

private:
	// Tree internal.
	WSlider*			ScrollBar;
	TArray<Int32>		RenderOrder;
	Int32				ScrollTop;
	Int32				CharHeight;
	Int32				iSelected;

	void ScrollBarChange( WWidget* Sender );
	void ScrollToNode( Int32 iNode );
	void ComputeOrder();
	Int32 XYToIndex( Int32 X, Int32 Y, Bool* AtSign=nullptr );
	Bool IsParentNode( Int32 iNode, Int32 TestParent ) const;
	Int32 FindLastChildren( Int32 iParent );
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/