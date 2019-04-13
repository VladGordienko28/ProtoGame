/*=============================================================================
	FrScript.cpp: Virtual machine implementation.
	Copyright Jun.2016 Vlad Gordienko.
	Major improvements, May. 2018.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
	FScript implementation.
-----------------------------------------------------------------------------*/

//
// Script constructor.
//
FScript::FScript()
	:	FResource(),
		ScriptFlags( SCRIPT_None ),
		iFamily( -1 ),
		Base( nullptr ),
		InstanceBuffer( nullptr ),
		StaticsBuffer( nullptr ),
		InstanceSize( 0 ),
		StaticsSize( 0 ),
		Thread( nullptr )
{
}


//
// Script destructor.
//
FScript::~FScript()
{
	// Kill all prototype components.
	if( !IsStatic() )
	{
		// Kill extra components.
		for( Int32 e=0; e<Components.size(); e++ )
			DestroyObject( Components[e], false );

		// Kill the base.
		DestroyObject( Base, false );
	}
	else
	{
		// No prototypes for static scripts.
		assert(Base == nullptr);
		assert(Components.size() == 0);
	}

	// Finalize the instance buffer.
	if( IsScriptable() )
	{
		if( IsStatic() )
		{
			assert(InstanceBuffer == nullptr);
			assert(InstanceSize == 0);
		}
		else
		{
			assert(InstanceBuffer);
		}

		freeandnil(InstanceBuffer);
		freeandnil(StaticsBuffer);
	}

	// Script objects.
	for( Int32 i=0; i<StaticFunctions.size(); i++ )
		delete StaticFunctions[i];
	for( Int32 i=0; i<Statics.size(); i++ )
		delete Statics[i];
	for( Int32 i=0; i<Methods.size(); i++ )
		delete Methods[i];
	for( Int32 i=0; i<Properties.size(); i++ )
		delete Properties[i];
	for( Int32 i=0; i<Enums.size(); i++ )
		delete Enums[i];
	for( Int32 i=0; i<Structs.size(); i++ )
		delete Structs[i];

	// kill the thread.
	freeandnil(Thread);

	// Release tables.
	Text.empty();
	Enums.empty();
	Structs.empty();
	Properties.empty();
	Methods.empty();
	Events.empty();
	ResTable.empty();
	Statics.empty();
	StaticFunctions.empty();
}


//
// Find an extra component by it's name.
// If component not found return nullptr.
//
FComponent* FScript::FindComponent( String InName )
{
	for( Int32 e=0; e<Components.size(); e++ )
		if( Components[e]->GetName() == InName )
			return Components[e];

	return nullptr;
}


//
// Find script function. If not found return nullptr.
//
CFunction* FScript::FindMethod( String TestName )
{
	for( Int32 i=0; i<Methods.size(); i++ )
		if( TestName == Methods[i]->Name )
			return Methods[i];

	return nullptr;
}


//
// Find a static function. If not found return nullptr.
//
CFunction* FScript::FindStaticFunction( String TestName )
{
	for( Int32 i=0; i<StaticFunctions.size(); i++ )
		if( TestName == StaticFunctions[i]->Name )
			return StaticFunctions[i];

	return nullptr;
}


//
// When some field changed in resource.
//
void FScript::EditChange()
{
	FResource::EditChange();
}


//
// Initialize script after loading.
//
void FScript::PostLoad()
{
	FResource::PostLoad();
}


/*-----------------------------------------------------------------------------
    Script import & export.
-----------------------------------------------------------------------------*/

//
// Import the script.
//
void FScript::Import( CImporterBase& Im )
{
	FResource::Import( Im );

	// Components and instance buffer, handled
	// in the file importer code.

	// Script text stored in another file, and
	// loaded by the file importer too.

	IMPORT_INTEGER( ScriptFlags );
	//IMPORT_STRING( iFamily );

	// All other fields and script objects
	// database will be restored after full 
	// compilation.
}


//
// Export the script.
//
void FScript::Export( CExporterBase& Ex )
{
	FResource::Export( Ex );

	// Components and instance buffer, handled
	// in the file exporter code.

	// Script text stored in another file, and
	// saved by the file exporter too.

	EXPORT_INTEGER( ScriptFlags );
	//EXPORT_STRING( iFamily );

	// All other fields and script objects
	// database will be restored after full 
	// compilation.
}


