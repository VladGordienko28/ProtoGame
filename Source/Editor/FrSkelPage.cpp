/*=============================================================================
	FrSkelPage.cpp: Skeleton editor.
	Created by Vlad Gordienko, Dec. 2017.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
	WSkeletonPage implementation.
-----------------------------------------------------------------------------*/

//
// Skeleton page constants.
//
#define SKEL_SCENE_SIZE			20.f
#define SKEL_SCENE_SIZE_HALF	(SKEL_SCENE_SIZE / 2.f)
#define FRAMES_PER_SECOND		25

#define SKEL_SCENE_BG_COLOR				TColor( 0x10, 0x10, 0x10, 0xff )
#define SKEL_SCENE_GRID_COLOR			TColor( 0x50, 0x6e, 0x6e, 0xff )
#define SKEL_SCENE_CENTER_COLOR			TColor( 0x6e, 0x6e, 0x50, 0xff )
#define SKEL_SCENE_RECORD_CENTER_COLOR	TColor( 0x6e, 0x50, 0x50, 0xff )
#define DEFAULT_IK_COLOR				TColor( 0x63, 0x96, 0xe0, 0xff )

	
//
// Skeleton page constructor.
//
WSkeletonPage::WSkeletonPage( FSkeleton* InSkeleton, WContainer* InOwner, WWindow* InRoot )
	:	WEditorPage( InOwner, InRoot ),
		Tool( SKT_Edit ),
		Gizmo(0.32f)
{
	/*
	//	For fast construction!!!
	if( !InSkeleton )
		InSkeleton	= NewObject<FSkeleton>();
		*/

	// Initialize level's variables.
	Caption			= InSkeleton->GetName();
	PageType		= PAGE_Skeleton;
	Color			= PAGE_COLOR_SKELETON;
	TabWidth		= Root->Font1->TextWidth( *Caption ) + 30;
	Skeleton		= InSkeleton;

	// Toolbar and buttons.
	ToolBar		= new WToolBar( this, Root );
	ToolBar->SetSize( 3000, 28 );


	EditButton				= new WButton( ToolBar, Root );
	EditButton->Caption		= L"Edit";
	EditButton->Tooltip		= L"Edit Tool";
	EditButton->bDown		= true;
	EditButton->EventClick	= WIDGET_EVENT(WSkeletonPage::ButtonEditClick); 
	EditButton->SetSize( 75, 22 );
	ToolBar->AddElement( EditButton );

	AddBoneButton				= new WButton( ToolBar, Root );
	AddBoneButton->Caption		= L"Add Bone";
	AddBoneButton->Tooltip		= L"Add Bone Tool";
	AddBoneButton->bDown		= false;
	AddBoneButton->EventClick	= WIDGET_EVENT(WSkeletonPage::ButtonAddBoneClick); 
	AddBoneButton->SetSize( 75, 22 );
	ToolBar->AddElement( AddBoneButton );

	AddMasterButton				= new WButton( ToolBar, Root );
	AddMasterButton->Caption		= L"Add Master";
	AddMasterButton->Tooltip		= L"Add Master Tool";
	AddMasterButton->bDown		= false;
	AddMasterButton->EventClick	= WIDGET_EVENT(WSkeletonPage::ButtonAddMasterClick); 
	AddMasterButton->SetSize( 75, 22 );
	ToolBar->AddElement( AddMasterButton );

	AddIKButton					= new WButton( ToolBar, Root );
	AddIKButton->Caption		= L"Add IK";
	AddIKButton->Tooltip		= L"Add IK Chain Tool";
	AddIKButton->bDown			= false;
	AddIKButton->EventClick		= WIDGET_EVENT(WSkeletonPage::ButtonAddIKClick); 
	AddIKButton->SetSize( 75, 22 );
	ToolBar->AddElement( AddIKButton );
	ToolBar->AddElement(nullptr);

	DrawLinksButton				= new WButton( ToolBar, Root );
	DrawLinksButton->Caption		= L"Draw Links";
	DrawLinksButton->Tooltip		= L"Draw Links";
	DrawLinksButton->bDown		= false;
	DrawLinksButton->bToggle	= true;
	DrawLinksButton->SetSize( 75, 22 );
	ToolBar->AddElement( DrawLinksButton );
	ToolBar->AddElement(nullptr);

	LinkButton				= new WButton( ToolBar, Root );
	LinkButton->Caption		= L"Link 'em";
	LinkButton->Tooltip		= L"Link 'em";
	LinkButton->bDown		= false;
	LinkButton->EventClick	= WIDGET_EVENT(WSkeletonPage::ButtonLinkClick); 
	LinkButton->SetSize( 75, 22 );
	ToolBar->AddElement( LinkButton );

	BreakLinksButton				= new WButton( ToolBar, Root );
	BreakLinksButton->Caption		= L"Break Links";
	BreakLinksButton->Tooltip		= L"Break all links from selected bones";
	BreakLinksButton->EventClick	= WIDGET_EVENT(WSkeletonPage::ButtonBreakLinksClick); 
	BreakLinksButton->SetSize( 75, 22 );
	ToolBar->AddElement( BreakLinksButton );



	/*


	PaintButton				= new WPictureButton( ToolBar, Root );
	PaintButton->Tooltip	= L"Model Paint Tool";
	PaintButton->Scale		= TSize( 16, 16 );
	PaintButton->Offset		= TPoint( 96, 32 );
	PaintButton->Picture	= Root->Icons;
	PaintButton->EventClick = WIDGET_EVENT(WLevelPage::ButtonPaintClick);
	PaintButton->SetSize( 22, 22 );
	ToolBar->AddElement( PaintButton );
	
	KeyButton				= new WPictureButton( ToolBar, Root );
	KeyButton->Tooltip		= L"Keyframe Edit Tool";
	KeyButton->Scale		= TSize( 16, 16 );
	KeyButton->Offset		= TPoint( 80, 32 );
	KeyButton->Picture		= Root->Icons;
	KeyButton->EventClick	= WIDGET_EVENT(WLevelPage::ButtonKeyClick);
	KeyButton->SetSize( 22, 22 );
	ToolBar->AddElement( KeyButton );
	ToolBar->AddElement(nullptr);
	*/



	/////////////////////////////////////////
	/////////////////////////////////////////////////
	////////////////////////////////////////////////



	SceneView.Zoom	= 1.f;
	SceneView.Coords	= math::Coords(math::Vector(0, 0), 0);
	SceneView.UnCoords	= SceneView.Coords.transpose();

	AnimationTrack			= new WAnimationTrack( this, Root );
	AnimationTrack->Align	= AL_Bottom;




	// Initialize gizmo tool.
	Gizmo.SetMode(GIZMO_Rotate);
	Gizmo.Reset();

	// Standard snap.
	TranslationSnap	= 0.25f;
	RotationSnap	= 2730;		// 15 deg.

	/*
	// Start building human skelet.
	TBoneInfo Body;
	Body.Color = TColor( 8, 110, 134, 255 );
	//Body.iPosCtrl = 21;
	//Body.iRotCtrl = 21;
	Body.Scale = 3.f;
	Body.Name = L"Body";
	Body.Type = SC_Bone;
	Skeleton->Bones.Push(Body);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(0, 0), 16384));

	TBoneInfo Head;
	Head.Color = TColor( 6, 134, 113, 255 );
	//Head.iPosCtrl = 0;
	//Head.iRotCtrl = 0;
	Head.Scale = 1.5f;
	Head.Name = L"Head";
	Head.Type = SC_Bone;
	Skeleton->Bones.Push(Head);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(0, 3), 16384));

	TBoneInfo EyeR;
	EyeR.Color = TColor( 214, 229, 116, 255 );
	//EyeR.iPosCtrl = 1;
	//EyeR.iRotCtrl = 16;
	EyeR.bLookAt = true;
	EyeR.Scale = 0.5f;
	EyeR.Name = L"EyeR";
	EyeR.Type = SC_Bone;
	Skeleton->Bones.Push(EyeR);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(-0.5f, 4), 0));

	TBoneInfo EyeL;
	EyeL.Color = TColor( 214, 229, 116, 255 );
	//EyeL.iPosCtrl = 1;
	//EyeL.iRotCtrl = 16;
	EyeL.bLookAt = true;
	EyeL.Scale = 0.5f;
	EyeL.Name = L"EyeL";
	EyeL.Type = SC_Bone;
	Skeleton->Bones.Push(EyeL);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(0.5f, 4), 0));

	TBoneInfo ArmR;
	ArmR.Color = TColor( 176, 26, 26, 255 );
	//ArmR.iPosCtrl = 0;
	//ArmR.iRotCtrl = 22;
	ArmR.Scale = 1.5f;
	ArmR.Name = L"ArmR";
	ArmR.Type = SC_Bone;
	Skeleton->Bones.Push(ArmR);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(-0.5f, 2.5), 32768));

	TBoneInfo ForeArmR;
	ForeArmR.Color = TColor( 176, 26, 26, 255 );
	//ForeArmR.iPosCtrl = 4;
	//ForeArmR.iRotCtrl = 22;
	ForeArmR.Scale = 1.5f;
	ForeArmR.Name = L"ForeArmR";
	ForeArmR.Type = SC_Bone;
	Skeleton->Bones.Push(ForeArmR);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(-2.f, 2.5), 16384));

	TBoneInfo HandR;
	HandR.Color = TColor( 176, 26, 26, 255 );
	//HandR.iPosCtrl = 5;
	//HandR.iRotCtrl = 17;
	HandR.Scale = 0.5f;
	HandR.Name = L"HandR";
	HandR.Type = SC_Bone;
	Skeleton->Bones.Push(HandR);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(-2.f, 4.0), 16384));

	TBoneInfo ArmL;
	ArmL.Color = TColor( 6, 134, 6, 255 );
	//ArmL.iPosCtrl = 0;
	//ArmL.iRotCtrl = 23;
	ArmL.Scale = 1.5f;
	ArmL.Name = L"ArmL";
	ArmL.Type = SC_Bone;
	Skeleton->Bones.Push(ArmL);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(+0.5f, 2.5), 0));

	TBoneInfo ForeArmL;
	ForeArmL.Color = TColor( 6, 134, 6, 255 );
	//ForeArmL.iPosCtrl = 7;
	//ForeArmL.iRotCtrl = 23;
	ForeArmL.Scale = 1.5f;
	ForeArmL.Name = L"ForeArmL";
	ForeArmL.Type = SC_Bone;
	Skeleton->Bones.Push(ForeArmL);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(+2.f, 2.5), 16384));

	TBoneInfo HandL;
	HandL.Color = TColor( 6, 134, 6, 255 );
	//HandL.iPosCtrl = 8;
	//HandL.iRotCtrl = 18;
	HandL.Scale = 0.5f;
	HandL.Name = L"HandL";
	HandL.Type = SC_Bone;
	Skeleton->Bones.Push(HandL);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(+2.f, 4.0), 0));

	TBoneInfo ThighR;
	ThighR.Color = TColor( 176, 26, 26, 255 );
	//ThighR.iPosCtrl = 0;
	//ThighR.iRotCtrl = -1;
	ThighR.Scale = 2.0f;
	ThighR.Name = L"ThighR";
	ThighR.Type = SC_Bone;
	Skeleton->Bones.Push(ThighR);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(-0.5, 0), 49152));

	TBoneInfo ShinR;
	ShinR.Color = TColor( 176, 26, 26, 255 );
	//ShinR.iPosCtrl = 10;
	//ShinR.iRotCtrl = -1;
	ShinR.Scale = 1.5f;
	ShinR.Name = L"ShinR";
	ShinR.Type = SC_Bone;
	Skeleton->Bones.Push(ShinR);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(-0.5, -2.0), 32768));

	TBoneInfo AnkleR;
	AnkleR.Color = TColor( 176, 26, 26, 255 );
	//AnkleR.iPosCtrl = 11;
	//AnkleR.iRotCtrl = 19;
	AnkleR.Scale = 0.5f;
	AnkleR.Name = L"AnkleR";
	AnkleR.Type = SC_Bone;
	Skeleton->Bones.Push(AnkleR);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(-2.0, -2.0), 32768));

	TBoneInfo ThighL;
	ThighL.Color = TColor( 6, 134, 6, 255 );
	//ThighL.iPosCtrl = 0;
	//ThighL.iRotCtrl = -1;
	ThighL.Scale = 2.0f;
	ThighL.Name = L"ThighL";
	ThighL.Type = SC_Bone;
	Skeleton->Bones.Push(ThighL);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(0.5, 0), 49152));

	TBoneInfo ShinL;
	ShinL.Color = TColor( 6, 134, 6, 255 );
	//ShinL.iPosCtrl = 13;
	//ShinL.iRotCtrl = -1;
	ShinL.Scale = 1.5f;
	ShinL.Name = L"ShinL";
	ShinL.Type = SC_Bone;
	Skeleton->Bones.Push(ShinL);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(0.5, -2.0), 0));

	TBoneInfo AnkleL;
	AnkleL.Color = TColor( 6, 134, 6, 255 );
	//AnkleL.iPosCtrl = 14;
	//AnkleL.iRotCtrl = 20;
	AnkleL.Scale = 0.5f;
	AnkleL.Name = L"AnkleL";
	AnkleL.Type = SC_Bone;
	Skeleton->Bones.Push(AnkleL);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(2.0, -2.0), 0));

	TBoneInfo EyeMaster;
	EyeMaster.Color = TColor( 228, 153, 184, 255 );
	//EyeMaster.iPosCtrl = 1;
	//EyeMaster.iRotCtrl = 1;
	EyeMaster.Scale = 0.4f;
	EyeMaster.Name = L"EyeMaster";
	EyeMaster.Type = SC_Master;
	Skeleton->Bones.Push(EyeMaster);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(4, 4), 0));

	TBoneInfo RHandMaster;
	RHandMaster.Color = TColor( 134, 6, 6, 255 );
	//RHandMaster.iPosCtrl = 21;
	//RHandMaster.iRotCtrl = 21;
	RHandMaster.Scale = 0.5f;
	RHandMaster.Name = L"RHandMaster";
	RHandMaster.Type = SC_Master;
	Skeleton->Bones.Push(RHandMaster);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(-2, 4), 0));

	TBoneInfo LHandMaster;
	LHandMaster.Color = TColor( 6, 134, 58, 255 );
	//LHandMaster.iPosCtrl = -1;
	//LHandMaster.iRotCtrl = -1;
	LHandMaster.Scale = 0.5f;
	LHandMaster.bFlipIK = true;
	LHandMaster.Name = L"LHandMaster";
	LHandMaster.Type = SC_Master;
	Skeleton->Bones.Push(LHandMaster);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(2, 4), 0));

	TBoneInfo RFootMaster;
	RFootMaster.Color = TColor( 134, 6, 6, 255 );
	//RFootMaster.iPosCtrl = -1;
	//RFootMaster.iRotCtrl = -1;
	RFootMaster.Scale = 0.5f;
	RFootMaster.Name = L"RFootMaster";
	RFootMaster.Type = SC_Master;
	Skeleton->Bones.Push(RFootMaster);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(-2, -2), 0));

	TBoneInfo LFootMaster;
	LFootMaster.Color = TColor( 6, 134, 58, 255 );
	//LFootMaster.iPosCtrl = -1;
	//LFootMaster.iRotCtrl = -1;
	LFootMaster.Scale = 0.5f;
	LFootMaster.Name = L"LFootMaster";
	LFootMaster.Type = SC_Master;
	Skeleton->Bones.Push(LFootMaster);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(2, -2), 0));

	TBoneInfo BodyMaster;
	BodyMaster.Color = TColor( 8, 8, 136, 255 );
	//BodyMaster.iPosCtrl = -1;
	//BodyMaster.iRotCtrl = -1;
	BodyMaster.Scale = 1.0f;
	BodyMaster.Name = L"BodyMaster";
	BodyMaster.Type = SC_Master;
	Skeleton->Bones.Push(BodyMaster);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(0, 0), 0));

	TBoneInfo RArmIK;
	RArmIK.Color = TColor( 99, 150, 224, 255 );
	//RArmIK.iPosCtrl = 17;
	//RArmIK.iRotCtrl = 17;
	RArmIK.Scale = 1.25f;
	RArmIK.Name = L"RArmIK";
	RArmIK.iEndJoint = 5;
	RArmIK.Type = SC_IKSolver;
	Skeleton->Bones.Push(RArmIK);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(-2, 4), 0));

	TBoneInfo LArmIK;
	LArmIK.Color = TColor( 99, 150, 224, 255 );
	//LArmIK.iPosCtrl = 18;
	//LArmIK.iRotCtrl = 18;
	LArmIK.Scale = 1.25f;
	LArmIK.Name = L"LArmIK";
	LArmIK.bFlipIK = false;
	LArmIK.iEndJoint = 8;
	LArmIK.Type = SC_IKSolver;
	Skeleton->Bones.Push(LArmIK);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(2, 4), 0));

	TBoneInfo RLegIK;
	RLegIK.Color = TColor( 99, 150, 224, 255 );
	//LArmIK.iPosCtrl = 18;
	//LArmIK.iRotCtrl = 18;
	RLegIK.Scale = 1.25f;
	RLegIK.Name = L"RLegIK";
	RLegIK.bFlipIK = false;
	RLegIK.Type = SC_IKSolver;
	Skeleton->Bones.Push(RLegIK);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(-2, -2), 0));

	TBoneInfo LLegIK;
	LLegIK.Color = TColor( 99, 150, 224, 255 );
	//LArmIK.iPosCtrl = 18;
	//LArmIK.iRotCtrl = 18;
	LLegIK.Scale = 1.25f;
	LLegIK.Name = L"LLegIK";
	LLegIK.bFlipIK = true;
	LLegIK.Type = SC_IKSolver;
	Skeleton->Bones.Push(LLegIK);
	Skeleton->RefPose.BonesPose.Push(TBonePose(TVector(2, -2), 0));


	// Start linking...
	Skeleton->LinkTo( 4, 0 );
	Skeleton->LinkTo( 3, 1 );
	Skeleton->LinkTo( 1, 0 );
	Skeleton->LinkTo( 0, 21 );
	Skeleton->LinkTo( 2, 1 );
	Skeleton->LinkLookAtTo( 2, 16 );
	Skeleton->LinkLookAtTo( 3, 16 );
	Skeleton->LinkTo( 7, 0 );
	Skeleton->LinkTo( 5, 4 );
	Skeleton->LinkTo( 8, 7 );
	Skeleton->LinkTo( 6, 5 );
	Skeleton->LinkTo( 9, 8 );
	Skeleton->LinkTo( 23, 18 );
	Skeleton->LinkTo( 22, 17 );
	Skeleton->LinkIKSolverTo( 22, 5 );
	Skeleton->LinkIKSolverTo( 23, 8 );
	Skeleton->LinkRotationTo( 6, 17 );
	Skeleton->LinkRotationTo( 9, 18 );

	Skeleton->LinkTo( 10, 0 );
	Skeleton->LinkTo( 13, 0 );
	Skeleton->LinkTo( 11, 10 );
	Skeleton->LinkTo( 14, 13 );
	Skeleton->LinkTo( 12, 11 );
	Skeleton->LinkTo( 15, 14 );

	Skeleton->LinkIKSolverTo( 24, 11 );
	Skeleton->LinkIKSolverTo( 25, 14 );

	Skeleton->LinkRotationTo( 12, 19 );
	Skeleton->LinkRotationTo( 15, 20 );

	//Skeleton->LinkTo( 16, 14 );


	Skeleton->BuildTransformationTable();
	*/


}
WSkeletonPage::~WSkeletonPage(){}

