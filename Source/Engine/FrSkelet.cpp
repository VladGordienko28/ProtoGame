/*=============================================================================
	FrSkelet.cpp: Skeleton implementation.
	Created by Vlad Gordienko, Dec. 2017.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
	FSkeleton implementation.
-----------------------------------------------------------------------------*/


	FSkeleton::FSkeleton(){}
	FSkeleton::~FSkeleton(){}
	


	void FSkeleton::PostLoad()
	{
		FResource::PostLoad();
		// Is required?
		BuildTransformationTable();
		RefPose.ComputeRefTransform(this);
	}


///////////////////////////////////////////////////////////////

//
// Import skeleton.
//
void FSkeleton::Import( CImporterBase& Im )
{
	FResource::Import( Im );

	// List of bones.
	Bones.SetNum(Im.ImportInteger(L"Bones.Num"));
	for( Int32 i=0; i<Bones.Num(); i++ )
	{
		TBoneInfo& B = Bones[i];
		B.Type = (ESkelCntrl)Im.ImportByte(*String::Format(L"Bones[%i].Type", i));
		B.Name = Im.ImportString(*String::Format(L"Bones[%i].Name", i));
		B.Color = Im.ImportColor(*String::Format(L"Bones[%i].Color", i));
		B.Flags = Im.ImportInteger(*String::Format(L"Bones[%i].Flags", i));
		B.iPosCtrl = Im.ImportInteger(*String::Format(L"Bones[%i].iPosCtrl", i));
		B.iRotCtrl = Im.ImportInteger(*String::Format(L"Bones[%i].iRotCtrl", i));
		B.Scale = Im.ImportFloat(*String::Format(L"Bones[%i].Scale", i));

		if( B.Type == SC_Bone )
		{
			B.bLookAt = Im.ImportBool(*String::Format(L"Bones[%i].bLookAt", i));
		}
		if( B.Type == SC_IKSolver )
		{
			B.bFlipIK = Im.ImportBool(*String::Format(L"Bones[%i].bFlipIK", i));
			B.iEndJoint = Im.ImportInteger(*String::Format(L"Bones[%i].iEndJoint", i));
		}
	}

	// RefPose.
	RefPose.BonesPose.SetNum(Bones.Num());
	for( Int32 i=0; i<RefPose.BonesPose.Num(); i++ )
	{
		TBonePose& P = RefPose.BonesPose[i];
		P.Coords.Origin = Im.ImportVector(*String::Format(L"RefPose.BonesPose[%i].Location", i));
		
		TAngle Rotation = Im.ImportAngle(*String::Format(L"RefPose.BonesPose[%i].Rotation", i));
		P.Coords.XAxis = AngleToVector(Rotation);
		P.Coords.YAxis = P.Coords.XAxis.Cross();
	}

	// Actions.
	Actions.SetNum(Im.ImportInteger(L"Actions.Num"));
	for( Int32 i=0; i<Actions.Num(); i++ )
	{
		TSkeletonAction& A = Actions[i];

		A.Name = Im.ImportString(*String::Format(L"Actions[%i].Name", i));
		A.BoneTracks.SetNum(Im.ImportInteger(*String::Format(L"Actions[%i].BoneTracks.Num", i)));
		for( Int32 j=0; j<A.BoneTracks.Num(); j++ )
		{
			TBoneTrack& T = A.BoneTracks[j];
			T.iBone = Im.ImportInteger(*String::Format(L"Actions[%i].BoneTracks[%i].iBone", i, j));
			T.PosKeys.Samples.SetNum(Im.ImportInteger(*String::Format(L"Actions[%i].BoneTracks[%i].PosKeys.Num", i, j)));
			T.RotKeys.Samples.SetNum(Im.ImportInteger(*String::Format(L"Actions[%i].BoneTracks[%i].RotKeys.Num", i, j)));

			for( Int32 k=0; k<T.PosKeys.Samples.Num(); k++ )
			{
				T.PosKeys.Samples[k].Type = (EInterpType)Im.ImportByte(*String::Format(L"Actions[%i].BoneTracks[%i].PosKeys[%i].Type", i, j, k));
				T.PosKeys.Samples[k].Input = Im.ImportFloat(*String::Format(L"Actions[%i].BoneTracks[%i].PosKeys[%i].Input", i, j, k));
				T.PosKeys.Samples[k].Output = Im.ImportVector(*String::Format(L"Actions[%i].BoneTracks[%i].PosKeys[%i].Output", i, j, k));
			}

			for( Int32 k=0; k<T.RotKeys.Samples.Num(); k++ )
			{
				T.RotKeys.Samples[k].Type = (EInterpType)Im.ImportByte(*String::Format(L"Actions[%i].BoneTracks[%i].RotKeys[%i].Type", i, j, k));
				T.RotKeys.Samples[k].Input = Im.ImportFloat(*String::Format(L"Actions[%i].BoneTracks[%i].RotKeys[%i].Input", i, j, k));
				T.RotKeys.Samples[k].Output = Im.ImportAngle(*String::Format(L"Actions[%i].BoneTracks[%i].RotKeys[%i].Output", i, j, k));
			}
		}
	}
}


