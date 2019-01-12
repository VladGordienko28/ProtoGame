/*=============================================================================
	FrClass.h: Meta classes system.
	Created by Vlad Gordienko, Jun. 2016.
	Extended by Vlad, Jan. 2018.
=============================================================================*/

/*-----------------------------------------------------------------------------
	CEnum.
-----------------------------------------------------------------------------*/

//
// Enumeration flags.
//
#define ENUM_None			0x0000			// No special flags;
#define ENUM_Native			0x0001			// This is C++ enum;
#define ENUM_NoAliases		0x0002			// Enum has no name aliases.


//
// A list of strings.
//
class CEnum
{
public:
	// Variables.
	String			Name;
	DWord			Flags;
	TArray<String>	Elements;

	// CEnum interface.
	CEnum( const Char* InName, DWord InFlags=ENUM_Native );
	CEnum( const Char* InName, DWord InFlags, FScript* Script );
	~CEnum();
	Integer AddElement( String NewElem );
	Integer FindElem( String TestName );
	String GetAliasOf( Integer i );
};


/*-----------------------------------------------------------------------------
    CProperty.
-----------------------------------------------------------------------------*/

// Script arrays limitation.
#define STATIC_ARR_MAX		255
#define DYNAMIC_ARRAY_MAX	8192
#define STRING_ARRAY_MAX	255


//
// Property flags.
//
#define PROP_None			0x0000		// Nothing.
#define PROP_Native			0x0001		// Its a native property.
#define PROP_Deprecated		0x0002		// Property should be killed.
#define PROP_Localized		0x0004		// Property should be localized.
#define PROP_Editable		0x0008		// Property is editable via ObjectInspector.
#define PROP_Const			0x0040		// Value is visible in ObjectInspector, but doesn't editable.
#define PROP_OutParm		0x0080		// Function output parameter.
#define PROP_Private		0x0100		// Property is private.
#define PROP_Public			0x0200		// Property is public.
#define PROP_Protected		0x0400		// Property is protected.
#define PROP_NoImEx			0x0800		// Don't import or export property.
#define PROP_Static			0x1000		// Script static variable.


//
// A property type.
//
enum EPropType
{
	TYPE_None,			// Invalid type.
	TYPE_Byte,			// Simple 1-byte integral or enumeration.
	TYPE_Bool,			// A logical variable.
	TYPE_Integer,		// Signed 32-bit value.
	TYPE_Float,			// IEEE float.
	TYPE_Angle,			// Angle value.
	TYPE_Color,			// Color.
	TYPE_String,		// Dynamic sizable string.
	TYPE_Vector,		// Point or direction in 2d space.
	TYPE_AABB,			// Axis aligned bounding box.
	TYPE_Resource,		// Resource such as FBitmap or FSound.
	TYPE_Entity,		// Entity - object in the level.
	TYPE_Delegate,		// Pointer to script method.
	TYPE_Struct,		// User defined struct.
	TYPE_MAX
};


//
// An information about property type.
//
class CTypeInfo
{
public:
	// Variables.
	EPropType	Type;
	Integer		ArrayDim;

	// Array dim indicates:
	// -1 - Dynamic array property.
	//  0 - Invalid property.
	//  1 - Simple value property.
	// >1 - Static array property.

	// Type addition information.
	union 
	{
		struct{ CEnum*		Enum;				  };		// TYPE_Byte;
		struct{ CClass*		Class;				  };		// TYPE_Resource;
		struct{ FScript*	Script; int iFamily;  };		// TYPE_Entity;
		struct{ CStruct*	Struct;				  };		// TYPE_Struct;
		struct{ Integer		iSignature;			  };		// TYPE_Delegate;
		struct{ void*		Inner;				  };		// Payload;
	};

	// Constructors.
	CTypeInfo();
	CTypeInfo( EPropType InType, Integer InArrDim = 1, void* InInner = nullptr );