void WSkeletonPage::OnKeyDown( Int32 Key ){}



void WSkeletonPage::TickPage( Float Delta )
{
	/*
	TBonePose& Pose = GetCurrentPose().BonesPose[21];

	static Float Timer = 0.f;
	Timer += Delta*0.5f;

	Pose.Coords = TCoords( TVector( Sin(Timer)*2.f, Cos(Timer*2.3f)*1.0f ), TAngle(Integer(sin(Timer)*16384)) );

	GetCurrentPose().ComputeTransforms(Skeleton);*/

	// Update tracker.
	AnimationTrack->Tick(Delta);

}
	//FSkeleton*		Skeleton;





// OnPaint add hint for bone name display.

void WSkeletonPage::OnOpen()
{
	// Open Skeleton properties.
	ShowBoneProperties(-1);
}


//
// Draw a controls linking arrow.
//
static void DrawArrow( CCanvas* Canvas, const math::Vector& From, const math::Vector& To, TColor Color )
{
	// Draw straight line.
	Canvas->DrawLine( From, To, Color, false );

	// Precompute arrowhead.
	math::Vector ArrowHead = math::Vector( 0.13f, 0.05f )*Canvas->View.Zoom;
	math::Coords AHCoords = math::Coords
	(
		To,
		math::vectorToAngle(From-To)
	).transpose();

	// Draw arrowhead.
	Canvas->DrawLine
	(
		math::transformPointBy( math::Vector( ArrowHead.x, ArrowHead.y ), AHCoords ),
		To,
		Color,
		false
	);
	Canvas->DrawLine
	(
		math::transformPointBy( math::Vector( ArrowHead.x, -ArrowHead.y ), AHCoords ),
		To,
		Color,
		false
	);
}


