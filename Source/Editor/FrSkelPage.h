/*=============================================================================
	FrSkelPage.h: Skeleton page.
	Created by Vlad Gordienko, Dec. 2017.
=============================================================================*/

/*-----------------------------------------------------------------------------
	WSkeletonPage.
-----------------------------------------------------------------------------*/

//
// An skeleton page.
//
class WSkeletonPage: public WEditorPage, public CRefsHolder
{
public:
	// Variables.
	FSkeleton*		Skeleton;

	// WSkeletonPage interface.
	WSkeletonPage( FSkeleton* InSkeleton, WContainer* InOwner, WWindow* InRoot );
	~WSkeletonPage();

	// WTabPage interface.
	void OnOpen();

	// WWidget interface.
	void OnKeyDown( Int32 Key );
	void OnMouseMove( EMouseButton Button, Int32 X, Int32 Y );
	void OnDblClick( EMouseButton Button, Int32 X, Int32 Y );   
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseUp( EMouseButton Button, Int32 X, Int32 Y );
	void OnMouseScroll( Int32 Delta );
	void OnPaint( CGUIRenderBase* Render );

	// WEditorPage interface.
	void RenderPageContent( CCanvas* Canvas );
	void TickPage( Float Delta );
	FResource* GetResource()
	{ 
		return Skeleton; 
	}

	// Refs holder!!!!!!!!!!!!!
	TPoint	BoneHintFrom;
	String	BoneHint;

	friend class WAnimationTrack;

private:
	// An skeleton editor tool.
	enum ESkelTool
	{
		SKT_Edit,
		SKT_AddBone,
		SKT_AddMaster,
		SKT_Link
	};

	
	// Internal variables.
	TViewInfo	SceneView;
	ESkelTool	Tool;
	CGizmo		Gizmo;
	Float		TranslationSnap;
	Int32		RotationSnap;

	// Internal widgets.
	WToolBar*			ToolBar;

	//!! picture buttons soon!
	WButton*			EditButton;
	WButton*			AddBoneButton;
	WButton*			AddMasterButton;
	WButton*			AddIKButton;
	WButton*			DrawLinksButton;
	WButton*			LinkButton;
	WButton*			BreakLinksButton;

	WAnimationTrack*	AnimationTrack;

	// Helper functions.
	TSkelPose& GetCurrentPose();
	Int32 GetBoneAt( Int32 X, Int32 Y );
	void ShowBoneProperties( Int32 iBone );
	void DrawLink( CCanvas* Canvas, Int32 i, Int32 j, TColor Color );
	void UnselectAll();
	math::Vector GetBoneCenter( Int32 iBone );
	String MakeUniqueBoneName( String Prefix );
	void UpdateGizmo( Int32 iNewSele = -1 );
	void BoneEditChange( WWidget* Sender );

	// Internal widgets events.
	void ButtonEditClick( WWidget* Sender );
	void ButtonAddBoneClick( WWidget* Sender );
	void ButtonAddMasterClick( WWidget* Sender );
	void ButtonAddIKClick( WWidget* Sender );
	void ButtonLinkClick( WWidget* Sender );
	void ButtonBreakLinksClick( WWidget* Sender );

	// Top level mouse events.
	virtual void OnMouseDrag( EMouseButton Button, Int32 X, Int32 Y, Int32 DeltaX, Int32 DeltaY );
	virtual void OnMouseBeginDrag( EMouseButton Button, Int32 X, Int32 Y );
	virtual void OnMouseEndDrag( EMouseButton Button, Int32 X, Int32 Y );
	virtual void OnMouseClick( EMouseButton Button, Int32 X, Int32 Y );
};


/*-----------------------------------------------------------------------------
	WAnimationTrack.
-----------------------------------------------------------------------------*/

//
// Skeleton page animation track.
//
class WAnimationTrack: public WContainer
{
public:
	// WAnimationTrack interface.
	WAnimationTrack( WSkeletonPage* InPage, WWindow* InRoot );
	~WAnimationTrack();
	void NoteMovement( Int32 iBone, Bool bOnlyRotation );
	void Tick( Float Delta );

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );
	void OnMouseDown( EMouseButton Button, Int32 X, Int32 Y );

	friend WSkeletonPage;

	// Accessors.
	Bool IsRecording() const;
	Bool IsPlaying() const;

protected:
	// Internal.
	WSkeletonPage*		Page;
	FSkeleton*			Skeleton;

	// Player specific.
	Int32				CurrentFrame;
	Float				PlayTime;

	// Internal widgets.
	WToolBar*			ToolBar;
	WButton*			AddActionButton;				// Picture button!!!
	WButton*			DeleteActionButton;				// 
	WComboBox*			ActionsList;
	WButton*			RecordButton;
	WButton*			PlayButton;


	WButton*			StopButton;//


	// Buttons "Step Forward" "Step Back" "Rewind" "JumpToEnd" 

	// Widgets notifications.
	void ButtonAddActionClick( WWidget* Sender );
	void ButtonDeleteActionClick( WWidget* Sender );
	void ButtonPlayClick( WWidget* Sender );
};


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/