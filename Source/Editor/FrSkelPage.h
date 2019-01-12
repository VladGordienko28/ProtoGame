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
	void OnKeyDown( Integer Key );
	void OnMouseMove( EMouseButton Button, Integer X, Integer Y );
	void OnDblClick( EMouseButton Button, Integer X, Integer Y );   
	void OnMouseDown( EMouseButton Button, Integer X, Integer Y );
	void OnMouseUp( EMouseButton Button, Integer X, Integer Y );
	void OnMouseScroll( Integer Delta );
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
	Integer		RotationSnap;

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
	Integer GetBoneAt( Integer X, Integer Y );
	void ShowBoneProperties( Integer iBone );
	void DrawLink( CCanvas* Canvas, Integer i, Integer j, TColor Color );
	void UnselectAll();
	TVector GetBoneCenter( Integer iBone );
	String MakeUniqueBoneName( String Prefix );
	void UpdateGizmo( Integer iNewSele = -1 );
	void BoneEditChange( WWidget* Sender );

	// Internal widgets events.
	void ButtonEditClick( WWidget* Sender );
	void ButtonAddBoneClick( WWidget* Sender );
	void ButtonAddMasterClick( WWidget* Sender );
	void ButtonAddIKClick( WWidget* Sender );
	void ButtonLinkClick( WWidget* Sender );
	void ButtonBreakLinksClick( WWidget* Sender );

	// Top level mouse events.
	virtual void OnMouseDrag( EMouseButton Button, Integer X, Integer Y, Integer DeltaX, Integer DeltaY );
	virtual void OnMouseBeginDrag( EMouseButton Button, Integer X, Integer Y );
	virtual void OnMouseEndDrag( EMouseButton Button, Integer X, Integer Y );
	virtual void OnMouseClick( EMouseButton Button, Integer X, Integer Y );
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
	void NoteMovement( Integer iBone, Bool bOnlyRotation );
	void Tick( Float Delta );

	// WWidget interface.
	void OnPaint( CGUIRenderBase* Render );
	void OnMouseDown( EMouseButton Button, Integer X, Integer Y );

	friend WSkeletonPage;

	// Accessors.
	Bool IsRecording() const;
	Bool IsPlaying() const;

protected:
	// Internal.
	WSkeletonPage*		Page;
	FSkeleton*			Skeleton;

	// Player specific.
	Integer				CurrentFrame;
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