//
// Return world location of bone or other control center.
// Warning, center is not a bone pivot!
//
math::Vector WSkeletonPage::GetBoneCenter( Int32 iBone )
{
	assert(iBone>=0 && iBone<Skeleton->Bones.size());
	TBonePose& Pose = GetCurrentPose().BonesPose[iBone];
	TBoneInfo& Info = Skeleton->Bones[iBone];

	return Info.Type == SC_Bone ? 
						Pose.Location + math::angleToVector(Pose.Rotation)*(Info.Scale*0.5f) :
						Pose.Location;
}


//
// Draw a link from the i'th bone to the j-th.
//
void WSkeletonPage::DrawLink( CCanvas* Canvas, Int32 i, Int32 j, TColor Color )
{
	DrawArrow( Canvas, GetBoneCenter(i), GetBoneCenter(j), Color );
}


//
// Unselect all bones.
//
void WSkeletonPage::UnselectAll()
{
	for( Int32 i=0; i<Skeleton->Bones.size(); i++ )
		Skeleton->Bones[i].Flags	&= ~BONE_Selected;
}







TSkelPose& WSkeletonPage::GetCurrentPose()
{
	return Skeleton->RefPose;
}
/*
	TVector Point = View.Deproject( X, Y );
	BoneHint = L"";

	BoneHintFrom = TPoint( X, Y );
	*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////


//
// Return index of bone or control at specified location.
//
Int32 WSkeletonPage::GetBoneAt( Int32 X, Int32 Y )
{
	math::Vector ScenePoint	= SceneView.Deproject( X, Y );
	TSkelPose& Pose		= GetCurrentPose();

	// Go through the bones.
	for( Int32 i=0; i<Skeleton->Bones.size(); i++ )
	{
		TBoneInfo& Bone = Skeleton->Bones[i];
		TBonePose& Tran	= Pose.BonesPose[i];

		math::Coords BoneCoords( Tran.Location, Tran.Rotation );
		math::Vector LocalPoint = math::transformPointBy( ScenePoint, BoneCoords );

		switch( Bone.Type )
		{
			case SC_Bone:
			{
				if( LocalPoint.x>=0.f && LocalPoint.x<=Bone.Scale )
					if( abs(LocalPoint.y) <= 0.2f )						// /// // // /// magic const!! eliminate it!!!
						return i;
				break;
			}
			case SC_Master:
			{
				if( math::distance(ScenePoint, Tran.Location) <= Bone.Scale )
					return i;
				break;
			}
			case SC_IKSolver:
			{
				const Float ThreshHold = 0.1f;			///////////////////////////////// another magic. eliminate it!
				if( abs(LocalPoint.y)<=Bone.Scale/2.f && abs(LocalPoint.x)<=ThreshHold )
					return i;
				if( abs(LocalPoint.x)<=Bone.Scale/2.f && abs(LocalPoint.y)<=ThreshHold )
					return i;
				break;
			}
		}
	}

	return -1;
}


//
// Open bone or control properties in ObjectInspector.
//
void WSkeletonPage::ShowBoneProperties( Int32 iBone )
{
	assert(iBone>=-1 && iBone<Skeleton->Bones.size());

	WObjectInspector* Inspector = GEditor->Inspector;
	Inspector->Empty();

	// If nothing selected, select Skeleton.
	if( iBone == -1 )
	{
		Inspector->SetEditObject(Skeleton);
		return;
	}

	TBoneInfo& Info = Skeleton->Bones[iBone];
	TBonePose& Pose = GetCurrentPose().BonesPose[iBone];

	// Shared properties.
	Inspector->AddCustomProperty( L"Name",		TYPE_String,	&Info.Name );
	Inspector->AddCustomProperty( L"Color",		TYPE_Color,		&Info.Color );
	Inspector->AddCustomProperty( L"Scale",		TYPE_Float,		&Info.Scale );
	Inspector->AddCustomProperty( L"Location",	TYPE_Vector,	&Pose.Location );
	Inspector->AddCustomProperty( L"Rotation",	TYPE_Angle,		&Pose.Rotation );

	// Control specific.
	switch( Info.Type )
	{
		case SC_Bone:
		{
			Inspector->SetCustomCaption(String::format(L"Inspector [Bone: %s]", *Info.Name));
			break;
		}
		case SC_Master:
		{
			Inspector->SetCustomCaption(String::format(L"Inspector [Master: %s]", *Info.Name));
			break;
		}
		case SC_IKSolver:
		{
			Inspector->SetCustomCaption(String::format(L"Inspector [IK Solver: %s]", *Info.Name));
			Inspector->AddCustomProperty( L"Flip?",	 TYPE_Bool,		&Info.bFlipIK );
			break;
		}
	}

	// Add inspector callback.
	Inspector->CustomHandlers.addUnique(WIDGET_EVENT(WSkeletonPage::BoneEditChange));
}


//
// When bone edited in inspector.
//
void WSkeletonPage::BoneEditChange( WWidget* Sender )
{
	GetCurrentPose().ComputeRefTransform(Skeleton);
}


//
// Generate unique bone name for skeleton.
//
String WSkeletonPage::MakeUniqueBoneName( String Prefix )
{
	Int32 i = 0;
	while( true )
	{
		String TestName = String::format( L"%s%d", *Prefix, i++ );
		if( !Skeleton->FindBone(TestName) )
			return TestName;
	}
}


/*-----------------------------------------------------------------------------
	Callbacks from widgets.
-----------------------------------------------------------------------------*/


