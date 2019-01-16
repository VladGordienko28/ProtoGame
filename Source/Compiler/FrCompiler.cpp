/*=============================================================================
    FrCompiler.cpp: FluScript compiler.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#include "Compiler.h"

/*-----------------------------------------------------------------------------
    Compiler declarations.
-----------------------------------------------------------------------------*/

//
// Script fields alignment bounds.
//
#if FLU_X64
	#define SCRIPT_PROP_ALIGN	8
#elif FLU_X32
	#define SCRIPT_PROP_ALIGN	4
#else 
	#error Bad platform configuration
#endif


//
// An access modifier.
//
enum EAccessModifier
{
	ACC_Public,
	ACC_Private
};


//
// Keyword constants.
//
#define KEYWORD( word ) static const Char* KW_##word = L#word;
#include "FrKeyword.h"
#undef KEYWORD


//
// Temporal word variable.
//
static UInt16 GTempWord	= 0xCAFE; 


//
// An current object context. It's should
// be tracked to reduce bytecode length.
//
enum EEntityContext
{
	CONT_This,
	CONT_Other,
	CONT_MAX
};


/*-----------------------------------------------------------------------------
    TToken.
-----------------------------------------------------------------------------*/

//
// Token type.
//
enum ETokenType
{
	TOK_None,
	TOK_Symbol,
	TOK_Const,
	TOK_Identifier
};


//
// A parsed lexeme.
//
struct TToken
{
public:
	// Text information.
	ETokenType		Type;
	String			Text;
	Int32			iLine;
	Int32			iPos;

	// An information about token
	// property type.
	CTypeInfo		TypeInfo;

	// Constant values if TOK_Const.
	union
	{
		UInt8			cByte;
		Bool			cBool;
		Int32			cInteger;
		Float			cFloat;
		FResource*		cResource;
		FEntity*		cEntity;
		UInt8			_Payload[sizeof(TRegister::Value)];
	};
	String			cString;

	// Not really simple types.
	TVector& cVector(){ return *(TVector*)_Payload; }
	TAngle& cAngle(){ return *(TAngle*)_Payload; }
	TColor& cColor(){ return *(TColor*)_Payload; }
	TRect& cAABB(){ return *(TRect*)_Payload; }
	TDelegate& cDelegate(){ return *(TDelegate*)_Payload; }
};


/*-----------------------------------------------------------------------------
    TStoredScript.
-----------------------------------------------------------------------------*/

//
// A stored script & entities properties
// to restore later. Since user don't want
// to set properties again and again after
// each compilation.
//
struct TStoredScript
{
public:
	FScript*				Script;			// Source script.
	SizeT					InstanceSize;	// Stored properties instance size.
	Array<CProperty*>		Properties;		// Old properties.
	Array<CEnum*>			Enums;			// Old enumeration, still referenced by properties above.
	Array<CStruct*>			Structs;		// Old structs, still referenced by properties.
	Array<CInstanceBuffer*>	Buffers;		// All instance buffers of this script from the entities and script.
};


/*-----------------------------------------------------------------------------
    TExprResult.
-----------------------------------------------------------------------------*/

//
// An information about result type
// of the compiled expression.
//
struct TExprResult
{
public:
	// Variables.
	CTypeInfo	Type;
	Bool		bLValue;
	UInt8		iReg;

	// Constructor.
	TExprResult()
		:	bLValue( false ),
			iReg( 0xff )
	{}
};


/*-----------------------------------------------------------------------------
    CCodeEmitter.
-----------------------------------------------------------------------------*/

//
// A bytecode emitter.
//
class CCodeEmitter: public CSerializer
{
public:
	// Variables.
	CBytecode*			Bytecode;

	// CCodeEmitter interface.
	CCodeEmitter();
	void SetBytecode( CBytecode* InBytecode );

	// CSerializer interface.
	void SerializeData( void* Mem, SizeT Count );
	void SerializeRef( FObject*& Obj );
	SizeT TotalSize();
	SizeT Tell();
};


//
// Emitter macro.
//
#define emit(v)			Serialize( Emitter, v );
#define emit_const(v)	Serialize( this, Emitter, Script, v );


/*-----------------------------------------------------------------------------
    TNest.
-----------------------------------------------------------------------------*/

// Nesting type.
enum ENestType
{
	NEST_Leading,
	NEST_For,
	NEST_While,
	NEST_Do,
	NEST_If,
	NEST_Switch,
	NEST_Foreach
};


// Address replacement type.
enum EReplType
{
	REPL_Break,
	REPL_Continue,
	REPL_Return,
	REPL_MAX
};


//
// List of addresses in nest.
//
struct TNest
{
public:
	// Variables.
	ENestType		Type;
	Array<UInt16>	Addrs[REPL_MAX];
};


/*-----------------------------------------------------------------------------
    CFamily.
-----------------------------------------------------------------------------*/

//
// A shared scripts interface.
//
class CFamily
{
public:
	// Variables.
	String				Name;		// An family name.
	Int32				iFamily;	// An unique family number.
	Array<FScript*>		Scripts;	// List of family members.
	Array<String>		VFNames;	// List of names of all virtual functions.
	Array<CFunction*>	Proto;		// Signature for each function to match, order same as in VFNames.
};


/*-----------------------------------------------------------------------------
	TDelegateInfo.
-----------------------------------------------------------------------------*/

//
// A delegate signature.
//
class TDelegateInfo
{
public:
	// A delegate signature parameter.
	class CParameter: public CTypeInfo
	{
	public:
		Bool bOut;

		CParameter()
			:	CTypeInfo(),
				bOut( false )
		{}
		CParameter( CTypeInfo InInfo, Bool InbOut )
			:	CTypeInfo(InInfo),
				bOut( InbOut )
		{}
	};

	String				Name;
	CTypeInfo			ResultType;
	Array<CParameter>	ParamsType;

	// TDelegateInfo interface.
	Bool MatchSignature( CFunction* Func ) const;
};


//
// A native event information(signature).
//
static struct TEventInfo
{
public:
	String				Name;
	Array<CTypeInfo>	Params;

	// TEventInfo interface.
	TEventInfo( String InName )
		:	Name(InName)
	{}
	Bool MatchSignature( CFunction* Other ) const
	{
		if( !(Other->Flags & FUNC_Event) )
			return false;
		if( Other->ParmsCount != Params.size() )
			return false;

		for( Int32 i=0; i<Params.size(); i++ )
			if( !Params[i].MatchWith(*Other->Locals[i]) )
				return false;

		return true;
	}
};


/*-----------------------------------------------------------------------------
    CCompiler.
-----------------------------------------------------------------------------*/

//
// A script compiler.
//
class CCompiler
{
public:
	// CCompiler public interface.
	CCompiler( CObjectDatabase* InDatabase, Array<String>& OutWarnings, Compiler::TError& OutFatalError );
	~CCompiler();
	Bool CompileAll();

	// Errors & warnings.
	void Error( const Char* Fmt, ... );
	void Warn( const Char* Fmt, ... );

private:
	// Errors.
	Array<String>&						Warnings;		
	Compiler::TError&					FatalError;	

	// Parsing.
	Int32								TextLine, TextPos;
	Int32								PrevLine, PrevPos;

	// General.
	CObjectDatabase*					Database;
	FScript*							Script;	
	Array<TStoredScript>				Storage;
	Array<FScript*>						AllScripts;
	Array<CFamily*>						Families;
	Array<TDelegateInfo>				DelegatesInfo;
	Array<TEventInfo>					GEventLookup;

	// First pass variables.
	EAccessModifier						Access;
	Array<TToken>						Constants;		

	// Second pass variables.
	CCodeEmitter						Emitter;
	CBytecode*							Bytecode;
	Int32								NestTop;
	TNest								Nest[16];
	Bool								bValidExpr;
	Bool								IsStaticScope;
	EEntityContext						Context;
	Bool								Regs[CFrame::NUM_REGS];
	
	// Top level functions.
	void StoreAllScripts();
	void RestoreAfterFailure();
	void RestoreAfterSuccess();
	void ParseHeader( FScript* InScript );
	void CompileFirstPass( FScript* InScript );
	void CompileSecondPass( FScript* InScript );

	// Expression compilation.
	TExprResult CompileExpr( const CTypeInfo& ReqType, Bool bForceR, Bool bAllowAssign = false, UInt32 InPri = 0 );
	Bool CompileEntityExpr( EEntityContext InContext, CTypeInfo Entity, UInt8 iConReg, TExprResult& Result );
	Bool CompileFuncCastExpr( const TToken& ToType, TExprResult& Result );
	UInt8 GetCast( EPropType Src, EPropType Dst );
	TExprResult CompileProtoExpr( FScript* Prototype );
	
	// Second pass compilation.
	void CompileCode( CBytecode* InCode );
	void CompileStatement();
	void CompileIf();
	void CompileSwitch();
	void CompileWhile();
	void CompileFor();
	void CompileDo();
	void CompileForeach();
	void CompileCommand();

	// Nest.
	void PushNest( ENestType InType );
	void PopNest();
	void ReplNest( EReplType Repl, UInt16 DestAddr );

	// Registers management.
	void ResetRegs();
	UInt8 GetReg();
	void FreeReg( UInt8 iReg );

	// Declarations compiling.
	void CompileDeclaration();
	CTypeInfo CompileVarType( Bool bSimpleOnly = false );
	void CompileEnumDecl();
	void CompileStructDecl();
	void CompileDelegateDecl();
	void CompileConstDecl();
	Bool CompileVarDecl( Array<CProperty*>& Vars, SizeT& VarsSize, UInt32 InFlags, Bool bSimpleOnly = false, Bool bDetectFunc = true );
	void CompileFunctionDecl();
	void CompileThreadDecl();
	
	// Lexical analysis.
	void GetToken( TToken& T, Bool bAllowNeg = false, Bool bAllowVect = false );
	void GotoToken( const TToken& T );
	Int32 ReadInteger( const Char* Purpose );
	Float ReadFloat( const Char* Purpose );
	String ReadString( const Char* Purpose );
	void RequireIdentifier( const Char* Name, const Char* Purpose );
	void RequireSymbol( const Char* Name, const Char* Purpose );
	String GetIdentifier( const Char* Purpose );
	String PeekIdentifier();
	String PeekSymbol();
	Bool MatchIdentifier( const Char* Name );
	Bool MatchSymbol( const Char* Name );

	// Characters parsing functions.
	Char _GetChar();
	Char GetChar();
	void UngetChar();
	
	// Searching.
	CFunction* FindFunction( const Array<CFunction*>& FuncList, String Name );
	CProperty* FindProperty( const Array<CProperty*>& List, String Name );
	TToken* FindConstant( String Name );
	Int32 FindDelegate( String Name );
	CEnum* FindEnum( FScript* Script, String Name, Bool bOwnOnly = false );
	CStruct* FindStruct( FScript* Script, String Name, Bool bOwnOnly = false );
	FScript* FindScript( String Name );
	CFamily* FindFamily( String Name );
	UInt8 FindEnumElement( String Elem );
	CNativeFunction* FindUnaryOp( String Name, const CTypeInfo& ArgType = TYPE_None );
	CNativeFunction* FindBinaryOp( String Name, const CTypeInfo& Arg1 = TYPE_None, const CTypeInfo& Arg2 = TYPE_None );
	CNativeFunction* FindNative( String Name );
	Int32 FindStaticEvent( String Name );

	// Miscs.
	void CollectAllEvents();
};


/*-----------------------------------------------------------------------------
    CCodeEmitter implementation.
-----------------------------------------------------------------------------*/

// 
// Code emitter constructor.
//
CCodeEmitter::CCodeEmitter()
	:	Bytecode(nullptr)
{
	Mode		= SM_Save;
}


//
// Set a bytecode to emitter code.
//
void CCodeEmitter::SetBytecode( CBytecode* InBytecode )
{
	assert(InBytecode);
	Bytecode	= InBytecode;
}


//
// Tell current location in bytecode.
//
SizeT CCodeEmitter::Tell()
{
	return Bytecode->Code.size();
}


//
// Return total size of the code.
//
SizeT CCodeEmitter::TotalSize()
{
	return Bytecode->Code.size();
}


//
// Push a data to the code.
//
void CCodeEmitter::SerializeData( void* Mem, SizeT Count )
{
	SizeT OldLoc = Tell();
	Bytecode->Code.setSize( Bytecode->Code.size() + Count );
	mem::copy( &Bytecode->Code[OldLoc], Mem, Count );
}


//
// Serialize reference to some object.
//
void CCodeEmitter::SerializeRef( FObject*& Obj )
{
	Int32 iObj = Obj ? Obj->GetId() : -1;
	Serialize( *this, iObj );
}


//
// Opcode serialization.
//
static void Serialize( CSerializer& S, EOpCode V )
{
	// Force to be byte.
	assert(S.GetMode() == SM_Save);
	S.SerializeData( &V, sizeof(UInt8) );
}


//
// Property type serialization.
//
static void Serialize( CSerializer& S, EPropType V )
{
	// Force to be byte.
	assert(S.GetMode() == SM_Save);
	S.SerializeData( &V, sizeof(UInt8) );
}


//
// TToken's constant value serialization.
//
static void Serialize( CCompiler* Compiler, CSerializer& S, FScript* Owner, TToken& Const )
{
	assert(Const.Type == TOK_Const);

	switch( Const.TypeInfo.Type )
	{
		case TYPE_Byte:
			Serialize( S, CODE_ConstByte );
			Serialize( S, Const.cByte );
			break;

		case TYPE_Bool:
			Serialize( S, CODE_ConstBool );
			Serialize( S, Const.cBool );
			break;

		case TYPE_Integer:
			Serialize( S, CODE_ConstInteger );
			Serialize( S, Const.cInteger );
			break;

		case TYPE_Float:
			Serialize( S, CODE_ConstFloat );
			Serialize( S, Const.cFloat );
			break;	

		case TYPE_Angle:
			Serialize( S, CODE_ConstAngle );
			Serialize( S, Const.cAngle() );
			break;	

		case TYPE_Color:
			Serialize( S, CODE_ConstColor );
			Serialize( S, Const.cColor() );
			break;	

		case TYPE_String:
			if( Const.cString.Len() >= STRING_ARRAY_MAX )
				Compiler->Error( L"Too long literal string" );

			Serialize( S, CODE_ConstString );
			Serialize( S, Const.cString );
			break;	

		case TYPE_Vector:
			Serialize( S, CODE_ConstVector );
			Serialize( S, Const.cVector() );
			break;	

		case TYPE_AABB:
			Serialize( S, CODE_ConstAABB );
			Serialize( S, Const.cAABB() );
			break;	

		case TYPE_Entity:
			Serialize( S, CODE_ConstEntity );
			Serialize( S, Const.cEntity );
			break;

		case TYPE_Resource:
			Serialize( S, CODE_ConstResource );
			if( Const.cResource )
			{
				// Add to list.
				UInt8 iRes = Owner->ResTable.addUnique( Const.cResource );
				assert(Owner->ResTable.size() < 256);
				Serialize( S, iRes );
			}
			else
			{
				// null resource.
				UInt8 iRes = 0xff;
				Serialize( S, iRes );
			}
			break;

		case TYPE_Delegate:
			Serialize( S, CODE_ConstDelegate );
			Serialize( S, Const.cDelegate().iMethod );
			Serialize( S, Const.cDelegate().Script );
			Serialize( S, Const.cDelegate().Context );
			break;

		default:
			fatal( L"Unsupported constant type %d", (UInt8)Const.TypeInfo.Type );
	}
}


/*-----------------------------------------------------------------------------
	TDelegateInfo implementation.
-----------------------------------------------------------------------------*/

//
// Return true, if signature of function and delegate are matched.
//
Bool TDelegateInfo::MatchSignature( CFunction* Func ) const
{
	assert(Func);

	if( Func->ParmsCount != ParamsType.size() )
		return false;

	if( !Func->ResultVar && ResultType.Type != TYPE_None )
		return false;

	if( Func->ResultVar && !Func->ResultVar->MatchWith(ResultType) )
		return false;

	for( Int32 i=0; i<Func->ParmsCount; i++ )
	{
		if( ParamsType[i].bOut && !(Func->Locals[i]->Flags & PROP_OutParm) )
			return false;

		if( !ParamsType[i].bOut && (Func->Locals[i]->Flags & PROP_OutParm) )
			return false;

		if( !Func->Locals[i]->MatchWith(ParamsType[i]) )
			return false;
	}

	return true;
}


/*-----------------------------------------------------------------------------
    CCompiler implementation.
-----------------------------------------------------------------------------*/

//
// Compiler constructor.
//
CCompiler::CCompiler( CObjectDatabase* InDatabase, Array<String>& OutWarnings, Compiler::TError& OutFatalError )
	:	Database( InDatabase ),
		Warnings( OutWarnings ),
		FatalError( OutFatalError ),
		Storage(),
		Emitter(),
		Families(),
		DelegatesInfo(),
		Bytecode( nullptr )
{
}


//
// Compiler destructor.
//
CCompiler::~CCompiler()
{
	// Destroy temporal families list.
	for( Int32 i=0; i<Families.size(); i++ )
		delete Families[i];
	Families.empty();
}


//
// Compilation entry point.
// Return true, if compilation successfully,
// otherwise return false and FatalError will has 
// an information about error.
//
Bool CCompiler::CompileAll()
{
	try
	{
		// Prepare for compilation.
		Warnings.empty();
		FatalError.ErrorLine	= -1;
		FatalError.ErrorPos		= -1;
		FatalError.Message		= L"Everything is fine";
		FatalError.Script		= nullptr;
		CollectAllEvents();

		info( L"** COMPILATION BEGAN **" );

		// Perform compilation step by step.
		StoreAllScripts();

		for( Int32 i=0; i<Storage.size(); i++ )
			ParseHeader( Storage[i].Script );

		for( Int32 i=0; i<Storage.size(); i++ )
		{
			CompileFirstPass( Storage[i].Script );
		}

		for( Int32 i=0; i<Storage.size(); i++ )
		{
			CompileSecondPass( Storage[i].Script );
		}

		RestoreAfterSuccess();

		// Count lines.
		Int32  NumLines = 0;
		for( Int32 i=0; i<Storage.size(); i++ )
			NumLines += Storage[i].Script->Text.size();

		// Everything ok, so notify and return.
		info( L"Compiler: COMPILATION SUCCESSFULLY" );
		info( L"Compiler: %d scripts compiled", Storage.size() );
		info( L"Compiler: %d lines compiled", NumLines );

		// Add to compilation log.
		Warnings.push( L"---" );
		Warnings.push(String::Format( L"%d scripts compiled", Storage.size() ));
		Warnings.push(String::Format( L"%d lines compiled", NumLines ));

		return true;
	}
	catch( ... )
	{
		// Something horrible happened.
		RestoreAfterFailure();
		debug( L"Compiler: COMPILATION ABORTED" );
		debug( L"Compiler: %s", *FatalError.Message );

		return false;
	}
}