	// CTypeInfo interface.	
	SizeT TypeSize( Bool bNoArray = false ) const;	
	String TypeName() const;
	String ToString( const void* Addr ) const;
	Bool MatchWith( const CTypeInfo& Other ) const;
	Bool CompareValue( const void* A, const void* B ) const;
	void CopyValue( void* Dst, const void* Src ) const;
	void DestroyValue( const void* Addr ) const;
	void SerializeValue( void* Addr, CSerializer& S ) const;
	void ImportValue( void* Addr, CImporterBase& Im, String Prefix ) const; 
	void ExportValue( const void* Addr, CExporterBase& Ex, String Prefix ) const;
	Bool NeedDestruction() const;
	Bool IsSimpleType() const;

	// Dynamic array helpers.
	void SetArrayLength( void* Addr, Integer NewLength ) const;
};


//
// A class or script property.
//
class CProperty: public CTypeInfo
{
public:
	// Variables.
	String		Name;
	DWord		Flags;
	SizeT		Offset;

	// Constructors.
	CProperty();
	~CProperty();

	// C++ constructor.
	CProperty
	( 
		CClass* Class, 
		const Char* InName, 
		EPropType PropType, 
		Integer InArrDim, 
		void* InInner,
		DWord InFlags,
		SizeT Offset 
	);

	// FluScript constructor.
	CProperty( CTypeInfo InType, String InName, DWord InFlags, SizeT InOffset );

	// CProperty interface.
	void Export( const void* Addr, CExporterBase& Ex ) const
	{
		ExportValue( Addr, Ex, Name );
	}
	void Import( void* Addr, CImporterBase& Im ) const
	{
		ImportValue( Addr, Im, Name );
	}

	String GetAliasName() const;
};


/*-----------------------------------------------------------------------------
	CStruct.
-----------------------------------------------------------------------------*/

//
// A structure flags.
//
#define STRUCT_None			0x0000			// Nothing;
#define STRUCT_Native		0x0001			// C++ struct.


//
// A FluScript struct.
//
class CStruct
{
public:
	// Variables.
	String				Name;
	DWord				Flags;
	TArray<CProperty*>	Members;
	SizeT				Size;

	// Native constructor.
	CStruct( const Char* InName, SizeT InSize );

	// FluScript constructor.
	CStruct( const Char* InName );

	// CStruct interface.
	~CStruct();
	CProperty* FindMember( const Char* InName ) const;
	void CopyValues( void* Dst, const void* Src ) const;
	void DestroyValues( void* Addr ) const;
	void SerializeValues( void* Addr, CSerializer& S ) const;
	void ImportValues( void* Addr, CImporterBase& Im, String Prefix ) const; 
	void ExportValues( const void* Addr, CExporterBase& Ex, String Prefix ) const;
	Bool CompareValues( const void* A, const void* B ) const;
	Bool NeedDestruction() const;
};


/*-----------------------------------------------------------------------------
	CVariant.
-----------------------------------------------------------------------------*/

//
// All possible script types. Warning: CVariant doesn't support
// array types, just simple value.
//
// This class is outdated and need to be removed.
//
class CVariant
{
public:
	// Variables.
	EPropType		Type;
	struct  
	{
		Byte		Value[16];
		String		StringValue;
	};

	// CVariant interface.
	CVariant();
	CVariant( Byte InByte );
	CVariant( Bool InBool );
	CVariant( Integer InInteger );
	CVariant( Float InFloat );
	CVariant( TAngle InAngle );
	CVariant( TColor InColor );
	CVariant( String InString );
	CVariant( const TVector& InVector );
	CVariant( const TRect& InRect );
	CVariant( FResource* InResource );
	CVariant( FEntity* InEntity );
	CVariant( const TDelegate& InDelegate );
};


/*-----------------------------------------------------------------------------
	CNativeFunction.
-----------------------------------------------------------------------------*/