void WSkeletonPage::ButtonEditClick( WWidget* Sender )
{
	Tool	= SKT_Edit;
	EditButton->bDown		= true;
	AddBoneButton->bDown	= false;
	AddMasterButton->bDown	= false;
	LinkButton->bDown		= false;
}
void WSkeletonPage::ButtonAddBoneClick( WWidget* Sender )
{
	Tool	= SKT_AddBone;
	EditButton->bDown		= false;
	AddBoneButton->bDown	= true;
	AddMasterButton->bDown	= false;
	LinkButton->bDown		= false;
}
void WSkeletonPage::ButtonAddMasterClick( WWidget* Sender )
{
	Tool	= SKT_AddMaster;
	EditButton->bDown		= false;
	AddBoneButton->bDown	= false;
	AddMasterButton->bDown	= true;
	LinkButton->bDown		= false;
}
void WSkeletonPage::ButtonLinkClick( WWidget* Sender )
{
	Tool	= SKT_Link;
	EditButton->bDown		= false;
	AddBoneButton->bDown	= false;
	AddMasterButton->bDown	= false;
	LinkButton->bDown		= true;
}

void WSkeletonPage::ButtonBreakLinksClick( WWidget* Sender )
{
	for( Int32 i=0; i<Skeleton->Bones.size(); i++ )
		if( Skeleton->Bones[i].Flags & BONE_Selected )
			Skeleton->BreakLinks( i );

	Skeleton->BuildTransformationTable();
	GetCurrentPose().ComputeRefTransform(Skeleton);
}

void WSkeletonPage::ButtonAddIKClick( WWidget* Sender )
{
	Int32 iEndBone = -1;
	for( Int32 i=0; i<Skeleton->Bones.size(); i++ )
	{
		TBoneInfo& Bone = Skeleton->Bones[i];
		if( Bone.Flags & BONE_Selected )
		{
			if( iEndBone != -1 )
			{
				iEndBone = -1;
				break;
			}
			iEndBone = i;
		}
	}

	if( iEndBone == -1 )
	{
		Root->ShowMessage( L"Please select only 1 bone to make IK chain", L"IK Chain" );
		return;
	}

	TBoneInfo& EndJoint = Skeleton->Bones[iEndBone];
	if( EndJoint.Type != SC_Bone )
	{
		Root->ShowMessage( L"Please select bone to make IK chain", L"IK Chain" );
		return;
	}
	if( EndJoint.iPosCtrl == -1 || Skeleton->Bones[EndJoint.iPosCtrl].Type != SC_Bone )
	{
		Root->ShowMessage( L"IK's end joint has no bone position controller", L"IK Chain" );
		return;
	}

	// Compute solver location.
	math::Vector Pivot =	GetCurrentPose().BonesPose[iEndBone].Location + 
					math::angleToVector(GetCurrentPose().BonesPose[iEndBone].Rotation)  *EndJoint.Scale;
	Pivot.snap(TranslationSnap);

	// Create solver itself.
	Int32 iIK = Skeleton->Bones.push(TBoneInfo
		( 
			SC_IKSolver, 
			MakeUniqueBoneName(L"IK"), 
			DEFAULT_IK_COLOR 
		));

	Skeleton->RefPose.BonesPose.push(TBonePose( Pivot, 0 ));
	assert(Skeleton->Bones.size()==Skeleton->RefPose.BonesPose.size());

	// Create links.
	Skeleton->Bones[iIK].Scale					= 1.25f;
	Skeleton->Bones[iIK].iEndJoint				= iEndBone;
	Skeleton->Bones[iEndBone].iRotCtrl			= iIK;
	Skeleton->Bones[EndJoint.iPosCtrl].iRotCtrl	= iIK;

	// Solve initial IK.
	Skeleton->BuildTransformationTable();
	GetCurrentPose().ComputeRefTransform(Skeleton);
}


/*-----------------------------------------------------------------------------
	SkelPage mouse functions.
-----------------------------------------------------------------------------*/

// Mouse capture internal.
// Two mouse buttons are independent.
static Bool		bLeftPressed	= false;
static Bool		bLWasMouse		= false;
static TPoint	LDwdPos			= TPoint( 0, 0 );
static TPoint	LLastPos		= TPoint( 0, 0 );
static Bool		bRightPressed	= false;
static Bool		bRWasMouse		= false;
static TPoint	RDwdPos			= TPoint( 0, 0 );
static TPoint	RLastPos		= TPoint( 0, 0 );


//
// On mouse up on page.
//
void WSkeletonPage::OnMouseUp( EMouseButton Button, Int32 X, Int32 Y )
{
	if( (Button == MB_Left) && (bLeftPressed) )
	{
		// Left mouse button has been released.
		if( bLWasMouse )
			OnMouseEndDrag( MB_Left, X, Y );
		else if( !bRightPressed )
			OnMouseClick( MB_Left, X, Y );

		bLeftPressed	= false;
		bLWasMouse		= false;	
	}
	else if( (Button == MB_Right) && (bRightPressed) )
	{
		// Right mouse button has been released.
		if( bRWasMouse )
			OnMouseEndDrag( MB_Right, X, Y );
		else if( !bLeftPressed )
			OnMouseClick( MB_Right, X, Y );

		bRightPressed	= false;
		bRWasMouse		= false;
	}
}


//
// On mouse press on page.
//
void WSkeletonPage::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y )
{
	if( Button == MB_Left )
	{
		// Left button pressed.
		bLeftPressed	= true;
		bLWasMouse		= false;
		LDwdPos			= TPoint( X, Y );
		LLastPos		= TPoint( X, Y );
	}
	else if( Button == MB_Right )
	{
		// Right button pressed.
		bRightPressed	= true;
		bRWasMouse		= false;
		RDwdPos			= TPoint( X, Y );
		RLastPos		= TPoint( X, Y );
	}
}