/*-----------------------------------------------------------------------------
    Script serialization.
-----------------------------------------------------------------------------*/

// Little hack, here we declare global script variable, its used in
// serialization of script objects, such as variables or functions,
// it's will be cool to use another non-standard signature of 
// 'Serialize' function, but I can't do it, since this functions
// will be used in TArray<T> serialization implicitly.
static FScript*	GScript;


//
// Enumeration reference serializator helper.
//
struct TEnumIndex
{
public:
	// Variables.
	Int32	iScript;
	Int32	iEnum;

	// TEnumIndex interface.
	TEnumIndex()
		:	iScript(-1),
			iEnum(-1)
	{
	}
	TEnumIndex( CEnum* Enum )
	{
		if( Enum )
		{
			if( Enum->Flags & ENUM_Native )
			{
				iScript = -1;
				iEnum = CClassDatabase::GEnums.find(Enum);
				assert(iEnum != -1);
			}
			else
			{
				iScript = GScript->GetId();
				iEnum = GScript->Enums.find(Enum);
				assert(iScript != -1 && iEnum != -1);
			}
		}
		else
		{
			iScript = -1;
			iEnum = -1;
		}
	}
	CEnum* GetEnum()
	{
		if( iScript != -1 && iEnum != -1 )
		{
			if( iScript != -1 )
			{
				return As<FScript>(GObjectDatabase->GObjects[iScript])->Enums[iEnum];
			}
			else
			{
				return CClassDatabase::GEnums[iEnum];
			}
		}
		else
		{
			return nullptr;
		}
	}
	friend void Serialize( CSerializer& S, TEnumIndex& V )
	{
		Serialize( S, V.iScript );
		Serialize( S, V.iEnum );
	}
};


//
// Structure reference serializator helper.
//
struct TStructIndex
{
public:
	// Variables.
	Int32	iScript;
	Int32	iStruct;

	// TStructIndex interface.
	TStructIndex()
		:	iScript(-1),
			iStruct(-1)
	{
	}
	TStructIndex( CStruct* Struct )
	{
		assert(Struct);
		if( Struct->Flags & STRUCT_Native )
		{
			iScript = -1;
			iStruct = CClassDatabase::GStructs.find(Struct);
			assert(iStruct != -1);
		}
		else
		{
			iScript = GScript->GetId();
			iStruct = GScript->Structs.find(Struct);
			assert(iScript != -1 && iStruct != -1);
		}
	}
	CStruct* GetStruct()
	{
		if( iScript != -1 && iStruct != -1 )
		{
			if( iScript != -1 )
			{
				return As<FScript>(GObjectDatabase->GObjects[iScript])->Structs[iStruct];
			}
			else
			{
				return CClassDatabase::GStructs[iStruct];
			}
		}
		else
		{
			return nullptr;
		}
	}
	friend void Serialize( CSerializer& S, TStructIndex& V )
	{
		Serialize( S, V.iScript );
		Serialize( S, V.iStruct );
	}
};


//
// Serialize an enumeration object.
//
void Serialize( CSerializer& S, CEnum*& V )
{
	if( S.GetMode() == SM_Load )
	{
		freeandnil(V);
		V	= new CEnum( L"", ENUM_None, GScript );
	}

	Serialize( S, V->Name );
	Serialize( S, V->Flags );
	Serialize( S, V->Elements );
}