//
// Native function flags.
//
#define NFUN_None				0x0000		// Nothing.
#define NFUN_UnaryOp			0x0001		// Unary operator.
#define NFUN_BinaryOp			0x0002		// Binary operator.
#define NFUN_Method				0x0004		// Native method being object.
#define NFUN_SuffixOp			0x0008		// Suffix unary operator.
#define NFUN_PrefixOp			0x0010		// Prefix unary operator.
#define NFUN_AssignOp			0x0020		// Assignment operator.
#define NFUN_Foreach			0x0040		// Foreach iteration function.
#define NFUN_Extended			0x0080		// Just C++ native function.


//
// Pointer to the native method.
//
typedef void (FObject::*TNativeMethod)( CFrame& Frame );
typedef void (*TNativeFunction)( CFrame& Frame );


//
// A native function description.
//
class CNativeFunction
{
public:
	// An information about function parameter.
	struct TParameter
	{
		CTypeInfo	Type;
		String		Name;
	};

	enum{ MAX_PARAMETERS = 8 };

	// Variables.
	DWord			Flags;
	String			Name;
	CClass*			Class;		// Class this method being or null.
	CTypeInfo		ResultType;
	TParameter		Params[MAX_PARAMETERS];
	Integer			NumParams;
	DWord			Priority;

	union
	{
		Integer			iOpCode;
		TNativeMethod	ptrMethod;
		TNativeFunction	ptrFunction;
	};

	// Function constructor.
	CNativeFunction( const Char* InName, DWord InFlags, Integer InOpCode );

	// Method constructor.
	CNativeFunction( const Char* InName, CClass* InClass, TNativeMethod InMethod );

	// Extended function constructor.
	CNativeFunction( const Char* InName, DWord InFlags, TNativeFunction InFunction );

	// CNativeFunction interface.
	Integer AddParameter( const Char* ParamName, const CTypeInfo& ParamType );
	void SetResultType( const CTypeInfo& InResultType );

	// Utils.
	String GetSignature() const;
};


/*-----------------------------------------------------------------------------
	CClass.
-----------------------------------------------------------------------------*/

//
// A Class flags.
//
#define CLASS_None			0x0000		// Nothing.
#define CLASS_Abstract		0x0001		// Class is abstract.
#define CLASS_Sterile		0x0002		// Class is 'final', no inheritance are allowed.
#define CLASS_Deprecated	0x0004		// Class marked as outdated.
#define CLASS_Highlight		0x0008		// Class has a special marker.
#define CLASS_SingleComp	0x0010		// Class of the single component in an entity.


//
// An object constructor.
//
typedef	FObject*(*TConstructor)();


//
// A meta class.
//
class CClass
{
public:
	// Variables.
	String			Name;
	DWord			Flags;
	CClass*			Super;
	TConstructor	Constructor;

	// Fields.
	TArray<CProperty*>			Properties;
	TArray<CNativeFunction*>	Methods;

	// CClass interface.
	CClass( const Char* InName, TConstructor InCnstr, CClass* InSuper, DWord InFlags );
	~CClass();
	void AddProperty( CProperty* InProp );
	void AddMethod( CNativeFunction* InMeth );
	CProperty* FindProperty( const Char* InName ) const;
	CNativeFunction* FindMethod( const Char* InName ) const;
	Bool IsA( CClass* SomeClass );

	// Utils.
	String GetAltName() const
	{
		return AltName;
	}

private:
	// Internal.
	String		AltName;
};


/*-----------------------------------------------------------------------------
	CClassDatabase.
-----------------------------------------------------------------------------*/

//
// A classes subsystem.
//
class CClassDatabase
{
public:
	// Static tables.
	static TArray<CClass*>			GClasses;
	static TArray<CEnum*>			GEnums;
	static TArray<CNativeFunction*>	GFuncs;
	static TArray<CStruct*>			GStructs;