//
// User scroll mouse wheel.
//
void WSkeletonPage::OnMouseScroll( Int32 Delta )
{
	if( Delta > 0 )
	{
		// Zoom out.
		while( Delta > 0 )
		{
			SceneView.Zoom	*= 1.05f;
			Delta			-= 120;
		}
	}
	else
	{
		// Zoom in.
		while( Delta < 0 )
		{
			SceneView.Zoom	*= 0.95f;
			Delta			+= 120;
		}
	}

	// Snap & Clamp zoom!
	SceneView.Zoom	= math::round(SceneView.Zoom*50.f)*0.02f;
	SceneView.Zoom	= clamp( SceneView.Zoom, 0.2f, 5.f );
}






//
// User click.
//
void WSkeletonPage::OnMouseClick( EMouseButton Button, Int32 X, Int32 Y )
{
	// Only left button works.
	if( Button == MB_Left )
	{
		// Unselect everything if no combo.
		if( !Root->bAlt && !Root->bCtrl )
			UnselectAll();

		// Pick bone.
		Int32 iBone = GetBoneAt( X, Y );
		ShowBoneProperties(iBone);

		if( iBone != -1 )
		{
			if( Root->bAlt )
				Skeleton->Bones[iBone].Flags &= ~BONE_Selected;
			else
				Skeleton->Bones[iBone].Flags |= BONE_Selected;
		}

		// Update gizmo location.
		UpdateGizmo(iBone);
	}
}


//
// Place and rotate gizmo according to selected skelecton 
// bones.
//
void WSkeletonPage::UpdateGizmo( Int32 iNewSele )
{
	Int32 iSelected = iNewSele;
	if( iSelected == -1 )
		for( Int32 i=0; i<Skeleton->Bones.size(); i++ )
			if( Skeleton->Bones[i].Flags & BONE_Selected )
			{
				iSelected	= i;
				break;
			}

	Gizmo.SetLocation
	( 
		iSelected != -1 ? 
			GetCurrentPose().BonesPose[iSelected].Location : 
			math::Vector(999.f, 999.f) 
	);
}

	


/*static TVector OldWorld;
	Integer iBone = GetBoneAt( X, Y );
	BoneHintFrom = TPoint( X, Y );
	BoneHint = iBone!=-1 ? Skeleton->Bones[iBone].Name : L"";

	// Dragging.
	TVector World = SceneView.Deproject(X, Y);

	if( Button == MB_Left )
	{
		for( Integer i = 0; i < Skeleton->Bones.Num(); i++ )
			if( Skeleton->Bones[i].Flags & BONE_Selected )
				Skeleton->RefPose.BonesPose[i].Location += (World - OldWorld);
	}

	OldWorld = World;*/


//
// User double click on page.
//
void WSkeletonPage::OnDblClick( EMouseButton Button, Int32 X, Int32 Y )
{
	WEditorPage::OnDblClick( Button, X, Y );
}


/*-----------------------------------------------------------------------------
	Top Level mouse functions.
-----------------------------------------------------------------------------*/

//
// Skeleton page drag kind.
//
enum ESkelDragType
{
	SKDR_None,
	SKDR_Observer,
	SKDR_Translate,
	SKDR_Rotate,
	SKDR_MakeBone,
	SKDR_Linking,
	//.........
};


//
// The information about skeleton dragging.
//
struct TSkelDragInfo
{
public:
	ESkelDragType	DragType;

	union
	{
		struct{ int dummy; };
		struct{ Int32 iNewBone; };			// SKDR_MakeBone.
		struct{ Int32 iFromBone; };			// SKDR_Linking.
		// make bone type.
	};

	TSkelDragInfo()
	{}
} DragInfo;



//
// Process mouse dragging.
//
void WSkeletonPage::OnMouseDrag( EMouseButton Button, Int32 X, Int32 Y, Int32 DeltaX, Int32 DeltaY )
{
	// Transform widget delta to scene vector.
	math::Vector Delta;
	math::Vector FOV	= SceneView.FOV;
	Delta.x		= +DeltaX * (FOV.x / Size.Width ) * SceneView.Zoom;
	Delta.y		= -DeltaY * (FOV.y / Size.Height) * SceneView.Zoom;
	Delta		= math::transformVectorBy( Delta, SceneView.UnCoords );

	// Process proper drag.
	switch( DragInfo.DragType )
	{
		case SKDR_Observer:
		{
			// Move scene observer.
			SceneView.Coords.origin	-= Delta;

			SceneView.Coords.origin.x	= clamp( SceneView.Coords.origin.x, -10.f, +10.f );		// PROPER SCALE@@@@@@@
			SceneView.Coords.origin.y	= clamp( SceneView.Coords.origin.y, -10.f, +10.f );

			SceneView.UnCoords		= SceneView.Coords.transpose();
			break;
		}
		case SKDR_Translate:
		{
			// Translate selected bones.
			TSkelPose& Pose = GetCurrentPose();
			for( Int32 i=0; i<Skeleton->Bones.size(); i++ )
				if( Skeleton->Bones[i].Flags & BONE_Selected )
				{
					// Perform translation in parent's space.
					TBoneInfo& Info = Skeleton->Bones[i];
					math::Coords ParentSpace = Info.iPosCtrl != -1 ? 
											math::Coords( Pose.BonesPose[Info.iPosCtrl].Location, Pose.BonesPose[Info.iPosCtrl].Rotation ) : 
											math::Coords::IDENTITY;

					Pose.BonesPose[i].Coords.origin.x	+= ParentSpace.xAxis * Delta;
					Pose.BonesPose[i].Coords.origin.y	+= ParentSpace.yAxis * Delta;
				}

			Pose.ComputeRefTransform( Skeleton );

			// And move gizmo.
			Gizmo.Move(Delta);
			break;
		}
		case SKDR_Rotate:
		{
			// Rotate selected bones.
			math::Angle DeltaRot;
			Gizmo.Perform( SceneView, math::Vector(X, Y), Delta, nullptr, &DeltaRot, nullptr );
			TSkelPose& Pose = GetCurrentPose();
			for( Int32 i=0; i<Skeleton->Bones.size(); i++ )
				if( Skeleton->Bones[i].Flags & BONE_Selected )
				{
					math::Angle OldAngle	= math::vectorToAngle(Pose.BonesPose[i].Coords.xAxis);
					Pose.BonesPose[i].Coords.xAxis	= math::angleToVector(OldAngle + DeltaRot);
					Pose.BonesPose[i].Coords.yAxis	= Pose.BonesPose[i].Coords.xAxis.cross();	
				}		
			Pose.ComputeRefTransform( Skeleton );
			break;
		}
		case SKDR_MakeBone:
		{
			// Create a bone or master.
			assert(DragInfo.iNewBone != -1);
			TBoneInfo& Info	= Skeleton->Bones[DragInfo.iNewBone];
			TBonePose& Pose = Skeleton->RefPose.BonesPose[DragInfo.iNewBone];

			// Update direction and scale.
			math::Vector EndPoint = SceneView.Deproject( X, Y );
			EndPoint.snap(TranslationSnap);
			Info.Scale	= math::distance( Pose.Coords.origin, EndPoint );
			if( Info.Type == SC_Bone )
			{
				Pose.Rotation		= math::vectorToAngle(EndPoint-Pose.Coords.origin);
				Pose.Coords.xAxis	= math::angleToVector(Pose.Rotation);
				Pose.Coords.yAxis	= Pose.Coords.xAxis.cross();
			}
			break;
		}
	}
}