//
// Export skeleton.
//
void FSkeleton::Export( CExporterBase& Ex )
{
	FResource::Export( Ex );

	// List of bones.
	Ex.ExportInteger( L"Bones.Num", Bones.Num() );
	for( Int32 i=0; i<Bones.Num(); i++ )
	{
		TBoneInfo& B = Bones[i];
		Ex.ExportByte( *String::Format(L"Bones[%i].Type", i), B.Type );
		Ex.ExportString( *String::Format(L"Bones[%i].Name", i), B.Name );
		Ex.ExportColor( *String::Format(L"Bones[%i].Color", i), B.Color );
		Ex.ExportInteger( *String::Format(L"Bones[%i].Flags", i), B.Flags );
		Ex.ExportInteger( *String::Format(L"Bones[%i].iPosCtrl", i), B.iPosCtrl );
		Ex.ExportInteger( *String::Format(L"Bones[%i].iRotCtrl", i), B.iRotCtrl );
		Ex.ExportFloat( *String::Format(L"Bones[%i].Scale", i), B.Scale );

		if( B.Type == SC_Bone )
		{
			Ex.ExportBool( *String::Format(L"Bones[%i].bLookAt", i), B.bLookAt );
		}
		if( B.Type == SC_IKSolver )
		{
			Ex.ExportBool( *String::Format(L"Bones[%i].bFlipIK", i), B.bFlipIK );
			Ex.ExportInteger( *String::Format(L"Bones[%i].iEndJoint", i), B.iEndJoint );
		}
	}

	// RefPose.
	for( Int32 i=0; i<RefPose.BonesPose.Num(); i++ )
	{
		TBonePose& P = RefPose.BonesPose[i];
		Ex.ExportVector( *String::Format(L"RefPose.BonesPose[%i].Location", i), P.Coords.Origin );
		Ex.ExportAngle( *String::Format(L"RefPose.BonesPose[%i].Rotation", i), VectorToAngle(P.Coords.XAxis) );
	}

	// Actions.
	Ex.ExportInteger( L"Actions.Num", Actions.Num() );
	for( Int32 i=0; i<Actions.Num(); i++ )
	{
		TSkeletonAction& A = Actions[i];

		Ex.ExportString( *String::Format(L"Actions[%i].Name", i), A.Name );
		Ex.ExportInteger( *String::Format(L"Actions[%i].BoneTracks.Num", i), A.BoneTracks.Num() );
		for( Int32 j=0; j<A.BoneTracks.Num(); j++ )
		{
			TBoneTrack& T = A.BoneTracks[j];
			Ex.ExportInteger( *String::Format(L"Actions[%i].BoneTracks[%i].iBone", i, j), T.iBone );
			Ex.ExportInteger( *String::Format(L"Actions[%i].BoneTracks[%i].PosKeys.Num", i, j), T.PosKeys.Samples.Num() );
			Ex.ExportInteger( *String::Format(L"Actions[%i].BoneTracks[%i].RotKeys.Num", i, j), T.RotKeys.Samples.Num() );

			for( Int32 k=0; k<T.PosKeys.Samples.Num(); k++ )
			{
				Ex.ExportByte( *String::Format(L"Actions[%i].BoneTracks[%i].PosKeys[%i].Type", i, j, k), T.PosKeys.Samples[k].Type );
				Ex.ExportFloat( *String::Format(L"Actions[%i].BoneTracks[%i].PosKeys[%i].Input", i, j, k), T.PosKeys.Samples[k].Input );
				Ex.ExportVector( *String::Format(L"Actions[%i].BoneTracks[%i].PosKeys[%i].Output", i, j, k), T.PosKeys.Samples[k].Output );
			}

			for( Int32 k=0; k<T.RotKeys.Samples.Num(); k++ )
			{
				Ex.ExportByte( *String::Format(L"Actions[%i].BoneTracks[%i].RotKeys[%i].Type", i, j, k), T.RotKeys.Samples[k].Type );
				Ex.ExportFloat( *String::Format(L"Actions[%i].BoneTracks[%i].RotKeys[%i].Input", i, j, k), T.RotKeys.Samples[k].Input );
				Ex.ExportAngle( *String::Format(L"Actions[%i].BoneTracks[%i].RotKeys[%i].Output", i, j, k), T.RotKeys.Samples[k].Output );
			}
		}
	}
}


