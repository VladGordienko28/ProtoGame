/*=============================================================================
	FrScript.h: FScript class.
	Created by Vlad Gordienko, Jun. 2016.
	Major improvements, May. 2018.
=============================================================================*/

/*-----------------------------------------------------------------------------
	Script objects.
-----------------------------------------------------------------------------*/

//
// An abstract code stream.
//
class CBytecode
{
public:
	// Variables.
	TArray<Byte>	Code;
	Integer			iLine;
	Integer			iPos;

	// CBytecode interface.
	CBytecode();
	virtual ~CBytecode();
};


//
// An entity thread code.
//
class CThreadCode: public CBytecode
{
public:
	// A label in the code.
	struct TLabel
	{
		String	Name;
		Word	Address;
	};

	// Variables.
	TArray<TLabel>		Labels;

	// CThreadCode interface.
	CThreadCode();
	Integer GetLabelId( const Char* InName );	
	Integer AddLabel( const Char* InName, Word InAddr );
};


//
// Function flags.
//
#define FUNC_None			0x0000		// No function flags;
#define FUNC_HasResult		0x0001		// Function has return value;
#define FUNC_Event			0x0002		// This is predefined event;
#define FUNC_Static			0x0004		// Static function;
#define FUNC_Shell			0x0008		// Shell-execution function;		
#define FUNC_Virtual		0x0010		// Virtual function;
#define FUNC_Override		0x0020		// Override function;


//
// A script function.
//
class CFunction: public CBytecode
{
public:
	// Variables.
	String				Name;
	DWord				Flags;
	TArray<CProperty*>	Locals;
	SizeT				FrameSize;
	Integer				ParmsCount;
	CProperty*			ResultVar;
 
	// CFunction interface.
	CFunction();
	~CFunction();
	String GetSignature() const;
};


/*-----------------------------------------------------------------------------
	CInstanceBuffer.
-----------------------------------------------------------------------------*/

//
// A properties values holder class.
//
class CInstanceBuffer
{
public:
	// Variables.
	TArray<CProperty*>&	Properties;
	TArray<Byte>		Data;

	// CInstanceBuffer interface.
	CInstanceBuffer( TArray<CProperty*>& InProperties );
	~CInstanceBuffer();
	void DestroyValues();
	void CopyValues( void* Source );
	void ImportValues( CImporterBase& Im );
	void ExportValues( CExporterBase& Ex );
	void SerializeValues( CSerializer& S );
	Bool NeedDestruction() const;
};


/*-----------------------------------------------------------------------------
	FScript.
-----------------------------------------------------------------------------*/

//
// Script flags.
//
#define SCRIPT_None			0x000001		// No flags;
#define SCRIPT_Compiled		0x000002		// Script is compiled;
#define SCRIPT_Static		0x000004		// Is a static script;
#define SCRIPT_Debug		0x000008		// Debug compiled script;
#define SCRIPT_Hybrid		0x000010		// Hybrid compiled script;
#define SCRIPT_Release		0x000020		// Release compiled script;
#define SCRIPT_Marked		0x000040		// Special script marker;
#define SCRIPT_Scriptable	0x000080		// Script has text and allowed to edit;


//
// A Script.
//
class FScript: public FResource
{
REGISTER_CLASS_H(FScript);
public:
	// Generic script information.
	DWord			ScriptFlags;
	Integer			iFamily;
	TArray<String>	Text;

	// Prototype components.
	TArray<FExtraComponent*>	Components;
	FBaseComponent*				Base;

	// Prototype and Static values.
	CInstanceBuffer*	InstanceBuffer;
	SizeT				InstanceSize;
	CInstanceBuffer*	StaticsBuffer;
	SizeT				StaticsSize;

	// All script tables.
	TArray<CEnum*>			Enums;
	TArray<CStruct*>		Structs;
	TArray<CProperty*>		Properties;
	TArray<CProperty*>		Statics;
	TArray<CFunction*>		Methods;
	TArray<CFunction*>		Events;
	TArray<CFunction*>		StaticFunctions;
	TArray<CFunction*>		VFTable;
	CThreadCode*			Thread;

	// Table of resources uses in bytecode.
	TArray<FResource*>			ResTable;

	// FScript interface.
	FScript();
	~FScript();
	FComponent* FindComponent( String InName );
	CFunction* FindMethod( String TestName );
	CFunction* FindStaticFunction( String TestName );

	// Static functions execution.
	void CallStaticFunction
	(
		String FuncName,
		VARIANT_PARM(P1),
		VARIANT_PARM(P2),
		VARIANT_PARM(P3),
		VARIANT_PARM(P4)
	);

	// FObject interface.
	void SerializeThis( CSerializer& S ) override;
	void EditChange() override;
	void PostLoad() override;
	void Import( CImporterBase& Im ) override;
	void Export( CExporterBase& Ex ) override;

	// Accessors.
	inline Bool IsScriptable() const
	{
		return ScriptFlags & SCRIPT_Scriptable;
	}
	inline Bool IsStatic() const
	{
		return ScriptFlags & SCRIPT_Static;
	}
};


/*-----------------------------------------------------------------------------
	List of all defined events.
-----------------------------------------------------------------------------*/

//
// Make a list of events as enum.
//
#define SCRIPT_EVENT(name, ...)	EVENT_##name,
enum EEventName
{
#include "FrEvent.h"
	_EVENT_MAX
};
#undef SCRIPT_EVENT


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/