//
// Start mouse drag.
//
void WSkeletonPage::OnMouseBeginDrag( EMouseButton Button, Int32 X, Int32 Y )
{
	// Unhighlight gizmo.
	Gizmo.SetAxis(GIAX_None);

	// Right or middle buttons used for navigation.
	if( Button==MB_Right || Button==MB_Middle )
	{
		DragInfo.DragType	= SKDR_Observer;
		return;
	}

	switch( Tool )
	{
		case SKT_Edit:
		{
			// Translate bones or camera.
			EGizmoAxis GizmoAxis = Gizmo.AxisAt( SceneView, X, Y );
			if( GizmoAxis != GIAX_None )
			{
				Gizmo.SetAxis(GizmoAxis);
				DragInfo.DragType	= SKDR_Rotate;
			}
			else if( GetBoneAt( X, Y ) != -1 )
				DragInfo.DragType	= SKDR_Translate;
			else
				DragInfo.DragType	= SKDR_Observer;
			break;
		}
		case SKT_AddBone:		// check for mouse button!!!!
		case SKT_AddMaster:
		{
			// Add a new bone.
			math::Vector Pivot = SceneView.Deproject( X, Y );
			Pivot.snap(TranslationSnap);

			Skeleton->Bones.push(TBoneInfo
				( 
					Tool==SKT_AddBone ? SC_Bone : SC_Master, 
					MakeUniqueBoneName(Tool==SKT_AddBone ? L"Bone" : L"Master"), 
					TColor(174, 186, 203, 255) 
				));	// constants to define!
			Skeleton->RefPose.BonesPose.push(TBonePose( Pivot, 0 ));
			Skeleton->BuildTransformationTable();

			assert(Skeleton->Bones.size()==Skeleton->RefPose.BonesPose.size());
			DragInfo.iNewBone	= Skeleton->Bones.size() - 1;
			DragInfo.DragType	= SKDR_MakeBone;
			break;
		}
		case SKT_Link:		// not any button, left only, other for observer.
		{
			// Linking.
			Int32 iFrom = GetBoneAt( X, Y );
			if( iFrom != -1 )
			{
				UnselectAll();
				Skeleton->Bones[iFrom].Flags	|= BONE_Selected;
				DragInfo.iFromBone	= iFrom;
				DragInfo.DragType	= SKDR_Linking;
			}
			break;
		}
	}
}



void WSkeletonPage::OnMouseEndDrag( EMouseButton Button, Int32 X, Int32 Y )
{
	switch( DragInfo.DragType )
	{
		case SKDR_MakeBone:
		{
			// Eliminate too short bone.
			assert(DragInfo.iNewBone != -1);
			TBoneInfo& Info	= Skeleton->Bones[DragInfo.iNewBone];
			if( Info.Scale <= 0.1f )		// constant!!!!!
			{
				Skeleton->Bones.removeShift(DragInfo.iNewBone);
				Skeleton->RefPose.BonesPose.removeShift(DragInfo.iNewBone);
				Skeleton->BuildTransformationTable();
				warn( L"Skel: Too short bone eliminated" );
			}

			DragInfo.iNewBone	= -1;
			break;
		}
		case SKDR_Linking:
		{
			Int32 iTo = GetBoneAt( X, Y );
			if( iTo != -1 && iTo != DragInfo.iFromBone )
			{
				// valid linking.
				Skeleton->LinkTo( DragInfo.iFromBone, iTo );
				UnselectAll();
				Skeleton->Bones[iTo].Flags	|= BONE_Selected;
				Skeleton->BuildTransformationTable();				// now it can crash client. hang.
				Skeleton->RefPose.ComputeRefTransform(Skeleton);
			}

			DragInfo.iFromBone	= -1;
			break;
		}
		case SKDR_Translate:
		{
			// Snap selected bones.
			TSkelPose& Pose = GetCurrentPose();
			for( Int32 i=0; i<Skeleton->Bones.size(); i++ )
				if( Skeleton->Bones[i].Flags & BONE_Selected )
				{
					Pose.BonesPose[i].Coords.origin.snap(TranslationSnap);
					AnimationTrack->NoteMovement( i, false );
				}

			Pose.ComputeRefTransform( Skeleton );
			UpdateGizmo();
			break;
		}
		case SKDR_Rotate:
		{
			// Snap selected bones.
			TSkelPose& Pose = GetCurrentPose();
			for( Int32 i=0; i<Skeleton->Bones.size(); i++ )
				if( Skeleton->Bones[i].Flags & BONE_Selected )
				{
					math::Angle Rotation = math::vectorToAngle(Pose.BonesPose[i].Coords.xAxis);
					Rotation.snap(RotationSnap);
					Pose.BonesPose[i].Coords.xAxis	= math::angleToVector(Rotation);
					Pose.BonesPose[i].Coords.yAxis	= Pose.BonesPose[i].Coords.xAxis.cross();
					AnimationTrack->NoteMovement( i, true );
				}		
			Pose.ComputeRefTransform( Skeleton );
			UpdateGizmo();
			break;
		}
	}



	Gizmo.SetAxis(GIAX_None);	// Don't highlight anymore.
	Gizmo.SetRotation(0);
	DragInfo.DragType	= SKDR_None;
}


//
// On mouse move in page.
//
void WSkeletonPage::OnMouseMove( EMouseButton Button, Int32 X, Int32 Y )
{
	// Is moved with hold button.
	if( bLeftPressed )
	{
		// Left button hold.
		if( !bLWasMouse )
			OnMouseBeginDrag( MB_Left, X, Y );

		OnMouseDrag( MB_Left, X, Y, X-LLastPos.X, Y-LLastPos.Y );
		bLWasMouse	= true;
		LLastPos	= TPoint( X, Y );
	}
	if( bRightPressed )
	{
		// Right button hold.
		if( !bRWasMouse )
			OnMouseBeginDrag( MB_Right, X, Y );

		OnMouseDrag( MB_Right, X, Y, X-RLastPos.X, Y-RLastPos.Y );
		bRWasMouse	= true;
		RLastPos	= TPoint( X, Y );
	}

	// Hint bone name below cursor.
	if( DragInfo.DragType != SKDR_Observer && DragInfo.DragType != SKDR_Rotate )
	{
		Int32 iBone	= GetBoneAt( X, Y );
		BoneHint		= iBone != -1 ? Skeleton->Bones[iBone].Name : L"";
		BoneHintFrom	= TPoint( X, Y );
	}
	else
	{
		BoneHint		= L"";
	}

	// Highlight gizmo.
	if( DragInfo.DragType != SKDR_Rotate )
		Gizmo.SetAxis(Gizmo.AxisAt(SceneView, X, Y));
}


#define LINK_POS_COLOR	TColor( 181, 201, 237, 255 )
#define LINK_ROT_COLOR	TColor( 223, 153, 153, 255 )
#define LINK_BOTH_COLOR	TColor( 152, 203, 102, 255 )