//
// Skeleton serialization.
//
void FSkeleton::SerializeThis( CSerializer& S )
{
	FResource::SerializeThis(S);

	Serialize( S, Bones );
	Serialize( S, RefPose );
	Serialize( S, Actions );

	// Warning: All tables initialized in PostLoad.
}


//
// Find skeleton bone.
//
TBoneInfo* FSkeleton::FindBone( String InName )
{
	for( Int32 i=0; i<Bones.Num(); i++ )
		if( String::CompareText( InName, Bones[i].Name ) == 0 )
			return &Bones[i];
	return nullptr;
}


//
// Find skeleton action.
//
Int32 FSkeleton::FindAction( String InName )
{
	for( Int32 i=0; i<Actions.Num(); i++ )
		if( String::CompareText( InName, Actions[i].Name ) == 0 )
			return i;
	return -1;
}


/*-----------------------------------------------------------------------------
	FSkeleton rendering.
-----------------------------------------------------------------------------*/

//
// Draw a wire bone.
//
static inline void DrawBone( CCanvas* Canvas, const TVector& Origin, TAngle Rotation, Float Size, Float Length, TColor Color )
{
	Float	Size2 = Size * 0.5f;
	TCoords BoneLocal( Origin, Rotation );
	TVector	BoneTip = Origin + BoneLocal.XAxis * Length;
	TVector	Wing1	= Origin + (BoneLocal.XAxis + BoneLocal.YAxis) * Size2;
	TVector	Wing2	= Origin + (BoneLocal.XAxis - BoneLocal.YAxis) * Size2;

	// Draw bone.
	TRenderPoly P;
	P.Texture		= nullptr;
	P.Color			= Color * 0.3f;
	P.Flags			= POLY_FlatShade | POLY_Ghost;
	P.NumVerts		= 4;
	P.Vertices[0]	= Origin;
	P.Vertices[1]	= Wing1;
	P.Vertices[2]	= BoneTip;
	P.Vertices[3]	= Wing2;
	Canvas->DrawPoly( P );

	// Draw wire.
	Canvas->DrawLine( Origin,	BoneTip,	Color, false );
	Canvas->DrawLine( Wing1,	Wing2,		Color, false );
	Canvas->DrawLine( Origin,	Wing1,		Color, false );
	Canvas->DrawLine( Origin,	Wing2,		Color, false );
	Canvas->DrawLine( Wing1,	BoneTip,	Color, false );
	Canvas->DrawLine( Wing2,	BoneTip,	Color, false );
}