	// Static functions.
	static CClass* StaticFindClass( const Char* InName );
	static CEnum* StaticFindEnum( const Char* InName );
	static CNativeFunction* StaticFindFunction( const Char* InName );
	static CStruct* StaticFindStruct( const Char* InName );
};


/*-----------------------------------------------------------------------------
	Helper macro.
-----------------------------------------------------------------------------*/

//
// Macro to figure out property offset.
//
#define PROPERTY_OFFSET(object, field) (SizeT)((Byte*)&((object*)nullptr)->field - (Byte*)nullptr) 


//
// A variant parameter definition.
//
#define VARIANT_PARM(name) const CVariant& name = CVariant()


//
// Finish marker of function parameters.
//
#define _HELPER_PARAM7_END
#define _HELPER_PARAM6_END
#define _HELPER_PARAM5_END
#define _HELPER_PARAM4_END
#define _HELPER_PARAM3_END
#define _HELPER_PARAM2_END
#define _HELPER_PARAM1_END
#define _HELPER_PARAM0_END


//
// Native function parameters parsers.
//
#define _HELPER_PARAM7_ARG( name, type, ... ) Func->AddParameter( L#name, type ); 
#define _HELPER_PARAM6_ARG( name, type, ... ) Func->AddParameter( L#name, type ); _HELPER_PARAM7_##__VA_ARGS__
#define _HELPER_PARAM5_ARG( name, type, ... ) Func->AddParameter( L#name, type ); _HELPER_PARAM6_##__VA_ARGS__
#define _HELPER_PARAM4_ARG( name, type, ... ) Func->AddParameter( L#name, type ); _HELPER_PARAM5_##__VA_ARGS__
#define _HELPER_PARAM3_ARG( name, type, ... ) Func->AddParameter( L#name, type ); _HELPER_PARAM4_##__VA_ARGS__
#define _HELPER_PARAM2_ARG( name, type, ... ) Func->AddParameter( L#name, type ); _HELPER_PARAM3_##__VA_ARGS__
#define _HELPER_PARAM1_ARG( name, type, ... ) Func->AddParameter( L#name, type ); _HELPER_PARAM2_##__VA_ARGS__
#define _HELPER_PARAM0_ARG( name, type, ... ) Func->AddParameter( L#name, type ); _HELPER_PARAM1_##__VA_ARGS__


//
// C++ to FluScript types convertation magic.
//

// [TYPE_Byte]
inline CTypeInfo _Cpp2FluType( Byte* Value )
{
	return CTypeInfo(TYPE_Byte, 1, nullptr);
}
template<SizeT ArrLen> inline CTypeInfo _Cpp2FluType( Byte(*Value)[ArrLen] )
{
	return CTypeInfo( TYPE_Byte, ArrLen, nullptr );
}
inline CTypeInfo _Cpp2FluType( TArray<Byte>* Value )
{
	return CTypeInfo( TYPE_Byte, -1, nullptr );
}

// [TYPE_Bool]
inline CTypeInfo _Cpp2FluType( Bool* Value )
{
	return CTypeInfo(TYPE_Bool, 1, nullptr);
}
template<SizeT ArrLen> inline CTypeInfo _Cpp2FluType( Bool(*Value)[ArrLen] )
{
	return CTypeInfo( TYPE_Bool, ArrLen, nullptr );
}
inline CTypeInfo _Cpp2FluType( TArray<Bool>* Value )
{
	return CTypeInfo( TYPE_Bool, -1, nullptr );
}

// [TYPE_Integer]
inline CTypeInfo _Cpp2FluType( Integer* Value )
{
	return CTypeInfo(TYPE_Integer, 1, nullptr);
}
template<SizeT ArrLen> inline CTypeInfo _Cpp2FluType( Integer(*Value)[ArrLen] )
{
	return CTypeInfo( TYPE_Integer, ArrLen, nullptr );
}
inline CTypeInfo _Cpp2FluType( TArray<Integer>* Value )
{
	return CTypeInfo( TYPE_Integer, -1, nullptr );
}