void WSkeletonPage::RenderPageContent( CCanvas* Canvas )
{
	TPoint P = ClientToWindow(TPoint::Zero);

	// Not really good place to compute!!!!!!!!!!!

	SceneView.FOV		= math::Vector( 20, Size.Height * 20.f / Size.Width );
	// ignore zoom!
	SceneView.bMirage	= false;
	SceneView.Bounds	= math::Rect( SceneView.Coords.origin, 
		SceneView.FOV.x * SceneView.Zoom, SceneView.FOV.y * SceneView.Zoom );

	SceneView.X	= P.X;
	SceneView.Y = P.Y;
	SceneView.Width = Size.Width;
	SceneView.Height = Size.Height;


	Canvas->PushTransform(SceneView);
	{
		// Render background and grid.
		TRenderRect Background;
		Background.Texture		= nullptr;
		Background.Bounds		= math::Rect( math::Vector(0.f, 0.f), SKEL_SCENE_SIZE );
		Background.Color		= SKEL_SCENE_BG_COLOR;
		Background.Flags		= POLY_FlatShade;
		Background.Rotation		= 0;
		Canvas->DrawRect(Background);

		Int32 CMinX = math::trunc(max<Float>( Canvas->View.Bounds.min.x, -SKEL_SCENE_SIZE_HALF ))*2;
		Int32 CMinY = math::trunc(max<Float>( Canvas->View.Bounds.min.y, -SKEL_SCENE_SIZE_HALF ))*2;
		Int32 CMaxX = math::trunc(min<Float>( Canvas->View.Bounds.max.x, +SKEL_SCENE_SIZE_HALF ))*2;
		Int32 CMaxY = math::trunc(min<Float>( Canvas->View.Bounds.max.y, +SKEL_SCENE_SIZE_HALF ))*2;

		for( Int32 i=CMinX; i<=CMaxX; i++ )
		{
			math::Vector V1( i/2.f, -SKEL_SCENE_SIZE_HALF );
			math::Vector V2( i/2.f, +SKEL_SCENE_SIZE_HALF );
			Canvas->DrawLine( V1, V2, SKEL_SCENE_GRID_COLOR, i & 1 );
		}
		for( Int32 i=CMinY; i<=CMaxY; i++ )
		{
			math::Vector V1( -SKEL_SCENE_SIZE_HALF, i/2.f );
			math::Vector V2( +SKEL_SCENE_SIZE_HALF, i/2.f );
			Canvas->DrawLine( V1, V2, SKEL_SCENE_GRID_COLOR, i & 1 );
		}




		// Draw centroid.
		TColor CentroidColor = AnimationTrack->IsRecording() ? SKEL_SCENE_RECORD_CENTER_COLOR : SKEL_SCENE_CENTER_COLOR;
		Canvas->DrawLine( math::Vector(0.f, -SKEL_SCENE_SIZE_HALF), math::Vector(0.f, +SKEL_SCENE_SIZE_HALF), CentroidColor, false );
		Canvas->DrawLine( math::Vector(-SKEL_SCENE_SIZE_HALF, 0.f), math::Vector(+SKEL_SCENE_SIZE_HALF, 0.f), CentroidColor, false );
		Canvas->DrawLineRect( math::Vector(0.f, 0.f), math::Vector(SKEL_SCENE_SIZE, SKEL_SCENE_SIZE), 0, CentroidColor, false );





/*

	// Compute bounds.


	// Pick colors.
	TColor GridColor0 = TColor( 0x40, 0x40, 0x40, 0xff );
	TColor GridColor1 = TColor( 0x80, 0x80, 0x80, 0xff );
	
	// Vertical lines.


	// Horizontal lines.
	for( Integer i=CMinY; i<=CMaxY; i++ )
	{
		TVector V1( -WORLD_HALF, i );
		TVector V2( +WORLD_HALF, i );
	
		if( !(i & 7) )
			Canvas->DrawLine( V1, V2, GridColor1, false );
		else if( !(i & 3) )
			Canvas->DrawLine( V1, V2, GridColor0, false );
		else
			Canvas->DrawLine( V1, V2, GridColor0, true );
	}
*/

		Skeleton->Render( Canvas, math::Vector(0,0), math::Vector(1, 1), Skeleton->RefPose );

		Canvas->DrawLineStar( math::Vector(5, 5), (Float)GPlat->Now()*65536.f, 3, COLOR_Gold, true );


		// Draw a links if need.
		if( DrawLinksButton->bDown )
		{
			for( Int32 i=0; i<Skeleton->Bones.size(); i++ )
			{
				TBoneInfo& Info = Skeleton->Bones[i];
				TBonePose& Pose = GetCurrentPose().BonesPose[i];

				if( Info.iPosCtrl == Info.iRotCtrl && Info.iPosCtrl != -1 )
				{
					// Both link.
					DrawLink( Canvas, i, Info.iPosCtrl, LINK_BOTH_COLOR );
				}
				else
				{
					if( Info.iPosCtrl != -1 )
						DrawLink( Canvas, i, Info.iPosCtrl, LINK_POS_COLOR );
					if( Info.iRotCtrl != -1 )
						DrawLink( Canvas, i, Info.iRotCtrl, LINK_ROT_COLOR );
				}
			}
		}

		// Temporary linking line.
		if( DragInfo.DragType == SKDR_Linking && DragInfo.iFromBone != -1 )
		{
			Canvas->DrawLine
			(
				SceneView.Deproject( LLastPos.X, LLastPos.Y ),
				GetBoneCenter(DragInfo.iFromBone),
				COLOR_White,
				false
			);
		}

		// Gizmo.
		Gizmo.Render(Canvas);
	}
	Canvas->PopTransform();
}


//
// GUI Page paint.
//
void WSkeletonPage::OnPaint( CGUIRenderBase* Render )
{
	//	Draw info and zoom here!!!!
	// FKNFKNKGENGERNGKRGNLKNRNGKENGKLGNERKLNG



	// Hint bone name.
	if( BoneHint )
	{
		TPoint P = ClientToWindow( TPoint::Zero );

		TSize HintSize = TSize( WWindow::Font1->TextWidth(*BoneHint), WWindow::Font1->Height );
		TPoint DrawPos( BoneHintFrom.X+16, BoneHintFrom.Y+8 );

		// Avoid out of window.
		if( DrawPos.X+HintSize.Width+10 > Size.Width )		DrawPos.X	-= HintSize.Width + 32;
		if( DrawPos.Y+HintSize.Height+5 > Size.Height )		DrawPos.Y	-= HintSize.Height + 16;
		DrawPos	+= P;

		Render->DrawRegion
		(
			DrawPos, 
			TSize( HintSize.Width+8, HintSize.Height+2 ),
			GUI_COLOR_TOOLTIP,
			GUI_COLOR_TOOLTIP,
			BPAT_Solid
		);
		Render->DrawText
		(	
			TPoint( DrawPos.X+4, DrawPos.Y+1 ),
			BoneHint,
			GUI_COLOR_TEXT,
			WWindow::Font1
		);
	}
}


/*-----------------------------------------------------------------------------
	WAnimationTrack implementation.
-----------------------------------------------------------------------------*/

//
// Animation track constructor.
//
WAnimationTrack::WAnimationTrack( WSkeletonPage* InPage, WWindow* InRoot )
	:	WContainer( InPage, InRoot ),
		Page( InPage ),
		Skeleton( InPage->Skeleton ),
		CurrentFrame( 0 )
{
	// Initialize own fields.
	SetSize( 300, 200 );
	Padding	= TArea( FORM_HEADER_SIZE, 0, 0, 0 );
	Caption	= L"Animation Track";

	// Create toolbar and buttons.
	ToolBar = new WToolBar( this, Root );
	ToolBar->SetSize( 3000, 28 );

	AddActionButton					= new WButton( ToolBar, Root );
	AddActionButton->Caption		= L"+";
	AddActionButton->Tooltip		= L"Add New Action...";
	AddActionButton->EventClick		= WIDGET_EVENT(WAnimationTrack::ButtonAddActionClick);	
	AddActionButton->SetSize( 25, 22 );
	ToolBar->AddElement( AddActionButton );

	DeleteActionButton					= new WButton( ToolBar, Root );
	DeleteActionButton->Caption			= L"-";
	DeleteActionButton->Tooltip			= L"Delete Selected Action...";
	DeleteActionButton->EventClick		= WIDGET_EVENT(WAnimationTrack::ButtonDeleteActionClick);	
	DeleteActionButton->SetSize( 25, 22 );
	ToolBar->AddElement( DeleteActionButton );


	ActionsList					= new WComboBox( ToolBar, Root );
	ActionsList->Tooltip		= L"Skeleton Actions";
	//ActionsList->EventChange	= WIDGET_EVENT(WLevelPage::ComboDragSnapChange);
	ActionsList->SetSize( 128, 22 );
	ToolBar->AddElement( ActionsList );
	ToolBar->AddElement( nullptr );

	RecordButton					= new WButton( ToolBar, Root );
	RecordButton->Caption			= L"Record";
	RecordButton->Tooltip			= L"Turn on Record Mode";
	RecordButton->bToggle			= true;
	RecordButton->bDown				= false;
	RecordButton->SetSize( 75, 22 );
	ToolBar->AddElement( RecordButton );

	PlayButton					= new WButton( ToolBar, Root );
	PlayButton->Caption			= L"Play";
	PlayButton->Tooltip			= L"Play Animation";
	PlayButton->bDown			= false;
	PlayButton->EventClick		= WIDGET_EVENT(WAnimationTrack::ButtonPlayClick);	
	PlayButton->SetSize( 75, 22 );
	ToolBar->AddElement( PlayButton );

/*

*/

	// Hacky.
	ActionsList->Empty();
	for( Int32 i=0; i<Skeleton->Actions.size(); i++ )
		ActionsList->AddItem( Skeleton->Actions[i].Name, (void*)i );

	ActionsList->ItemIndex = ActionsList->Items.size()-1;

}