//
// Draw a wire master control.
//
static inline void DrawMaster( CCanvas* Canvas, const TVector& Origin, TAngle Rotation, Float Size, TColor Color )
{
	TRenderPoly P;
	P.Texture		= nullptr;
	P.Color			= Color * 0.3f;
	P.Flags			= POLY_FlatShade | POLY_Ghost;
	P.NumVerts		= 16;
	
	TAngle Walk = Rotation;
	for( Int32 i=0; i<16; i++ )
	{
		P.Vertices[i] = Origin + TVector(Walk.GetCos(), Walk.GetSin()) * Size;
		Walk += 65536/16;
	}

	Canvas->DrawPoly(P);

	TVector Prev = P.Vertices[15];
	for( Int32 i=0; i<16; i++ )
	{
		TVector This = P.Vertices[i];

		Canvas->DrawLine( Prev, This, Color, false );

		Prev = This;
	}
}


//
// Render skeleton.
//
void FSkeleton::Render
( 
	CCanvas* Canvas, 
	const TVector& Origin, 
	const TVector& Scale, 
	const TSkelPose& Pose 
)
{
	assert(Pose.BonesPose.Num() == Bones.Num());
	for( Int32 i=0; i<Bones.Num(); i++ )
	{
		TBoneInfo& Info = Bones[i];
		const TBonePose& P = Pose.BonesPose[i];

		TColor RenderColor = (Info.Flags & BONE_Selected) ? Info.Color*2.f : Info.Color;

		switch(Info.Type)
		{
			case SC_Bone:
				DrawBone( Canvas, P.Location, P.Rotation, 0.2f, Info.Scale, RenderColor );
				break;

			case SC_Master:
				//Canvas->DrawCircle( P.Location, Info.Scale, RenderColor, false, 16 );
				DrawMaster( Canvas, P.Location, P.Rotation, Info.Scale, RenderColor );
				break;
				// ngon + fulfill.

			case SC_IKSolver:
				Canvas->DrawLineStar( P.Location, P.Rotation, Info.Scale, RenderColor, false );
				break;
		}



		/*
		TVector End = P.Location + AngleToVector(P.Rotation)*Info.Length;

		Canvas->DrawLine( P.Location, End, Info.Color, false );
		Canvas->DrawPoint( P.Location, 5.f, Info.Color );*/

	}
}


/*-----------------------------------------------------------------------------
	TSkelPose implementation.
-----------------------------------------------------------------------------*/

//
// Skeleton Pose constructor.
//
TSkelPose::TSkelPose()
{}


//
// Hacky table of Matrices being to each bone. This is not only
// table of RefMarices but also animation computed.
//
static TArray<TCoords> GBones;


//
// Compute all transformations for ref-pose.
//
void TSkelPose::ComputeRefTransform( FSkeleton* Skel )
{
	// Make sure hack table size fits skeleton.
	if( GBones.Num() < Skel->Bones.Num() )
		GBones.SetNum(Skel->Bones.Num());

	// Just copy bone-by-bone.
	for( Int32 i=0; i<BonesPose.Num(); i++ )
		GBones[i] = BonesPose[i].Coords;

	// Solve equations.
	SolveSkeleton(Skel);
}


//
// Compute all transform for animation.
//
void TSkelPose::CumputeAnimFrame( FSkeleton* Skel, TSkeletonAction& Action, Float Time )
{
	// Make sure hack table size fits skeleton.
	if( GBones.Num() < Skel->Bones.Num() )
		GBones.SetNum(Skel->Bones.Num());

	// Just copy bone-by-bone.
	for( Int32 i=0; i<BonesPose.Num(); i++ )
		GBones[i] = BonesPose[i].Coords;

	// Apply animation transforms.
	for( Int32 i=0; i<Action.BoneTracks.Num(); i++ )
	{
		TBoneTrack& Track = Action.BoneTracks[i];
		GBones[Track.iBone].Origin = Track.PosKeys.SampleAt( Time, GBones[Track.iBone].Origin );
		
		TAngle OldRotation = VectorToAngle(GBones[Track.iBone].XAxis); 
		TAngle NewRotation = Track.RotKeys.SampleAt( Time, OldRotation );
		if( OldRotation != NewRotation )
		{
			GBones[Track.iBone].XAxis = AngleToVector(NewRotation);
			GBones[Track.iBone].YAxis = GBones[Track.iBone].XAxis.Cross();
		}
	}

	// Solve equations.
	SolveSkeleton(Skel);
}