//
// Serialize a script property or struct member.
//
void Serialize( CSerializer& S, CProperty*& V )
{
	if( S.GetMode() == SM_Load )
	{
		freeandnil(V);
		V	= new CProperty();
	}

	Serialize( S, V->Name );
	Serialize( S, V->Flags );
	Serialize( S, V->Offset );

	SerializeEnum( S, V->Type );
	Serialize( S, V->ArrayDim );

	// Serialize additional type-specific data.
	if( V->Type == TYPE_Entity )
	{
		Serialize( S, V->Script );
	}
	else if( V->Type == TYPE_Delegate )
	{
		Serialize( S, V->iSignature );
	}
	else if( V->Type == TYPE_Resource )
	{
		if( S.GetMode() == SM_Load )
		{
			Int32 iClass;
			Serialize( S, iClass );
			V->Class = CClassDatabase::GClasses[iClass];
		}
		else
		{
			assert(V->Class);
			Int32 iClass = CClassDatabase::GClasses.find(V->Class);
			assert(iClass != -1);
			Serialize( S, iClass );
		}
	}
	else if( V->Type == TYPE_Byte )
	{
		if( S.GetMode() == SM_Load )
		{
			TEnumIndex Index;
			Serialize( S, Index );
			V->Enum = Index.GetEnum();
		}
		else
		{
			TEnumIndex Index(V->Enum);
			Serialize( S, Index );
		}
	}
	else if( V->Type == TYPE_Struct )
	{
		if( S.GetMode() == SM_Load )
		{
			TStructIndex Index;
			Serialize( S, Index );
			V->Struct = Index.GetStruct();
		}
		else
		{
			TStructIndex Index(V->Struct);
			Serialize( S, Index );
		}
	}
}


//
// Serialize a script function.
//
void Serialize( CSerializer& S, CFunction*& V )
{
	if( S.GetMode() == SM_Load )
	{
		freeandnil(V);
		V	= new CFunction();
	}

	Serialize( S, V->Name );
	Serialize( S, V->Flags ); 
	Serialize( S, V->Locals );
	Serialize( S, V->FrameSize );
	Serialize( S, V->ParmsCount );
	Serialize( S, V->Code );

	// Result variables.
	if( V->Flags & FUNC_HasResult )
	{
		if( S.GetMode() == SM_Load )
		{
			Int32 Index;
			Serialize( S, Index );
			V->ResultVar	= (Index!=-1) ? V->Locals[Index] : nullptr;
		}
		else if( S.GetMode() == SM_Save )
		{
			Int32 Index = V->ResultVar ? V->Locals.find(V->ResultVar) : -1;
			Serialize( S, Index );
		}
	}
}


//
// Serialize a thread label.
//
void Serialize( CSerializer& S, CThreadCode::TLabel& V )
{
	Serialize( S, V.Address );
	Serialize( S, V.Name );
}


//
// Serialize a script thread.
//
void Serialize( CSerializer& S, CThreadCode*& V )
{
	if( S.GetMode() == SM_Load )
	{
		freeandnil(V);
		Bool bUsed;
		Serialize( S, bUsed );
		if( bUsed )
		{
			V	= new CThreadCode();
			Serialize( S, V->Labels );
			Serialize( S, V->Code );
		}
	}
	else
	{
		Bool bUsed = V != nullptr;
		Serialize( S, bUsed );
		if( bUsed )
		{
			Serialize( S, V->Labels );
			Serialize( S, V->Code );
		}
	}
}


//
// Serialize an struct object.
//
void Serialize( CSerializer& S, CStruct*& V )
{
	if( S.GetMode() == SM_Load )
	{
		freeandnil(V);
		V	= new CStruct( L"" );
	}

	Serialize( S, V->Name );
	Serialize( S, V->Flags );
	Serialize( S, V->Members );
	Serialize( S, V->Flags );
}