// [TYPE_Float]
inline CTypeInfo _Cpp2FluType( Float* Value )
{
	return CTypeInfo(TYPE_Float, 1, nullptr);
}
template<SizeT ArrLen> inline CTypeInfo _Cpp2FluType( Float(*Value)[ArrLen] )
{
	return CTypeInfo( TYPE_Float, ArrLen, nullptr );
}
inline CTypeInfo _Cpp2FluType( TArray<Float>* Value )
{
	return CTypeInfo( TYPE_Float, -1, nullptr );
}

// [TYPE_Angle]
inline CTypeInfo _Cpp2FluType( TAngle* Value )
{
	return CTypeInfo(TYPE_Angle, 1, nullptr);
}
template<SizeT ArrLen> inline CTypeInfo _Cpp2FluType( TAngle(*Value)[ArrLen] )
{
	return CTypeInfo( TYPE_Angle, ArrLen, nullptr );
}
inline CTypeInfo _Cpp2FluType( TArray<TAngle>* Value )
{
	return CTypeInfo( TYPE_Angle, -1, nullptr );
}

// [TYPE_Color]
inline CTypeInfo _Cpp2FluType( TColor* Value )
{
	return CTypeInfo(TYPE_Color, 1, nullptr);
}
template<SizeT ArrLen> inline CTypeInfo _Cpp2FluType( TColor(*Value)[ArrLen] )
{
	return CTypeInfo( TYPE_Color, ArrLen, nullptr );
}
inline CTypeInfo _Cpp2FluType( TArray<TColor>* Value )
{
	return CTypeInfo( TYPE_Color, -1, nullptr );
}

// [TYPE_String]
inline CTypeInfo _Cpp2FluType( String* Value )
{
	return CTypeInfo(TYPE_String, 1, nullptr);
}
template<SizeT ArrLen> inline CTypeInfo _Cpp2FluType( String(*Value)[ArrLen] )
{
	return CTypeInfo( TYPE_String, ArrLen, nullptr );
}
inline CTypeInfo _Cpp2FluType( TArray<String>* Value )
{
	return CTypeInfo( TYPE_String, -1, nullptr );
}

// [TYPE_Vector]
inline CTypeInfo _Cpp2FluType( TVector* Value )
{
	return CTypeInfo(TYPE_Vector, 1, nullptr);
}
template<SizeT ArrLen> inline CTypeInfo _Cpp2FluType( TVector(*Value)[ArrLen] )
{
	return CTypeInfo( TYPE_Vector, ArrLen, nullptr );
}
inline CTypeInfo _Cpp2FluType( TArray<TVector>* Value )
{
	return CTypeInfo( TYPE_Vector, -1, nullptr );
}

// [TYPE_AABB]
inline CTypeInfo _Cpp2FluType( TRect* Value )
{
	return CTypeInfo(TYPE_AABB, 1, nullptr);
}
template<SizeT ArrLen> inline CTypeInfo _Cpp2FluType( TRect(*Value)[ArrLen] )
{
	return CTypeInfo( TYPE_AABB, ArrLen, nullptr );
}
inline CTypeInfo _Cpp2FluType( TArray<TRect>* Value )
{
	return CTypeInfo( TYPE_AABB, -1, nullptr );
}

// [TYPE_Entity]
inline CTypeInfo _Cpp2FluType( FEntity** Value )
{
	return CTypeInfo(TYPE_Entity, 1, nullptr);
}
template<SizeT ArrLen> inline CTypeInfo _Cpp2FluType( FEntity*(*Value)[ArrLen] )
{
	return CTypeInfo( TYPE_Entity, ArrLen, nullptr );
}
inline CTypeInfo _Cpp2FluType( TArray<FEntity*>* Value )
{
	return CTypeInfo( TYPE_Entity, -1, nullptr );
}