//
// Solve all transformation for current pose. It's pretty
// slow function, so call only when something changed.
//
void TSkelPose::SolveSkeleton( FSkeleton* Skel )
{
	assert(Skel);
	assert(Skel->Bones.Num() == BonesPose.Num());
	assert(Skel->TransformTable.Num() == Skel->Bones.Num());
	assert(GBones.Num() >= Skel->Bones.Num());

	for( Int32 iBone=0; iBone<Skel->Bones.Num(); iBone++ )
	{
		TBoneInfo& ThisInfo = Skel->Bones[Skel->TransformTable[iBone]];
		TBonePose& BoneResult = BonesPose[Skel->TransformTable[iBone]];
		TCoords&   BoneCoords = GBones[Skel->TransformTable[iBone]];	

		// Compute position of current bone.
		if( ThisInfo.iPosCtrl != -1 )
		{
			// Compute according to parent 
			TBonePose& ParentResult = BonesPose[ThisInfo.iPosCtrl];
			TCoords FromParentSpace = TCoords( ParentResult.Location, ParentResult.Rotation ).Transpose();
			BoneResult.Location	= TransformPointBy( BoneCoords.Origin, FromParentSpace );
		}
		else
		{
			// No position controller, so use this transform.
			BoneResult.Location	= BoneCoords.Origin;
		}

		// Compute rotation of current bone.
		if( ThisInfo.iRotCtrl != -1 )
		{
			TBoneInfo& ParentInfo	= Skel->Bones[ThisInfo.iRotCtrl];
			TBonePose& ParentResult	= BonesPose[ThisInfo.iRotCtrl];

			if( ParentInfo.Type == SC_IKSolver )
			{
				// Solve IK equation.
				if( ParentInfo.iEndJoint != -1 )
				{
					TBoneInfo& Chain2Info = Skel->Bones[ParentInfo.iEndJoint];
					if( Chain2Info.iPosCtrl != -1 )
					{
						TBonePose& Chain2 = BonesPose[ParentInfo.iEndJoint];
						TBonePose& Chain1 = BonesPose[Chain2Info.iPosCtrl];
						TBoneInfo& Chain1Info = Skel->Bones[Chain2Info.iPosCtrl];

						TVector Dir = ParentResult.Location - Chain1.Location;
						if( Dir.Size() < (Chain1Info.Scale + Chain2Info.Scale) )
						{
							Float Cos2 = (Sqr(Dir.X)+Sqr(Dir.Y)-Sqr(Chain1Info.Scale)-Sqr(Chain2Info.Scale))/(2.f*Chain1Info.Scale*Chain2Info.Scale);
							TAngle Angle2 = TAngle(acosf(Cos2));

							if( ParentInfo.bFlipIK )
								Angle2 = -Angle2;

							Float Sin2 = Angle2.GetSin();

							Float P3 = ArcTan2( Dir.Y, Dir.X );
							Float P4 = ArcTan((Chain2Info.Scale*Sin2)/(Chain1Info.Scale+Chain2Info.Scale*Cos2));
							TAngle Angle1 = TAngle(P3-P4);

							BoneResult.Rotation = &ThisInfo == &Chain2Info ? Angle1+Angle2 : Angle1;
						}
						else
						{
							// We cant compute IK.
							BoneResult.Rotation	= VectorToAngle(Dir);
						}
					}
					else
					{
						// IK chain broken, so no computations.
						BoneResult.Rotation	= VectorToAngle(BoneCoords.XAxis);
					}
				}
				else
				{
					// IK broken, so no computations.
					BoneResult.Rotation	= VectorToAngle(BoneCoords.XAxis);
				}
			}
			else
			{
				// Simple rotation.
				if( ThisInfo.bLookAt ) 
					BoneResult.Rotation	= VectorToAngle(ParentResult.Location - BoneResult.Location);
				else
					BoneResult.Rotation	= ParentResult.Rotation + VectorToAngle(BoneCoords.XAxis);
			}
		}
		else
		{
			// No rotation controller, so use transform.
			BoneResult.Rotation	= VectorToAngle(BoneCoords.XAxis);
		}
	}
}