//
// Serialize script.
//
void FScript::SerializeThis( CSerializer& S )
{
	// Call parent.
	FResource::SerializeThis( S );

	// Set self.
	GScript		= this;

	// General variables.
	Serialize( S, ScriptFlags );
	Serialize( S, Components );
	Serialize( S, Base );

	if( IsScriptable() )
	{
		// Don't actually store/restore script text, so client uses only
		// compiled version.
		if( S.GetMode() == SM_Undefined )
			Serialize( S, Text );

		// Family only for non-static scripts.
		if( !IsStatic() )
			Serialize( S, iFamily );

		// Generic stuff.
		Serialize( S, ResTable );
		Serialize( S, InstanceSize );
		Serialize( S, StaticsSize );

		// Enumeration objects.
		Serialize( S, Enums );

		// Struct objects.
		Serialize( S, Structs );

		// Serialize non-static stuff.
		if( !IsStatic() )
		{
			// Properties.
			Serialize( S, Properties );

			// Methods.
			Serialize( S, Methods );

			// Events and VF table.
			if( S.GetMode() == SM_Load )
			{
				Int32 NumEvents, NumVF;
				Serialize( S, NumEvents );
				Serialize( S, NumVF );
				assert(_EVENT_MAX == NumEvents);

				VFTable.empty();
				Events.empty();
				VFTable.setSize(NumVF);
				Events.setSize(NumEvents);

				for( Int32 i=0; i<NumEvents; i++ )
				{
					Int32 Tmp;
					Serialize( S, Tmp );
					Events[i]	= Tmp != -1 ? Methods[Tmp] : nullptr;
				}
				for( Int32 i=0; i<NumVF; i++ )
				{
					Int32 Tmp;
					Serialize( S, Tmp );
					VFTable[i]	= Tmp != -1 ? Methods[Tmp] : nullptr;
				}
			}
			else if( S.GetMode() == SM_Save )
			{
				Int32 NumEvents	= Events.size(),
						NumVF		= VFTable.size();
				Serialize( S, NumEvents );
				Serialize( S, NumVF );

				for( Int32 i=0; i<NumEvents; i++ )
				{
					Int32 Tmp	= Methods.find(Events[i]);
					Serialize( S, Tmp );
				}
				for( Int32 i=0; i<NumVF; i++ )
				{
					Int32 Tmp	= Methods.find(VFTable[i]);
					Serialize( S, Tmp );
				}
			}

			// Serialize thread.
			Serialize( S, Thread );

			// And finally instance buffer.
			if( S.GetMode() == SM_Load )
			{
				freeandnil(InstanceBuffer);
				InstanceBuffer	= new CInstanceBuffer( Properties );
				InstanceBuffer->Data.setSize( InstanceSize );
			}
			InstanceBuffer->SerializeValues( S );
		}

		// Other stuff.
		Serialize( S, Statics );
		Serialize( S, StaticFunctions );

		// Static instance buffer.
		if( S.GetMode() == SM_Load )
		{
			freeandnil(StaticsBuffer);
			StaticsBuffer	= new CInstanceBuffer( Statics );
			StaticsBuffer->Data.setSize( StaticsSize );
		}
		if( StaticsBuffer )
			StaticsBuffer->SerializeValues( S );
	}
}


/*-----------------------------------------------------------------------------
	Script objects implementation.
-----------------------------------------------------------------------------*/

//
// Bytecode constructor.
//
CBytecode::CBytecode()
	:	Code(),	
		iLine(-1),
		iPos(-1)
{
}


//
// Bytecode destructor.
//
CBytecode::~CBytecode()
{
	Code.empty();
}


//
// Thread code constructor.
//
CThreadCode::CThreadCode()
	:	CBytecode()
{
}


//
// Find a label by it's name.
// Returns label index, if not found return -1.
//
Int32 CThreadCode::GetLabelId( const Char* InName )
{
	for( Int32 i=0; i<Labels.size(); i++ )
		if( Labels[i].Name == InName )
			return i;

	return -1;
}


//
// Add a new label. If label already exists
// break the app. Return index of the new label.
//
Int32 CThreadCode::AddLabel( const Char* InName, UInt16 InAddr )
{
	assert(GetLabelId(InName) == -1);

	TLabel Label;
	Label.Address	= InAddr;
	Label.Name		= InName;

	return Labels.push( Label );
}


//
// CFunction constructor.
//
CFunction::CFunction()
	:	CBytecode(),
		Flags( FUNC_None ),
		FrameSize( 0 ),
		ParmsCount( 0 ),
		ResultVar( nullptr )
{
}
 

//
// CFunction destructor.
//
CFunction::~CFunction()
{
	// Destroy local variables.
	for( Int32 i=0; i<Locals.size(); i++ )
		delete Locals[i];

	Locals.empty();
}


//
// Return a function signature as string.
//
String CFunction::GetSignature() const
{
	String Args;
	for( Int32 i=0; i<ParmsCount; i++ )
	{
		Args += Locals[i]->TypeName() + L" " + Locals[i]->Name;
		if( i != ParmsCount-1 )
			Args += L", ";
	}

	return	String::format
	(
		L"%s %s(%s)",
		ResultVar ? *ResultVar->TypeName() : L"fn",
		*Name,
		*Args 
	);
}


