/*=============================================================================
    FrEngine.h: Engine general include file.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/
#pragma once

// Disable annoying warnings.
#pragma warning(disable : 4200)
#pragma warning(disable : 4244)

// C++ includes.
#include <math.h>			// to be removed!!
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <stdarg.h>
#include <stdio.h>
#include <type_traits>
#include <typeinfo.h> 

   
// Partial classes tree.
class CStruct;
class CClass;
class FObject;
	class FEntity;
	class FComponent;
		class FBaseComponent;
			class FCameraComponent;
			class FZoneComponent;
			class FPhysicComponent;
		class FExtraComponent;
			class FLogicComponent;
	class FResource;
		class FFont;
		class FTexture;
			class FMaterial;
			class FBitmap;
		class FLevel;
		class FScript;
		class FAnimation;
		class FSkeleton;
		class FProjectInfo;
	class FModifier;
		class FMaterialLayer;
class CCanvas;
class CBlockManager;
class CInstanceBuffer;
class CFrame;
class CThreadFrame;
class CCollisionHash;
class CNavigator;
class CPhysics;
enum EPathType;
enum EEventName;
struct TDelegate;
struct TViewInfo;
template<class K, class V> class TMap;


// Flu includes.
#include "Core/Core.h"
#include "Math/Math.h"

using namespace flu; //todo: remove this;


namespace flu
{
namespace envi
{
	class TimeOfDay;
}
}

// Engine includes.
#include "Core\FrBase.h"
#include "Core\FrRand.h"
#include "Core\FrMemory.h"
#include "Core\FrMath.h"
#include "Core\FrColor.h"
#include "Core\FrExpImp.h"
#include "Core\FrMap.h"
#include "Core\FrEncode.h"
#include "Core\FrClass.h"
#include "Core\FrObject.h"
#include "Core\FrBuff.h"
#include "Core\FrFile.h"
#include "Core\FrIni.h"
#include "Core\FrCorUtils.h"
#include "Core\FrNetBas.h"

// Profiling
#include "Chart/EngineProfiler.h"
#include "Chart/EngineChart.h"

// Environment
#include "Environment/TimeOfDay.h"
#include "Environment/Sky.h"
#include "Environment/Satellite.h"
#include "Environment/EnvironmentContext.h"
#include "Environment/Environment.h"

// Physics engine
#include "Physics/PhysicsUtils.h"

#include "FrCom.h"
#include "FrBlock.h"
#include "FrRes.h"
#include "FrModi.h"
#include "FrAudio.h"
#include "FrOpCode.h"
#include "FrScript.h"
#include "FrCode.h"
#include "FrEntity.h"
#include "FrBitmap.h"
#include "FrMaterial.h"
#include "FrAnim.h"
#include "FrFont.h"
#include "FrRender.h"
#include "FrDbgDraw.h"
#include "FrSkelet.h"
#include "FrComTyp.h"
#include "FrGFX.h"
#include "FrInput.h"
#include "FrLevel.h"
#include "FrCollHash.h"
#include "FrProject.h"
#include "FrApp.h"
#include "FrDemoEff.h"
#include "FrPhysEng.h"
#include "FrPath.h"

// Database.
#include "Database.h"


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/