// [TYPE_Resource]
template<class T> CTypeInfo _Cpp2FluType( T** Value )
{
	return CTypeInfo( TYPE_Resource, 1, T::MetaClass );
}
template<class T, SizeT ArrLen> inline CTypeInfo _Cpp2FluType( T*(*Value)[ArrLen] )
{
	return CTypeInfo( TYPE_Resource, ArrLen, T::MetaClass );
}
template<class T> CTypeInfo _Cpp2FluType( TArray<T*>* Value )
{
	return CTypeInfo( TYPE_Resource, -1, T::MetaClass );
}

// [TYPE_Delegate]
inline CTypeInfo _Cpp2FluType( TDelegate* Value )
{
	return CTypeInfo(TYPE_Delegate, 1, nullptr);
}
template<SizeT ArrLen> inline CTypeInfo _Cpp2FluType( TDelegate(*Value)[ArrLen] )
{
	return CTypeInfo( TYPE_Delegate, ArrLen, nullptr );
}
inline CTypeInfo _Cpp2FluType( TArray<TDelegate>* Value )
{
	return CTypeInfo( TYPE_Delegate, -1, nullptr );
}

// Hacky macro to retrive enum or struct value.
template<class E> inline CTypeInfo _Cpp2FluType( E* Value )
{
	Char TypeName[64];
	SizeT UnusedSize;
	mbstowcs_s( &UnusedSize, TypeName, arr_len(TypeName), typeid(E).name(), arr_len(TypeName) );
	if( String::Copy(TypeName, 0, 5) == L"enum " )
	{
		// This is enum.
		String EnumName = String::Delete(TypeName, 0, 5);
		CEnum* Enum = CClassDatabase::StaticFindEnum(*EnumName);
		if( !Enum )
			error( L"Enumeration \"%s\" is not registered", *EnumName );

		return CTypeInfo( TYPE_Byte, 1, Enum );
	}
	else if( String::Copy(TypeName, 0, 7) == L"struct " )
	{
		// This is struct.
		String StructName = String::Delete(TypeName, 0, 7);
		CStruct* Struct = CClassDatabase::StaticFindStruct(*StructName);	

		// Freaky way to ignore namespace prefix.
		if( !Struct )
			for( Integer i=0; i<CClassDatabase::GStructs.Num(); i++ )
				if( String::Pos(CClassDatabase::GStructs[i]->Name, StructName) != -1 )
				{
					Struct = CClassDatabase::GStructs[i];
					break;
				}

		if( !Struct )
			error( L"Struct \"%s\" is not registered", *StructName );

		return CTypeInfo( TYPE_Struct, 1, Struct );
	}
	else
	{
		error(L"Unknown parameter name \"%s\"", TypeName);
		return TYPE_None;
	}
}


/*-----------------------------------------------------------------------------
	Classes macro.
-----------------------------------------------------------------------------*/

//
// Class .h registration.
//
#define REGISTER_CLASS_H( cls )\
private:\
	typedef cls ClassType;\
public:\
	static CClass* MetaClass;\
	static void AutoRegisterClass();\
	static void AutoRegisterFields();\
	inline friend void Serialize( CSerializer& S, cls*& V )\
	{\
		S.SerializeRef( *(FObject**)&V );\
	}\