/*-----------------------------------------------------------------------------
    CInstanceBuffer implementation.
-----------------------------------------------------------------------------*/

//
// Instance buffer constructor.
//
CInstanceBuffer::CInstanceBuffer( Array<CProperty*>& InProperties )
	:	Properties(InProperties),
		Data()
{
}


//
// Instance buffer destructor.
//
CInstanceBuffer::~CInstanceBuffer()
{
	// Destroy own values here.
	DestroyValues();
}


//
// Destroy all own values.
//
void CInstanceBuffer::DestroyValues()
{
	for( Int32 i=0; i<Properties.size(); i++ )
	{
		CProperty* P = Properties[i];
		P->DestroyValue( &Data[P->Offset] );
	}
}


//
// Copy values from Source.
// Procedure doesn't reallocate buffer size.
// So set proper size somewhere else.
//
void CInstanceBuffer::CopyValues( void* Source )
{
	for( Int32 i=0; i<Properties.size(); i++ )
	{
		CProperty* P = Properties[i];
		P->CopyValue( &Data[P->Offset], &((UInt8*)Source)[P->Offset] );
	}
}


//
// Export entire instance buffer values.
//
void CInstanceBuffer::ExportValues( CExporterBase& Ex )
{
	for( Int32 i=0; i<Properties.size(); i++ )
	{
		CProperty* P = Properties[i];
		P->Export( &Data[P->Offset], Ex );
	}
}


//
// Import entire instance buffer values.
//
void CInstanceBuffer::ImportValues( CImporterBase& Im )
{
	for( Int32 i=0; i<Properties.size(); i++ )
	{
		CProperty* P = Properties[i];
		P->Import( &Data[P->Offset], Im );
	}
}


//
// Serialize all values in buffer.
//
void CInstanceBuffer::SerializeValues( CSerializer& S )
{
	for( Int32 i=0; i<Properties.size(); i++ )
	{
		CProperty* P = Properties[i];
		P->SerializeValue( &Data[P->Offset], S );
	}
}


//
// Return true, if at least one value should be destroyed.
//
Bool CInstanceBuffer::NeedDestruction() const
{
	for( Int32 i=0; i<Properties.size(); i++ )
		if( Properties[i]->NeedDestruction() )
			return true;

	return false;
}


/*-----------------------------------------------------------------------------
    Registration.
-----------------------------------------------------------------------------*/	