/*-----------------------------------------------------------------------------
	Skeleton linking.
-----------------------------------------------------------------------------*/

//
// Link bone to parent. 
//
Bool FSkeleton::LinkTo( Int32 iBone, Int32 iParent )
{
	TBoneInfo& ThisInfo = Bones[iBone];
	TBonePose& ThisPose = RefPose.BonesPose[iBone];

	// Unlink from old parent.
	if( ThisInfo.iPosCtrl!=-1 || ThisInfo.iRotCtrl!=-1 )
		BreakLinks(iBone);

	// Get parent's coords system.
	if( iParent != -1 )
	{
		// Transform own coords system to parent space.
		TBonePose& ParentPose = RefPose.BonesPose[iParent];
		TCoords ParentSpace( ParentPose.Location, ParentPose.Rotation );

		ThisPose.Coords	= TCoords
		(
			TransformPointBy( ThisPose.Coords.Origin, ParentSpace ),
			VectorToAngle(ThisPose.Coords.XAxis) - ParentPose.Rotation
		);

		ThisInfo.iPosCtrl	= iParent;
		ThisInfo.iRotCtrl	= iParent;
		ThisInfo.bLookAt	= false;
	}
	else
	{
		// Link to world.
		ThisPose.Coords	= TCoords::Identity;
	}

	return true;	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11 make real check!!! 
}


/////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

/*
//
// Link control to parent. Return true, if successfully linked and
// return false if recursion happened.
//
Bool FSkeleton::LinkControl( Integer iControl, Integer iParent )
{
	TControl&	Control		= Controls[iControl];
	TAngle		ParentRot	= OrientationTransform(iParent, iControl);

	TCoords ParentCoords
	(
		PositionTransform( iParent ),
		ParentRot
	);

	Control.iPosConstraint	= iParent;
	Control.iRotConstraint	= iParent;
	Control.Position		= TransformPointBy( Control.Position, ParentCoords );
	Control.Orientation		= Control.Orientation - ParentRot;
	Control.bLookAt			= false;

	return true;	// No recursion test yet!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
}
*/

Bool FSkeleton::LinkLookAtTo( Int32 iBone, Int32 iParent )
{ 
	// Unlink rotation from parent!!!

	TBoneInfo& ThisInfo = Bones[iBone];
	TBonePose& ThisPose = RefPose.BonesPose[iBone];

	ThisInfo.iRotCtrl	= iParent;
	ThisInfo.bLookAt	= true;

	return 0;	// no real check!!

}


Bool FSkeleton::LinkRotationTo( Int32 iBone, Int32 iParent )
{
	TBoneInfo& ThisInfo = Bones[iBone];
	TBonePose& ThisPose = RefPose.BonesPose[iBone];

	// Unroll rotation from old parent.
	if( ThisInfo.iRotCtrl != -1 )
	{
		TBonePose& OldParent = RefPose.BonesPose[ThisInfo.iRotCtrl];

		ThisPose.Coords.XAxis	= AngleToVector(VectorToAngle(ThisPose.Coords.XAxis) + OldParent.Rotation);
		ThisPose.Coords.YAxis = ThisPose.Coords.XAxis.Cross();
	}


	ThisInfo.iRotCtrl	= iParent;
	ThisInfo.bLookAt	= false;

	return 0;	// no real check!!
}


Bool FSkeleton::LinkIKSolverTo( Int32 iIKSolver, Int32 iEndJoint )
{ 
	TBoneInfo& EndJoint		= Bones[iEndJoint];
	TBoneInfo& StartJoint	= Bones[EndJoint.iPosCtrl];
	TBoneInfo& Solver		= Bones[iIKSolver];

	StartJoint.iRotCtrl		= iIKSolver;
	EndJoint.iRotCtrl		= iIKSolver;
	Solver.iEndJoint		= iEndJoint;


	return 0;

/*
//
// Set IK solver to joint and it parent. Return true if successfully added.
//
Bool FSkeleton::SetIKSolver( Integer iIK, Integer iEndJoint )
{
	assert(Controls[iIK].Type == SKEL_IKSolver);
	assert(Controls[iEndJoint].iPosConstraint != -1);	// RETURN FALSE????

	TControl&	IK			= Controls[iIK];
	TControl&	EndJoint	= Controls[iEndJoint];
	TControl&	StartJoint	= Controls[EndJoint.iPosConstraint];

	EndJoint.Orientation		= 0A;
	StartJoint.Orientation		= 0A;
	EndJoint.iRotConstraint		= iIK;
	StartJoint.iRotConstraint	= iIK;
	IK.iEndJoint				= iEndJoint;

	return true;	// No recursion test yet!!!!!!!!!!!
}
*/

}




