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
	Integer AddNode( String InName, Integer IniParent=-1, void* InData=nullptr );
	void Empty();
	void SelectNext();
	void SelectPrev();
	void OnChange();
	void OnDoubleClick();
	Integer FindNode( String TestName, Integer iParent=-1 );

	// Utility.
	void AlphabetSort();
	void ExpandAll();
	void CollapseAll();
	void* DataOf( Integer iNode );

	// WWidget interface.
	void OnDblClick( EMouseButton Button, Integer X, Integer Y ) override;  
	void OnPaint( CGUIRenderBase* Render ) override;
	void OnResize() override;
	void OnMouseDown( EMouseButton Button, Integer X, Integer Y ) override;
	void OnMouseUp( EMouseButton Button, Integer X, Integer Y ) override;
	void OnKeyDown( Integer Key ) override;
	void OnMouseScroll( Integer Delta ) override;

	// Accessors.
	inline void SetSelected( Integer i, Bool bNotify=true )
	{
		iSelected = i;
		ScrollToNode( i );
		if( bNotify )
			OnChange();
	};
	inline Integer GetSelected() const
	{
		return iSelected;
	}

	// A gui tree node.
	class TNode
	{
	public:
		// Variables.
		String		Name;
		Integer		Level;
		Integer		iParent;
		Integer		NumChildren;
		void*		Data;
		Bool		bExpanded;

		// TNode interface.
		TNode( String InName, Integer InLevel, Integer IniParent, void* InData )
			:	Name( InName ), Level( InLevel ),
				iParent( IniParent ), Data( InData ),
				bExpanded( true ), NumChildren(0)
		{}
	};
	TArray<TNode>		Nodes;

private:
	// Tree internal.
	WSlider*			ScrollBar;
	TArray<Integer>		RenderOrder;
	Integer				ScrollTop;
	Integer				CharHeight;
	Integer				iSelected;

	void ScrollBarChange( WWidget* Sender );
	void ScrollToNode( Integer iNode );
	void ComputeOrder();
	Integer XYToIndex( Integer X, Integer Y, Bool* AtSign=nullptr );
	Bool IsParentNode( Integer iNode, Integer TestParent ) const;
	Integer FindLastChildren( Integer iParent );
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/