//
// Base class .cpp registration.
//
#define REGISTER_BASE_CLASS_CPP( cls, flags )\
static FObject* Cntor##cls()\
{\
	return new cls();\
}\
CClass* cls::MetaClass = nullptr;\
void cls::AutoRegisterClass()\
{\
	static CClass This( L#cls, Cntor##cls, nullptr, flags );\
	MetaClass = &This;\
	CClassDatabase::GClasses.Push(MetaClass);\
}\
void cls::AutoRegisterFields(){}\


//
// Class .cpp registration.
//
#define REGISTER_CLASS_CPP( cls, super, flags )\
static FObject* Cntor##cls()\
{\
	return new cls();\
}\
CClass* cls::MetaClass = nullptr;\
void cls::AutoRegisterClass()\
{\
	assert(super::MetaClass);\
	static CClass This( L#cls, Cntor##cls, super::MetaClass, flags );\
	MetaClass = &This;\
	CClassDatabase::GClasses.Push(MetaClass);\
}\
void cls::AutoRegisterFields()\


/*-----------------------------------------------------------------------------
	Enum macro.
-----------------------------------------------------------------------------*/

//
// Enumeration header.
//
#define BEGIN_ENUM( name )	\
	static CEnum _G_##name( L#name, ENUM_Native );\
	{\
		CEnum* Enum = &_G_##name;\
		assert(!CClassDatabase::StaticFindEnum(*Enum->Name));	\
		CClassDatabase::GEnums.Push(Enum);\


//
// Enumeration footer.
//
#define END_ENUM	}


//
// Enumeration element.
//
#define ENUM_ELEM( element ) Enum->AddElement(L#element);	


/*-----------------------------------------------------------------------------
	Struct macro.
-----------------------------------------------------------------------------*/

//
// Struct header.
//
#define BEGIN_STRUCT( name )\
	static CStruct _G_##name( L#name, sizeof(name) );\
	{	\
		typedef name StructType;\
		CStruct* Struct = &_G_##name;\
		assert(!CClassDatabase::StaticFindStruct(*Struct->Name));\
		CClassDatabase::GStructs.Push(Struct);\
	

//
// Struct footer.
//
#define END_STRUCT }


//
// Struct member.
//
#define STRUCT_MEMBER(name)\
	{\
		CProperty* Member = new CProperty\
		( \
			_Cpp2FluType(&((StructType*)nullptr)->name), \
			L#name, PROP_None, \
			PROPERTY_OFFSET(StructType, name) \
		);\
		Struct->Members.Push(Member);\
	}\


/*-----------------------------------------------------------------------------
	Property macro.
-----------------------------------------------------------------------------*/

//
// Add a new property to class.
//
#define ADD_PROPERTY( name, flags )	\
{	\
	CTypeInfo Type = _Cpp2FluType(&((ClassType*)nullptr)->name);\
	CProperty* Prop = new CProperty \
	( \
		MetaClass, \
		L#name, \
		Type.Type, Type.ArrayDim, \
		(void*)Type.Inner, flags, \
		PROPERTY_OFFSET(ClassType, name) \
	);	\
	MetaClass->AddProperty( Prop );	\
}	\


/*-----------------------------------------------------------------------------
	Function and Method macro.
-----------------------------------------------------------------------------*/

//
// Declare a native method.
//
#define DECLARE_METHOD( name, resulttype, ... )	\
{\
	CNativeFunction* Func = new CNativeFunction( L#name, ClassType::MetaClass, (TNativeMethod)(&ClassType::native##name) );\
	_HELPER_PARAM0_##__VA_ARGS__;	\
	Func->SetResultType(resulttype);\
	ClassType::MetaClass->AddMethod(Func);\
	CClassDatabase::GFuncs.Push(Func);\
}\


//
// Declare a native function.
//
#define DECLARE_FUNCTION( iopcode, name, resulttype, ... )\
{	\
	CNativeFunction* Func = new CNativeFunction( L#name, NFUN_None, iopcode );	\
	_HELPER_PARAM0_##__VA_ARGS__;	\
	Func->SetResultType(resulttype);\
	CClassDatabase::GFuncs.Push(Func);	\
}	\


//
// Declare a native extended function.
//
#define DECLARE_EX_FUNCTION( name, resulttype, ... )\
{	\
	CNativeFunction* Func = new CNativeFunction( L#name, NFUN_None | NFUN_Extended, (TNativeFunction)(&flu##name) );	\
	_HELPER_PARAM0_##__VA_ARGS__;	\
	Func->SetResultType(resulttype);\
	CClassDatabase::GFuncs.Push(Func);	\
}	\


//
// Declare a native iteration function.
//
#define DECLARE_ITERATOR( iopcode, name, resulttype, ... )\
{	\
	CNativeFunction* Func = new CNativeFunction( L#name, NFUN_Foreach, iopcode );	\
	_HELPER_PARAM0_##__VA_ARGS__;	\
	Func->SetResultType(resulttype);\
	CClassDatabase::GFuncs.Push(Func);	\
}	\


//
// Declare a native unary operator.
//
#define DECLARE_UNARY_OP( iopcode, name, resulttype, arg1type )	\
{	\
	CNativeFunction* Func = new CNativeFunction( L#name, NFUN_UnaryOp, iopcode );	\
	Func->AddParameter( L"a", arg1type ); \
	Func->SetResultType(resulttype);\
	CClassDatabase::GFuncs.Push(Func);\
}	\


//
// Declare a native suffix operator.
//
#define DECLARE_SUFFIX_OP( iopcode, name, resulttype, arg1type )	\
{	\
	CNativeFunction* Func = new CNativeFunction( L#name, NFUN_UnaryOp | NFUN_SuffixOp, iopcode );	\
	Func->AddParameter( L"a", arg1type ); \
	Func->SetResultType(resulttype);\
	CClassDatabase::GFuncs.Push(Func);\
}\


//
// Declare a native binary assignment operator
//
#define DECLARE_BIN_ASS_OP( iopcode, name, priority, resulttype, arg1type, arg2type )	\
{	\
	CNativeFunction* Func = new CNativeFunction( L#name, NFUN_BinaryOp | NFUN_AssignOp, iopcode );	\
	Func->Priority = priority;	\
	Func->AddParameter( L"a", arg1type ); \
	Func->AddParameter( L"b", arg2type ); \
	Func->SetResultType(resulttype);\
	CClassDatabase::GFuncs.Push(Func);	\
}	\


//
// Declare a native binary operator.
//
#define DECLARE_BIN_OP( iopcode, name, priority, resulttype, arg1type, arg2type )	\
{	\
	CNativeFunction* Func = new CNativeFunction( L#name, NFUN_BinaryOp, iopcode );	\
	Func->Priority = priority;	\
	Func->AddParameter( L"a", arg1type ); \
	Func->AddParameter( L"b", arg2type ); \
	Func->SetResultType(resulttype);\
	CClassDatabase::GFuncs.Push(Func);	\
}	\


/*-----------------------------------------------------------------------------
	Resources property type.
-----------------------------------------------------------------------------*/

#define TYPE_SOUND		CTypeInfo( TYPE_Resource,	1,	FSound::MetaClass )
#define TYPE_TEXTURE	CTypeInfo( TYPE_Resource,	1,	FTexture::MetaClass )
#define TYPE_BITMAP		CTypeInfo( TYPE_Resource,	1,	FBitmap::MetaClass )
#define TYPE_MATERIAL	CTypeInfo( TYPE_Resource,	1,	FMaterial::MetaClass )
#define TYPE_MUSIC		CTypeInfo( TYPE_Resource,	1,	FMusic::MetaClass )
#define TYPE_ANIMATION	CTypeInfo( TYPE_Resource,	1,	FAnimation::MetaClass )
#define TYPE_SCRIPT		CTypeInfo( TYPE_Resource,	1,	FScript::MetaClass )
#define TYPE_LEVEL		CTypeInfo( TYPE_Resource,	1,	FLevel::MetaClass )
#define TYPE_FONT		CTypeInfo( TYPE_Resource,	1,	FFont::MetaClass )
#define TYPE_SKELETON	CTypeInfo( TYPE_Resource,	1,	FSkeleton::MetaClass )


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/