Bool FSkeleton::BreakRotLink( Int32 iBone )
{
	TBoneInfo& Info = Bones[iBone];
	TBonePose& Pose = RefPose.BonesPose[iBone];

	// Restore rotation from the world space. 
	if( Info.iRotCtrl != -1 )
	{
		Pose.Coords.XAxis	= AngleToVector(Pose.Rotation);
		Pose.Coords.YAxis	= Pose.Coords.XAxis.Cross();


		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// Destroy all links from the IK.
		Info.iRotCtrl		= -1;
	}


	// Always success?
	return true;
}

/////////////////////////
///////////////////////////////////////////////////////////////


//
// Break all position links from the iBone.
//
Bool FSkeleton::BreakPosLink( Int32 iBone )
{
	TBoneInfo& Info = Bones[iBone];
	TBonePose& Pose = RefPose.BonesPose[iBone];

	if( Info.iPosCtrl != -1 )
	{
		Pose.Coords.Origin	= Pose.Location;
		Info.iPosCtrl		= -1;
	}

	return true;
}


//
// Break all links from the iBone.
//
Bool FSkeleton::BreakLinks( Int32 iBone )
{
	return BreakPosLink(iBone) && BreakRotLink(iBone);
}


/*-----------------------------------------------------------------------------
	Skeleton tables.
-----------------------------------------------------------------------------*/

//
// Compute table of bones transformation order.
// Assume no circular dependencies.
//
void FSkeleton::BuildTransformationTable()
{
#if 1
	// Mark all bones as not processed.
	for( Int32 i=0; i<Bones.Num(); i++ )
		Bones[i].Flags &= ~(BONE_PosProcessed | BONE_RotProcessed);

	Bool TryAgain = true;
	TransformTable.Empty();

	// Go through bones tree.
	while( TransformTable.Num() != Bones.Num() )
	{
		TryAgain = false;

		for( Int32 i=0; i<Bones.Num(); i++ )
		{
			TBoneInfo& Bone = Bones[i];

			if( TransformTable.FindItem(i) == -1 )
			{

				// If bone has no position controller, so we can compute postion.
				if( Bone.iPosCtrl == -1 )
					Bone.Flags |= BONE_PosProcessed;
			
				// If bone has no rotation controller, so we can compute rotation.
				if( Bone.iRotCtrl == -1 )
					Bone.Flags |= BONE_RotProcessed;

				// Position of bone depend on parent position and rotation.
				if( Bone.iPosCtrl != -1 && (Bones[Bone.iPosCtrl].Flags & BONE_Processed) )
					Bone.Flags |= BONE_PosProcessed;

				// Rotation of bone depend only on parent rotation.
				if( Bone.iRotCtrl != -1 && (Bones[Bone.iRotCtrl].Flags & BONE_RotProcessed) && !Bone.bLookAt )
					Bone.Flags |= BONE_RotProcessed;

				// Look at target so bone rotation depend on controller position.
				if( Bone.iRotCtrl != -1 && Bone.bLookAt && (Bones[Bone.iRotCtrl].Flags & BONE_PosProcessed) )
					Bone.Flags |= BONE_RotProcessed;

				// IK chain, so rotation depend on IK position.
				if( Bone.Type==SC_IKSolver && (Bone.Flags & BONE_PosProcessed) )
				{
					if( Bone.iEndJoint != -1 )
					{
						TBoneInfo& EndJoint = Bones[Bone.iEndJoint];
						EndJoint.Flags |= BONE_RotProcessed;

						if( EndJoint.iPosCtrl != -1 )
							Bones[EndJoint.iPosCtrl].Flags |= BONE_RotProcessed;
					}
				}

				// Add to processed if position and rotation can be solved and
				// parent are already in list.
				if(		(Bone.Flags & BONE_Processed) && 
						(Bone.iPosCtrl==-1 || TransformTable.FindItem(Bone.iPosCtrl)!=-1) && 
						(Bone.iRotCtrl==-1 || TransformTable.FindItem(Bone.iRotCtrl)!=-1) )
				{
					TransformTable.Push(i);
					info( L"%d", i );
					TryAgain = true;
				}
			}
		}
		info(L"---");
	}



#else
	// Mark all bones as not processed.
	for( Integer i=0; i<Bones.Num(); i++ )
		Bones[i].Flags &= ~BONE_Processed;


	Bool TryAgain = true;
	TransformTable.Empty();

	// Go through bones tree.
	while( TryAgain )
	{
		TryAgain = false;

		for( Integer i=0; i<Bones.Num(); i++ )
		{
			TBoneInfo& Bone = Bones[i];

			if( !(Bone.Flags & BONE_Processed) )
				if( Bone.iPosCtrl==-1 || (Bones[Bone.iPosCtrl].Flags & BONE_Processed) )
					if( Bone.iRotCtrl==-1 || (Bones[Bone.iRotCtrl].Flags & BONE_Processed) )
					{
						Bone.Flags |= BONE_Processed;
						TransformTable.Push(i);
						TryAgain = true;
					}
		}
	}
#endif
	/*
	for( Integer i=0; i<TransformTable.Num(); i++ )
		log( L"%d", TransformTable[i] );
		*/
	assert(TransformTable.Num() == Bones.Num());
}