WAnimationTrack::~WAnimationTrack(){}




/*
//
// Skeleton page animation track.
//
class WAnimationTrack:: public WContainer
{
public:
	// WAnimationTrack interface.


	friend WSkeletonPage;

protected:
	// Internal widgets.
	WToolBar*			ToolBar;

};
*/

void WAnimationTrack::OnPaint( CGUIRenderBase* Render )
{
	TPoint Base = ClientToWindow(TPoint::Zero);
	Render->SetClipArea( Base, Size );

	// Draw bg.
	Render->DrawRegion
	(
		Base,
		Size,
		TColor( 0x45, 0x45, 0x45, 0xff ),
		GUI_COLOR_FORM_BORDER,
		BPAT_Solid
	);
	
	// Draw header.
	Render->DrawRegion
	(
		Base, 
		TSize( Size.Width, FORM_HEADER_SIZE ),
		TColor( 0x33, 0x33, 0x33, 0xff ),
		GUI_COLOR_FORM_BORDER,
		BPAT_Diagonal
	);
	Render->DrawText
	( 
		TPoint( Base.X + 5, Base.Y+(FORM_HEADER_SIZE-Root->Font1->Height)/2 ), 
		Caption, 
		GUI_COLOR_TEXT, 
		Root->Font1 
	);

	// Draw track names panel.
	Render->DrawRegion
	(
		TPoint( Base.X+1, Base.Y+48 ),
		TSize( 128-1, Size.Height-49 ),
		TColor( 0x22, 0x22, 0x22, 0xff ),
		TColor( 0x22, 0x22, 0x22, 0xff ),
		BPAT_Solid
	);

	// Draw current frame marker.
	Render->DrawRegion
	(
		TPoint( Base.X+128 + CurrentFrame*256/FRAMES_PER_SECOND, Base.Y + 48 ),
		TSize( 1, Size.Height-49 ),
		COLOR_Yellow,
		COLOR_Yellow,
		BPAT_Solid
	);


	// Draw tracks.
	if( ActionsList->ItemIndex != -1 )
	{
		TSkeletonAction& Action = Skeleton->Actions[ActionsList->ItemIndex];

		// Foreach track.
		for( Int32 i=0; i<Action.BoneTracks.size(); i++ )
		{
			TBoneTrack& Track = Action.BoneTracks[i];
			String TrackName = Skeleton->Bones[Track.iBone].Name;

			// Track name.
			Int32 TrackY = Base.Y + 64+i*13;
			Render->DrawText
			(
				TPoint( Base.X+8, TrackY ),
				TrackName,
				GUI_COLOR_TEXT,
				Root->Font1
			);

			// Track line.
			Int32 TrackX1 =  12 + Root->Font1->TextWidth(*TrackName);
			Render->DrawRegion
			(
				TPoint( Base.X + TrackX1, TrackY+8 ),
				TSize( Size.Width-TrackX1-1, 2 ),
				TColor( 0x58, 0x58, 0x58, 0xff ),
				TColor( 0x58, 0x58, 0x58, 0xff ),
				BPAT_Solid
			);

			// Draw keys.
			///////////////////////////////////////////////////////

			for( Int32 i=0; i<Track.PosKeys.Samples.size(); i++ )
			{
				Render->DrawRegion
				(
					TPoint( Base.X - 3 + 128 + Track.PosKeys.Samples[i].Input * 256.f, TrackY+6 ),		// 1 sec = 256 pxs.
					TSize( 6, 6 ),
					COLOR_Blue,
					COLOR_Blue,
					BPAT_Solid
				);
			}

			for( Int32 i=0; i<Track.RotKeys.Samples.size(); i++ )
			{
				Render->DrawRegion
				(
					TPoint( Base.X - 3 + 128 + Track.RotKeys.Samples[i].Input * 256.f, TrackY+6 ),		// 1 sec = 256 pxs.
					TSize( 6, 6 ),
					COLOR_Red,
					COLOR_Red,
					BPAT_Solid
				);
			}


		}
	}
}



void WAnimationTrack::OnMouseDown( EMouseButton Button, Int32 X, Int32 Y )
{
	WContainer::OnMouseDown( Button, X, Y );

	CurrentFrame = (X-128) * FRAMES_PER_SECOND / 256;

	if( CurrentFrame < 0 )
		CurrentFrame = 0;

}



void WAnimationTrack::ButtonAddActionClick( WWidget* Sender )
{
	static Int32 gCounter = 0;
	Skeleton->Actions.push(TSkeletonAction(String::format(L"Action_%d", gCounter)));
	gCounter++;

	ActionsList->Empty();
	for( Int32 i=0; i<Skeleton->Actions.size(); i++ )
		ActionsList->AddItem( Skeleton->Actions[i].Name, (void*)i );

	ActionsList->ItemIndex = ActionsList->Items.size()-1;



	// Note movement!!!   called from page to make key or edit!!
}
void WAnimationTrack::ButtonDeleteActionClick( WWidget* Sender )
{
}

// We've got a message, that bone have moved or rotated.
// It's place to add keys and so on.
void WAnimationTrack::NoteMovement( Int32 iBone, Bool bOnlyRotation )
{
	assert(iBone != -1);

	// Don't add, if no action selected.
	if( ActionsList->ItemIndex == -1 )
		return;
	
	TSkeletonAction& Action = Skeleton->Actions[ActionsList->ItemIndex];

	// Add key only in recording mode.
	if( !IsRecording() )
		return;

	// Find appropriate bone track or create new one.
	Int32 iTrack = -1;
	for( Int32 i=0; i<Action.BoneTracks.size(); i++ )
		if( Action.BoneTracks[i].iBone == iBone )
		{
			iTrack = i;
			break;
		}

	// Track not found, so create new one.
	if( iTrack == -1 )
	{
		iTrack = Action.BoneTracks.push(TBoneTrack(iBone));
	}

	TBoneTrack& Track = Action.BoneTracks[iTrack];
	TBoneInfo& BoneInfo = Skeleton->Bones[iBone];
	TBonePose& BonePose = Page->GetCurrentPose().BonesPose[iBone];
	Float CurrentTime	= (Float)CurrentFrame / (Float)FRAMES_PER_SECOND;

	// Add a random key.
	if( bOnlyRotation )
	{
		// Rotation key.
		Track.RotKeys.AddSample( CurrentTime, math::vectorToAngle(BonePose.Coords.xAxis) );
	}
	else
	{
		// Translation key.
		Track.PosKeys.AddSample( CurrentTime, BonePose.Coords.origin );
	}




	/*

	for( Integer i=0; i<Acti.BoneTracks.Num(); i++ )
		if( Acti.BoneTracks[i].iBone == iBone )
		{
			// Already in list of bones.
			return;
		}*/

	// Add new track.


	/*
	// Tmp, fill with masters!!!

	for( Integer i=0; i<Skeleton->Bones.Num(); i++ )
		if( Skeleton->Bones[i].Type == SC_Master )
			*/
}


void WAnimationTrack::Tick( Float Delta )
{
	// Play current animation.
	if( IsPlaying() )
	{
		PlayTime	+= Delta;
		if( PlayTime > 1.f )	// Loop.
			PlayTime = 0.f;

		CurrentFrame = PlayTime * FRAMES_PER_SECOND;

		Skeleton->RefPose.CumputeAnimFrame( Skeleton, Skeleton->Actions[ActionsList->ItemIndex], PlayTime );
	}
}



void WAnimationTrack::ButtonPlayClick( WWidget* Sender )
{
	// Toggle play button.
	PlayButton->bDown ^= 1;

	if( IsPlaying() )
	{
		// Rewind.
		CurrentFrame	= 0;
		PlayTime		= 0.f;
	}
	else
	{

	}
}

////////////////////////////////////////////////////////////////////////////////////////

//
// Return true, if tracker in record mode.
//
Bool WAnimationTrack::IsRecording() const
{
	return RecordButton->bDown;
}


//
// Return true, if animation tracker play animation.
//
Bool WAnimationTrack::IsPlaying() const
{
	return PlayButton->bDown;
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/