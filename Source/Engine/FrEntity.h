/*=============================================================================
    FrEntity.h: Entity class.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    FEntity.
-----------------------------------------------------------------------------*/

//
// An level entity.
//
class FEntity: public FObject
{
REGISTER_CLASS_H(FEntity);
public:
	// General info.
	FLevel*						Level;
	FScript*					Script;
	CInstanceBuffer*			InstanceBuffer;
	CThreadFrame*				Thread;

	// Components.
	FBaseComponent*				Base;
	TArray<FExtraComponent*>	Components;

	// FEntity interface.
	FEntity();
	~FEntity();
	void Init( FScript* InScript, FLevel* InLevel );
	void BeginPlay();
	void EndPlay();

	// FObject interface.
	void SerializeThis( CSerializer& S );
	void PostLoad();
	void Import( CImporterBase& Im );
	void Export( CExporterBase& Ex );

	// FluScript custom function entry point.
	void CallFunction
	(
		String FuncName,
		VARIANT_PARM(P1),
		VARIANT_PARM(P2),
		VARIANT_PARM(P3),
		VARIANT_PARM(P4)
	);

	//
	// Register FluScript events as FEntity methods.
	// Here's are miracles begins.
	//
	#define _HELPER_PARMDECL5_END
	#define _HELPER_PARMDECL4_END
	#define _HELPER_PARMDECL3_END
	#define _HELPER_PARMDECL2_END
	#define _HELPER_PARMDECL1_END
	#define _HELPER_PARMDECL0_END

	#define _HELPER_PARMDECL5_ARG(name, type, ...) , type name
	#define _HELPER_PARMDECL4_ARG(name, type, ...) , type name _HELPER_PARMDECL5_##__VA_ARGS__
	#define _HELPER_PARMDECL3_ARG(name, type, ...) , type name _HELPER_PARMDECL4_##__VA_ARGS__
	#define _HELPER_PARMDECL2_ARG(name, type, ...) , type name _HELPER_PARMDECL3_##__VA_ARGS__
	#define _HELPER_PARMDECL1_ARG(name, type, ...) , type name _HELPER_PARMDECL2_##__VA_ARGS__
	#define _HELPER_PARMDECL0_ARG(name, type, ...) type name _HELPER_PARMDECL1_##__VA_ARGS__

	#define _HELPER_PARMCOPY5_END
	#define _HELPER_PARMCOPY4_END
	#define _HELPER_PARMCOPY3_END
	#define _HELPER_PARMCOPY2_END
	#define _HELPER_PARMCOPY1_END
	#define _HELPER_PARMCOPY0_END

	#define _HELPER_PARMCOPY5_ARG(name, type, ...) *(type*)(&Frame.Locals[Event->Locals[5]->Offset]) = name;
	#define _HELPER_PARMCOPY4_ARG(name, type, ...) *(type*)(&Frame.Locals[Event->Locals[4]->Offset]) = name; _HELPER_PARMCOPY5_##__VA_ARGS__
	#define _HELPER_PARMCOPY3_ARG(name, type, ...) *(type*)(&Frame.Locals[Event->Locals[3]->Offset]) = name; _HELPER_PARMCOPY4_##__VA_ARGS__
	#define _HELPER_PARMCOPY2_ARG(name, type, ...) *(type*)(&Frame.Locals[Event->Locals[2]->Offset]) = name; _HELPER_PARMCOPY3_##__VA_ARGS__
	#define _HELPER_PARMCOPY1_ARG(name, type, ...) *(type*)(&Frame.Locals[Event->Locals[1]->Offset]) = name; _HELPER_PARMCOPY2_##__VA_ARGS__
	#define _HELPER_PARMCOPY0_ARG(name, type, ...) *(type*)(&Frame.Locals[Event->Locals[0]->Offset]) = name; _HELPER_PARMCOPY1_##__VA_ARGS__

	#define SCRIPT_EVENT( name, ... )\
	void name( _HELPER_PARMDECL0_##__VA_ARGS__ )\
	{\
		if( !Script->IsScriptable() || Script->Events.Num()==0 )\
			return;\
	\
		CFunction* Event = Script->Events[EVENT_##name];\
		if( !Event )\
			return;\
	\
		try\
		{\
			CFrame Frame( this, Event, 1, nullptr );\
			_HELPER_PARMCOPY0_##__VA_ARGS__; \
			Frame.ProcessCode( nullptr );\
		}\
		catch( ... )\
		{\
			GOutput->ScriptErrorf( L"Script: Event \"" L#name "\" Runtime error" );\
		}\
	}\

	#include "FrEvent.h"

	#undef SCRIPT_EVENT

	#undef _HELPER_PARMCOPY5_ARG
	#undef _HELPER_PARMCOPY4_ARG
	#undef _HELPER_PARMCOPY3_ARG
	#undef _HELPER_PARMCOPY2_ARG
	#undef _HELPER_PARMCOPY1_ARG
	#undef _HELPER_PARMCOPY0_ARG

	#undef _HELPER_PARMCOPY5_END
	#undef _HELPER_PARMCOPY4_END
	#undef _HELPER_PARMCOPY3_END
	#undef _HELPER_PARMCOPY2_END
	#undef _HELPER_PARMCOPY1_END
	#undef _HELPER_PARMCOPY0_END

	#undef _HELPER_PARMDECL5_ARG
	#undef _HELPER_PARMDECL4_ARG
	#undef _HELPER_PARMDECL3_ARG
	#undef _HELPER_PARMDECL2_ARG
	#undef _HELPER_PARMDECL1_ARG
	#undef _HELPER_PARMDECL0_ARG

	#undef _HELPER_PARMDECL0_END
	#undef _HELPER_PARMDECL1_END
	#undef _HELPER_PARMDECL2_END
	#undef _HELPER_PARMDECL3_END
	#undef _HELPER_PARMDECL4_END
	#undef _HELPER_PARMDECL5_END
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/