//
// Collect all possible script events.
//
void CCompiler::CollectAllEvents()
{
	GEventLookup.empty();

	#define _HELPER_PARMLOOK5_END
	#define _HELPER_PARMLOOK4_END
	#define _HELPER_PARMLOOK3_END
	#define _HELPER_PARMLOOK2_END
	#define _HELPER_PARMLOOK1_END
	#define _HELPER_PARMLOOK0_END

	#define _HELPER_PARMLOOK5_ARG(name, type, ...) type* name=nullptr; Event.Params.push(_Cpp2FluType(name));
	#define _HELPER_PARMLOOK4_ARG(name, type, ...) type* name=nullptr; Event.Params.push(_Cpp2FluType(name)); _HELPER_PARMLOOK5_##__VA_ARGS__
	#define _HELPER_PARMLOOK3_ARG(name, type, ...) type* name=nullptr; Event.Params.push(_Cpp2FluType(name)); _HELPER_PARMLOOK4_##__VA_ARGS__
	#define _HELPER_PARMLOOK2_ARG(name, type, ...) type* name=nullptr; Event.Params.push(_Cpp2FluType(name)); _HELPER_PARMLOOK3_##__VA_ARGS__
	#define _HELPER_PARMLOOK1_ARG(name, type, ...) type* name=nullptr; Event.Params.push(_Cpp2FluType(name)); _HELPER_PARMLOOK2_##__VA_ARGS__
	#define _HELPER_PARMLOOK0_ARG(name, type, ...) type* name=nullptr; Event.Params.push(_Cpp2FluType(name)); _HELPER_PARMLOOK1_##__VA_ARGS__

	#define SCRIPT_EVENT( name, ... )\
	{\
		TEventInfo Event(L#name);\
		_HELPER_PARMLOOK0_##__VA_ARGS__ \
		GEventLookup.push(Event);\
	}\

	#include "../Engine/FrEvent.h"

	#undef _HELPER_PARMLOOK5_END
	#undef _HELPER_PARMLOOK4_END
	#undef _HELPER_PARMLOOK3_END
	#undef _HELPER_PARMLOOK2_END
	#undef _HELPER_PARMLOOK1_END
	#undef _HELPER_PARMLOOK0_END

	#undef _HELPER_PARMLOOK5_ARG
	#undef _HELPER_PARMLOOK4_ARG
	#undef _HELPER_PARMLOOK3_ARG
	#undef _HELPER_PARMLOOK2_ARG
	#undef _HELPER_PARMLOOK1_ARG
	#undef _HELPER_PARMLOOK0_ARG

	#undef SCRIPT_EVENT
}


//
// Parse a script header and figure out it 
// name and family.
//
void CCompiler::ParseHeader( FScript* InScript )
{
	// Prepare for parsing.
	Script				= InScript;
	Script->iFamily		= -1;
	TextLine			= 0;
	TextPos				= 0;
	PrevLine			= 0;
	PrevPos				= 0;

	// Compile header.
	Bool bStatic = MatchIdentifier( KW_static );
	RequireIdentifier( KW_script, L"script declaration" );
	RequireIdentifier( *Script->GetName(), L"script declaration" );

	if( bStatic )
	{
		if( !Script->IsStatic() )
			Error( L"'%s' is not a static script", *Script->GetName() );
	}
	else
	{
		if( Script->IsStatic() )
			Error( L"'%s' is a static script. You miss 'static' keyword?", *Script->GetName() );
	}

	// Parse family.
	if( MatchSymbol(L":") )
	{
		// Don't use family for static scripts.
		if( bStatic )
			Error( L"Static Scripts is should be orphan" );

		// Add to family or create new one.
		RequireIdentifier( KW_family, L"script family" );
		String FamilyName	= GetIdentifier(L"script family");
		CFamily* MyFamily = FindFamily( FamilyName );

		if( !MyFamily )
		{
			// Create new one.
			MyFamily			= new CFamily();
			MyFamily->Name		= FamilyName;
			MyFamily->Scripts.push(Script);
			MyFamily->iFamily	= Families.push( MyFamily );
			debug( L"Compiler: Created new family '%s'", *MyFamily->Name );
		}
		else
		{
			// Add to exist.
			assert(MyFamily->Scripts.find(Script)==-1);
			MyFamily->Scripts.push(Script);
		}

		Script->iFamily	= MyFamily->iFamily;
	}
}


//
// Compile script first pass.
//
void CCompiler::CompileFirstPass( FScript* InScript )
{
	// Prepare compiler.
	Access				= ACC_Public;
	Script				= InScript;
	TextLine			= 0;
	TextPos				= 0;
	PrevLine			= 0;
	PrevPos				= 0;	

	// Compile header.
	MatchIdentifier( KW_static );
	RequireIdentifier( KW_script, L"script header" );
	RequireIdentifier( *Script->GetName(), L"script header" );

	if( MatchSymbol(L":") )
	{
		// Family.
		assert(!Script->IsStatic());
		RequireIdentifier( KW_family, L"script header" );
		/*Script->Family	=*/ GetIdentifier(L"script family");
	}

	RequireSymbol( L"{", L"script body" );
	
	// Compile declaration by declaration.
	for( ; ; )
	{
		if( MatchSymbol( L"}" ) )
		{
			// End of script.
			break;
		}
		else if( MatchIdentifier(KW_public) )
		{
			// Start public section.
			Access = ACC_Public;
			RequireSymbol( L":", L"public section" );
		}
		else if( MatchIdentifier(KW_private) )
		{
			// Start private section.
			Access = ACC_Private;
			RequireSymbol( L":", L"private section" );

			if( Script->IsStatic() )
				Warn( L"'private' modifier is disabled within static script" );
		}
		else
		{
			// Any single declaration.
			CompileDeclaration();
		}
	}
}


//
// Compile script second pass.
//
void CCompiler::CompileSecondPass( FScript* InScript )
{
	// Set target.
	Script			= InScript;
	IsStaticScope	= false;

	// Fill complete VF table for script, its ok
	// if some slots are nullptr.
	CFamily* Family = Script->iFamily != -1 ? Families[Script->iFamily] : nullptr;
	if( Family )
	{
		assert(Family->VFNames.size()==Family->Proto.size());
		Script->VFTable.setSize( Family->VFNames.size() );
		for( Int32 i=0; i<Script->VFTable.size(); i++ )
			Script->VFTable[i] = FindFunction( Script->Methods, Family->VFNames[i] );
	}
	else
		Script->VFTable.empty();

	// Fill list of events.
	Script->Events.empty();
	Script->Events.setSize(_EVENT_MAX);
	for( Int32 i=0; i<Script->Methods.size(); i++ )
		if( Script->Methods[i]->Flags & FUNC_Event )
		{ 
			CFunction*	Func	= Script->Methods[i];
			Int32		iEvent	= FindStaticEvent(Func->Name);
			assert(iEvent != -1);
			Script->Events[iEvent]	= Func;
		}

	if( Script->IsStatic() )
	{
		assert(Script->Methods.size() == 0);
		assert(Script->Thread == nullptr);
		assert(Script->Properties.size() == 0);
	}

	// Compile actor thread if it specified. It's important to compile the thread
	// before functions.
	IsStaticScope = false;
	if( Script->Thread )
		CompileCode( Script->Thread );

	// Compile all functions.
	IsStaticScope = false;
	for( Int32 i=0; i<Script->Methods.size(); i++ )
		CompileCode( Script->Methods[i] );

	IsStaticScope = true;
	for( Int32 i=0; i<Script->StaticFunctions.size(); i++ )
		CompileCode( Script->StaticFunctions[i] );
}


//
// Compile function or thread body.
//
void CCompiler::CompileCode( CBytecode* InCode )
{
	// Set compiling target.
	Emitter.SetBytecode( InCode );
	NestTop			= 0;
	Bytecode		= InCode;
	Context			= CONT_This;

	// Move caret to the function start.
	TextLine	=	PrevLine	= Bytecode->iLine;
	TextPos		=	PrevPos		= Bytecode->iPos;
	assert(PeekSymbol()==L"{");

	PushNest( NEST_Leading );

	// Compile function code.
	CompileStatement();

	ReplNest( REPL_Return, Emitter.Tell() );		
	PopNest();

	// Add special mark to the end of the bytecode.
	emit( CODE_EOC );

    // Check code size.
	if( Bytecode->Code.size() >= MAX_UINT16 )
		Error( L"Too large function >64kB of code" );

#if 0
	// Debug it!
	log( L"%s::%s", *Script->GetName(), Bytecode != Script->Thread ? *((CFunction*)Bytecode)->Name : L"Thread" );
	for( Integer i=0; i<Bytecode->Code.Num(); i++ )
		log( L"[%d]=%d", i, Bytecode->Code[i] );
#endif
}


/*-----------------------------------------------------------------------------
    Expression compiling.
-----------------------------------------------------------------------------*/

//
// Macro to convert L-value to the R-value.
//
#define emit_ltor( lval )\
if( lval.Type.ArrayDim != 1 || lval.Type.Type == TYPE_Struct )\
{\
	Error( L"Incorrect L-value to R-value converstion" );\
}\
if( lval.bLValue )\
{\
	if( lval.Type.Type == TYPE_String )\
	{\
		emit( CODE_LToRString );\
		emit( lval.iReg );\
	}\
	else if( lval.Type.TypeSize(true) == 4 )\
	{\
		emit( CODE_LToRDWord );\
		emit( lval.iReg );\
	}\
	else\
	{\
		UInt8 VarSize = lval.Type.TypeSize(true);\
		emit( CODE_LToR );\
		emit( lval.iReg );\
		emit( VarSize );\
	}\
	lval.bLValue = false;\
}\


//
// Macro to handle different assignments.
// lside - left side of assignment, should be l-value.
// rside - right side of assignment, should be r-value! 
//
#define emit_assign( lside, rside )\
{\
	if( lside.Type.Type == TYPE_String )\
	{\
		emit( CODE_AssignString );\
		emit( lside.iReg );\
		emit( rside.iReg );\
	}\
	else if( lside.Type.TypeSize(true) == sizeof(UInt32) )\
	{\
		emit( CODE_AssignDWord );\
		emit( lside.iReg );\
		emit( rside.iReg );\
	}\
	else\
	{\
		UInt8 VarSize = lside.Type.TypeSize(true);\
		emit( CODE_Assign );\
		emit( lside.iReg );\
		emit( rside.iReg );\
		emit( VarSize );\
	}\
}\


//
// Macro to emit multi-byte opcode.
//
#define emit_opcode( iopcode )\
{\
	UInt8 code = (UInt8)iopcode;\
	emit(code);\
}\


//
// Compile a top-level expression. The most valuable function.
// Return information about compiled expression, its type and result register.
// If ReqType specified( not TYPE_None ), required to expression matched this type
// tries to apply type cast, if failed its abort compilation.
//
//	ReqType			- Required expression type, or TYPE_None, if doesn't really matter.
//	bForceR			- Force expression be an R-value and return value in R register.
//	bAllowAssign	- Whether allow assignment in expression.
//	InPri			- Binary operator priority, don't touch it, use just 0.
//
TExprResult CCompiler::CompileExpr( const CTypeInfo& ReqType, Bool bForceR, Bool bAllowAssign, UInt32 InPri )
{
	// Scrap.
	CProperty*			Prop;
	TToken*				Const;
	UInt8				iEnumEl;
	CNativeFunction*	Native;
	FScript*			CastScript;
	CFamily*			CastFamily;
	CFunction*			Function;

	// Prepare for expression compilation.
	TToken T;
	TExprResult ExprRes;
	GetToken( T, false, false );

	//
	// Step 1: Figure out the base expression.
	//
	if( T.Text == L"(" )
	{
		// Bracket, compile subexpression.
		ExprRes = CompileExpr( TYPE_None, false, false, 0 );
		RequireSymbol( L")", L"expression bracket" );
	}
	else if( T.Type == TOK_Const )
	{
		// Literal constant.
		ExprRes.bLValue		= false;
		ExprRes.iReg		= GetReg();
		ExprRes.Type		= T.TypeInfo;

		emit_const( T );
		emit( ExprRes.iReg );
	}
	else if( T.Text == KW_this )
	{
		// Reference to the self.
		if( IsStaticScope )
			Error( L"'this' is not allows in static script" );

		ExprRes.bLValue			= false;
		ExprRes.iReg			= GetReg();
		ExprRes.Type			= CTypeInfo( TYPE_Entity, 1, Script );
		ExprRes.Type.iFamily	= Script->iFamily;

		emit( CODE_This );
		emit( ExprRes.iReg );
	}
	else if( T.Text == KW_assert )
	{
		// Assertion.
		UInt16 Line = TextLine + 1;
		UInt8 iReg = CompileExpr( TYPE_Bool, true, false, 0 ).iReg;

		emit( CODE_Assert );
		emit( Line );
		emit( iReg );

		bValidExpr		= true;
		ExprRes.Type	= TYPE_None;
		FreeReg( iReg );
	}
	else if(	
				T.Text == KW_info ||
				T.Text == KW_debug ||
				T.Text == KW_warn ||
				T.Text == KW_error	
		)
	{
		// Some kind of printf in C.
		RequireSymbol( L"(", *T.Text );
		Array<TExprResult> Args;

		String Text = ReadString( L"format string" );
		String Fmt = Text;

		// Parse each arg.
		do
		{
			Int32 i = String::Pos( L"%", Text );
			if( i == -1 )
				break;

			RequireSymbol( L",", L"log arguments" );
			Text = String::Delete( Text, i, 1 );
			Char Symbol = Text(i);
			EPropType ArgType;

			switch( Symbol )
			{
				case 'i':
				case 'd':
					ArgType = TYPE_Integer;
					break;

				case 'b':
					ArgType = TYPE_Bool;
					break;

				case 'f':
					ArgType = TYPE_Float;
					break;

				case 's':
					ArgType	= TYPE_String;
					break;

				case 'v':
					ArgType	= TYPE_Vector;
					break;

				case 'r':
					ArgType	= TYPE_Resource;
					break;

				case 'c':
					ArgType	= TYPE_Color;
					break;

				case 'a':
					ArgType	= TYPE_AABB;
					break;

				case 'x':
					ArgType	= TYPE_Entity;
					break;

				default:
					Char Str[2] = { Symbol, '\0' }; 
					Error( L"Unknown format symbol '%s' in log", Str );
					break;
			}

			// Collect registers.
			Args.push( CompileExpr( ArgType, true, false, 0 ) );
		}
		while( true );

		RequireSymbol( L")", L"log" );

		ELogLevel LogLevel = ELogLevel::Debug;
		if( T.Text == KW_info )		LogLevel = ELogLevel::Info; else
		if( T.Text == KW_debug )	LogLevel = ELogLevel::Debug; else
		if( T.Text == KW_warn )		LogLevel = ELogLevel::Warning; else
		if( T.Text == KW_error )	LogLevel = ELogLevel::Error;

		emit( CODE_Log );
		SerializeEnum( Emitter, LogLevel );
		emit( Fmt );
		for( Int32 i=0; i<Args.size(); i++ )
		{
			emit( Args[i].Type.Type );
			emit( Args[i].iReg );
			FreeReg( Args[i].iReg );
		}

		bValidExpr		= true;
		ExprRes.Type	= TYPE_None;
	}
	else if( T.Text == KW_new )
	{
		// Create a new entity.
		if( IsStaticScope )
			Error( L"'new' is not allows in static script" );

		TToken S;
		GetToken( S );
		GotoToken( S );
		FScript* Known;
		TExprResult ScriptExpr;

		if( (Known = FindScript(GetIdentifier(L"script name"))) != nullptr  )
		{
			// Script known.
			ScriptExpr.bLValue		= false;
			ScriptExpr.iReg			= GetReg();
			ScriptExpr.Type			= TYPE_SCRIPT;
			ScriptExpr.Type.iFamily	= Known->iFamily;

			emit( CODE_ConstResource );
			UInt8 iRes = Script->ResTable.addUnique( Known );
			assert(Script->ResTable.size() < 256);
			emit( iRes );
			emit( ScriptExpr.iReg );

			ExprRes.bLValue			= false;
			ExprRes.iReg			= GetReg();
			ExprRes.Type			= CTypeInfo( TYPE_Entity, 1, Known );
			ExprRes.Type.iFamily	= Known->iFamily;
		}
		else
		{
			// Should use RTTI.
			GotoToken( S );
			ScriptExpr = CompileExpr( TYPE_SCRIPT, true, false, 0 );

			ExprRes.bLValue			= false;
			ExprRes.iReg			= GetReg();
			ExprRes.Type			= CTypeInfo( TYPE_Entity, 1 );
			ExprRes.Type.iFamily	= -1;
		}

		emit( CODE_New );
		emit( ScriptExpr.iReg );
		emit( ExprRes.iReg );

		bValidExpr				= true;
		FreeReg( ScriptExpr.iReg );
	}
	else if( T.Text == KW_delete )
	{
		// Delete an entity.
		if( IsStaticScope )
			Error( L"'delete' is not allows in static script" );

		TExprResult EntityExpr = CompileExpr( TYPE_Entity, true, false, 0 );
		emit( CODE_Delete );
		emit( EntityExpr.iReg );

		FreeReg( EntityExpr.iReg );
		ExprRes.bLValue		= false;
		ExprRes.Type		= TYPE_None;
		bValidExpr			= true;
	}
	else if( T.Text == KW_label )
	{
		// A current thread label.
		if( !Script->Thread || IsStaticScope )
			Error( L"Script has no thread" );

		ExprRes.bLValue			= false;
		ExprRes.iReg			= GetReg();
		ExprRes.Type			= TYPE_Integer;

		emit( CODE_Label );
		emit( ExprRes.iReg );
	}
	else if( T.Text == KW_proto )
	{
		// Prototype value.
		if( Script->IsStatic() )
			Error( L"Static script has no prototype" );

		ExprRes = CompileProtoExpr( Script ); 
	}
	else if( T.Text == L"[" )
	{
		// Vector constructor.
		TExprResult	X	= CompileExpr( TYPE_Float, true, false, 0 );
		RequireSymbol( L",", L"vector constructor" );
		TExprResult Y	= CompileExpr( TYPE_Float, true, false, 0 );
		RequireSymbol( L"]", L"vector constructor" );

		ExprRes.bLValue		= false;
		ExprRes.iReg		= GetReg();
		ExprRes.Type.Type	= TYPE_Vector;

		emit( CODE_VectorCnstr );
		emit( ExprRes.iReg );
		emit( X.iReg );
		emit( Y.iReg );

		FreeReg( X.iReg );
		FreeReg( Y.iReg );
	}
	else if( T.Text == L"@" )
	{
		// Label constant.
		if( !Script->Thread || Script->IsStatic() )
			Error( L"Labels doesn't allowed, without thread" );

		String LabName = GetIdentifier( L"label" );
		Int32 iLabel = Script->Thread->GetLabelId( *LabName );

		if( iLabel == -1 )
			Error( L"Label '@%s' not found", *LabName );

		// Store as integer constant.
		ExprRes.bLValue	= false;
		ExprRes.iReg	= GetReg();
		ExprRes.Type	= TYPE_Integer;

		emit( CODE_ConstInteger );
		emit( iLabel );
		emit( ExprRes.iReg );
	}
	else if	( 
				Bytecode != Script->Thread && 
				(Prop = FindProperty( ((CFunction*)Bytecode)->Locals, T.Text) ) 
			)
	{
		// A local variable.
		CFunction*	Owner	= (CFunction*)Bytecode;

		ExprRes.bLValue		= true;
		ExprRes.iReg		= GetReg();
		ExprRes.Type		= *Prop;

		UInt16 Offset	= Prop->Offset;

		emit( CODE_LocalVar );
		emit( ExprRes.iReg );
		emit( Offset );
	}
	else if( Prop = FindProperty( Script->Statics, T.Text ) )
	{
		// An own static property.
		ExprRes.bLValue		= true;
		ExprRes.iReg		= GetReg();
		ExprRes.Type		= *Prop;

		UInt16 Offset	= Prop->Offset;

		emit( CODE_StaticProperty );
		emit( Script );
		emit( ExprRes.iReg );
		emit( Offset );
	}
	else if( Native = FindUnaryOp( T.Text, TYPE_None ) )
	{
		// Unary operator.
		TExprResult Arg = CompileExpr( TYPE_None, false, false, 777 );
		if( Arg.Type.Type == TYPE_None || Arg.Type.ArrayDim != 1 )
			Error( L"Bad argument in unary operator '%s'", *T.Text );

		Native = FindUnaryOp( T.Text, Arg.Type );
		if( !Native )
			Error( L"Operator '%s' cannot be applied to operand of type %s", *T.Text, *Arg.Type.TypeName() );

		// It an inc/dec operator or regular.
		if( Native->Flags & NFUN_SuffixOp )
		{
			// L-value operator.
			if( !Arg.bLValue )
				Error( L"The right-hand side of an assignment must be a variable" );

			emit_opcode( Native->iOpCode );
			emit( Arg.iReg );

			ExprRes.bLValue		= true;
			ExprRes.iReg		= Arg.iReg;
			ExprRes.Type		= Native->ResultType;
			bValidExpr			= true;
		}
		else
		{
			// R-value operator.
			emit_ltor( Arg );

			// Types are matched?
			if( Native->Params[0].Type.Type != Arg.Type.Type )	// It's ok, since it's a simple types.
			{
				// Add cast.
				UInt8 Cast = GetCast( Arg.Type.Type, Native->Params[0].Type.Type );
				assert(Cast != 0x00);

				emit( Cast );
				emit( Arg.iReg );
			}

			emit_opcode( Native->iOpCode );
			emit( Arg.iReg );

			ExprRes.bLValue		= false;
			ExprRes.Type		= Native->ResultType;
			ExprRes.iReg		= Arg.iReg;
		}
	}
	else if( Native = FindNative(T.Text) )
	{
		// Native function call.
		Array<UInt8>	ArgRegs;

		if( Native->Flags & NFUN_Foreach )
			Error( L"Iteration function '%s' is not allowed here", *Native->Name );

		RequireSymbol( L"(", L"function call" );
		for( Int32 i=0; i<Native->NumParams; i++ )
		{
			ArgRegs.push( CompileExpr( Native->Params[i].Type, true, false, 0 ).iReg );
			if( i < Native->NumParams-1 )
				RequireSymbol( L",", L"arguments" ); 
		}
		RequireSymbol( L")", L"function call" );

		if (Native->Flags & NFUN_Extended)
		{
			// Emit as extended.
			UInt16 iExtended = CClassDatabase::GFuncs.find(Native);

			emit_opcode(CODE_CallExtended);
			emit(iExtended);
		}
		else
		{
			// Emit as simple.
			emit_opcode( Native->iOpCode );
		}

		for( Int32 i=0; i<ArgRegs.size(); i++ )
		{
			emit( ArgRegs[i] );
			FreeReg( ArgRegs[i] );
		}

		if( Native->ResultType.Type != TYPE_None )
		{
			// Has result.
			ExprRes.Type		= Native->ResultType;
			ExprRes.iReg		= GetReg();
			ExprRes.bLValue		= false;
			emit( ExprRes.iReg );
		}
		else
		{
			// No result.
			ExprRes.bLValue		= false;
			ExprRes.iReg		= -1;
			ExprRes.Type		= TYPE_None;
		}

		bValidExpr	= true;
	}
	else if( Function = FindFunction( Script->StaticFunctions, T.Text ) )
	{
		// An own static function call.
		Array<UInt8>	ArgRegs;

		RequireSymbol( L"(", L"function call" );
		for( Int32 i=0; i<Function->ParmsCount; i++ )
		{
			ArgRegs.push( CompileExpr( Function->Locals[i]->Type, true, false, 0 ).iReg );
			if( i < Function->ParmsCount-1 )
				RequireSymbol( L",", L"arguments" ); 
		}
		RequireSymbol( L")", L"function call" );

		Int32 iStatic = Script->StaticFunctions.find(Function);
		assert(iStatic != -1 && iStatic <= 255);
		UInt8 Tmp = iStatic;

		emit( CODE_CallStatic );
		emit( Script );
		emit( Tmp );

		for( Int32 i=0; i<ArgRegs.size(); i++ )
		{
			emit( ArgRegs[i] );
			FreeReg( ArgRegs[i] );
		}

		if( Function->ResultVar != nullptr )
		{
			// Has result.
			ExprRes.Type		= *Function->ResultVar;
			ExprRes.iReg		= GetReg();
			ExprRes.bLValue		= false;
			emit( ExprRes.iReg );
		}
		else
		{
			// No result.
			ExprRes.bLValue		= false;
			ExprRes.iReg		= -1;
			ExprRes.Type		= TYPE_None;
		}

		bValidExpr	= true;
	}
	else if( CastScript = FindScript( T.Text ) )
	{
		// Entity script cast or prototype value.
		if( MatchSymbol(L".") )
		{
			// Proto or static.
			if( MatchIdentifier( KW_proto ) )
			{
				// Prototype sub-expression.
				if( CastScript->IsStatic() )
					Error( L"Script '%s' is not prototypable", *CastScript->GetName() );

				ExprRes = CompileProtoExpr( CastScript );
			}
			else
			{
				// Static subexression.
				String StaticName = GetIdentifier(L"Static script field");
				if( Prop = FindProperty( CastScript->Statics, StaticName ) )
				{
					// An script static property.
					ExprRes.bLValue		= true;
					ExprRes.iReg		= GetReg();
					ExprRes.Type		= *Prop;

					UInt16 Offset	= Prop->Offset;

					emit( CODE_StaticProperty );
					emit( CastScript );
					emit( ExprRes.iReg );
					emit( Offset );
				}
				else if( Function = FindFunction( CastScript->StaticFunctions, StaticName ) )
				{
					// An own static function call.
					Array<UInt8>	ArgRegs;

					RequireSymbol( L"(", L"function call" );
					for( Int32 i=0; i<Function->ParmsCount; i++ )
					{
						ArgRegs.push( CompileExpr( Function->Locals[i]->Type, true, false, 0 ).iReg );
						if( i < Function->ParmsCount-1 )
							RequireSymbol( L",", L"arguments" ); 
					}
					RequireSymbol( L")", L"function call" );

					Int32 iStatic = CastScript->StaticFunctions.find(Function);
					assert(iStatic != -1 && iStatic <= 255);
					UInt8 Tmp = iStatic;

					emit( CODE_CallStatic );
					emit( CastScript );
					emit( Tmp );

					for( Int32 i=0; i<ArgRegs.size(); i++ )
					{
						emit( ArgRegs[i] );
						FreeReg( ArgRegs[i] );
					}

					if( Function->ResultVar != nullptr )
					{
						// Has result.
						ExprRes.Type		= *Function->ResultVar;
						ExprRes.iReg		= GetReg();
						ExprRes.bLValue		= false;
						emit( ExprRes.iReg );
					}
					else
					{
						// No result.
						ExprRes.bLValue		= false;
						ExprRes.iReg		= -1;
						ExprRes.Type		= TYPE_None;
					}

					bValidExpr	= true;
				}
				else
					Error( L"Static property '%s' not found in script '%s'", *StaticName, *CastScript->GetName() );
			}
		}
		else
		{
			// Typecast.
			RequireSymbol( L"(", L"explicit cast" );
			TExprResult Ent	= CompileExpr( TYPE_Entity, true, false, 0 );
			RequireSymbol( L")", L"explicit cast" );

			ExprRes.bLValue			= false;
			ExprRes.iReg			= Ent.iReg;
			ExprRes.Type			= CTypeInfo( TYPE_Entity, 1, CastScript );
			ExprRes.Type.iFamily	= CastScript->iFamily;
	
			emit( CODE_EntityCast );
			emit( Ent.iReg );
			emit( ExprRes.Type.Script );
		}
	}
	else if( CastFamily = FindFamily( T.Text ) )
	{
		// Entity family cast.
		RequireSymbol( L"(", L"explicit cast" );
		TExprResult Ent	= CompileExpr( TYPE_Entity, true, false, 0 );
		RequireSymbol( L")", L"explicit cast" );

		ExprRes.bLValue			= false;
		ExprRes.iReg			= Ent.iReg;
		ExprRes.Type			= CTypeInfo( TYPE_Entity, 1, Ent.Type.Script );
		ExprRes.Type.iFamily	= CastFamily->iFamily;
		assert(CastFamily->Scripts.size()>0 && CastFamily->iFamily!=-1);

		emit( CODE_FamilyCast );
		emit( Ent.iReg );
		emit( CastFamily->iFamily );
	}
	else if( Const = FindConstant(T.Text) )
	{
		// Named constant.
		ExprRes.bLValue		= false;
		ExprRes.Type.Type	= Const->TypeInfo.Type;
		ExprRes.iReg		= GetReg();

		emit_const( *Const );
		emit( ExprRes.iReg );
	}
	else if( (iEnumEl = FindEnumElement( T.Text )) != 0xff )
	{
		// Of desperation, search enum element.
		ExprRes.bLValue		= false;
		ExprRes.iReg		= GetReg();
		ExprRes.Type		= TYPE_Byte;

		emit( CODE_ConstByte );
		emit( iEnumEl );
		emit( ExprRes.iReg );
	}
	else if( CompileFuncCastExpr(T, ExprRes) )
	{
		// Functional typecast already compiled.
	}
	else
	{
		// Try last thing.
		GotoToken( T );
		if( IsStaticScope || !CompileEntityExpr( CONT_This, CTypeInfo( TYPE_Entity, 1, Script ), 0xff, ExprRes ) )
			Error( L"Expected expression, got '%s'", *T.Text );
	}


	//
	// Step 1.5: Check for none result, is it has sense to continue parse expression.
	//
	if( ExprRes.Type.Type == TYPE_None )
	{
		if( ReqType.Type != TYPE_None )
			Error( L"'%s' expected but 'void' found", *ReqType.TypeName() );

		return ExprRes;
	}


	//
	// Step 2: Members.
	//
	for( ; ; )
	{
		if( MatchSymbol( L"[" ) )
		{
			// Get an array element.
			if( ExprRes.Type.ArrayDim == -1 )
			{
				// Dynamic array.
				if( !ExprRes.bLValue )
					Error( L"Dynamic array variable excepting" );

				// Get index subexpression.
				TExprResult Index = CompileExpr( TYPE_Integer, true, false, 0 );
				RequireSymbol( L"]", L"array index" );

				CTypeInfo Elem( ExprRes.Type.Type, 1, ExprRes.Type.Inner );
				UInt8 InnerSize	= Elem.TypeSize(true);

				emit( CODE_DynArrayElem );
				emit( ExprRes.iReg );
				emit( Index.iReg );
				emit( InnerSize );

				// Replace with inner.
				FreeReg( Index.iReg );
				ExprRes.Type.ArrayDim	= 1;
			}
			else
			{			
				// Static array.
				if( ExprRes.Type.ArrayDim <= 1 )
					Error( L"Array type excepting" );
	
				if( !ExprRes.bLValue )
					Error( L"Array variable excepting" );

				// Get index subexpression.
				TExprResult Index = CompileExpr( TYPE_Integer, true, false, 0 );
				RequireSymbol( L"]", L"array index" );

				UInt8 InnerSize	= ExprRes.Type.TypeSize(true);
				UInt8 ArrSize	= ExprRes.Type.ArrayDim;
				emit( CODE_ArrayElem );
				emit( ExprRes.iReg );
				emit( Index.iReg );
				emit( InnerSize );
				emit( ArrSize );

				// Replace with inner.
				FreeReg( Index.iReg );
				ExprRes.Type.ArrayDim	= 1;
			}
		}
		else if( MatchSymbol( L"." ) )
		{
			if( ExprRes.Type.ArrayDim > 1 )
			{
				// Static array length.
				RequireIdentifier( KW_length, L"'length' method" );

				// Array length, store as integer constant.
				emit( CODE_ConstInteger );
				emit( ExprRes.Type.ArrayDim );
				emit( ExprRes.iReg );

				ExprRes.bLValue			= false;
				ExprRes.Type			= CTypeInfo( TYPE_Integer );
				break;
			}
			else if( ExprRes.Type.ArrayDim == 1 && ExprRes.Type.Type == TYPE_String )
			{
				// String length.
				RequireIdentifier( KW_length, L"'length' method" );

				emit_ltor( ExprRes );
				emit( CODE_Length );
				emit( ExprRes.iReg );

				ExprRes.bLValue			= false;
				ExprRes.Type			= CTypeInfo( TYPE_Integer );
				break;
			}
			if( ExprRes.Type.ArrayDim == -1 )
			{
				// Dynamic array methods.
				if( MatchIdentifier(KW_length) )
				{
					// Set or get dynamic array length.
					if( bAllowAssign && MatchSymbol(L"=") )
					{
						// Set length.
						TExprResult NewLength = CompileExpr( TYPE_Integer, true, false, 0 );

						CTypeInfo Elem( ExprRes.Type.Type, 1, ExprRes.Type.Inner );
						UInt8 InnerSize	= Elem.Type != TYPE_String ? Elem.TypeSize(true) : 0;

						emit(CODE_DynSetLength);
						emit(ExprRes.iReg);
						emit(NewLength.iReg);
						emit(InnerSize);

						FreeReg(NewLength.iReg);
						ExprRes.Type	= TYPE_None;
						bValidExpr	= true;
						break;
					}
					else
					{
						// Get length.
						emit( CODE_DynGetLength );
						emit( ExprRes.iReg );

						ExprRes.bLValue		= false;
						ExprRes.Type		= TYPE_Integer;
						break;
					}
				}
				else if( MatchIdentifier(KW_push) )
				{
					// Push a new item to array.
					CTypeInfo Inner( ExprRes.Type.Type, 1, ExprRes.Type.Inner );
					UInt8 InnerSize = Inner.Type != TYPE_String ? Inner.TypeSize() : 0;

					RequireSymbol(L"(", L"push");
					TExprResult NewElem = CompileExpr( Inner, true, false, 0 );
					RequireSymbol(L")", L"push");

					emit( CODE_DynPush );
					emit( ExprRes.iReg );
					emit( NewElem.iReg );
					emit( InnerSize );

					FreeReg(NewElem.iReg);
					ExprRes.bLValue		= false;
					ExprRes.Type		= TYPE_Integer;
					bValidExpr			= true;
					break;
				}
				if( MatchIdentifier(KW_pop) )
				{
					// Pop item from array.
					CTypeInfo Inner( ExprRes.Type.Type, 1, ExprRes.Type.Inner );
					UInt8 InnerSize = Inner.Type != TYPE_String ? Inner.TypeSize() : 0;

					RequireSymbol( L"(", L"pop" );
					RequireSymbol( L")", L"pop" );

					emit( CODE_DynPop );
					emit( ExprRes.iReg );
					emit( InnerSize );

					ExprRes.bLValue = false;
					ExprRes.Type.ArrayDim = 1;
					bValidExpr = true;
					break;
				}
				if( MatchIdentifier(KW_remove) )
				{
					// Remove i'th array item.
					RequireSymbol(L"(", L"remove");
					TExprResult I = CompileExpr( TYPE_Integer, true, false, 0 );
					RequireSymbol(L")", L"remove");

					CTypeInfo Inner( ExprRes.Type.Type, 1, ExprRes.Type.Inner );
					UInt8 InnerSize = Inner.Type != TYPE_String ? Inner.TypeSize() : 0;

					emit( CODE_DynRemove );
					emit( ExprRes.iReg );
					emit( I.iReg );
					emit( InnerSize );

					FreeReg(I.iReg);
					ExprRes.bLValue		= false;
					ExprRes.Type		= TYPE_None;
					bValidExpr			= true;
					break;
				}
				else
					Error(L"Unknown dynamic array method '%s'", *PeekIdentifier());
			}
			else if( ExprRes.Type.Type == TYPE_Struct )
			{
				// Custom structure member.
				String MemberName = GetIdentifier( L"member name" );
				CProperty* Member = ExprRes.Type.Struct->FindMember(*MemberName);
				if( !Member )
					Error( L"'%s' is not a struct member", *MemberName );

				UInt8 Offset = Member->Offset; 

				ExprRes.Type	= *Member;
				ExprRes.bLValue	= true;
				emit( CODE_LMember );
				emit( ExprRes.iReg );
				emit( Offset );
			}
			else if( ExprRes.Type.Type == TYPE_Vector )
			{
				// Vector member.
				String Elem = String::LowerCase(GetIdentifier( L"vector member" ));
				UInt8 Offset = 0xff;
				Offset = Elem == L"x" ? PROPERTY_OFFSET(TVector, X) : Elem == L"y" ? PROPERTY_OFFSET(TVector, Y) : 0xff;
				if( Offset == 0xff )
					Error( L"Unknown vector member '%s'", *Elem );
				ExprRes.Type.Type	= TYPE_Float;
				emit( ExprRes.bLValue ? CODE_LMember : CODE_RMember );
				emit( ExprRes.iReg );
				emit( Offset );
			}
			else if( ExprRes.Type.Type == TYPE_AABB )
			{
				// TRect member.
				String Elem = String::LowerCase(GetIdentifier( L"aabb member" ));
				UInt8 Offset = 0xff;
				Offset = Elem == L"min" ? PROPERTY_OFFSET(TRect, Min) : Elem == L"max" ? PROPERTY_OFFSET(TRect, Max) : 0xff;
				if( Offset == 0xff )
					Error( L"Unknown aabb member '%s'", *Elem );
				ExprRes.Type.Type	= TYPE_Vector;
				emit( ExprRes.bLValue ? CODE_LMember : CODE_RMember );
				emit( ExprRes.iReg );
				emit( Offset );
			}
			else if( ExprRes.Type.Type == TYPE_Color )
			{
				// Color member.
				String Elem = String::LowerCase(GetIdentifier( L"color member" ));
				UInt8 Offset = 0xff;
				Offset =	Elem == L"r" ? PROPERTY_OFFSET(TColor, R) : 
							Elem == L"g" ? PROPERTY_OFFSET(TColor, G) :  
							Elem == L"b" ? PROPERTY_OFFSET(TColor, B) : 
							Elem == L"a" ? PROPERTY_OFFSET(TColor, A) : 0xff;

				if( Offset == 0xff )
					Error( L"Unknown color member '%s'", *Elem );
				ExprRes.Type.Type	= TYPE_Byte;
				emit( ExprRes.bLValue ? CODE_LMember : CODE_RMember );
				emit( ExprRes.iReg );
				emit( Offset );
			}
			else if( ExprRes.Type.Type == TYPE_Resource )
			{
				// Get a resource property.
				if( !ExprRes.Type.Class )
					Error( L"Bad resource class" );

				String MemberName = GetIdentifier( L"resource member" );

				if( CProperty* Prop = ExprRes.Type.Class->FindProperty(*MemberName) )
				{
					// Resource property.
					if( !(Prop->Flags & PROP_Editable) )
						Error( L"Property '%s' is not editable", *MemberName );

						UInt16 Offset = Prop->Offset;
						emit_ltor( ExprRes );
						emit( CODE_ResourceProperty );
						emit( ExprRes.iReg );
						emit( Offset );

						ExprRes.bLValue	= true;
						ExprRes.Type	= *Prop;

						// If property are const, force it to be const.
						if( Prop->Flags & PROP_Const )
						{
							if( Prop->ArrayDim != 1 || Prop->Type == TYPE_Struct )
								Error( L"Constant array or struct is not accessible" );

							emit_ltor( ExprRes );
							ExprRes.bLValue	= false;
						}
				}
				else if( CNativeFunction* Native = ExprRes.Type.Class->FindMethod(*MemberName) )
				{
					// Resource method.
					UInt16 iNative = CClassDatabase::GFuncs.find( Native );
					Array<UInt8> ArgRegs;

					RequireSymbol( L"(", L"resource native method" );
					for( Int32 i=0; i<Native->NumParams; i++ )
					{
						ArgRegs.push( CompileExpr( Native->Params[i].Type, true, false, 0 ).iReg );

						if( i < Native->NumParams-1 )
							RequireSymbol( L",", L"arguments" ); 
					}
					RequireSymbol( L")", L"resource native method" );

					emit_ltor( ExprRes );
					emit( CODE_ResourceMethod );
					emit( ExprRes.iReg );
					emit( iNative );
					for( Int32 i=0; i<ArgRegs.size(); i++ )
					{
						emit( ArgRegs[i] );
						FreeReg( ArgRegs[i] );
					}
					FreeReg(ExprRes.iReg);

					if( Native->ResultType.Type != TYPE_None )
					{
						// Has result.
						ExprRes.Type	= Native->ResultType;
						ExprRes.iReg	= GetReg();
						ExprRes.bLValue	= false;
						emit( ExprRes.iReg );
					}
					else
					{
						// No result.
						ExprRes.bLValue	= false;
						ExprRes.iReg	= -1;
						ExprRes.Type	= TYPE_None;
					}

					bValidExpr	= true;
				}
				else
					Error( L"Member '%s' not found in '%s'", *MemberName, *ExprRes.Type.Class->GetAltName() );
			}
			else if( ExprRes.Type.Type == TYPE_Entity )
			{
				// Compile other entity subexpression.
				emit_ltor(ExprRes);
				if( !CompileEntityExpr( CONT_Other, ExprRes.Type, ExprRes.iReg, ExprRes ) )
					Error( L"Missing entity sub-expression" );
			}
			else
				Error( L"Struct or object type expected" );
		}
		else
			break;
	}


	//	
	// Step 3: Postfix operators.
	//
	if( PeekSymbol() == L"++" || PeekSymbol() == L"--" )
	{
		GetToken( T, false, false );
		CNativeFunction* SuffOp = FindUnaryOp( T.Text, ExprRes.Type );

		if( !SuffOp )
			Error
				( 
					L"Postfix operator '%s' not applicable to %s operand type", 
					*T.Text, 
					*ExprRes.Type.TypeName() 
				);

		if( !ExprRes.bLValue || ExprRes.Type.ArrayDim != 1 || ExprRes.Type.Type == TYPE_Struct )
			Error( L"The right-hand side of an assignment must be a variable" );

		assert(ExprRes.Type.Type != TYPE_String);
		
		// Sorry, its works wrong way!
		// Not real C-style rule :( But it not so bad.
		emit_opcode( SuffOp->iOpCode );
		emit( ExprRes.iReg );

		emit_ltor( ExprRes );

		ExprRes.Type	= SuffOp->ResultType;
		ExprRes.bLValue	= false;
		bValidExpr		= true;
	}

	//
	// Step 4: Assignment.
	//
	if( MatchSymbol( L"=" ) )
	{
		if( !bAllowAssign )
			Error( L"The assignment doesn't allowed here" );

		if( !ExprRes.bLValue )
			Error( L"The left-hand side of an assignment must be a variable" );

		if( ExprRes.Type.ArrayDim != 1 )
			Error( L"An array assignment doesn't allowed" );

		if( ExprRes.Type.Type == TYPE_Struct )
			Error( L"A struct assignment doesn't allowed" );

		// Try delegate construction.
		if( ExprRes.Type.Type == TYPE_Delegate )
		{
			TToken FuncName;
			GetToken( FuncName, false, false );
			if( FuncName.Type == TOK_Identifier )
			{
				CFunction* Func = FindFunction( Script->Methods, FuncName.Text );
				if( Func )
				{
					TDelegateInfo& Info = DelegatesInfo[ExprRes.Type.iSignature];
					if( !Info.MatchSignature(Func) )
						Error( L"Signatures of delegate and function are mismatched" );

					Int32 iFunc = Script->Methods.find(Func);
					assert(iFunc != -1);
					emit(CODE_DelegateCnstr);
					emit(ExprRes.iReg);
					emit(iFunc);

					// Finish him!
					FreeReg( ExprRes.iReg );
					bValidExpr		= true;
					ExprRes.Type	= TYPE_None;
					return ExprRes;
				}
				else
					GotoToken(FuncName);
			}
			else
				GotoToken(FuncName);
		}

		// Compile right side of assignment.
		TExprResult Right = CompileExpr( ExprRes.Type, true, false, 0 );

		// Emit assignment.
		emit_assign( ExprRes, Right );

		// Finish him!
		FreeReg( ExprRes.iReg );
		FreeReg( Right.iReg );

		bValidExpr		= true;
		ExprRes.Type	= TYPE_None;
		return ExprRes;
	}


	//
	// Step 5: Delegate call.
	//
	if( ExprRes.Type.Type == TYPE_Delegate && MatchSymbol(L"(") )
	{
		emit_ltor(ExprRes);
		Array<UInt8>	ArgRegs;

		assert(ExprRes.Type.iSignature != -1);
		TDelegateInfo& Info = DelegatesInfo[ExprRes.Type.iSignature];

		for( Int32 i=0; i<Info.ParamsType.size(); i++ )
		{
			TDelegateInfo::CParameter& P = Info.ParamsType[i];

			if( P.bOut )
			{
				// Output property.
				TExprResult Out = CompileExpr( P, false, false, 0 );
				if( !Out.bLValue )
					Error( L"The right-hand side of an assignment must be a variable" );
				ArgRegs.push( Out.iReg );
			}
			else
			{
				// Regular property.
				ArgRegs.push( CompileExpr( P, true, false, 0 ).iReg );
			}

			if( i != Info.ParamsType.size()-1 )
				RequireSymbol( L",", L"delegate arguments" );
		}
		RequireSymbol( L")", L"delegate call" );

		emit( CODE_CallDelegate );
		emit( ExprRes.iReg );

		for( Int32 i=0; i<ArgRegs.size(); i++ )
		{
			emit( ArgRegs[i] );
			FreeReg( ArgRegs[i] );
		}

		if( Info.ResultType.Type != TYPE_None )
		{
			// Has result.
			FreeReg(ExprRes.iReg);
			ExprRes.Type = Info.ResultType;
			ExprRes.iReg = GetReg();
			ExprRes.bLValue = false;
			emit( ExprRes.iReg );
		}
		else
		{
			// No result.
			ExprRes.bLValue = false;
			ExprRes.iReg = -1;
			ExprRes.Type = TYPE_None;
		}

		bValidExpr	= true;
	}


	//
	// Step 6: Binary operators.
	//
OperLoop:

	// Get operator symbol.
	GetToken( T, false, false );

	if( T.Text == KW_is )
	{
		// 'Is' operator.
		if( ExprRes.Type.Type != TYPE_Entity || ExprRes.Type.ArrayDim != 1 )
			Error( L"Entity type required in 'is' operator" );

		TExprResult ScriptExpr = CompileExpr( TYPE_SCRIPT, true, false );

		emit_ltor( ExprRes );
		emit( CODE_Is );
		emit( ExprRes.iReg );
		emit( ScriptExpr.iReg );

		FreeReg( ScriptExpr.iReg );

		ExprRes.bLValue		= false;
		ExprRes.Type.Type	= TYPE_Bool;
		goto OperLoop;
	}	
	else if( T.Text == KW_in )
	{
		// 'In' operator.
		if( ExprRes.Type.Type != TYPE_Entity || ExprRes.Type.ArrayDim != 1 )
			Error( L"Entity type required in 'in' operator" );

		String FamilyName = GetIdentifier( L"family name" );
		CFamily* Family = FindFamily( FamilyName );
		if( !Family )
			Error( L"Family '%s' not found", *FamilyName );
		assert(Family->Scripts.size()>0);

		emit_ltor( ExprRes );
		emit( CODE_In );
		emit( ExprRes.iReg );
		emit( Family->iFamily );	

		ExprRes.bLValue		= false;
		ExprRes.Type.Type	= TYPE_Bool;
		goto OperLoop;
	}
	else if( ( T.Text == L"==" || T.Text == L"!=" ) && InPri < 7  )
	{
		// Comparison operators.
		if( ExprRes.Type.ArrayDim != 1 )
			Error( L"Arrays comparison doesn't allowed" );
		if( ExprRes.Type.Type == TYPE_Struct )
			Error( L"Structs comparison doesn't allowed" );

		emit_ltor( ExprRes );

		TExprResult Other = CompileExpr( ExprRes.Type, true, false, 7 );
		UInt8 VarSize = ExprRes.Type.Type == TYPE_String ? 0 : ExprRes.Type.TypeSize(true);

		emit( T.Text == L"==" ? CODE_Equal : CODE_NotEqual );
		emit( ExprRes.iReg );
		emit( Other.iReg );
		emit( VarSize );

		FreeReg( Other.iReg );
		ExprRes.bLValue		= false;
		ExprRes.Type		= TYPE_Bool;
		goto OperLoop;
	}
	else if( ( T.Text == L"&&" && InPri < 3 ) || ( T.Text == L"||" && InPri < 2 ) )
	{
		// Short circuit operators.
		Bool	bAnd	= T.Text == L"&&";
		Int32 Prior	= bAnd ? 3 : 2;

		if( ExprRes.Type.Type != TYPE_Bool || ExprRes.Type.ArrayDim != 1 )
			Error( L"Bool type expected in '%s' operator", *T.Text );

		emit_ltor( ExprRes );

		if( bAnd )
		{
			// Logical 'and'.
			UInt16 DstExpr1, DstExpr2, DstOut;
			TExprResult Result;
			Result.bLValue			= false;
			Result.iReg				= GetReg();
			Result.Type				= TYPE_Bool;

			emit( CODE_JumpZero );
			DstExpr1	 = Emitter.Tell();
			emit( GTempWord );
			emit( ExprRes.iReg );

			TExprResult Second = CompileExpr( TYPE_Bool, true, false, Prior );

			emit( CODE_JumpZero );
			DstExpr2	= Emitter.Tell();
			emit( GTempWord );
			emit( Second.iReg );

			// Emit true.
			Bool bTrue = true;
			emit( CODE_ConstBool );
			emit( bTrue );
			emit( Result.iReg );

			emit( CODE_Jump );
			DstOut	= Emitter.Tell();
			emit( GTempWord );

			*(UInt16*)&Bytecode->Code[DstExpr1]	= Emitter.Tell();
			*(UInt16*)&Bytecode->Code[DstExpr2]	= Emitter.Tell();

			// Emit false.
			Bool bFalse = false;
			emit( CODE_ConstBool );
			emit( bFalse );
			emit( Result.iReg );

			*(UInt16*)&Bytecode->Code[DstOut]	= Emitter.Tell();

			FreeReg( ExprRes.iReg );
			FreeReg( Second.iReg );
			ExprRes	= Result;
		}
		else
		{
			// Logical 'or'.
			UInt16 DstOut1, DstOut2, DstExpr2, DstZr;
			Bool bTrue = true, bFalse = false;
			TExprResult Result;
			Result.bLValue			= false;
			Result.iReg				= GetReg();
			Result.Type				= TYPE_Bool;

			emit( CODE_JumpZero );
			DstExpr2	 = Emitter.Tell();
			emit( GTempWord );
			emit( ExprRes.iReg );

			emit( CODE_ConstBool );
			emit( bTrue );
			emit( Result.iReg );

			emit( CODE_Jump );
			DstOut1		= Emitter.Tell();
			emit( GTempWord );

			*(UInt16*)&Bytecode->Code[DstExpr2]	= Emitter.Tell();
			TExprResult Second = CompileExpr( TYPE_Bool, true, false, Prior );

			emit( CODE_JumpZero );
			DstZr	 = Emitter.Tell();
			emit( GTempWord );
			emit( Second.iReg );

			emit( CODE_ConstBool );
			emit( bTrue );
			emit( Result.iReg );

			emit( CODE_Jump );
			DstOut2		= Emitter.Tell();
			emit( GTempWord );

			*(UInt16*)&Bytecode->Code[DstZr]	= Emitter.Tell();
			emit( CODE_ConstBool );
			emit( bFalse );
			emit( Result.iReg );

			*(UInt16*)&Bytecode->Code[DstOut1]	= Emitter.Tell();
			*(UInt16*)&Bytecode->Code[DstOut2]	= Emitter.Tell();

			FreeReg( ExprRes.iReg );
			FreeReg( Second.iReg );
			ExprRes	= Result;
		}

		goto OperLoop;
	}
	else
	{
		// Try to find standard.
		CNativeFunction* Oper = FindBinaryOp( T.Text, TYPE_None, TYPE_None );

		// Test operator and priority.
		if( Oper && InPri < Oper->Priority )
		{
			// Compile second expression to figure out exactly operator
			// since Oper is wrong, due operator overloading.
			TExprResult Second = CompileExpr( TYPE_None, true, false, Oper->Priority );
			if( Second.Type.Type == TYPE_None || Second.Type.ArrayDim != 1 || Second.Type.Type == TYPE_Struct )
				Error( L"Bad second argument in operator '%s'", *T.Text );

			// Search second time to find exactly.
			Oper	= FindBinaryOp( T.Text, ExprRes.Type, Second.Type );
			if( !Oper )
				Error( L"Operator '%s' not applicable to '%s' and '%s'", *T.Text, *ExprRes.Type.TypeName(), *Second.Type.TypeName() );

			if( Oper->Flags & NFUN_AssignOp )
			{
				// It's an assignment operator, such as += or <<=.
				if( !ExprRes.bLValue )
					Error( L"The left-hand side of an assignment must be a variable" );

				// No cast required for first.
				assert( Oper->Params[0].Type.Type == ExprRes.Type.Type );

				// Cast second if any.
				if( Second.Type.Type != Oper->Params[1].Type.Type )
				{
					UInt8 Cast = GetCast( Second.Type.Type, Oper->Params[0].Type.Type );
					emit( Cast );
					emit( Second.iReg );
				}

				emit_opcode( Oper->iOpCode );
				emit( ExprRes.iReg );
				emit( Second.iReg );
				FreeReg( Second.iReg );

				// Result are r value.
				ExprRes.bLValue	= false;
				ExprRes.Type	= Oper->ResultType;

				bValidExpr	= true;
			}
			else
			{
				// It's a regular operator.
				emit_ltor( ExprRes );

				// Apply cast if any.
				if( ExprRes.Type.Type != Oper->Params[0].Type.Type )
				{
					UInt8 Cast = GetCast( ExprRes.Type.Type, Oper->Params[0].Type.Type );
					emit( Cast );
					emit( ExprRes.iReg );
				}
				if( Second.Type.Type != Oper->Params[1].Type.Type )
				{
					UInt8 Cast = GetCast( Second.Type.Type, Oper->Params[0].Type.Type );
					emit( Cast );
					emit( Second.iReg );
				}

				// Emit operator.
				emit_opcode( Oper->iOpCode );
				emit( ExprRes.iReg );
				emit( Second.iReg );
				FreeReg( Second.iReg );

				// Store result.
				ExprRes.Type	= Oper->ResultType;
				ExprRes.bLValue	= false;
			}

			goto OperLoop;
		}
		else
		{
			// It's not an operator, or let over one CompileExpr handle it.
			GotoToken( T );
		}	
	}


	//
	// Step 7: Ternary conditional.
	//
	if( InPri == 0 && MatchSymbol(L"?") )
	{
		// Bool expression.
		emit_ltor( ExprRes );
		if( ExprRes.Type.Type != TYPE_Bool || ExprRes.Type.ArrayDim != 1 )
			Error( L"Bool type expected in '?' operator" );

		UInt16 DstExpr2, DstOut;

		emit( CODE_JumpZero );
		DstExpr2		= Emitter.Tell();
		emit( GTempWord );
		emit( ExprRes.iReg );

		TExprResult First = CompileExpr( TYPE_None, true, false, 0 );
		if( First.Type.Type == TYPE_None || First.Type.ArrayDim != 1 || First.Type.Type == TYPE_Struct )
			Error( L"Bad ternary operand type" );

		emit( CODE_Jump );
		DstOut			= Emitter.Tell();
		emit( GTempWord );

		RequireSymbol( L":", L"ternary operator" );
		*(UInt16*)&Bytecode->Code[DstExpr2]	= Emitter.Tell();
		TExprResult Second = CompileExpr( First.Type, true, false, 0 );

		*(UInt16*)&Bytecode->Code[DstOut]	= Emitter.Tell();

		TExprResult Result;
		Result.bLValue		= false;
		Result.iReg			= GetReg();
		Result.Type			= First.Type;

		emit( CODE_ConditionalOp );
		emit( Result.iReg );
		emit( First.iReg );
		emit( Second.iReg );
		emit( ExprRes.iReg );

		FreeReg( ExprRes.iReg );
		FreeReg( First.iReg );
		FreeReg( Second.iReg );
		ExprRes	= Result;
	}


	//
	// Step 8: Implicit type casting.
	//
	if( ExprRes.bLValue && bForceR )
	{
		// Force result to be r-value.
		if( ExprRes.Type.ArrayDim != 1 )
			Error( L"Arrays is not allowed here" );
		if( ExprRes.Type.Type == TYPE_Struct )
			Error( L"Structs is not allowed here" );

		emit_ltor( ExprRes );
	}

	if( ReqType.Type == TYPE_None || ExprRes.Type.MatchWith(ReqType) /*bad show -> */ || ReqType.MatchWith(ExprRes.Type) )			
	{
		// Result type, doesn't really matter, or types are matched.
		// Expression compilation ok.
		return ExprRes;
	} 
	else
	{
		// Typecast required.
		emit_ltor( ExprRes );
		
		// Try to apply implicit typecast.
		UInt8 Cast = GetCast( ExprRes.Type.Type, ReqType.Type );

		if( Cast != 0x00 )
		{
			// Found cast.
			emit( Cast );
			emit( ExprRes.iReg );
			ExprRes.Type = ReqType;
			return ExprRes;
		}
		else
		{
			// Error in expression.
			Error( L"Incompatible types: '%s' and '%s'", *ReqType.TypeName(), *ExprRes.Type.TypeName() );
			return ExprRes;
		}
	}
}


//
// Compile functional cast sub-expression.
//
Bool CCompiler::CompileFuncCastExpr( const TToken& ToType, TExprResult& Result )
{
	if( ToType.Type != TOK_Identifier )
		return false;

	// Retrieve destination type.
	CTypeInfo DestType;
	
	if( ToType.Text == KW_byte )				DestType = TYPE_Byte;
	else if( ToType.Text == KW_bool )			DestType = TYPE_Bool;
	else if( ToType.Text == KW_integer )		DestType = TYPE_Integer;
	else if( ToType.Text == KW_float )			DestType = TYPE_Float;
	else if( ToType.Text == KW_angle )			DestType = TYPE_Angle;
	else if( ToType.Text == KW_color )			DestType = TYPE_Color;
	else if( ToType.Text == KW_string )			DestType = TYPE_String;
	else if( ToType.Text == KW_vector )			DestType = TYPE_Vector;
	else if( ToType.Text == KW_aabb )			DestType = TYPE_AABB;
	else if( ToType.Text == KW_entity )			DestType = TYPE_Entity;
	else
		return false;

	// Compile subexpression.
	RequireSymbol( L"(", L"functional cast" );
	TExprResult SourceValue = CompileExpr( TYPE_None, true, false, 0 );
	RequireSymbol( L")", L"functional cast" );

	// Detect no-type.
	if( SourceValue.Type.Type == TYPE_None )
		Error(L"'void' cannot be converted");

	// Check for matching types.
	if( SourceValue.Type.MatchWith(DestType) )
	{
		Warn( L"Convertation to the same type" );
		Result = SourceValue;
		return true;
	}

	// Matrix of typecast.
	UInt8 CastMatrix[TYPE_MAX/* SourceType */][TYPE_MAX/* DestinationType */] =
	{
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // TYPE_None;
		{0, 0, CAST_ByteToBool, CAST_ByteToInteger, CAST_ByteToFloat, 0, 0, CAST_ByteToString, 0, 0, 0, 0, 0, 0}, // TYPE_Byte;
		{0, 0, 0, CAST_BoolToInteger, 0, 0, 0, CAST_BoolToString, 0, 0, 0, 0, 0, 0}, // TYPE_Bool;
		{0, CAST_IntegerToByte, CAST_IntegerToBool, 0, CAST_IntegerToFloat, CAST_IntegerToAngle, CAST_IntegerToColor, CAST_IntegerToString, 0, 0, 0, 0, 0, 0}, // TYPE_Integer;
		{0, CAST_FloatToByte, CAST_FloatToBool, CAST_FloatToInteger, 0, CAST_FloatToAngle, 0, CAST_FloatToString, 0, 0, 0, 0, 0, 0}, // TYPE_Float;
		{0, 0, 0, CAST_AngleToInteger, CAST_AngleToFloat, 0, 0, CAST_AngleToString, CAST_AngleToVector, 0, 0, 0, 0, 0}, // TYPE_Angle;
		{0, 0, 0, CAST_ColorToInteger, 0, 0, 0, CAST_ColorToString, 0, 0, 0, 0, 0, 0}, // TYPE_Color;
		{0, CAST_StringToByte, CAST_StringToBool, CAST_StringToInteger, CAST_StringToFloat, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // TYPE_String;
		{0, 0, CAST_VectorToBool, 0, 0, CAST_VectorToAngle, 0, CAST_VectorToString, 0, 0, 0, 0, 0, 0}, // TYPE_Vector;
		{0, 0, CAST_AabbToBool, 0, 0, 0, 0, CAST_AabbToStrnig, 0, 0, 0, 0, 0, 0}, // TYPE_AABB;
		{0, 0, CAST_ResourceToBool, 0, 0, 0, 0, CAST_ResourceToString, 0, 0, 0, 0, 0, 0}, // TYPE_Resource;
		{0, 0, CAST_EntityToBool, 0, 0, 0, 0, CAST_EntityToString, 0, 0, 0, 0, 0, 0}, // TYPE_Entity;
		{0, 0, CAST_DelegateToBool, 0, 0, 0, 0, CAST_DelegateToString, 0, 0, 0, 0, 0, 0}, // TYPE_Delegate;
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} // TYPE_Struct;
	};

	UInt8 OpCode = CastMatrix[SourceValue.Type.Type][DestType.Type];
	if( OpCode == 0 )
	{
		Error( L"'%s' cannot be converted to '%s'", *SourceValue.Type.TypeName(), *DestType.TypeName() );
	}

	emit(OpCode);
	emit(SourceValue.iReg);

	Result.bLValue = false;
	Result.iReg = SourceValue.iReg;
	Result.Type = DestType;
	return true;
}


//
// Compile prototype variable expression.
//
TExprResult CCompiler::CompileProtoExpr( FScript* Prototype )
{
	assert(Prototype);
	assert(!Prototype->IsStatic());

	// Getting start.
	TToken T;
	CProperty* Property;
	TExprResult Result;
	RequireSymbol( L".", L"proto variable" );
	GetToken( T, false, false );

	if( Property = FindProperty( Prototype->Properties, T.Text ) )
	{
		// Instance buffer value.
		UInt8 iSource	= 0xff;
		UInt16 Offset	= Property->Offset;

		Result.bLValue	= true;
		Result.iReg		= GetReg();
		Result.Type		= *Property;

		if( !Result.Type.IsSimpleType() )
			Error( L"Only simple proto variables allowed" );

		emit( CODE_ProtoProperty );
		emit( Prototype );
		emit( iSource );
		emit( Result.iReg );
		emit( Offset );
		emit_ltor(Result);
	}
	else if( Property = Prototype->Base->GetClass()->FindProperty(*T.Text) )
	{
		// Base property.
		UInt8 iSource	= 0xfe;
		UInt16 Offset	= Property->Offset;

		Result.bLValue	= true;
		Result.iReg		= GetReg();
		Result.Type		= *Property;

		if( !Result.Type.IsSimpleType() )
			Error( L"Only simple proto variables allowed" );

		emit( CODE_ProtoProperty );
		emit( Prototype );
		emit( iSource );
		emit( Result.iReg );
		emit( Offset );
		emit_ltor(Result);
	}
	else if( T.Text==KW_base || T.Text==L"$" )
	{
		// Component property.
		FComponent* Component;
		UInt8 iSource = 0xfe;
		if( T.Text == L"$" )
		{
			// Extra component.
			String Name = GetIdentifier( L"component name" );
			Component	= Prototype->FindComponent( Name );
			if( !Component )
				Error( L"Component '%s' not found in '%s'", *Name, *Prototype->GetName() );
			iSource		= Prototype->Components.find( (FExtraComponent*)Component );
		}
		else
		{
			// Base component, deprecated access.
			Component	= Prototype->Base;
			iSource		= 0xfe;
		}

		RequireSymbol( L".", L"component field" );
		CClass*	Class	= Component->GetClass();
		String Field	= GetIdentifier( L"component field" );

		if( Property = Class->FindProperty(*Field) )
		{
			UInt16 Offset		= Property->Offset;

			Result.bLValue	= true;
			Result.iReg		= GetReg();
			Result.Type		= *Property;

			if( !Result.Type.IsSimpleType() )
				Error( L"Only simple proto variables allowed" );

			emit( CODE_ProtoProperty );
			emit( Prototype );
			emit( iSource );
			emit( Result.iReg );
			emit( Offset );
			emit_ltor(Result);
		}
		else
			Error( L"Unknown component '%s' property '%s'", *Class->GetAltName(), *Field );
	}
	else
		Error( L"Proto property expected, got '%s'", *T.Text );

	return Result;
}


//
// Emit context switch if required.
//
#define emit_context( contype, conreg )\
if( contype == CONT_This )\
{\
	/* This expression in 'this' context.*/\
	if( Context != CONT_This )\
	{\
		emit( CODE_ThisContext );\
		Context	= CONT_This;\
	}	\
}\
else\
{\
	/* Other context, switch it anyway.*/ \
	assert(conreg != 0xff);\
	emit( CODE_Context );\
	emit( conreg );\
	FreeReg( conreg );\
	Context	= CONT_Other;\
}\


//
// Compile an entity subexpression such as components
// relative and functions call. Return true, if parsing 
// was successfully.
//
// Here:
//	- InContext: an entity context, since about 90% of work in 'this' context it increase speed.
//	- Entity: type of entity script and/or family.
//	- iConReg: index of entity context register, if 'this' use 0xff.
//	- Result: gotten result type, it also possible TYPE_None in functions.
//
Bool CCompiler::CompileEntityExpr( EEntityContext InContext, CTypeInfo Entity, UInt8 iConReg, TExprResult& Result )
{
#if 0
	// Does we have something to start with?
	if( !Entity.Script && Entity.iFamily==-1 )
		return false;
#else
	// If we have untyped entity without event family.
	if( !Entity.Script && Entity.iFamily==-1 )
	{
		CProperty*			Property;
		CNativeFunction*	Native;

		TToken T;
		GetToken( T, false, false );

		// Skip 'base', if any.
		if( T.Text == KW_base )
		{
			RequireSymbol( L".", L"base" );
			GetToken( T, false, false );
		}

		// Decide what we got...
		if( Property = FBaseComponent::MetaClass->FindProperty(*T.Text) )
		{
			// An base property from the entity context. 
			// Kinda of symbiosis.
			UInt16 Offset		= Property->Offset;

			Result.bLValue	= true;
			Result.iReg		= GetReg();
			Result.Type		= *Property;

			emit_context( InContext, iConReg );
			emit( CODE_BaseProperty );
			emit( Result.iReg );
			emit( Offset );

			// If property are const, force it to be const.
			if( Property->Flags & PROP_Const )
			{
				if( !Property->IsSimpleType() )
					Error( L"Only simple const values allowed" );

				emit_ltor( Result );
				Result.bLValue	= false;
			}

			return true;
		}
		else if( Native = FBaseComponent::MetaClass->FindMethod(*T.Text) )
		{
			// Native method call from base component.
			UInt16 iNative = CClassDatabase::GFuncs.find( Native );
			Array<UInt8> ArgRegs;

			RequireSymbol( L"(", L"native method" );
			for( Int32 i=0; i<Native->NumParams; i++ )
			{
				ArgRegs.push( CompileExpr( Native->Params[i].Type, true, false, 0 ).iReg );

				if( i < Native->NumParams-1 )
					RequireSymbol( L",", L"arguments" ); 
			}
			RequireSymbol( L")", L"native method" );

			emit_context( InContext, iConReg );
			emit( CODE_BaseMethod );
			emit( iNative );
			for( Int32 i=0; i<ArgRegs.size(); i++ )
			{
				emit( ArgRegs[i] );
				FreeReg( ArgRegs[i] );
			}

			if( Native->ResultType.Type != TYPE_None )
			{
				// Has result.
				Result.Type		= Native->ResultType;
				Result.iReg		= GetReg();
				Result.bLValue	= false;
				emit( Result.iReg );
			}
			else
			{
				// No result.
				Result.bLValue	= false;
				Result.iReg		= -1;
				Result.Type		= TYPE_None;
			}

			bValidExpr	= true;
			return true;
		}
		else
		{
			Error( L"Unknown field '%s' in abstract base", *T.Text );
			return false;
		}
	}
#endif

	// Grab a token to recognize and store code location
	// for rollback in case of failure.
	UInt16 CodeStart = Emitter.Tell();
	TToken T;
	GetToken( T, false, false );
	/*
	// Switch context if required.
	if( InContext == CONT_This )
	{
		// This expression in 'this' context.
		if( Context != CONT_This )
		{
			Byte iReg = GetReg();
			emit( CODE_This );
			emit( iReg );
			emit( CODE_Context );
			emit( iReg );
			FreeReg( iReg );
		}
	}
	else
	{
		// Other context, switch it anyway.
		assert(iConReg != 0xff);
		emit( CODE_Context );
		emit( iConReg );
		FreeReg( iConReg );
	}		
	*/

	// Objects.
	FScript* ConScript = Entity.Script;
	CFamily* ConFamily = ConScript ? (ConScript->iFamily!=-1 ? Families[ConScript->iFamily] : nullptr) : (Entity.iFamily!=-1 ? Families[Entity.iFamily] : nullptr);
	CProperty* Property;
	CFunction* Function;
	CNativeFunction*	Native;
	Int32	 iUnified;

	if( ConScript && (Property = FindProperty( ConScript->Properties, T.Text )) )
	{
		// An entity property.
		Result.bLValue	= true;
		Result.iReg		= GetReg();
		Result.Type		= *Property;

		emit_context( InContext, iConReg );

		UInt16 Offset	= Property->Offset;
		emit( CODE_EntityProperty );
		emit( Result.iReg );
		emit( Offset );

		return true;
	}
	else if( ConScript && (Property = ConScript->Base->GetClass()->FindProperty(*T.Text)) )
	{
		// An base property from the entity context. 
		// Kinda of symbiosis.
		UInt16 Offset		= Property->Offset;

		Result.bLValue	= true;
		Result.iReg		= GetReg();
		Result.Type		= *Property;

		emit_context( InContext, iConReg );
		emit( CODE_BaseProperty );
		emit( Result.iReg );
		emit( Offset );

		// If property are const, force it to be const.
		if( Property->Flags & PROP_Const )
		{
			if( !Property->IsSimpleType() )
				Error( L"Only simple const values allowed" );

			emit_ltor( Result );
			Result.bLValue	= false;
		}

		return true;
	}
	else if( ConScript && (Function = FindFunction(ConScript->Methods, T.Text)) )
	{
		// An script function, doesn't matter it regular or unified cause
		// we know actual script!
		UInt8			iFunc = ConScript->Methods.find(Function);
		Array<UInt8>	ArgRegs;

		RequireSymbol( L"(", L"method call" );
		for( Int32 i=0; i<Function->ParmsCount; i++ )
		{
			CProperty* Param = Function->Locals[i];

			if( Param->Flags & PROP_OutParm )
			{
				// Output property.
				TExprResult Out = CompileExpr( *Param, false, false, 0 );
				if( !Out.bLValue )
					Error( L"The right-hand side of an assignment must be a variable" );
				ArgRegs.push( Out.iReg );
			}
			else
			{
				// Regular property.
				ArgRegs.push( CompileExpr( *Param, true, false, 0 ).iReg );
			}

			if( i != Function->ParmsCount-1 )
				RequireSymbol( L",", L"method arguments" );
		}
		RequireSymbol( L")", L"method call" );

		emit_context( InContext, iConReg );
		emit( CODE_CallMethod );
		emit( iFunc );

		for( Int32 i=0; i<ArgRegs.size(); i++ )
		{
			emit( ArgRegs[i] );
			FreeReg( ArgRegs[i] );
		}

		if( Function->ResultVar )
		{
			// Has result.
			Result.Type		= *Function->ResultVar;
			Result.iReg		= GetReg();
			Result.bLValue	= false;
			emit( Result.iReg );
		}
		else
		{
			// No result.
			Result.bLValue	= false;
			Result.iReg		= -1;
			Result.Type		= TYPE_None;
		}

		bValidExpr	= true;
		return true;
	}
	else if( ConScript && (Native = ConScript->Base->GetClass()->FindMethod(*T.Text)) )
	{
		// Native method call from base component.
		UInt16 iNative = CClassDatabase::GFuncs.find( Native );
		Array<UInt8> ArgRegs;

		RequireSymbol( L"(", L"native method" );
		for( Int32 i=0; i<Native->NumParams; i++ )
		{
			ArgRegs.push( CompileExpr( Native->Params[i].Type, true, false, 0 ).iReg );

			if( i < Native->NumParams-1 )
				RequireSymbol( L",", L"arguments" ); 
		}
		RequireSymbol( L")", L"native method" );

		emit_context( InContext, iConReg );
		emit( CODE_BaseMethod );
		emit( iNative );
		for( Int32 i=0; i<ArgRegs.size(); i++ )
		{
			emit( ArgRegs[i] );
			FreeReg( ArgRegs[i] );
		}

		if( Native->ResultType.Type != TYPE_None )
		{
			// Has result.
			Result.Type		= Native->ResultType;
			Result.iReg		= GetReg();
			Result.bLValue	= false;
			emit( Result.iReg );
		}
		else
		{
			// No result.
			Result.bLValue	= false;
			Result.iReg		= -1;
			Result.Type		= TYPE_None;
		}

		bValidExpr	= true;
		return true;
	}
	else if( ConFamily && (iUnified = ConFamily->VFNames.find(T.Text)) != -1 )
	{
		// VF function call.
		CFunction*		Proto = ConFamily->Proto[iUnified];
		UInt8			Index = iUnified;
		Array<UInt8>	ArgRegs;

		RequireSymbol( L"(", L"method call" );
		for( Int32 i=0; i<Proto->ParmsCount; i++ )
		{
			ArgRegs.push( CompileExpr( *Proto->Locals[i], true, false, 0 ).iReg );

			if( i != Proto->ParmsCount-1 )
				RequireSymbol( L",", L"method arguments" );
		}
		RequireSymbol( L")", L"method call" );

		emit_context( InContext, iConReg );
		emit( CODE_CallVF );
		emit( Index );

		for( Int32 i=0; i<ArgRegs.size(); i++ )
		{
			emit( ArgRegs[i] );
			FreeReg( ArgRegs[i] );
		}

		if( Proto->ResultVar )
		{
			// Has result.
			Result.Type		= *Proto->ResultVar;
			Result.iReg		= GetReg();
			Result.bLValue	= false;
			emit( Result.iReg );
		}
		else
		{
			// No result.
			Result.bLValue	= false;
			Result.iReg		= -1;
			Result.Type		= TYPE_None;
		}

		bValidExpr	= true;
		return true;
	}
	else if( ConScript && (T.Text==KW_base || T.Text==L"$") )
	{
		// Component property or method call.
		FComponent* Component;
		UInt8 iCompon = 0xff;
		if( T.Text == L"$" )
		{
			// Extra component.
			String Name = GetIdentifier( L"component name" );
			Component	= ConScript->FindComponent( Name );

			if( !Component )
				Error( L"Component '%s' not found in '%s'", *Name, *ConScript->GetName() );

			iCompon		= ConScript->Components.find( (FExtraComponent*)Component );
		}
		else
		{
			// Base component, deprecated access.
			Component	= ConScript->Base;
			iCompon		= 0xff;
		}

		RequireSymbol( L".", L"component field" );
		CClass*	Class	= Component->GetClass();
		String Field	= GetIdentifier( L"component field" );

		if( Property = Class->FindProperty(*Field) )
		{
			// Component property.
			UInt16 Offset		= Property->Offset;

			Result.bLValue	= true;
			Result.iReg		= GetReg();
			Result.Type		= *Property;

			emit_context( InContext, iConReg );
			if( Class->IsA(FBaseComponent::MetaClass) )
			{
				emit( CODE_BaseProperty );
			}
			else
			{
				emit( CODE_ComponentProperty );
				emit( iCompon );
			}
			emit( Result.iReg );
			emit( Offset );

			// If property are const, force it to be const.
			if( Property->Flags & PROP_Const )
			{
				if( !Property->IsSimpleType() )
					Error( L"Only simple const values allowed" );

				emit_ltor( Result );
				Result.bLValue	= false;
			}
		}
		else if( Native = Class->FindMethod(*Field) )
		{
			// Native function call.
			UInt16 iNative = CClassDatabase::GFuncs.find( Native );
			Array<UInt8> ArgRegs;

			RequireSymbol( L"(", L"native method" );
			for( Int32 i=0; i<Native->NumParams; i++ )
			{
				ArgRegs.push( CompileExpr( Native->Params[i].Type, true, false, 0 ).iReg  );
				if( i < Native->NumParams-1 )
					RequireSymbol( L",", L"arguments" ); 
			}
			RequireSymbol( L")", L"native method" );

			emit_context( InContext, iConReg );
			if( Class->IsA(FBaseComponent::MetaClass) )
			{
				emit( CODE_BaseMethod );
				emit( iNative );
			}
			else
			{
				emit( CODE_ComponentMethod );
				emit( iNative );
				emit( iCompon );
			}

			for( Int32 i=0; i<ArgRegs.size(); i++ )
			{
				emit( ArgRegs[i] );
				FreeReg( ArgRegs[i] );
			}

			if( Native->ResultType.Type != TYPE_None )
			{
				// Has result.
				Result.Type		= Native->ResultType;
				Result.iReg		= GetReg();
				Result.bLValue	= false;
				emit( Result.iReg );
			}	
			else
			{
				// No result.
				Result.bLValue	= false;
				Result.iReg		= -1;
				Result.Type		= TYPE_None;
			}

			bValidExpr	= true;
		}
		else
			Error( L"Field '%s' not found in '%s'", *Field, *Class->GetAltName() );

		return true;
	}

	// Failed parse entity subexpression, rollback
	// and return false.
	Emitter.Bytecode->Code.setSize( CodeStart );
	GotoToken( T );
	return false;
}


//
// Get implicit cast code, if types are matched or no cast return
// 0x00, otherwise return code of cast.
//
UInt8 CCompiler::GetCast( EPropType Src, EPropType Dst )
{
	if( Src == TYPE_Byte	&& Dst == TYPE_Integer )
			return CAST_ByteToInteger;

	if( Src == TYPE_Byte	&& Dst == TYPE_Float )
			return CAST_ByteToFloat;

	if( Src == TYPE_Integer	&& Dst == TYPE_Float )
			return CAST_IntegerToFloat;

	if( Src == TYPE_Integer	&& Dst == TYPE_Byte )
			return CAST_IntegerToByte;

	if( Src == TYPE_Integer	&& Dst == TYPE_Angle )
			return CAST_IntegerToAngle;

	if( Src == TYPE_Angle	&& Dst == TYPE_Integer )
			return CAST_AngleToInteger;

	return 0x00;
}


/*-----------------------------------------------------------------------------
    Statements.
-----------------------------------------------------------------------------*/

//
// Compile statement between {..}, or just 
// single operator.
//
void CCompiler::CompileStatement()
{
	// Reset current context, its messy a little, but
	// may cause crash in really rare cases.
	Context	= CONT_Other;

	Bool bSingleLine = !MatchSymbol(L"{");

	do 
	{
		if( MatchSymbol(L"}") )
		{
			if( !bSingleLine )
				break;
			else
				Error( L"Unexpected '}'" );
		}

		if( PeekIdentifier() == KW_if )
		{
			CompileIf();
		}
		else if( PeekIdentifier() == KW_for )
		{
			CompileFor();
		}
		else if( PeekIdentifier() == KW_foreach )
		{
			CompileForeach();
		}
		else if( PeekIdentifier() == KW_do )
		{
			CompileDo();
		}
		else if( PeekIdentifier() == KW_while )
		{
			CompileWhile();
		}
		else if( PeekIdentifier() == KW_switch )
		{
			CompileSwitch();
		}
		else if( PeekSymbol() == L"{" )
		{
			// Sub nest.
			CompileStatement();
		}
		else if(	PeekIdentifier() == KW_break ||
					PeekIdentifier() == KW_continue ||
					PeekIdentifier() == KW_return ||
					PeekIdentifier() == KW_wait ||
					PeekIdentifier() == KW_sleep ||	
					PeekIdentifier() == KW_interrupt ||	
					PeekIdentifier() == KW_goto ||
					PeekIdentifier() == KW_stop	)
		{
			// Flow command.
			CompileCommand();
		}
		else if( MatchSymbol(L"@") )
		{
			// A thread label.
			if( Bytecode != Script->Thread || Script->IsStatic() /*|| NestTop > 1*/ )
				Error( L"Labels is not allowed here" );

			CThreadCode* Thread = (CThreadCode*)Bytecode;
			String Name	= GetIdentifier( L"thread label" );
			Int32 iLabel;

			if( (iLabel = Thread->GetLabelId(*Name)) == -1 )
				Error( L"Label '%s' not found", *Name );

			// Set address of label.
			Thread->Labels[iLabel].Address	= Emitter.Tell();
			RequireSymbol( L":", L"label" );
		}
		else if	( 
					Bytecode != Script->Thread && 
					CompileVarDecl
								( 
									((CFunction*)Bytecode)->Locals, 
									((CFunction*)Bytecode)->FrameSize,
									PROP_None,
									false,
									false
								) 
				)
		{
			// Local variables compiled.
		}
		else
		{
			// Just expression.
			bValidExpr	= false;
			ResetRegs();
			CompileExpr( TYPE_None, false, true, 0 );

			if( bValidExpr == false )
				Error( L"Bad expression" );

			RequireSymbol( L";", L"statement" );
		}
	} while( !bSingleLine );
}


//
// Compile 'if' statement.
//
void CCompiler::CompileIf()
{
	assert(MatchIdentifier(KW_if));
	PushNest(NEST_If);

	UInt16 DstElse;

	// Logical condition.
	RequireSymbol( L"(", L"if" );
	{
		ResetRegs();
		UInt8 iReg = CompileExpr( TYPE_Bool, true, false, 0 ).iReg;

		// Emit header.
		emit( CODE_JumpZero );
		DstElse	= Emitter.Tell();
		emit( GTempWord );
		emit( iReg );	
	}
	RequireSymbol( L")", L"if" );

	// 'then' statement.
	CompileStatement();

	// Optional 'else' statement.
	if( MatchIdentifier(KW_else) )
	{
		// With 'else'.
		emit( CODE_Jump );
		UInt16 DstEnd	= Emitter.Tell();
		emit( GTempWord );

		*(UInt16*)&Bytecode->Code[DstElse]	= Emitter.Tell();
		CompileStatement();
		*(UInt16*)&Bytecode->Code[DstEnd]		= Emitter.Tell();
	}
	else
	{
		// Without 'else'.
		*(UInt16*)&Bytecode->Code[DstElse]	= Emitter.Tell();
	}

	PopNest();
}


//
// Compile 'while' statement.
//
void CCompiler::CompileWhile()
{
	assert(MatchIdentifier(KW_while));
	PushNest(NEST_While);

	UInt16 AddrStart, DstEnd;

	// Logical condition.
	RequireSymbol( L"(", L"while" );
	{
		AddrStart	= Emitter.Tell();

		ResetRegs();
		UInt8 iReg = CompileExpr( TYPE_Bool, true, false, 0 ).iReg;

		// Emit header.
		emit( CODE_JumpZero );
		DstEnd	= Emitter.Tell();
		emit( GTempWord );
		emit( iReg );	
	}
	RequireSymbol( L")", L"while" );

	// statement.
	CompileStatement();

	// Post statement stuff.
	emit( CODE_Jump );
	emit( AddrStart );
	*(UInt16*)&Bytecode->Code[DstEnd]	= Emitter.Tell();

	// Fix & pop nest.
	ReplNest( REPL_Break, Emitter.Tell() );
	ReplNest( REPL_Continue, AddrStart );
	PopNest();
}


//
// Compile 'do' statement.
//
void CCompiler::CompileDo()
{
	assert(MatchIdentifier(KW_do));
	PushNest(NEST_Do);

	UInt16 AddrStart, AddrCond, DstEnd;
	AddrStart	= Emitter.Tell();

	// Statement.
	CompileStatement();

	RequireIdentifier( KW_while, L"do..while" );
	RequireSymbol( L"(", L"do loop" );
	{
		AddrCond	= Emitter.Tell();

		ResetRegs();
		UInt8 iReg = CompileExpr( TYPE_Bool, true, false, 0 ).iReg;

		// Emit footer.
		emit( CODE_JumpZero );
		DstEnd	= Emitter.Tell();
		emit( GTempWord );
		emit( iReg );	
	}
	RequireSymbol( L")", L"do loop" );

	// Post statement stuff.
	emit( CODE_Jump );
	emit( AddrStart );
	*(UInt16*)&Bytecode->Code[DstEnd]	= Emitter.Tell();

	// Fix & pop nest.
	ReplNest( REPL_Break, Emitter.Tell() );
	ReplNest( REPL_Continue, AddrCond );
	PopNest();
}


//
// Compile 'for' statement.
//
void CCompiler::CompileFor()
{
	assert(MatchIdentifier(KW_for));
	PushNest(NEST_For);

	UInt16 AddrBegin, AddrContinue, DstEnd = 0xffff;

	// Init expression.
	RequireSymbol( L"(", L"for statement" );
	if( !MatchSymbol( L";" ) )
	{
		ResetRegs();
		CompileExpr( TYPE_None, false, true, 0 );
		RequireSymbol( L";", L"for statement" );
	}

	// Cond expression.
	AddrBegin	= Emitter.Tell();
	if( !MatchSymbol(L";") )
	{
		ResetRegs();
		UInt8 iReg = CompileExpr( TYPE_Bool, true, false, 0 ).iReg;

		// Emit header.
		emit( CODE_JumpZero );
		DstEnd	= Emitter.Tell();
		emit( GTempWord );
		emit( iReg );

		RequireSymbol( L";", L"for statement" );
	}

	// Skip loop expression, for now.
	TToken LoopExprTok;
	GetToken( LoopExprTok, false, false );
	GotoToken( LoopExprTok );
	{
		Int32 NumBrk = 1;
		TToken T;
		while( NumBrk > 0 )
		{
			GetToken( T, false, false );
			if( T.Text == L"(" )
				NumBrk++;
			else if( T.Text == L")" )
				NumBrk--;
		}
	}

	// Statement.
	CompileStatement();

	// Post statement.
	TToken EndToken;
	GetToken( EndToken, false, false );
	AddrContinue	= Emitter.Tell();
	GotoToken( LoopExprTok );
	if( !MatchSymbol(L")") )
	{
		ResetRegs();
		CompileExpr( TYPE_None, false, true, 0 );
		RequireSymbol( L")", L"for statement" );
	}
	GotoToken( EndToken );

	emit( CODE_Jump );
	emit( AddrBegin );

	if( DstEnd != 0xffff )
		*(UInt16*)&Bytecode->Code[DstEnd]	= Emitter.Tell();

	// Fix break and pop nest.
	ReplNest( REPL_Continue, AddrContinue );
	ReplNest( REPL_Break, Emitter.Tell() );
	PopNest();
}


//
// Compile 'switch' statement.
//
void CCompiler::CompileSwitch()
{
	assert(MatchIdentifier(KW_switch));
	PushNest(NEST_Switch);

	TExprResult Expr;
	UInt16 DstJmpTab, DstDefaultAddr, DstOut;
	Array<Int32>	Labels;
	Array<UInt16>	Addrs;
	UInt8			Size;
	Bool			bDefaultFound = false;

	// Switch expression.
	RequireSymbol( L"(", L"switch" );
	{
		ResetRegs();
		Expr = CompileExpr( TYPE_None, true, false, 0 );
		Size = Expr.Type.TypeSize(true);

		if	( 
				Expr.Type.ArrayDim != 1 || 
				!( Expr.Type.Type==TYPE_Byte || Expr.Type.Type==TYPE_Integer ) 
			)
				Error( L"Integral expression type in 'switch'" );

		// Emit header.
		emit( CODE_Switch );
		emit( Size );
		emit( Expr.iReg );
		DstDefaultAddr	= Emitter.Tell();
		emit( GTempWord );
		DstJmpTab		= Emitter.Tell();
		emit( GTempWord );

		FreeReg(Expr.iReg);
	}
	RequireSymbol( L")", L"switch" );

	// Process switch body.
	RequireSymbol( L"{", L"switch" );
	for( ; ; )
	{
		if( MatchSymbol( L"}" ) )
		{
			// End of switch.
			break;
		}
		else if( MatchIdentifier( KW_default ) )
		{
			// Default label.
			if( bDefaultFound )
				Error( L"Default label already appeared in this switch" );

			bDefaultFound	= true;
			*(UInt16*)&Bytecode->Code[DstDefaultAddr]	= Emitter.Tell();
			RequireSymbol( L":", L"default" );
		}
		else if( MatchIdentifier( KW_case ) )
		{
			// Add a new label.
			if( bDefaultFound )
				Error( L"Case label follows after default label" );

			TToken T;
			Int32 Value;
			GetToken( T, true, false );

			if( T.Type == TOK_Const )
			{
				// Literal value.
				if( T.TypeInfo.Type == TYPE_Integer )
					Value = T.cInteger;
				else if( T.TypeInfo.Type == TYPE_Byte )
					Value = T.cByte;
				else
					Error( L"Integral expression required in 'switch'" );		
			}
			else if( T.Type == TOK_Identifier )
			{
				// Enumeration name.
				Value	= FindEnumElement(T.Text);
				if( Value == 0xff )
					Error( L"Enumeration element '%s' not found", *T.Text );
			}
			else 
				Error( L"Missing label expression" );

			if( Labels.find(Value) != -1 )
				Error( L"Case label value already appeared in this switch" );

			// Add to list.
			Labels.push( Value );
			Addrs.push( Emitter.Tell() );

			RequireSymbol( L":", L"case label" );
		}
		else
		{
			// CompileStatement handle it properly.
			CompileStatement();
		}
	}

	// Skip table.
	emit( CODE_Jump );
	DstOut	= Emitter.Tell();
	emit( GTempWord );

	// Emit jump table.
	*(UInt16*)&Bytecode->Code[DstJmpTab]	= Emitter.Tell();
	assert(Labels.size() == Addrs.size());
	UInt8 NumLabs = Labels.size();
	emit( NumLabs );
	for( Int32 i=0; i<Labels.size(); i++ )
	{
		if( Expr.Type.Type == TYPE_Byte )
			emit( *(UInt8*)&Labels[i] )
		else
			emit( *(Int32*)&Labels[i] );
	
		emit( Addrs[i] );
	}

	// Set out address.
	*(UInt16*)&Bytecode->Code[DstOut]	= Emitter.Tell();

	if( !bDefaultFound )
		*(UInt16*)&Bytecode->Code[DstDefaultAddr]	= Emitter.Tell();

	// Fix & pop nest.
	ReplNest( REPL_Break, Emitter.Tell() );
	PopNest();
}


//
// Compile command. Return, Break, Continue.
// Note: Bru-bru hate it!   :3
// Also it invoke thread's flow control.
//
void CCompiler::CompileCommand()
{
	if( MatchIdentifier( KW_return ) )
	{
		// Return from fn.
		if( Bytecode == Script->Thread )
			Error( L"'return' is no allowed here" );

		if( !MatchSymbol(L";") )
		{
			// Return with result.
			CFunction* Func = (CFunction*)Bytecode;
			if( !Func->ResultVar )
				Error( L"Return value allow only in functions" );

			ResetRegs();
			TExprResult Left;
			TExprResult Right = CompileExpr( *Func->ResultVar, true, false, 0 );

			Left.bLValue	= true;
			Left.Type		= *Func->ResultVar;
			Left.iReg		= GetReg();

			UInt16 Offset		= Func->ResultVar->Offset;
			emit( CODE_LocalVar );
			emit( Left.iReg );
			emit( Offset );

			emit_assign( Left, Right );

			RequireSymbol( L";", L"return" );
		}

		emit( CODE_Jump );
		Nest[0].Addrs[REPL_Return].push( Emitter.Tell() );
		emit( GTempWord );
	}
	else if( MatchIdentifier( KW_break ) )
	{
		// Break, doesn't handle 'switch' statement.
		Int32 LoopNest = NestTop - 1;
		while(	( LoopNest > 0 )&&
				( Nest[LoopNest].Type != NEST_Do )&&
				( Nest[LoopNest].Type != NEST_For )&&
				( Nest[LoopNest].Type != NEST_Foreach )&&
				( Nest[LoopNest].Type != NEST_Switch )&&
				( Nest[LoopNest].Type != NEST_While ) )
					LoopNest--;

		if( LoopNest == 0 )
			Error( L"No enclosing loop out of which to break or continue" );

		emit( CODE_Jump );
		Nest[LoopNest].Addrs[REPL_Break].push( Emitter.Tell() );
		emit( GTempWord );
		RequireSymbol( L";", L"break or continue" );
	}
	else if( MatchIdentifier( KW_continue ) )
	{
		// Continue.
		Int32 LoopNest = NestTop - 1;
		while(	( LoopNest > 0 )&&
				( Nest[LoopNest].Type != NEST_Do )&&
				( Nest[LoopNest].Type != NEST_For )&&
				( Nest[LoopNest].Type != NEST_Foreach )&&
				( Nest[LoopNest].Type != NEST_While ) )
					LoopNest--;

		if( LoopNest == 0 )
			Error( L"No enclosing loop out of which to break or continue" );

		emit( CODE_Jump );
		Nest[LoopNest].Addrs[REPL_Continue].push( Emitter.Tell() );
		emit( GTempWord );
		RequireSymbol( L";", L"break or continue" );
	}
	else if( MatchIdentifier( KW_stop ) )
	{
		// Stop thread execution.
		if( Bytecode != Script->Thread || IsStaticScope )
			Error( L"'stop' is not allowed here" );

		emit( CODE_Stop );
		RequireSymbol( L";", L"stop" );
	}
	else if( MatchIdentifier( KW_interrupt ) )
	{
		// Interrupt thread execution.
		if( Bytecode != Script->Thread || IsStaticScope )
			Error( L"'interrupt' is not allowed here" );

		emit( CODE_Interrupt );
		RequireSymbol( L";", L"interrupt" );
	}
	else if( MatchIdentifier( KW_wait ) )
	{
		// Make thread wait until expression become true.
		if( Bytecode != Script->Thread || IsStaticScope )
			Error( L"'wait' is not allowed here" );

		ResetRegs();
		UInt16 WaitExpr = Emitter.Tell();		
		UInt8 iReg = CompileExpr( TYPE_Bool, true, false, 0 ).iReg;

		emit( CODE_Wait );
		emit( WaitExpr );
		emit( iReg );

		RequireSymbol( L";", L"wait" );
	}
	else if( MatchIdentifier( KW_sleep ) )
	{
		// Sleep the thread.
		// Zzzzzz....
		if( Bytecode != Script->Thread || IsStaticScope )
			Error( L"'sleep' is not allowed here" );

		ResetRegs();
		UInt8 iReg = CompileExpr( TYPE_Float, true, false, 0 ).iReg;
		emit( CODE_Sleep );
		emit( iReg );

		RequireSymbol( L";", L"sleep" );
	}
	else if( MatchIdentifier( KW_goto ) )
	{
		// Goto an label in the thread.
		if( !Script->Thread || IsStaticScope )
			Error( L"Script has no thread" );

		ResetRegs();
		UInt8 iReg = CompileExpr( TYPE_Integer, true, false, 0 ).iReg;
		emit( CODE_Goto );
		emit( iReg );

		RequireSymbol( L";", L"sleep" );
	}
	else
		assert(false);
}


//
// Compile 'foreach' statement.
//
void CCompiler::CompileForeach()
{
	assert(MatchIdentifier(KW_foreach));

	// Don't foreaching within static script.
	if( IsStaticScope )
		Error( L"'foreach' is not allowed within static scripts" );

	// Doesn't allowed nested foreach loops.
	for( Int32 i=0; i<NestTop; i++ )
		if( Nest[i].Type == NEST_Foreach )
			Error( L"Nested 'foreach' is not allowed" );

	// Don't use foreach in threads. To predic some really
	// uncatchy gpf errors.
	if( Bytecode == Script->Thread )
		Error( L"'foreach' loop is not allowed in thread" );

	PushNest(NEST_Foreach);
	
	UInt16 AddrStart, DstEnd;
	UInt16 PropAddr;
	FScript* PropScript;

	// Foreach header.
	RequireSymbol( L"(", L"foreach" );
	{
		// Get loop control variable.
		String PropName = GetIdentifier(L"'foreach' loop control");
		CProperty* Control = FindProperty( ((CFunction*)Bytecode)->Locals, PropName );
		if( !Control )
			Error( L"Loop control variable '%s' is not found", *PropName );
		if( Control->ArrayDim != 1 || Control->Type != TYPE_Entity )
			Error( L"'foreach' loop control variable must be simple local variable" );
		PropAddr	= Control->Offset;
		PropScript	= Control->Script;

		// Separation.
		RequireSymbol( L":", L"foreach" );

		// Our iteration function.
		{
			String IterName	= GetIdentifier( L"Iteration function" );
			CNativeFunction* Iter	= FindNative(IterName);
			if( !Iter )
				Error( L"Iteration function '%s' not found", *IterName );
			if( !(Iter->Flags & NFUN_Foreach) )
				Error( L"'%s' is no an iteration function", *IterName );

			Array<UInt8>	ArgRegs;
			ResetRegs();
			RequireSymbol( L"(", L"function call" );
			for( Int32 i=0; i<Iter->NumParams; i++ )
			{
				ArgRegs.push( CompileExpr( Iter->Params[i].Type, true, false, 0 ).iReg );

				if( i < Iter->NumParams-1 )
					RequireSymbol( L",", L"arguments" ); 
			}
			RequireSymbol( L")", L"function call" );	

			// Emit call.
			emit_opcode( Iter->iOpCode );
			for( Int32 i=0; i<ArgRegs.size(); i++ )
			{
				emit( ArgRegs[i] );
				FreeReg( ArgRegs[i] );
			}
			assert(Iter->ResultType.Type == TYPE_None);
		}

		// Emit foreach header.
		AddrStart		= Emitter.Tell();
		emit(CODE_Foreach);
		emit(PropAddr);
		emit(PropScript);
		DstEnd			= Emitter.Tell();
		emit(GTempWord);
	}
	RequireSymbol( L")", L"foreach" );

	// statement.
	CompileStatement();

	// Post statement stuff.
	emit( CODE_Jump );
	emit( AddrStart );
	*(UInt16*)&Bytecode->Code[DstEnd]	= Emitter.Tell();

	// Fix & pop nest.
	ReplNest( REPL_Break, Emitter.Tell() );
	ReplNest( REPL_Continue, AddrStart );
	PopNest();
}


/*-----------------------------------------------------------------------------
    Declaration compiling.
-----------------------------------------------------------------------------*/

//
// Compile a single declaration.
//
void CCompiler::CompileDeclaration()
{
	if( PeekIdentifier() == KW_enum )
	{
		// Enumeration.
		CompileEnumDecl();
	}
	else if( PeekIdentifier() == KW_const )
	{
		// Constant.
		CompileConstDecl(); 
	}
	else if( PeekIdentifier() == KW_thread )
	{
		// Thread.
		if( Script->IsStatic() )
			Error( L"Thread is not allowed within static script" );

		CompileThreadDecl();
	}
	else if( PeekIdentifier() == KW_delegate )
	{
		// Delegate declaration.
		CompileDelegateDecl();
	}
	else if( PeekIdentifier() == KW_struct )
	{
		// Structure declaration.
		CompileStructDecl();
	}
	else if( !Script->IsStatic() && (PeekIdentifier() == KW_fn ||
			 PeekIdentifier() == KW_event) )
	{
		// Function.
		CompileFunctionDecl();
	}
	else if( PeekIdentifier() == KW_static && CompileVarDecl
				(
					Script->Statics,
					Script->StaticsSize,
					PROP_Static | PROP_Public,
					false,
					true
				) )
	{
		// Static variable or function.
	}
	else if( !Script->IsStatic() && CompileVarDecl
				( 
					Script->Properties, 
					Script->InstanceSize,
					Access == ACC_Public ? PROP_Editable : PROP_None,
					false,
					true 
				) )
	{
		// Compile property, or function.
		// C style syntax, need to parse more to figure it out.
	}
	else
		Error( L"Expected declaration, got '%s'", *PeekIdentifier() );
}


//
// Compile a variable type as a CTypeInfo. 
// If it's not a type, return TYPE_None and return to the previous location.
// bNoArray - arrays a restricted here, so don't allowed in parameters.
//
CTypeInfo CCompiler::CompileVarType( Bool bSimpleOnly )
{
	TToken TypeName;
	CClass* ResClass;
	CStruct* TestStruct;
	Int32 iDelegate;
	CTypeInfo TypeInfo;
	GetToken( TypeName, false, false );

	// Figure out property type.
	if( TypeName.Text == KW_byte )
	{
		// Simple byte.
		TypeInfo.Type	= TYPE_Byte;
		TypeInfo.Enum	= nullptr;
	}
	else if( TypeName.Text == KW_bool )
	{
		// Bool.
		TypeInfo.Type	= TYPE_Bool;
	}
	else if( TypeName.Text == KW_integer )
	{
		// Integer variable.
		TypeInfo.Type	= TYPE_Integer;
	}
	else if( TypeName.Text == KW_float )
	{
		// Float.
		TypeInfo.Type	= TYPE_Float;
	}
	else if( TypeName.Text == KW_angle )
	{
		// Angle.
		TypeInfo.Type	= TYPE_Angle;
	}
	else if( TypeName.Text == KW_color )
	{
		// Color.
		TypeInfo.Type	= TYPE_Color;
	}
	else if( TypeName.Text == KW_string )
	{
		// String.
		TypeInfo.Type	= TYPE_String;
	}
	else if( TypeName.Text == KW_vector )
	{
		// Vector.
		TypeInfo.Type	= TYPE_Vector;
	}
	else if( TypeName.Text == KW_aabb )
	{
		// AABB.
		TypeInfo.Type	= TYPE_AABB;
	}
	else if( TypeName.Text == KW_entity )
	{
		// Void entity.
		TypeInfo.Type		= TYPE_Entity;
		TypeInfo.iFamily	= -1;
		TypeInfo.Script		= nullptr;
	}
	else if( TypeInfo.Enum = FindEnum( Script, TypeName.Text, false ) )
	{
		// Enumeration byte.
		TypeInfo.Type	= TYPE_Byte;
	}
	else if( ResClass = CClassDatabase::StaticFindClass( *(String(L"F")+TypeName.Text )) )
	{
		// Resource.
		if( !ResClass->IsA(FResource::MetaClass) )
			Error( L"'%s' is not resource derived class", *ResClass->GetAltName() );

		TypeInfo.Type	= TYPE_Resource;
		TypeInfo.Class	= ResClass;
	}
	else if( TypeInfo.Script = FindScript( TypeName.Text ) )
	{
		// Typed entity.
		TypeInfo.Type		= TYPE_Entity;
		TypeInfo.iFamily	= TypeInfo.Script->iFamily;
	}
	else if( (TypeInfo.iFamily = FindFamily(TypeName.Text) ? FindFamily(TypeName.Text)->iFamily : -1) != -1 )
	{
		// Family entity.
		TypeInfo.Type	= TYPE_Entity;
		TypeInfo.Script	= nullptr;
	}
	else if( (iDelegate = FindDelegate(TypeName.Text)) != -1 )
	{
		// Delegate.
		TypeInfo.Type		= TYPE_Delegate;
		TypeInfo.iSignature	= iDelegate;
	}
	else if( (TestStruct = FindStruct(Script, TypeName.Text, false)) != nullptr )
	{
		// Struct.
		TypeInfo.Type		= TYPE_Struct;
		TypeInfo.Struct		= TestStruct;
		if( bSimpleOnly )
			Error( L"Structure type is not allowed here" );
	}
	else
	{
		// Not a type, give up.
		TypeInfo.Type	= TYPE_None;
		GotoToken( TypeName );
		return TypeInfo;
	}

	// Array size.
	if( MatchSymbol(L"[") )
	{
		if( bSimpleOnly )
			Error( L"Array variables is not allowed here" );

		if( TypeInfo.Type == TYPE_Struct )
			Error( L"Array of struct is not allowed" );

		if( MatchSymbol(L"]") )
		{
			// Dynamic array.
			TypeInfo.ArrayDim	= -1;
		}
		else
		{
			// Static array.
			TypeInfo.ArrayDim = ReadInteger( L"array size" );
			if( TypeInfo.ArrayDim <=0 || TypeInfo.ArrayDim > STATIC_ARR_MAX )
				Error( L"Bad array size" );

			RequireSymbol( L"]", L"array" );
		}
	}
	else
		TypeInfo.ArrayDim = 1;

	return TypeInfo;
}


//
// Compile an enumeration declaration.
//
void CCompiler::CompileEnumDecl()
{
	assert(MatchIdentifier(KW_enum));

	// Read and test enumeration name.
	String Name		= GetIdentifier( L"enumeration name" );
	if( FindEnum( Script, Name, false ) )
		Error( L"Enumeration '%s' already exists", *Name );

	RequireSymbol( L"{", L"enumeration" );

	// Allocate enumeration.
	CEnum*	Enum	= new CEnum( *Name, ENUM_None, Script );
	Script->Enums.push(Enum);

	// Parse elements.
	do 
	{
		String Elem	= GetIdentifier( L"enumeration" );
		//log( L"%s::%s", *Enum->Name, *Elem );

		if( Enum->Elements.find(Elem) != -1 )
			Error( L"Enumeration element '%s' redeclarated", *Elem );

		if( Enum->AddElement(Elem) > 255 )
			Error( L"Too many enumeration elements" );
		
	} while( MatchSymbol(L",") );

	RequireSymbol( L"}", L"enumeration" );
}


//
// Compile a structure declaration.
//
void CCompiler::CompileStructDecl()
{
	assert(MatchIdentifier(KW_struct));

	// Read and test structure name.
	String Name		= GetIdentifier( L"structure name" );
	if( FindStruct( Script, Name, false ) )
		Error( L"Structure '%s' already exists", *Name );

	RequireSymbol( L"{", L"structure" );

	// Allocate structure.
	CStruct*	Struct	= new CStruct( *Name );
	Script->Structs.push(Struct);

	// Parse line by line.
	do 
	{
		Bool bLineRecognized = CompileVarDecl
		( 
			Struct->Members, 
			Struct->Size, 
			PROP_None,
			false,
			false 
		);

		if( !bLineRecognized )
			Error( L"Bad member declartion" );

	} while( !MatchSymbol(L"}") );

	if( Struct->Size == 0 || Struct->Members.size() == 0 )
		Error( L"Struct shouldn't be empty" );

	if( Struct->Size >= 256 )
		Error( L"Too large struct" );

	// Dbg.
	info(L"Struct \"%s\" size=%d", *Struct->Name, Struct->Size);
	for( Int32 i=0; i<Struct->Members.size(); i++ )
		info
		(
			L"Member namy=\"%s\"; type=\"%s\"; offset=%d", 
			*Struct->Members[i]->Name, 
			*Struct->Members[i]->TypeName(), 
			Struct->Members[i]->Offset 
		);
}


//
// Compile a constant declaration.
//
void CCompiler::CompileConstDecl()
{
	assert(MatchIdentifier(KW_const));

	// Read and test constant name.
	String Name		= GetIdentifier( L"constant name" );
	if( FindConstant( Name ) )
		Error( L"Constant '%s' already exists", *Name );

	RequireSymbol( L"=", L"constant" );

	// Get constant value.
	TToken Const;
	GetToken( Const, true, true );

	if( Const.Type == TOK_Identifier )
	{
		// Reference other?
		TToken* Other = FindConstant( Const.Text );

		if( Other )
			Const = *Other;
		else
			Error( L"Constant '%s' not found", *Const.Text );
	}
	else if( Const.Type != TOK_Const )
		Error( L"Missing constant value" );

	// Complete constant and store.
	Const.Text	= Name;
	Constants.push(Const);		

	// Close the line.
	RequireSymbol( L";", L"constant" );
}


//
// Compile a property declaration. 
// bDetectFunc - used in global scope, in case possible function declaration.
//
Bool CCompiler::CompileVarDecl( Array<CProperty*>& Vars, SizeT& VarsSize, UInt32 InFlags, Bool bSimpleOnly, Bool bDetectFunc )
{
	// Store source location, maybe its a function.
	TToken SourceLoc;
	GetToken( SourceLoc );
	GotoToken( SourceLoc );

	// Static or not?
	Bool bStaticVar = MatchIdentifier(KW_static);
	if( Script->IsStatic() && !bStaticVar && &Vars == &Script->Statics )
		Error( L"'static' is not allowed here" );

	// !!Hack to parse static fn.
	if( bDetectFunc && bStaticVar && PeekIdentifier() == KW_fn )
	{
		GotoToken( SourceLoc );
		CompileFunctionDecl();
		return true;
	}

	CTypeInfo TypeInfo = CompileVarType( bSimpleOnly );
	if( TypeInfo.Type == TYPE_None || PeekSymbol() == L"." )
	{
		GotoToken( SourceLoc );
		return false;
	}

	// Parse variables.
	do 
	{
		String PropName	= GetIdentifier( L"property name" );

		// Check maybe it a part of function declaration.
		if( bDetectFunc && MatchSymbol( L"(" ) )
		{
			// YES! Its a function.
			GotoToken( SourceLoc );
			CompileFunctionDecl();
			return true;
		}
	
		// Check new name.
		if( FindProperty( Vars, PropName ) )
			Error( L"Property '%s' redeclarated", *PropName );

		// Allocate new property.
		CProperty* Property = new CProperty
		(
			TypeInfo,
			PropName,
			InFlags,
			VarsSize
		);

		// Add property to the list.
		Vars.push( Property );
		VarsSize = align( VarsSize + TypeInfo.TypeSize(), SCRIPT_PROP_ALIGN );

		// Not really good place for it, but its works well.
		// Compile variable initialization, but for locals only.
		if( Bytecode && (&(((CFunction*)Bytecode)->Locals) == &Vars) )
		{
			if( MatchSymbol( L"=" ) )
			{
				// Compile expr.
				if( Property->ArrayDim != 1 )
					Error( L"An array initialization isn't allowed" );
				if( !Property->IsSimpleType() )
					Error( L"Only simple types initialization allowed" );

				ResetRegs();

				TExprResult Left;
				Left.bLValue		= true;
				Left.iReg			= GetReg();
				Left.Type			= *Property;

				TExprResult Right	= CompileExpr( *Property, true, false, 0 );

				// Emit local variable.
				UInt16 Offset = Property->Offset;
				emit( CODE_LocalVar );
				emit( Left.iReg );
				emit( Offset );

				// Emit assignment.
				emit_assign( Left, Right );

				// Finish.
				FreeReg( Left.iReg );
				FreeReg( Right.iReg );
				bValidExpr	= true;
			}
		}

	} while( MatchSymbol(L",") );

	// End of declaration.
	RequireSymbol( L";", L"property" );
	return true;
}


//
// Compile thread declaration.
//
void CCompiler::CompileThreadDecl()
{
	assert(!Script->IsStatic());
	assert(MatchIdentifier(KW_thread));

	// Check, maybe script already has thread.
	if( Script->Thread )
		Error( L"Thread already declared in '%s'", *Script->GetName() );

	// Allocate thread.
	CThreadCode* Thread = new CThreadCode();
	Script->Thread = Thread;

	// Store thread location.
	TToken T;

	if( PeekSymbol() == L"{" )
	{
		GetToken( T );
		Thread->iLine	= T.iLine;
		Thread->iPos	= T.iPos;
	}
	else
		Error( L"Thread body not defined" );

	// Skip body.
	Int32 Level = 1;
	do 
	{
		GetToken( T );
		if( T.Text == L"{")
		{
			// Push nest level.
			Level++;
		}
		else if( T.Text == L"}" )
		{
			// Pop nest level.
			Level--;
		}
		else if( T.Text == L"@" )
		{
			// Possible label decl.
			String Name	= GetIdentifier( L"thread label" );
			if( MatchSymbol(L":") )
			{
				// Yes! Its a declaration.
				if( Thread->GetLabelId(*Name) != -1 )
					Error( L"Label '%s' redefined", *Name );

				CThreadCode::TLabel Label;
				Label.Address	= 0xffff;
				Label.Name		= Name;
				Thread->Labels.push( Label );
			}
		}
	} while( Level != 0 );
}


//
// Compile function declaration.
//
void CCompiler::CompileFunctionDecl()
{
	// Static check.
	Bool bStaticFunc = MatchIdentifier(KW_static);
	if( Script->IsStatic() && !bStaticFunc )
		Error( L"Non-static functions is not allowed within static script" );

	// Allocate function.
	CFunction* Function = new CFunction();
	if( bStaticFunc ) Function->Flags |= FUNC_Static;

	Array<CFunction*>& FuncList = bStaticFunc ? Script->StaticFunctions : Script->Methods;
	FuncList.push( Function );

	if( MatchIdentifier(KW_event) )
	{
		if( bStaticFunc )
			Error( L"Events should't be static" );

		// Its an event.
		Function->ResultVar	= nullptr;
		Function->Flags		= FUNC_Event;

		// Add to appropriate list.
		Script->Events.push(Function);
	}
	else if( MatchIdentifier(KW_fn) )
	{
		// Its a simple function, without result.
		Function->ResultVar	= nullptr;
		Function->Flags		= FUNC_None;
	}
	else  
	{
		// With result.
		CTypeInfo ResType = CompileVarType( true );
		if( ResType.Type == TYPE_None )
			return;

		Function->ResultVar	= new CProperty( ResType, KW_result, PROP_None, Function->FrameSize );
		Function->FrameSize = align( Function->FrameSize + ResType.TypeSize(), SCRIPT_PROP_ALIGN );
		Function->Flags		= FUNC_HasResult;
		// Warning: Result not added to list of locals, now, since parameters
		// should be before the result variable.
	}

	// Get function name.
	Function->Name = GetIdentifier( L"function name" );
	if( (FindFunction( Script->Methods, Function->Name ) != Function && FindFunction( Script->Methods, Function->Name ) != nullptr) ||
		(FindFunction( Script->StaticFunctions, Function->Name ) != Function && FindFunction( Script->StaticFunctions, Function->Name ) != nullptr)	)
			Error( L"Function '%s' redeclarated", *Function->Name );

	RequireSymbol( L"(", L"function parameters" );

	// Parse parameters.
	if( !MatchSymbol(L")") )
	{
		for( ; ; )
		{
			// Output modifier.
			UInt32 ParmFlags = MatchIdentifier(KW_out) ? PROP_OutParm : 0;

			// Param type.
			CTypeInfo ParamType = CompileVarType( true );
			if( ParamType.Type == TYPE_None )
				Error( L"Unknown parameter %d type", Function->Locals.size() );

			// Param name.
			String ParamName = GetIdentifier( L"parameter name" );
			if( FindProperty( Function->Locals, ParamName ) )
				Error( L"Parameter '%s' redeclarated", *ParamName );

			// Allocate parameter.
			CProperty*	Property	= new CProperty( ParamType, ParamName, ParmFlags, Function->FrameSize );
			Function->Locals.push( Property );
			Function->FrameSize = align( Function->FrameSize + ParamType.TypeSize(), SCRIPT_PROP_ALIGN );
			Function->ParmsCount++;

			if( MatchSymbol( L"," ) )
			{
				// Parse more.
			}
			else
			{
				// No more parameters.
				RequireSymbol( L")", L"parameters list" );
				break;
			}
		}
	}

	// Insert result property after the parameters.
	if( Function->ResultVar )
		Function->Locals.push( Function->ResultVar );

	// Test event name.
	if( Function->Flags & FUNC_Event )
	{
		Int32 iEvent = FindStaticEvent(Function->Name);
		if( iEvent == -1 )
			Error( L"Native event '%s' is not defined", *Function->Name );	
		if( !GEventLookup[iEvent].MatchSignature(Function) )
			Error( L"Signature of native event is mismatched" );
	}

	// Function is unified?
	if( MatchIdentifier(KW_unified) )
	{
		if( Script->iFamily == -1 )
			Error( L"Script '%s' without family", *Script->GetName() );
		if( bStaticFunc )
			Error( L"Unified function cannot be static" );

		CFamily* Family = Families[Script->iFamily];
		Int32 iProto = Family->VFNames.find(Function->Name);
		if( iProto == -1 )
		{
			// Add a new function and it signature.
			Family->VFNames.push(Function->Name);
			Family->Proto.push(Function);
			assert(Family->VFNames.size() == Family->Proto.size());
		}
		else
		{
			// Test signature.
			CFunction* Other = Family->Proto[iProto];
			assert(Other->Name==Function->Name);

			// Match it.
			if( Other->ParmsCount != Function->ParmsCount )
				Error( L"Signature '%s' parameters count mismatched", *Other->Name );

			// Results.
			if( Function->ResultVar )
			{
				if( Other->ResultVar )
				{
					if( !Function->ResultVar->MatchWith(*Other->ResultVar) )
						Error( L"Signature '%s' result type mismatched", *Other->Name );
				}
				else
					Error( L"Signature '%s' result type missing", *Other->Name );
			}
			else if( Other->ResultVar )
				Error( L"Signature '%s' result type missing", *Other->Name );

			// Match parameters.
			for( Int32 i=0; i<Function->ParmsCount; i++ )
				if( !Function->Locals[i]->MatchWith(*Other->Locals[i]) )
					Error
						( 
							L"Signature '%s' parameter %d types mismatched '%s' and '%s'", 
							*Function->Name, 
							i+1,
							*Function->Locals[i]->TypeName(), 
							*Other->Locals[i]->TypeName() 
						);
		}
	}
	else if( MatchIdentifier(KW_shell) )
	{
		// This is shell exposed function.
		if( !bStaticFunc )
			Error( L"'shell' function must be 'static'" );

		Function->Flags |= FUNC_Shell;
	}

	// Store function location.
	TToken T;

	if( PeekSymbol() == L"{" )
	{
		GetToken( T );
		Function->iLine	= T.iLine;
		Function->iPos	= T.iPos;
	}
	else
		Error( L"Function body not defined" );

	// Skip body.
	Int32 Level = 1;
	do 
	{
		GetToken( T );
		if( T.Text == L"{")
		{
			// Push nest level.
			Level++;
		}
		else if( T.Text == L"}" )
		{
			// Pop nest level.
			Level--;
		}
	} while( Level != 0 );	
}	


//
// Compile delegate declaration.
//
void CCompiler::CompileDelegateDecl()
{
	assert(MatchIdentifier(KW_delegate));

	TDelegateInfo Info;

	// Parse result type.
	if( MatchIdentifier(KW_fn) )
	{
		// Without result.
		Info.ResultType	= TYPE_None;
	}
	else
	{
		// With result.
		Info.ResultType	= CompileVarType(true);
		if( Info.ResultType.Type == TYPE_None )
			Error( L"Unrecognized delegate result type" );
	}

	// Read and test delegate name.
	Info.Name	= GetIdentifier(L"delegate name");
	if( FindDelegate(Info.Name) != -1 )
		Error( L"Delegate '%s' already exists", *Info.Name );

	RequireSymbol( L"(", L"delegate parameters" );

	// Parse parameters.
	if( !MatchSymbol(L")") )
	{
		for( ; ; )
		{
			Bool bOutParam = MatchIdentifier(KW_out);

			CTypeInfo ParamType = CompileVarType( true );
			if( ParamType.Type == TYPE_None )
				Error( L"Unknown parameter %d type", Info.ParamsType.size() );

			// Unused param name.
			String ParamName = GetIdentifier( L"parameter name" );

			TDelegateInfo::CParameter Param( ParamType, bOutParam );
			Info.ParamsType.push(Param);

			if( MatchSymbol( L"," ) )
			{
				// Parse more.
			}
			else
			{
				// No more parameters.
				RequireSymbol( L")", L"parameters list" );
				break;
			}
		}
	}

	// Add to the list of delegates.
	DelegatesInfo.push( Info );
	RequireSymbol( L";", L"delegate" );
}


/*-----------------------------------------------------------------------------
    Nest stack functions.
-----------------------------------------------------------------------------*/

//
// Push a new stack.
//
void CCompiler::PushNest( ENestType InType )
{
	// Check for overflow.
	if( NestTop >= arr_len(Nest) )
		Error( L"Nest level exceed maximum" );

	// Reset the nest.
	Nest[NestTop].Type	= InType;
	Nest[NestTop].Addrs[REPL_Break].empty();
	Nest[NestTop].Addrs[REPL_Return].empty();
	Nest[NestTop].Addrs[REPL_Continue].empty();

	NestTop++;
}


//
// Pop top nest from the stack.
//
void CCompiler::PopNest()
{
	NestTop--;
	assert(NestTop >= 0);

	Nest[NestTop].Addrs[REPL_Break].empty();
	Nest[NestTop].Addrs[REPL_Return].empty();
	Nest[NestTop].Addrs[REPL_Continue].empty();
}


//
// Replace an addresses at the top nest level.
//
void CCompiler::ReplNest( EReplType Repl, UInt16 DestAddr )
{
	TNest& N = Nest[NestTop-1];

	for( Int32 i=0; i<N.Addrs[Repl].size(); i++ )
		*(UInt16*)&Bytecode->Code[N.Addrs[Repl][i]]	= DestAddr;
}


/*-----------------------------------------------------------------------------
    Registers management.
-----------------------------------------------------------------------------*/

//
// Reset all registers, mark them as
// unused.
//
void CCompiler::ResetRegs()
{
	mem::zero( Regs, sizeof(Regs) );
}


//
// Get an available register index, return
// it's index, if registers are overflow,
// it's stops compilation.
//
UInt8 CCompiler::GetReg()
{
	Int32 iAvail = -1;
	for( Int32 i=0; i<arr_len(Regs); i++ )
		if( Regs[i] == false )
		{
			iAvail = i;
			break;
		}

	if( iAvail == -1 )
		Error( L"Internal compiler error. Try to reorganize your expression." );

	// Mark gotten register as 'in use'.
	Regs[iAvail] = true;
	return iAvail;
}


//
// Release a register, mark it as unused.
//
void CCompiler::FreeReg( UInt8 iReg )
{
	assert(iReg>=0 && iReg<arr_len(Regs));
	Regs[iReg] = false;
}


/*-----------------------------------------------------------------------------
    Objects search.
-----------------------------------------------------------------------------*/

//
// Find an enumeration, if enumeration not found return null.
// bOwnOnly - is should search only in the script, or in native 
// database too.
//
CEnum* CCompiler::FindEnum( FScript* Script, String Name, Bool bOwnOnly )
{
	// Searching in script.
	for( Int32 i=0; i<Script->Enums.size(); i++ )
		if( Name == Script->Enums[i]->Name )
			return Script->Enums[i];

	// Native database.
	if( !bOwnOnly )
		for( Int32 i=0; i<CClassDatabase::GEnums.size(); i++ )
			if( Name == CClassDatabase::GEnums[i]->Name )
				return CClassDatabase::GEnums[i];

	// Not found.
	return nullptr;
}


//
// Find a struct, if struct not found return null.
// bOwnOnly - is should search only in the script, or in native 
// database too.
//
CStruct* CCompiler::FindStruct( FScript* Script, String Name, Bool bOwnOnly )
{
	// Searching in script.
	for( Int32 i=0; i<Script->Structs.size(); i++ )
		if( Name == Script->Structs[i]->Name )
			return Script->Structs[i];

	// Native database.
	if( !bOwnOnly )
		for( Int32 i=0; i<CClassDatabase::GStructs.size(); i++ )
			if( Name == CClassDatabase::GStructs[i]->Name )
				return CClassDatabase::GStructs[i];

	// Not found.
	return nullptr;
}


//
// Find a script by name.
//
FScript* CCompiler::FindScript( String Name )
{
	for( Int32 i=0; i<AllScripts.size(); i++ )
		if( Name == AllScripts[i]->GetName() )
			return AllScripts[i];

	return nullptr;
}


//
// Find a family by name, if not found return nullptr.
//
CFamily* CCompiler::FindFamily( String Name )
{
	for( Int32 i=0; i<Families.size(); i++ )
		if( Name == Families[i]->Name )
			return Families[i];

	return nullptr;
}


//
// Find a named constant, if constant are not found return nullptr.
// Otherwise return pointer to the token constant.
// Warning: constant are shared object!
//
TToken* CCompiler::FindConstant( String Name )
{
	for( Int32 i=0; i<Constants.size(); i++ )
		if( Constants[i].Text == Name )
			return &Constants[i];

	return nullptr;
}


//
// Find delegate signature, if signature not found return -1.
// All delegates are shared objects.
//
Int32 CCompiler::FindDelegate( String Name )
{
	for( Int32 i=0; i<DelegatesInfo.size(); i++ )
		if( Name == DelegatesInfo[i].Name )
			return i;

	return -1;
}


//
// Find a property in the list, if not found return null.
//
CProperty* CCompiler::FindProperty( const Array<CProperty*>& List, String Name )
{
	for( Int32 i=0; i<List.size(); i++ )
		if( Name == List[i]->Name )
			return List[i];

	return nullptr;
}


//
// Find a function in the script.
//
CFunction* CCompiler::FindFunction( const Array<CFunction*>& FuncList, String Name )
{
	for( Int32 i=0; i<FuncList.size(); i++ )
		if( Name == FuncList[i]->Name )
			return FuncList[i];

	return nullptr;
}


//
// Find an unary operator by name and argument type.
// U can use TYPE_None, just to figure out is it an unary operator.
// Also, it try to apply type cast.
// If no operator found, return nullptr.
//
CNativeFunction* CCompiler::FindUnaryOp( String Name, const CTypeInfo& ArgType )
{
	if( ArgType.Type == TYPE_None )
	{
		// Just figure it out.
		for( Int32 i=0; i<CClassDatabase::GFuncs.size(); i++ )
		{
			CNativeFunction* F = CClassDatabase::GFuncs[i];

			if( (F->Flags & NFUN_UnaryOp) && 
				(F->Name == Name) )
				return F;
		}
	}
	else
	{
		// Try to find without typecast.
		for( Int32 i=0; i<CClassDatabase::GFuncs.size(); i++ )
		{
			CNativeFunction* F = CClassDatabase::GFuncs[i];

			if( ( F->Flags & NFUN_UnaryOp ) && 
				( F->Name == Name ) && 
				( F->Params[0].Type.Type == ArgType.Type ) )
				return F;
		}

		// Try to find with cast.
		for( Int32 i=0; i<CClassDatabase::GFuncs.size(); i++ )
		{
			CNativeFunction* F = CClassDatabase::GFuncs[i];

			if( ( F->Flags & NFUN_UnaryOp ) && 
				( F->Name == Name ) && 
				( GetCast( ArgType.Type, F->Params[0].Type.Type ) != 0x00 ) &&
				( !(F->Flags & NFUN_SuffixOp) ) )
				return F;
		}
	}

	return nullptr;
}


//
// Find a binary operator by name and arguments type.
// U can use TYPE_None, just to figure out is binary operator
// are exists, also it try to apply type cast.
// If no operator found, return nullptr.
//
CNativeFunction* CCompiler::FindBinaryOp( String Name, const CTypeInfo& Arg1, const CTypeInfo& Arg2 )
{
	if( Arg1.Type == TYPE_None || Arg2.Type == TYPE_None )
	{
		// Just figure out.
		for( Int32 i=0; i<CClassDatabase::GFuncs.size(); i++ )
		{
			CNativeFunction* F = CClassDatabase::GFuncs[i];

			if( ( F->Flags & NFUN_BinaryOp ) && 
				( F->Name == Name ) )
				return F;
		}
	}
	else
	{
		// Try to find without typecast.
		for( Int32 i=0; i<CClassDatabase::GFuncs.size(); i++ )
		{
			CNativeFunction* F = CClassDatabase::GFuncs[i];

			if( ( F->Flags & NFUN_BinaryOp ) && 
				( F->Name == Name ) && 
				( F->Params[0].Type.Type == Arg1.Type ) &&
				( F->Params[1].Type.Type == Arg2.Type ) )
					return F;
		}

		// Try to find with cast.
		for( Int32 i=0; i<CClassDatabase::GFuncs.size(); i++ )
		{
			CNativeFunction* F = CClassDatabase::GFuncs[i];
			if( !(F->Flags & NFUN_BinaryOp) || ( F->Name != Name ) )
				continue;

			if( F->Flags & NFUN_AssignOp )
			{
				// Only second operand uses cast.
				if( ( F->Params[0].Type.Type == Arg1.Type ) && 
					( F->Params[1].Type.Type == Arg2.Type || GetCast( Arg2.Type, F->Params[1].Type.Type ) != 0x00 ) )
						return F;
			}
			else
			{
				// Both operands support cast.
				if( ( F->Params[0].Type.Type == Arg1.Type || GetCast( Arg1.Type, F->Params[0].Type.Type ) != 0x00 ) && 
					( F->Params[1].Type.Type == Arg2.Type || GetCast( Arg2.Type, F->Params[1].Type.Type ) != 0x00 ) )
						return F;
			}
		}
	}

	return nullptr;
}


//
// Find an enumeration element, search in own enums and
// native. If element not found return 0xff.
//
UInt8 CCompiler::FindEnumElement( String Elem )
{
	// Searching in script.
	for( Int32 iEnum=0; iEnum<Script->Enums.size(); iEnum++ )
	{
		CEnum* Enum = Script->Enums[iEnum];

		for( Int32 i=0; i<Enum->Elements.size(); i++ )
			if( Elem == Enum->Elements[i] )
				return i;
	}

	// Native database.
	for( Int32 iEnum=0; iEnum<CClassDatabase::GEnums.size(); iEnum++ )
	{
		CEnum* Enum = CClassDatabase::GEnums[iEnum];

		for( Int32 i=0; i<Enum->Elements.size(); i++ )
			if( Elem == Enum->Elements[i] )
				return i;
	}

	// Not found.
	return 0xff;
}


//
// Find an native function.
//
CNativeFunction* CCompiler::FindNative( String Name )
{
	for( Int32 i=0; i<CClassDatabase::GFuncs.size(); i++ )
	{
		CNativeFunction* F = CClassDatabase::GFuncs[i];

		if( ( !(F->Flags & (NFUN_BinaryOp | NFUN_UnaryOp | NFUN_Method)) ) && 
			( F->Name == Name ) )
			return F;
	}

	return nullptr;
}


//
// Get an index of script event in global table.
// If no event found, return -1.
//
Int32 CCompiler::FindStaticEvent( String Name )
{
	assert(GEventLookup.size() == _EVENT_MAX);

	for( Int32 i=0; i<GEventLookup.size(); i++ )
		if( GEventLookup[i].Name == Name )
			return i;
	return -1;
}


/*-----------------------------------------------------------------------------
    Lexemes functions.
-----------------------------------------------------------------------------*/

//
// Peek the next identifier and get back to
// the previous text position.
//
String CCompiler::PeekIdentifier()
{
	TToken T;
	GetToken( T );
	GotoToken( T );
	return T.Text;
}


//
// Peek the next symbol and return to
// the previous text position.
//
String CCompiler::PeekSymbol()
{
	TToken T;
	GetToken( T );
	GotoToken( T );
	return T.Text;
}


//
// Match the name with the next lexeme (should be an identifier),
// if they are matched return true
// otherwise return false and get back to the previous text position.
//
Bool CCompiler::MatchIdentifier( const Char* Name )
{
	TToken T;
	GetToken( T );

	if( T.Text == Name )
	{
		return true;
	}
	else
	{
		GotoToken( T );
		return false;
	}
}


//
// Match the name with the next lexeme (should be a symbol),
// if they are matched return true
// otherwise return false and get back to the previous text position.
//
Bool CCompiler::MatchSymbol( const Char* Name )
{
	TToken T;
	GetToken( T );

	if( T.Text == Name )
	{
		return true;
	}
	else
	{
		GotoToken( T );
		return false;
	}
}


//
// Goto to the token's location in the text.
//
void CCompiler::GotoToken( const TToken& T )
{
	TextLine	=	PrevLine	= T.iLine;
	TextPos		=	PrevPos		= T.iPos;
}


//
// Read an identifier, and return it name. If lexeme
// is not an identifier this cause error.
//
String CCompiler::GetIdentifier( const Char* Purpose )
{
	TToken T;
	GetToken( T, false, false );

	if( T.Type != TOK_Identifier )
		Error( L"Missing identifier for %s", Purpose );

	return T.Text;
}


//
// Require an identifier with the same name, if names are mismatched
// abort compilation.
//
void CCompiler::RequireIdentifier( const Char* Name, const Char* Purpose )
{
	TToken T;
	GetToken( T );

	if( T.Text != Name )
		Error( L"Expected '%s' for %s, got '%s'", Name, Purpose, *T.Text );
}


//
// Require a symbol with the same name, if names are mismatched
// abort compilation.
//
void CCompiler::RequireSymbol( const Char* Name, const Char* Purpose )
{
	TToken T;
	GetToken( T );

	if( T.Text != Name )
		Error( L"Expected '%s' for %s, got '%s'", Name, Purpose, *T.Text );
}


//
// Read a literal integer constant.
//
Int32 CCompiler::ReadInteger( const Char* Purpose )
{
	TToken T;
	GetToken( T, true, true );

	if( T.Type != TOK_Const || T.TypeInfo.Type != TYPE_Integer )
		Error( L"Missing integer constant in %s", Purpose );

	return T.cInteger;
}


//
// Read a literal float constant.
//
Float CCompiler::ReadFloat( const Char* Purpose )
{
	TToken T;
	GetToken( T, true, true );

	if( T.Type != TOK_Const || T.TypeInfo.Type != TYPE_Float )
		Error( L"Missing float constant in %s", Purpose );

	return T.cFloat;
}


//
// Read a literal string constant.
//
String CCompiler::ReadString( const Char* Purpose )
{
	TToken T;
	GetToken( T, true, true );

	if( T.Type != TOK_Const || T.TypeInfo.Type != TYPE_String )
		Error( L"Missing string constant in %s", Purpose );

	return T.cString;
}


//
// Grab the next lexeme from the text.
//   T - Is a gotten token.
//   bAllowNeg - hint to parser, whether parse negative numbers if got '-'.
//   bAllowVect - hint to parser, whether parse '[' as symbol or vector?
//
void CCompiler::GetToken( TToken& T, Bool bAllowNeg, Bool bAllowVect )
{
	// Read characters to the buffer first,
	// to avoid multiple slow string reallocation.
	Char Buffer[128] = {};
	Char* Walk = Buffer;
	Char C;

	// Skip unused blank symbols before
	// lexeme.
	do 
	{
		C = GetChar();
	}while( ( C == 0x20 )||( C == 0x09 )||
			( C == 0x0d )||( C == 0x0a ) );

	// Preinitialize the token.
	mem::zero( Buffer, sizeof(Buffer) );
	T.Text				= L"";
	T.Type				= TOK_None;
	T.iLine				= PrevLine;
	T.iPos				= PrevPos;
	T.TypeInfo.ArrayDim	= 1;

	if( IsDigit( C ) )
	{
		// It's a numeric constant.
		Bool bGotDot = false;
		while( IsDigit(C) || C == L'.' )
		{
			*Walk = C;
			Walk++;
			if( C == L'.' )
			{
				if( bGotDot )
					Error( L"Bad float constant" );
				bGotDot	= true;
			}
			C = GetChar();
		}
		UngetChar();

		// Setup token.
		T.Text = Buffer;
		if( bGotDot )
		{
			// Float constant.
			T.Type			= TOK_Const;
			T.TypeInfo.Type	= TYPE_Float;
			T.Text.ToFloat( T.cFloat, 0.f );	
		}
		else
		{
			// Integer constant.
			T.Type			= TOK_Const;
			T.TypeInfo.Type	= TYPE_Integer;
			T.Text.ToInteger( T.cInteger, 0 );		
		}
	}
	else if( IsLetter( C ) )
	{
		// Its an identifier or keyword.
		while( IsLetter(C) || IsDigit(C) )
		{
			*Walk = C;
			Walk++;
			C	= GetChar();
		}
		UngetChar();
		T.Text	= Buffer;

		if( T.Text == KW_null )
		{													
			// Null resource const.
			T.Type					= TOK_Const;
			T.TypeInfo.Type			= TYPE_Resource;
			T.TypeInfo.Class		= FResource::MetaClass;
			T.cResource	= nullptr;
		}
		else if( T.Text == KW_true ||
				 T.Text == KW_false )
		{
			// Bool const.
			T.Type					= TOK_Const;
			T.TypeInfo.Type			= TYPE_Bool;
			T.cBool					= T.Text == KW_true;				
		}
		else if( T.Text == KW_undefined )
		{
			// Null entity const.
			T.Type					= TOK_Const;
			T.TypeInfo.Type			= TYPE_Entity;
			T.cEntity				= nullptr;
			T.TypeInfo.Script		= nullptr;
			T.TypeInfo.iFamily		= -1;
		}
		else if( T.Text == KW_nowhere )
		{
			// Delegate to nowhere.
			T.Type					= TOK_Const;
			T.TypeInfo				= CTypeInfo( TYPE_Delegate, 1, (void*)-1 );
			T.cDelegate().iMethod	= -1;
			T.cDelegate().Context	= nullptr;
			T.cDelegate().Script	= nullptr;
		}
		else
		{
			// Regular identifier.
			T.Type					= TOK_Identifier;			
		}
	}
	else if( C != L'\0' )
	{
		// It's a symbol, or something starts with it.
		Buffer[0]		= C;
		
		// Figure out, maybe two symbols?
		Char D = GetChar();

		if(	(( C == '-' )&&( D == '-' )) ||
			(( C == '+' )&&( D == '+' )) ||
			(( C == ':' )&&( D == ':' )) ||
			(( C == '>' )&&( D == '>' )) ||
			(( C == '<' )&&( D == '<' )) ||
			(( C == '>' )&&( D == '=' )) ||
			(( C == '<' )&&( D == '=' )) ||
			(( C == '=' )&&( D == '=' )) ||
			(( C == '!' )&&( D == '=' )) ||
			(( C == '|' )&&( D == '|' )) ||
			(( C == '&' )&&( D == '&' )) ||
			(( C == '^' )&&( D == '^' )) ||
			(( C == '+' )&&( D == '=' )) ||
			(( C == '-' )&&( D == '=' )) ||
			(( C == '*' )&&( D == '=' )) ||
			(( C == '/' )&&( D == '=' )) ||
			(( C == '&' )&&( D == '=' )) ||
			(( C == '|' )&&( D == '=' )) ||
			(( C == '^' )&&( D == '=' )) ||
			(( C == '-' )&&( D == '>' )) 
		  )
			Buffer[1] = D; 
		else
			UngetChar();
			
		T.Text	= Buffer;

		if( T.Text == L"[" && bAllowVect )
		{
			// Vector constant.
			T.Type				= TOK_Const;
			T.TypeInfo.Type		= TYPE_Vector;
			T.cVector().X		= ReadFloat( L"vector X component" );
			RequireSymbol( L",", L"vector" );
			T.cVector().Y		= ReadFloat( L"vector Y component" );
			RequireSymbol( L"]", L"vector" );
		}
		else if( T.Text == L"-" && bAllowNeg )
		{
			// Negative value.
			TToken U;
			GetToken( U, false, false );

			if( U.Type != TOK_Const )
				Error( L"Constant expected" );

			if( U.TypeInfo.Type == TYPE_Integer )
			{
				// Negative integer.
				T.Text				= T.Text + U.Text;
				T.Type				= TOK_Const;
				T.TypeInfo.Type		= TYPE_Integer;
				T.cInteger			= -U.cInteger;						
			}
			else if( U.TypeInfo.Type == TYPE_Float )
			{
				// Negative float.
				T.Text				= T.Text + U.Text;
				T.Type				= TOK_Const;
				T.TypeInfo.Type		= TYPE_Float;
				T.cFloat			= -U.cFloat;			
			}
			else
				Error( L"Numeric constant excepted" );
		}
		else if( T.Text == L"#" )
		{
			// Resource reference.
			String ResName	= GetIdentifier( L"resource reference" );
			FResource* Res	= (FResource*)Database->FindObject( ResName );

			if( !Res )
				Warn( L"Resource '%s' not found. Reference will set null", *ResName );

			T.Type					= TOK_Const;
			T.TypeInfo.Type			= TYPE_Resource;
			T.TypeInfo.Class		= Res ? Res->GetClass() : FResource::MetaClass;
			T.cResource				= Res;										
		}
		else if( T.Text == L"\"" )
		{
			// Literal string constant.
			mem::zero( Buffer, sizeof(Buffer) );
			Walk = Buffer;

			do 
			{
				C = _GetChar();		// Don't skip comment things in literal text.
				if( C == L'"' || C == L'\0' )
					break;
				*Walk	= C;
				Walk++;
			} while( true );

			// String constant.
			T.Text					= String::Format( L"\"%s\"", Buffer );
			T.cString				= Buffer;
			T.Type					= TOK_Const;
			T.TypeInfo.Type			= TYPE_String;				
		}
		else
		{
			// Regular symbol.
			T.Type			= TOK_Symbol;			
		}
	}
	else
	{	
		// Its a bad symbol, maybe end of code.
		Error( L"Expected lexeme, got end of script" );
	}
}


/*-----------------------------------------------------------------------------
    Characters functions.
-----------------------------------------------------------------------------*/

//
// Read and processed a character from the text,
// this function are skip any kinds of comments.
//
Char CCompiler::GetChar()
{
	Int32 PL, PP;
	Char Result;

Loop:
	Result	= _GetChar();
	PL		= PrevLine;
	PP		= PrevPos;

	if( Result == L'/' )
	{
		// Probably it's comment.
		Char C	= _GetChar();
		UngetChar();

		if( C == L'/' )
		{
			// Single line comment.
			TextLine++;
			TextPos	= 0;
			goto Loop;
		}
		else if( C == L'*' )
		{
			// Multiline comment.
			while( true )
			{
				C = _GetChar();
				if( C == L'*' )
				{
					// Maybe end of comment detected?
					Char D	= _GetChar();
					UngetChar();
					if( D == L'/' )
					{
						_GetChar();
						goto Loop;
					}
				}
				else if( C == L'\0' )
				{
					// Error, comment are never ended.
					return C;
				}
			}
		}
		else
		{
			// Restore original location.
			PrevLine	= PL;
			PrevPos		= PP;
		}
	}

	return Result;
}


//
// Return to the previous position in the text.
// Only one level of undo are support, but it's enough.
//
void CCompiler::UngetChar()
{
	TextLine	= PrevLine;
	TextPos		= PrevPos;
}


//
// Read a character from the text, do not use it, use
// GetChar instead, since this routine doesn't skip
// any comments.
//
Char CCompiler::_GetChar()
{
	// Store old location.
	PrevLine	= TextLine;
	PrevPos		= TextPos;

	// If end of text - return \0 symbol.
	if( TextLine >= Script->Text.size() )
		return L'\0';

	// If end of line, goto next line and return
	// separator whitespace.
	if( TextPos >= Script->Text[TextLine].Len() )
	{
		TextPos		= 0;
		TextLine++;
		return L' ';
	}

	// Regular character.
	return Script->Text[TextLine](TextPos++);
}


/*-----------------------------------------------------------------------------
    Compiler errors & warnings.
-----------------------------------------------------------------------------*/

//
// Abort compilation, because some fatal error
// happened.
//
void CCompiler::Error( const Char* Fmt, ... )
{
	Char Msg[1024] = {};
	va_list ArgPtr;
	va_start( ArgPtr, Fmt );
	_vsnwprintf( Msg, arr_len(Msg), Fmt, ArgPtr );
	va_end( ArgPtr );

	// Init fatal error info.
	FatalError.ErrorLine	= TextLine + 1;
	FatalError.ErrorPos		= TextPos + 1;
	FatalError.Script		= Script;
	FatalError.Message		= String::Format
	( 
		L"[Error] %s(%d): %s", 
		Script ? *Script->GetName() : L"", 
		TextLine+1, 
		Msg 
	);

	// Abort it!
	throw nullptr;
}


//
// Add a new compiler warning.
//
void CCompiler::Warn( const Char* Fmt, ... )
{
	Char Msg[1024] = {};
	va_list ArgPtr;
	va_start( ArgPtr, Fmt );
	_vsnwprintf( Msg, arr_len(Msg), Fmt, ArgPtr );
	va_end( ArgPtr );

	Warnings.push( String::Format
	( 
		L"[Warning] %s(%d): %s", 
		*Script->GetName(), 
		TextLine+1, 
		Msg ) 
	);
}


/*-----------------------------------------------------------------------------
    Script storage.
-----------------------------------------------------------------------------*/

//
// Collect all script, store their values,
// and entities of course, and prepare for
// the script compilation.
//
void CCompiler::StoreAllScripts()
{
	// Walk through all scripts.
	for( Int32 i=0; i<Database->GObjects.size(); i++ )
		if( Database->GObjects[i] && Database->GObjects[i]->IsA(FScript::MetaClass) )
		{
			FScript* S = (FScript*)Database->GObjects[i];

			// Add any script to list, for searching.
			AllScripts.push(S);

			// Add only script with the text.
			if( S->IsScriptable() )
			{
				// Script should have an instance buffer.

				// Add to the storage.
				TStoredScript Stored;

				if( !S->IsStatic() )
				{
					assert(S->InstanceBuffer);

					Stored.Properties	= S->Properties;
					Stored.InstanceSize	= S->InstanceSize;
					Stored.Buffers.push( S->InstanceBuffer );

					// Collect instance buffers from the entities.
					for( Int32 i=0; i<Database->GObjects.size(); i++ )
						if( Database->GObjects[i] && Database->GObjects[i]->IsA(FEntity::MetaClass) )
						{
							FEntity* Entity = (FEntity*)Database->GObjects[i];

							if( Entity->Script == S )
							{
								// Same as S.
								assert(Entity->InstanceBuffer);
								Stored.Buffers.push( Entity->InstanceBuffer );
							}
						}
				}
				else
				{
					Stored.InstanceSize	= 0;
				}

				Stored.Script		= S;
				Stored.Enums		= S->Enums;
				Stored.Structs		= S->Structs;

				// Add to list.
				Storage.push(Stored);

				// Cleanup all script's objects.
				freeandnil(S->Thread);
				freeandnil(S->StaticsBuffer);
				for( Int32 f=0; f<S->Methods.size(); f++ )
					freeandnil(S->Methods[f]);
				for( Int32 f=0; f<S->StaticFunctions.size(); f++ )
					freeandnil(S->StaticFunctions[f]);
				for( Int32 p=0; p<S->Statics.size(); p++ )
					freeandnil(S->Statics[p]);
				S->Enums.empty();
				S->Structs.empty();
				S->Properties.empty();
				S->Methods.empty();
				S->Events.empty();
				S->VFTable.empty();
				S->Statics.empty();
				S->StaticFunctions.empty();
				S->InstanceSize	= S->StaticsSize = 0;
				S->ResTable.empty();
			}
		}
}


//
// Restore all scripts after compilation
// failure.
//
void CCompiler::RestoreAfterFailure()
{
	for( Int32 i=0; i<Storage.size(); i++ )
	{
		TStoredScript&	Stored = Storage[i];
		FScript*		S		= Stored.Script;

		// Clean up all newly created script objects.
		freeandnil(S->Thread);
		for( Int32 f=0; f<S->Methods.size(); f++ )
			freeandnil( S->Methods[f] );

		for( Int32 f=0; f<S->StaticFunctions.size(); f++ )
			freeandnil( S->StaticFunctions[f] );

		for( Int32 p=0; p<S->Statics.size(); p++ )
			freeandnil( S->Statics[p] );

		for( Int32 p=0; p<S->Properties.size(); p++ )
			freeandnil( S->Properties[p] );

		for( Int32 e=0; e<S->Enums.size(); e++ )
			freeandnil( S->Enums[e] );

		for( Int32 s=0; s<S->Structs.size(); s++ )
			freeandnil( S->Structs[s] );

		S->Enums.empty();
		S->Structs.empty();
		S->Properties.empty();
		S->Statics.empty();
		S->Methods.empty();
		S->Events.empty();
		S->VFTable.empty();
		S->StaticFunctions.empty();
		S->InstanceSize		= 0;
		S->StaticsSize		= 0;
		S->iFamily			= -1;
		S->ResTable.empty();

		assert(S->StaticsBuffer == nullptr);

		// Copy old properties and enumerations
		// from the storage.
		S->Properties	= Stored.Properties;
		S->Enums		= Stored.Enums;
		S->Structs		= Stored.Structs;
		S->InstanceSize	= Stored.InstanceSize;

		// The information in all CInstanceBuffer are
		// still valid and well.
	}
}


//
// Restore old data after successful
// compilation.
//
void CCompiler::RestoreAfterSuccess()
{
	for( Int32 iSlot=0; iSlot<Storage.size(); iSlot++ )
	{
		TStoredScript& Stored = Storage[iSlot];
		FScript* Script	= Stored.Script;

		assert(Script->IsScriptable());
		assert(Script->StaticsBuffer == nullptr);

		if( !Script->IsStatic() )
		{
			for( Int32 iBuff=0; iBuff<Stored.Buffers.size(); iBuff++ )
			{
				CInstanceBuffer* Buffer = Stored.Buffers[iBuff];
				Array<UInt8> NewData(Script->InstanceSize);

				for( Int32 iNew=0; iNew<Script->Properties.size(); iNew++ )
				{
					CProperty* NewProp = Script->Properties[iNew];

					for( Int32 iOld=0; iOld<Stored.Properties.size(); iOld++ )
					{
						CProperty* OldProp = Stored.Properties[iOld];

						// Match old and new property.
						if( NewProp->Name == OldProp->Name && NewProp->MatchWith(*OldProp) )
						{
							// Copy value from old property to the new.
							NewProp->CopyValue
											( 
												&NewData[NewProp->Offset], 
												&Buffer->Data[OldProp->Offset] 
											);
						}
					}
				}

				// Destroy old instance buffer data.
				Exchange( Script->Properties, Stored.Properties );
				Buffer->DestroyValues();
				Exchange( Script->Properties, Stored.Properties );

				// Copy new data.
				Buffer->Data	= NewData;
			}
		}

		// Allocate new StaticBuffer.
		Script->StaticsBuffer = new CInstanceBuffer(Script->Statics);
		Script->StaticsBuffer->Data.setSize(Script->StaticsSize);

		if( Script->Statics.size() > 0 )
			assert(Script->StaticsSize > 0);

		// Destroy old storage data.
		for( Int32 i=0; i<Stored.Properties.size(); i++ )
			freeandnil(Stored.Properties[i]);

		for( Int32 i=0; i<Stored.Enums.size(); i++ )
			freeandnil(Stored.Enums[i]);

		Stored.Buffers.empty();
		Stored.Enums.empty();
		Stored.Structs.empty();
		Stored.Properties.empty();
		Stored.InstanceSize	= 0;
	}
}


/*-----------------------------------------------------------------------------
    Public compiler functions.
-----------------------------------------------------------------------------*/

//
// Compile all scripts in the database.
//
Bool Compiler::CompileAllScripts
( 
	CObjectDatabase* InDatabase, 
	Array<String>& OutWarnings, 
	TError& OutFatalError 
)
{
	if( !InDatabase )
		return false;

	CCompiler Compiler( InDatabase, OutWarnings, OutFatalError );

	return Compiler.CompileAll();
}


//
// Drop all scripts in the database.
//
Bool Compiler::DropAllScripts( CObjectDatabase* InDatabase )
{
	if( !InDatabase )
		return false;

	// Walk through all objects.
	for( Int32 i=0; i<InDatabase->GObjects.size(); i++ )
	{
		if( InDatabase->GObjects[i] && InDatabase->GObjects[i]->IsA(FScript::MetaClass) )
		{
			FScript* Script	= As<FScript>(InDatabase->GObjects[i]);

			// Only scripts with text.
			if( !Script->IsScriptable() )
				continue;

			// Destroy bytecodes.
			freeandnil(Script->Thread);
			for( Int32 iFunc=0; iFunc<Script->Methods.size(); iFunc++ )
				freeandnil(Script->Methods[iFunc]);

			// Clear tables.
			Script->Methods.empty();
			Script->Events.empty();
			Script->VFTable.empty();
			Script->ResTable.empty();
		}
	}

	return true;
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/