/*-----------------------------------------------------------------------------
	TBoneInfo implementation.
-----------------------------------------------------------------------------*/

//
// Simple bone constructor.
//
TBoneInfo::TBoneInfo()
	:	Type( SC_Bone ),
		Name( L"" ),
		Color( COLOR_Tomato ),
		Flags( BONE_None ),
		iPosCtrl( -1 ),
		iRotCtrl( -1 ),
		Scale( 1.f )
{
}


//
// In-editor bone constructor.
//
TBoneInfo::TBoneInfo( ESkelCntrl InType, String InName, TColor InColor )
	:	Type( InType ),
		Name( InName ),
		Color( InColor ),
		Flags( BONE_None ),
		iPosCtrl( -1 ),
		iRotCtrl( -1 ),
		Scale( 0.01f )
{
	switch( Type )
	{
		case SC_Bone:
			bLookAt		= false;
			break;

		case SC_Master:
			break;

		case SC_IKSolver:
			bFlipIK		= false;
			iEndJoint	= -1;
			break;
	}
}


//
// Bone info serialization.
//
void Serialize( CSerializer& S, TBoneInfo& V )
{
	SerializeEnum( S, V.Type );
	Serialize( S, V.Name );
	Serialize( S, V.Color );
	Serialize( S, V.Flags );
	Serialize( S, V.iPosCtrl );
	Serialize( S, V.iRotCtrl );
	Serialize( S, V.Scale );

	switch( V.Type )
	{
		case SC_Bone:
			Serialize( S, V.bLookAt );
			break;

		case SC_Master:
			break;

		case SC_IKSolver:
			Serialize( S, V.bFlipIK );
			Serialize( S, V.iEndJoint );
			break;
	}
}


/*-----------------------------------------------------------------------------
	TSkeletonAction implementation.
-----------------------------------------------------------------------------*/

//
// Default action constructor.
//
TSkeletonAction::TSkeletonAction()
	:	Name()
{}


//
// In-editor action constructor.
//
TSkeletonAction::TSkeletonAction( String InName )
	:	Name( InName )
{
}


/*-----------------------------------------------------------------------------
	Registration.
-----------------------------------------------------------------------------*/

REGISTER_CLASS_CPP( FSkeleton, FResource, CLASS_None )
{
	ADD_PROPERTY( Placeholder, PROP_Editable );
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/