REGISTER_CLASS_CPP( FScript, FResource, CLASS_Sterile )
{
	// All suffix operators.
	DECLARE_SUFFIX_OP( UN_Inc_Integer,	++,	TYPE_Integer,	TYPE_Integer );
	DECLARE_SUFFIX_OP( UN_Inc_Float,	++,	TYPE_Float,		TYPE_Float );
	DECLARE_SUFFIX_OP( UN_Dec_Integer,	--,	TYPE_Integer,	TYPE_Integer );
	DECLARE_SUFFIX_OP( UN_Dec_Float,	--, TYPE_Float,		TYPE_Float );

	// All unary operators.
	DECLARE_UNARY_OP( UN_Plus_Integer,	+,	TYPE_Integer,	TYPE_Integer );
	DECLARE_UNARY_OP( UN_Plus_Float,	+,	TYPE_Float,		TYPE_Float );
	DECLARE_UNARY_OP( UN_Plus_Vector,	+,	TYPE_Vector,	TYPE_Vector );
	DECLARE_UNARY_OP( UN_Plus_Color,	+,	TYPE_Color,		TYPE_Color );
	DECLARE_UNARY_OP( UN_Minus_Integer, -,	TYPE_Integer,	TYPE_Integer );
	DECLARE_UNARY_OP( UN_Minus_Float,	-,	TYPE_Float,		TYPE_Float );
	DECLARE_UNARY_OP( UN_Minus_Vector,	-,	TYPE_Vector,	TYPE_Vector );
	DECLARE_UNARY_OP( UN_Minus_Color,	-,	TYPE_Color,		TYPE_Color );
	DECLARE_UNARY_OP( UN_Not_Bool,		!,	TYPE_Bool,		TYPE_Bool );
	DECLARE_UNARY_OP( UN_Not_Integer,	~,	TYPE_Integer,	TYPE_Integer );

	// All binary operators.
	DECLARE_BIN_OP( BIN_Mult_Integer,		*,	11,		TYPE_Integer,	TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_OP( BIN_Mult_Float,			*,	11,		TYPE_Float,		TYPE_Float,		TYPE_Float );
	DECLARE_BIN_OP( BIN_Mult_Color,			*,	11,		TYPE_Color,		TYPE_Color,		TYPE_Color );
	DECLARE_BIN_OP( BIN_Mult_Vector,		*,	11,		TYPE_Vector,	TYPE_Vector,	TYPE_Float );
	DECLARE_BIN_OP( BIN_Div_Integer,		/,	11,		TYPE_Integer,	TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_OP( BIN_Div_Float,			/,	11,		TYPE_Float,		TYPE_Float,		TYPE_Float );
	DECLARE_BIN_OP( BIN_Mod_Integer,		%,	11,		TYPE_Integer,	TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_OP( BIN_Add_Integer,		+,	10,		TYPE_Integer,	TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_OP( BIN_Add_Float,			+,	10,		TYPE_Float,		TYPE_Float,		TYPE_Float );
	DECLARE_BIN_OP( BIN_Add_Color,			+,	10,		TYPE_Color,		TYPE_Color,		TYPE_Color );
	DECLARE_BIN_OP( BIN_Add_String,			+,	10,		TYPE_String,	TYPE_String,	TYPE_String );
	DECLARE_BIN_OP( BIN_Add_Vector,			+,	10,		TYPE_Vector,	TYPE_Vector,	TYPE_Vector );
	DECLARE_BIN_OP( BIN_Sub_Integer,		-,	10,		TYPE_Integer,	TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_OP( BIN_Sub_Float,			-,	10,		TYPE_Float,		TYPE_Float,		TYPE_Float );
	DECLARE_BIN_OP( BIN_Sub_Color,			-,	10,		TYPE_Color,		TYPE_Color,		TYPE_Color );
	DECLARE_BIN_OP( BIN_Sub_Vector,			-,	10,		TYPE_Vector,	TYPE_Vector,	TYPE_Vector );
	DECLARE_BIN_OP( BIN_Shr_Integer,		>>,	9,		TYPE_Integer,	TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_OP( BIN_Shl_Integer,		<<,	9,		TYPE_Integer,	TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_OP( BIN_Less_Integer,		<,	8,		TYPE_Bool,		TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_OP( BIN_Less_Float,			<,	8,		TYPE_Bool,		TYPE_Float,		TYPE_Float );
	DECLARE_BIN_OP( BIN_LessEq_Integer,		<=,	8,		TYPE_Bool,		TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_OP( BIN_LessEq_Float,		<=,	8,		TYPE_Bool,		TYPE_Float,		TYPE_Float );
	DECLARE_BIN_OP( BIN_Greater_Integer,	>,	8,		TYPE_Bool,		TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_OP( BIN_Greater_Float,		>,	8,		TYPE_Bool,		TYPE_Float,		TYPE_Float );
	DECLARE_BIN_OP( BIN_GreaterEq_Integer,	>=,	8,		TYPE_Bool,		TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_OP( BIN_GreaterEq_Float,	>=,	8,		TYPE_Bool,		TYPE_Float,		TYPE_Float );
	DECLARE_BIN_OP( BIN_And_Integer,		&,	6,		TYPE_Integer,	TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_OP( BIN_Xor_Integer,		^,	5,		TYPE_Integer,	TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_OP( BIN_Cross_Vector,		^,	11,		TYPE_Float,		TYPE_Vector,	TYPE_Vector );
	DECLARE_BIN_OP( BIN_Or_Integer,			|,	4,		TYPE_Integer,	TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_OP( BIN_Dot_Vector,			|,	11,		TYPE_Float,		TYPE_Vector,	TYPE_Vector );

	// All assignment operators.
	DECLARE_BIN_ASS_OP( BIN_AddEqual_Integer,	+=,	1,	TYPE_Integer,	TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_ASS_OP( BIN_AddEqual_Float,		+=,	1,	TYPE_Float,		TYPE_Float,		TYPE_Float );
	DECLARE_BIN_ASS_OP( BIN_AddEqual_Vector,	+=,	1,	TYPE_Vector,	TYPE_Vector,	TYPE_Vector );
	DECLARE_BIN_ASS_OP( BIN_AddEqual_String,	+=,	1,	TYPE_String,	TYPE_String,	TYPE_String );
	DECLARE_BIN_ASS_OP( BIN_AddEqual_Color,		+=,	1,	TYPE_Color,		TYPE_Color,		TYPE_Color );
	DECLARE_BIN_ASS_OP( BIN_SubEqual_Integer,	-=,	1,	TYPE_Integer,	TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_ASS_OP( BIN_SubEqual_Float,		-=,	1,	TYPE_Float,		TYPE_Float,		TYPE_Float );
	DECLARE_BIN_ASS_OP( BIN_SubEqual_Vector,	-=,	1,	TYPE_Vector,	TYPE_Vector,	TYPE_Vector );
	DECLARE_BIN_ASS_OP( BIN_SubEqual_Color,		-=,	1,	TYPE_Color,		TYPE_Color,		TYPE_Color );
	DECLARE_BIN_ASS_OP( BIN_MulEqual_Integer,	*=,	1,	TYPE_Integer,	TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_ASS_OP( BIN_MulEqual_Float,		*=,	1,	TYPE_Float,		TYPE_Float,		TYPE_Float );
	DECLARE_BIN_ASS_OP( BIN_MulEqual_Color,		*=,	1,	TYPE_Color,		TYPE_Color,		TYPE_Color );
	DECLARE_BIN_ASS_OP( BIN_DivEqual_Integer,	/=,	1,	TYPE_Integer,	TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_ASS_OP( BIN_DivEqual_Float,		/=,	1,	TYPE_Float,		TYPE_Float,		TYPE_Float );
	DECLARE_BIN_ASS_OP( BIN_ModEqual_Integer,	%=,	1,	TYPE_Integer,	TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_ASS_OP( BIN_ShlEqual_Integer,	<<=,1,	TYPE_Integer,	TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_ASS_OP( BIN_ShrEqual_Integer,	>>=,1,	TYPE_Integer,	TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_ASS_OP( BIN_AndEqual_Integer,	&=,	1,	TYPE_Integer,	TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_ASS_OP( BIN_XorEqual_Integer,	^=,	1,	TYPE_Integer,	TYPE_Integer,	TYPE_Integer );
	DECLARE_BIN_ASS_OP( BIN_OrEqual_Integer,	|=,	1,	TYPE_Integer,	TYPE_Integer,	TYPE_Integer );
	
	// Standard native functions.
	DECLARE_FUNCTION( OP_Abs, abs, TYPE_Float, ARG(f, TYPE_Float, END) );
	DECLARE_FUNCTION( OP_ArcTan, arctan, TYPE_Float, ARG(f, TYPE_Float, END) );
	DECLARE_FUNCTION( OP_ArcTan2, arctan2, TYPE_Float, ARG(y, TYPE_Float, ARG(x, TYPE_Float, END)) );
	DECLARE_FUNCTION( OP_Cos, cos, TYPE_Float, ARG(f, TYPE_Float, END) );
	DECLARE_FUNCTION( OP_Sin, sin, TYPE_Float, ARG(f, TYPE_Float, END) );
	DECLARE_FUNCTION( OP_Sqrt, sqrt, TYPE_Float, ARG(f, TYPE_Float, END) );
	DECLARE_FUNCTION( OP_Distance, distance, TYPE_Float, ARG(a, TYPE_Vector, ARG(B, TYPE_Vector, END)) );
	DECLARE_FUNCTION( OP_Ln, ln, TYPE_Float, ARG(f, TYPE_Float, END) );
	DECLARE_FUNCTION( OP_Exp, exp, TYPE_Float, ARG(f, TYPE_Float, END) );
	DECLARE_FUNCTION( OP_Frac, frac, TYPE_Float, ARG(f, TYPE_Float, END) );
	DECLARE_FUNCTION( OP_Round, round, TYPE_Integer, ARG(f, TYPE_Float, END) );
	DECLARE_FUNCTION( OP_Normalize, normalize, TYPE_Vector, ARG( v, TYPE_Vector, END ) );
	DECLARE_FUNCTION( OP_Random, random, TYPE_Integer, ARG(limitPlusOne, TYPE_Integer, END) );
	DECLARE_FUNCTION( OP_RandomF, randomf, TYPE_Float, END );
	DECLARE_FUNCTION( OP_MinF, minf, TYPE_Float, ARG(a, TYPE_Float, ARG(b, TYPE_Float, END)) );
	DECLARE_FUNCTION( OP_MaxF, maxf, TYPE_Float, ARG(a, TYPE_Float, ARG(b, TYPE_Float, END)) );
	DECLARE_FUNCTION( OP_ClampF, clampf, TYPE_Float, ARG(v, TYPE_Float, ARG(a, TYPE_Float, ARG(b, TYPE_Float, END))) );
	DECLARE_FUNCTION( OP_VectorSize, vsize, TYPE_Float, ARG(vect, TYPE_Vector, END) );
	DECLARE_FUNCTION( OP_RGBA, rgba, TYPE_Color, ARG(r, TYPE_Byte, ARG(g, TYPE_Byte, ARG(b, TYPE_Byte, ARG(a, TYPE_Byte, END)))) );
	DECLARE_FUNCTION( OP_CharAt, charAt, TYPE_String, ARG(str, TYPE_String, ARG(i, TYPE_Integer, END)) );
	DECLARE_FUNCTION( OP_IndexOf, indexOf, TYPE_Integer, ARG(needle, TYPE_String, ARG(hayStack, TYPE_String, END)) );
	DECLARE_FUNCTION( OP_Execute, execute, TYPE_None, ARG(cmd, TYPE_String, END) );
	DECLARE_FUNCTION( OP_Now, now, TYPE_Float, END );

	// Engine native functions.
	DECLARE_FUNCTION( OP_PlaySoundFX, PlaySoundFX, TYPE_None, ARG(sound, TYPE_SOUND, ARG(gain, TYPE_Float, ARG(pitch, TYPE_Float, END))) );
	DECLARE_FUNCTION( OP_PlayMusic, PlayMusic, TYPE_None, ARG(track, TYPE_MUSIC, ARG(fadeTime, TYPE_Float, END)) );
	DECLARE_FUNCTION( OP_KeyIsPressed, KeyIsPressed, TYPE_Bool, ARG(testKey, TYPE_Integer, END) );
	DECLARE_FUNCTION( OP_GetScreenCursor, GetScreenCursor, TYPE_Vector, END );
	DECLARE_FUNCTION( OP_GetWorldCursor, GetWorldCursor, TYPE_Vector, END );
	DECLARE_FUNCTION( OP_Localize, Localize, TYPE_String, ARG(section, TYPE_String, ARG(key, TYPE_String, END)) );
	DECLARE_FUNCTION( OP_GetScript, GetScript, TYPE_SCRIPT, ARG(ent, TYPE_Entity, END) );
	DECLARE_FUNCTION( OP_StaticPush, StaticPush, TYPE_None, ARG(key, TYPE_String, ARG(value, TYPE_String, END)) );
	DECLARE_FUNCTION( OP_StaticPop, StaticPop, TYPE_String, ARG(key, TYPE_String, ARG(default, TYPE_String, END)) );
	DECLARE_FUNCTION( OP_TravelTo, TravelTo, TYPE_None, ARG(nextLevel, TYPE_LEVEL, ARG(bCopyLevel, TYPE_Bool, END)) );
	DECLARE_FUNCTION( OP_FindEntity, FindEntity, TYPE_Entity, ARG(name, TYPE_String, END) );
	DECLARE_FUNCTION( OP_MatchKeyCombo, MatchKeyCombo, TYPE_Bool, ARG(seq, TYPE_String, END) );

	// Iterators.
	DECLARE_ITERATOR( IT_AllEntities, AllEntities, TYPE_None, ARG(script, TYPE_SCRIPT, END) );
	DECLARE_ITERATOR( IT_RectEntities, RectEntities, TYPE_None, ARG(script, TYPE_SCRIPT, ARG(bounds, TYPE_AABB, END)) );
	DECLARE_ITERATOR( IT_TouchedEntities, TouchedEntities, TYPE_None, END );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/