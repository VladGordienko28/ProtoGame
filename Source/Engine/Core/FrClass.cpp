/*=============================================================================
    FrClass.cpp: Classes system implementation.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

#include "../Engine.h"

/*-----------------------------------------------------------------------------
	CClassDatabase implementation.
-----------------------------------------------------------------------------*/

//
// Whether use user-friendly aliases instead of
// real properties and enum names.
//
#define USE_ALIASES		1


//
// Global tables.
//
Array<CClass*>				CClassDatabase::GClasses;
Array<CEnum*>				CClassDatabase::GEnums;
Array<CNativeFunction*>		CClassDatabase::GFuncs;
Array<CStruct*>				CClassDatabase::GStructs;


//
// Find a meta class by its name,
// if class not found, return null.
//
CClass* CClassDatabase::StaticFindClass( const Char* InName )
{
	// Pretty sad, O(n), but used seldom,
	// so, it's doesn't critical.
	for( Int32 i=0; i<GClasses.size(); i++ )
		if( GClasses[i]->Name == InName )
			return GClasses[i];

	return nullptr;
}


//
// Find a enumeration by its name,
// if enumeration not found, return null.
//
CEnum* CClassDatabase::StaticFindEnum( const Char* InName )
{
	// Pretty sad, O(n), but used seldom,
	// so, it's doesn't critical.
	for( Int32 i=0; i<GEnums.size(); i++ )
		if( GEnums[i]->Name == InName )
			return GEnums[i];

	return nullptr;
}


//
// Find a function by its name,
// if function not found, return null.
//
CNativeFunction* CClassDatabase::StaticFindFunction( const Char* InName )
{
	// Pretty sad, O(n), but used seldom,
	// so, it's doesn't critical.
	for( Int32 i=0; i<GFuncs.size(); i++ )
		if( GFuncs[i]->Name == InName )
			return GFuncs[i];

	return nullptr;
}


//
// Find a struct by its name,
// if struct not found, return null.
//
CStruct* CClassDatabase::StaticFindStruct( const Char* InName )
{
	// Pretty sad, O(n), but used seldom,
	// so, it's doesn't critical.
	for( Int32 i=0; i<GStructs.size(); i++ )
		if( GStructs[i]->Name == InName )
			return GStructs[i];

	return nullptr;
}


/*-----------------------------------------------------------------------------
	CClass implementation.
-----------------------------------------------------------------------------*/

//
// Class constructor.
//
CClass::CClass( const Char* InName, TConstructor InCnstr, CClass* InSuper, UInt32 InFlags )
	:	Name( InName ),
		Flags( InFlags ),
		Super( InSuper ),
		Constructor( InCnstr ),
		Properties(),
		Methods()
{
	// Test for valid inheritance.
	if( FObject::MetaClass && this != FObject::MetaClass )
		assert(Super != nullptr);

	// Notify about old class.
	if( Flags & CLASS_Deprecated )
		warn( L"Class \"%s\" is outdated", *Name );

	// Generate friendly class name.
	assert(Name(0) == L'F');
	AltName = String::Copy( Name, 1, Name.Len()-1 );

	// Prevent classes duplication.
	assert(!CClassDatabase::StaticFindClass(InName));
}


//
// Meta-Class destructor.
//
CClass::~CClass()
{
	// Kill properties.
	for( Int32 iProp=0; iProp<Properties.size(); iProp++ )
		delete Properties[iProp];
	Properties.empty();

	// Kill methods.
	for( Int32 iMeth=0; iMeth<Methods.size(); iMeth++ )
		delete Methods[iMeth];
	Methods.empty();
}


//
// Add property to class.
//
void CClass::AddProperty( CProperty* InProp )
{
	assert(FindProperty(*InProp->Name) == nullptr);
	Properties.push( InProp );
}


//
// Add method to class.
//
void CClass::AddMethod( CNativeFunction* InMeth )
{
	assert(FindMethod(*InMeth->Name) == nullptr);
	Methods.push( InMeth );
}


//
// Find a property by name, if no property
// found return null.
//
CProperty* CClass::FindProperty( const Char* InName ) const
{
	for( Int32 iProp=0; iProp<Properties.size(); iProp++ )
		if( Properties[iProp]->Name == InName )
			return Properties[iProp];

	// Property not found, but maybe it in the super class.
	return Super ? Super->FindProperty(InName) : nullptr;
}


//
// Find a method by name, if no method
// found return null.
//
CNativeFunction* CClass::FindMethod( const Char* InName ) const
{
	for( Int32 iMeth=0; iMeth<Methods.size(); iMeth++ )
		if( Methods[iMeth]->Name == InName )
			return Methods[iMeth];

	// Method not found, but maybe it in the super class.
	return Super ? Super->FindMethod(InName) : nullptr;
}


//
// Return true, if SomeClass is a parent class of
// this class.
//
Bool CClass::IsA( CClass* SomeClass )
{
	for(  CClass* C = this; C; C = C->Super )
		if( C == SomeClass )
			return true;

	return false;
}


/*-----------------------------------------------------------------------------
	CNativeFunction implementation.
-----------------------------------------------------------------------------*/

//
// Native function constructor.
//
CNativeFunction::CNativeFunction( const Char* InName, UInt32 InFlags, Int32 InOpCode )
	:	Flags( InFlags ),
		Name( InName ),
		Class( nullptr ),
		iOpCode( InOpCode ),
		NumParams( 0 ),
		Priority( 0 )
{
	// Test for duplicates.
	if( !(Flags & (NFUN_UnaryOp | NFUN_BinaryOp)) )
		for( Int32 i=0; i<CClassDatabase::GFuncs.size(); i++ )
		{
			CNativeFunction* Other = CClassDatabase::GFuncs[i];

			if( Class == Other->Class && Name == Other->Name )
				fatal( L"Function \"%s\" redeclarated", *Name );
		}
}


//
// Native function constructor.
//
CNativeFunction::CNativeFunction( const Char* InName, UInt32 InFlags, TNativeFunction InFunction )
	:	Flags( InFlags ),
		Name( InName ),
		Class( nullptr ),
		ptrFunction( InFunction ),
		NumParams( 0 ),
		Priority( 0 )
{
	assert(Flags & NFUN_Extended);

	for( Int32 i=0; i<CClassDatabase::GFuncs.size(); i++ )
	{
		CNativeFunction* Other = CClassDatabase::GFuncs[i];

		if( Class == Other->Class && Name == Other->Name )
			fatal( L"Function \"%s\" redeclarated", *Name );
	}
}


//
// Native method constructor.
//
CNativeFunction::CNativeFunction( const Char* InName, CClass* InClass, TNativeMethod InMethod )
	:	Flags( NFUN_Method ),
		Name( InName ),
		Class( InClass ),
		NumParams( 0 ),
		Priority( 0 ),
		ptrMethod(InMethod)
{
	// Horrible tests.
	assert(Class);
	assert(Class->IsA(FComponent::MetaClass) || Class->IsA(FResource::MetaClass));
	if( Class->FindMethod(*Name) )
		fatal( L"Method \"%s\" redeclarated in class \"%s\"", *Name, *Class->GetAltName() );
}


//
// Return a function signature as a string.
//
String CNativeFunction::GetSignature() const
{
	String Args;
	for( Int32 i=0; i<NumParams; i++ )
	{
		Args += Params[i].Type.TypeName() + L" " + Params[i].Name;
		if( i != NumParams-1 )
			Args += L", ";
	}

	return String::Format
	(
		L"%s %s(%s)",
		ResultType.Type != TYPE_None ? *ResultType.TypeName() : L"fn",
		Flags & (NFUN_UnaryOp | NFUN_BinaryOp) ? *(String(L"operator")+Name) : *Name,
		*Args
	);
}


//
// Set function result type.
//
void CNativeFunction::SetResultType( const CTypeInfo& InResultType )
{
	ResultType = InResultType;
}


//
// Add a new parameter to native function and return it index.
//
Int32 CNativeFunction::AddParameter( const Char* ParamName, const CTypeInfo& ParamType )
{
	if( NumParams >= MAX_PARAMETERS )
		fatal(L"Too many parameters in function \"%s\"", *Name);

	// Check for name duplication.
	for( Int32 i=0; i<NumParams; i++ )
		if( Params[i].Name && Params[i].Name == ParamName )
			fatal( L"Parameter \"%s\" duplicated in function \"%s\"", ParamName, *Name );

	Params[NumParams].Name	= ParamName;
	Params[NumParams].Type	= ParamType;

	return NumParams++;
}


/*-----------------------------------------------------------------------------
	CVariant implementation.
-----------------------------------------------------------------------------*/

CVariant::CVariant()
	:	Type( TYPE_None )
{}
CVariant::CVariant( UInt8 InByte )
	:	Type( TYPE_Byte )
{
	*((UInt8*)(&Value[0])) = InByte;
}
CVariant::CVariant( Bool InBool )
	:	Type( TYPE_Bool )
{
	*((Bool*)(&Value[0])) = InBool;
}
CVariant::CVariant( Int32 InInteger )
	:	Type( TYPE_Integer )
{
	*((Int32*)(&Value[0])) = InInteger;
}
CVariant::CVariant( Float InFloat )
	:	Type( TYPE_Float )
{
	*((Float*)(&Value[0])) = InFloat;
}
CVariant::CVariant( math::Angle InAngle )
	:	Type( TYPE_Angle )
{
	*((math::Angle*)(&Value[0])) = InAngle;
}
CVariant::CVariant( TColor InColor )
	:	Type( TYPE_Color )
{
	*((TColor*)(&Value[0])) = InColor;
}
CVariant::CVariant( String InString )
	:	Type( TYPE_String )
{
	StringValue = InString;
}
CVariant::CVariant( const math::Vector& InVector )
	:	Type( TYPE_Vector )
{
	*((math::Vector*)(&Value[0])) = InVector;
}
CVariant::CVariant( const TRect& InRect )
	:	Type( TYPE_AABB )
{
	*((TRect*)(&Value[0])) = InRect;
}
CVariant::CVariant( FResource* InResource )
	:	Type( TYPE_Resource )
{
	*((FResource**)(&Value[0])) = InResource;
}
CVariant::CVariant( FEntity* InEntity )
	:	Type( TYPE_Entity )
{
	*((FEntity**)(&Value[0])) = InEntity;
}
CVariant::CVariant( const TDelegate& InDelegate )
	:	Type( TYPE_AABB )
{
	*((TDelegate*)(&Value[0])) = InDelegate;
}


/*-----------------------------------------------------------------------------
	CStruct implementation.
-----------------------------------------------------------------------------*/

//
// Native struct constructor.
//
CStruct::CStruct( const Char* InName, SizeT InSize )
	:	Name( InName ),
		Flags( STRUCT_Native ),
		Size( InSize )
{
}


//
// Script constructor.
//
CStruct::CStruct( const Char* InName )
	:	Name( InName ),
		Flags( STRUCT_None ),
		Size( 0 )
{
}


//
// Struct destructor.
//
CStruct::~CStruct()
{
	for( Int32 i=0; i<Members.size(); i++ )
		freeandnil(Members[i]);
}


//
// Find member.
//
CProperty* CStruct::FindMember( const Char* InName ) const
{
	for( Int32 i=0; i<Members.size(); i++ )
		if( Members[i]->Name == InName )
			return Members[i];
	return nullptr;
}


//
// Copy struct data.
//
void CStruct::CopyValues( void* Dst, const void* Src ) const
{
	for( auto& P : Members )
	{
		P->CopyValue( (UInt8*)Dst + P->Offset, (UInt8*)Src + P->Offset );
	}
}


//
// Destroy struct data.
//
void CStruct::DestroyValues( void* Addr ) const
{
	for( Int32 i=0; i<Members.size(); i++ )
	{
		CProperty* P = Members[i];
		if( P->NeedDestruction() )
			P->DestroyValue((UInt8*)Addr + P->Offset);
	}
}


//
// Serialize struct data.
//
void CStruct::SerializeValues( void* Addr, CSerializer& S ) const
{
	for( auto& P : Members )
	{
		P->SerializeValue( (UInt8*)Addr + P->Offset, S );
	}
}


//
// Import struct values.
//
void CStruct::ImportValues( void* Addr, CImporterBase& Im, String Prefix ) const
{
	for( auto& P : Members )
	{
		P->ImportValue( (UInt8*)Addr + P->Offset, Im, String::Format(L"%s.%s", *Prefix, *P->Name) );
	}
}


//
// Export struct data.
//
void CStruct::ExportValues( const void* Addr, CExporterBase& Ex, String Prefix ) const
{
	for( auto& P : Members )
	{
		P->ExportValue( (UInt8*)Addr + P->Offset, Ex, String::Format(L"%s.%s", *Prefix, *P->Name) );
	}
}


//
// Compare struct values.
//
Bool CStruct::CompareValues( const void* A, const void* B ) const
{
	for( auto& P : Members )
	{
		if( !P->CompareValue( (UInt8*)A + P->Offset, (UInt8*)B + P->Offset ) )
			return false;
	}
	return true;
}


//
// Return true, if struct data need to be destroyed.
//
Bool CStruct::NeedDestruction() const
{
	for( Int32 i=0; i<Members.size(); i++ )
		if( Members[i]->NeedDestruction() )
			return true;

	return false;
}


/*-----------------------------------------------------------------------------
    CProperty implementation.
-----------------------------------------------------------------------------*/

//
// CProperty default constructor.
// Don't shy to use it, it's not a CTypeInfo::CTypeInfo.
// But if you using it, you follows wrong way!
//
CProperty::CProperty()
	:	CTypeInfo( TYPE_None, 0, nullptr ),
		Flags( 0 ),
		Offset( 0 )
{
}


//
// CProperty native constructor.
//
CProperty::CProperty
( 
	CClass* Class, 
	const Char* InName, 
	EPropType PropType, 
	Int32 InArrDim, 
	void* InInner,
	UInt32 InFlags,
	SizeT InOffset 
)
	:	CTypeInfo( PropType, InArrDim, InInner ),
		Name( InName ),
		Flags( PROP_Native | InFlags ),
		Offset( InOffset )
{
}


//
// CProperty script or function constructor.
//
CProperty::CProperty( CTypeInfo InType, String InName, UInt32 InFlags, SizeT InOffset )
	:	CTypeInfo( InType ),
		Name( InName ),
		Flags( InFlags ),
		Offset( InOffset )
{
}


//
// CProperty destructor.
//
CProperty::~CProperty()
{
	// Nothing to destroy here..
}


//
// Return a cool human-readable property name.
//
String CProperty::GetAliasName() const
{
#ifdef USE_ALIASES
	// Alias.
	if( String::Pos( L"_", Name ) != -1 )
	{
		// Something crazy.
		return Name;
	}
	else
	{
		// Should be valid property.
		Char Buffer[64] = {}, *Walk = Buffer;
		assert(Name.Len() < arraySize(Buffer));
		Bool bBool = Name(0) == 'b';
		Int32 i = (Int32)bBool;
		Char PrevChar = '\0';
		while( i < Name.Len() )
		{
			Char ThisChar = Name(i);
			if( (ThisChar>='A'&&ThisChar<='Z')&&(PrevChar>='a'&&PrevChar<='z')&&(Name(i+1)!='\0') )
				*Walk++ = ' ';
			*Walk++ = ThisChar;
			PrevChar = ThisChar;
			i++;
		}
		if( bBool )
			*Walk++ = '?';

		return Buffer;
	}
#else
	// No aliases.
	return Name;
#endif
}


/*-----------------------------------------------------------------------------
	CTypeInfo implementation.
-----------------------------------------------------------------------------*/

//
// Default constructor.
// Avoid to use it.
//
CTypeInfo::CTypeInfo()
	:	Type( TYPE_None ),
		ArrayDim( 1 ),
		Inner( nullptr ),
		iFamily( -1 )
{
}


//
// Type info regular constructor.
//
CTypeInfo::CTypeInfo( EPropType InType, Int32 InArrDim, void* InInner )
	:	Type( InType ),
		ArrayDim( InArrDim ),
		Inner( InInner ),
		iFamily( -1 )
{
}


//
// Return size of this type.
// bNoArray - size of the array inner only.
//
SizeT CTypeInfo::TypeSize( Bool bNoArray ) const
{
	if( ArrayDim >= 1 )
	{
		// Not dynamic array.
		static const SizeT TypeSizes[TYPE_MAX] =
		{
			0,						//	TYPE_None
			sizeof( UInt8 ),		//	TYPE_Byte
			sizeof( Bool ),			//	TYPE_Bool
			sizeof( Int32 ),		//	TYPE_Integer
			sizeof( Float ),		//	TYPE_Float
			sizeof( math::Angle ),	//	TYPE_Angle
			sizeof( TColor ),		//	TYPE_Color	
			sizeof( String ),		//	TYPE_String
			sizeof( math::Vector ),	//	TYPE_Vector
			sizeof( TRect ),		//	TYPE_AABB
			sizeof( FResource* ),	//	TYPE_Resource
			sizeof( FEntity* ),		//	TYPE_Entity
			sizeof( TDelegate ),	//	TYPE_Delegate
			0						//	TYPE_Struct
		};

		SizeT ElemSize = Type == TYPE_Struct ? Struct->Size : TypeSizes[Type];
		return bNoArray ? ElemSize : ElemSize * ArrayDim;
	}
	else
	{
		// Dynamic array.
		return sizeof(ArrayPOD);
	}
}


//
// Return the name of the type.
//
String CTypeInfo::TypeName() const
{
	static const Char* BasicNames[TYPE_MAX]	=
	{
		L"none",
		L"byte",
		L"bool",
		L"integer",
		L"float",
		L"angle",
		L"color",
		L"string",
		L"vector",
		L"aabb",
		L"resource",
		L"entity",
		L"delegate",
		L"struct"
	};

	// Make name based on addition information if any.
	String Base =	( Type == TYPE_Resource && Class ) ? Class->GetAltName() :
					( Type == TYPE_Entity && Script ) ? Script->GetName() :
					( Type == TYPE_Struct && Struct ) ? Struct->Name :
					( Type == TYPE_Byte && Enum ) ? Enum->Name :	BasicNames[Type];

	return	ArrayDim == 1 ?	Base :
			ArrayDim == -1 ? Base + L"[]" : String::Format( L"%s[%d]", *Base, ArrayDim );
}


//
// Destroy a values at the given address.
//
void CTypeInfo::DestroyValue( const void* Addr ) const
{
	if( ArrayDim != -1 )
	{
		// Simple value.
		if( Type == TYPE_String )
		{
			for( Int32 i=0; i<ArrayDim; i++ )
				(&((String*)Addr)[i])->~String();
		}
		else if( Type == TYPE_Struct )
		{
			assert(Struct);
			for( Int32 i=0; i<ArrayDim; i++ )
				Struct->DestroyValues( (UInt8*)Addr + Struct->Size*i );
		}
	}
	else
	{
		// Dynamic array.
		ArrayPOD* Array = (ArrayPOD*)Addr;
		if( Array->size > 0 )
		{
			CTypeInfo Elems( Type, Array->size, Inner );
			Elems.DestroyValue( Array->data );
			array::reallocate( Array->data, Array->size, 0, Elems.TypeSize(false) );
		}
	}
}


//
// Copy a value from Src to Dst, this function is
// optimized, via mem::copy for speed.
//
void CTypeInfo::CopyValue( void* Dst, const void* Src ) const
{
	if( ArrayDim != -1 )
	{
		// Copy simple value.
		if( Type == TYPE_String )
		{
			for( Int32 i=0; i<ArrayDim; i++ )
				((String*)Dst)[i] = ((String*)Src)[i];
		}
		else if( Type == TYPE_Struct )
		{
			assert(Struct);
			for( Int32 i=0; i<ArrayDim; i++ )
			{
				SizeT Offset = i * Struct->Size;
				Struct->CopyValues( (UInt8*)Dst+Offset, (UInt8*)Src+Offset );
			}
		}
		else
		{
			mem::copy( Dst, Src, TypeSize(false) );
		}
	}
	else
	{
		// Copy dynamic array.
		ArrayPOD* DArray = (ArrayPOD*)Dst;
		ArrayPOD* SArray = (ArrayPOD*)Src;

		DestroyValue( Dst );
		SetArrayLength( Dst, SArray->size );
		assert(DArray->size == SArray->size);

		CTypeInfo Elem( Type, 1, Inner );
		if( SArray->size > 0 )
		{
			if( Elem.NeedDestruction() )
			{
				Elem.ArrayDim = SArray->size;
				Elem.CopyValue( DArray->data, SArray->data );
			}
			else
			{
				mem::copy( DArray->data, SArray->data, Elem.TypeSize(true)*SArray->size );
			}
		}
	}
}


//
// Compare two values, return true, if they are matched.
// Pass entire array to test.
//
Bool CTypeInfo::CompareValue( const void* A, const void* B ) const
{
	if( ArrayDim != -1 )
	{
		// Simple value.
		if( Type == TYPE_String )
		{
			for( Int32 i=0; i<ArrayDim; i++ )
				if( ((String*)A)[i] != ((String*)B)[i] )
					return false;

			return true;
		}
		else if( Type == TYPE_Struct )
		{
			for( Int32 i=0; i<ArrayDim; i++ )
			{
				SizeT Offset = i * Struct->Size;
				if( !Struct->CompareValues( (UInt8*)A+Offset, (UInt8*)B+Offset ) )
					return false;
			}
			return true;
		}
		else
		{
			return mem::cmp( A, B, TypeSize(false) );
		}
	}
	else
	{
		// Dynamic array.
		ArrayPOD* AArray = (ArrayPOD*)A;
		ArrayPOD* BArray = (ArrayPOD*)B;
		if( AArray->size != BArray->size )
			return false;

		if( AArray->size > 0 )
		{
			CTypeInfo Elems( Type, AArray->size, Inner );
			return Elems.CompareValue( AArray->data, BArray->data );
		}
		return true;
	}
}


//
// Custom version of structs matching. This hard way
// required in CCompiler::RestoreAfterSuccess.
//
Bool CustomMatchStructs( const CStruct* A, const CStruct* B )
{
	assert(A && B);

	// Simple matching.
	if( A == B )
		return true;

	// Simple rejection.
	if( A->Size != B->Size )
		return false;
	if( A->Members.size() != B->Members.size() )
		return false;

	// Member by member matching.
	for( Int32 i=0; i<A->Members.size(); i++ )
	{
		if( !A->Members[i]->MatchWith(*B->Members[i]) )
			return false;
	}

	// Identical.
	return true;
}


//
// Match types, return true if
// they are identical.
//
Bool CTypeInfo::MatchWith( const CTypeInfo& Other ) const
{
	if( ArrayDim != Other.ArrayDim )
		return false;

	if( Type == TYPE_Entity )
	{
		// Compare entity by it script.
		return Other.Type == TYPE_Entity && ((Other.Script && Script) ? Other.Script == Script : true);
	}
	else if( Type == TYPE_Resource )
	{
		// Compare resource by it class.
		return Other.Type == TYPE_Resource && ( Class && Other.Class ? Other.Class->IsA( Class ) : true );
	}
	else if( Type == TYPE_Delegate )
	{
		// Each delegate type has unique index in compiler table.
		return  Other.Type == TYPE_Delegate && Other.iSignature == iSignature;
	}
	else if( Type == TYPE_Struct )
	{
		// Match structures.
		return Other.Type == TYPE_Struct && CustomMatchStructs( Struct, Other.Struct );
	}
	else
	{
		// Just compare types.
		return Type == Other.Type;
	}
}


//
// Convert a value to the string, used by ObjectInspector.
//
String CTypeInfo::ToString( const void* Addr ) const
{
	// Don't show entire array.
	if( ArrayDim > 1 || ArrayDim == -1 || Type == TYPE_Struct )
		return L"[...]";

	// Single value.
	switch( Type )
	{
		case TYPE_Byte:
		{
			UInt8 Value = *(UInt8*)Addr;
			if( Enum )
				return Value < Enum->Elements.size() ? *Enum->GetAliasOf(Value) : L"BAD_INDEX";
			else
				return String::FromInteger(Value);
		}
		case TYPE_Bool:
		{
			return *(Bool*)Addr ? L"True" : L"False";
		}
		case TYPE_Integer:
		{
			return String::FromInteger(*(Int32*)Addr);
		}
		case TYPE_Float:
		{
			return String::FromFloat(*(Float*)Addr);
		}
		case TYPE_Angle:
		{
			return String::Format( L"%.2f deg.", (*(math::Angle*)Addr).toDegs() );
		}
		case TYPE_Color:
		{
			TColor Value = *(TColor*)Addr;
			return String::Format( L"#%02x%02x%02x", Value.R, Value.G, Value.B );
		}
		case TYPE_String:
		{
			return *(String*)Addr;
		}	
		case TYPE_Vector:
		{
			math::Vector Value = *(math::Vector*)Addr;
			return String::Format( L"[%.2f, %.2f]", Value.x, Value.y );
		}
		case TYPE_AABB:
		{
			TRect Value = *(TRect*)Addr;
			return String::Format( L"(%2.f, %2.f, %2.f, %2.f )", Value.Min.x, Value.Min.y, Value.Max.x, Value.Max.y );
		}
		case TYPE_Resource:
		{
			FResource* Value = *(FResource**)Addr;
			return Value ? Value->GetName() : L"???";
		}
		case TYPE_Entity:
		{
			FEntity* Value = *(FEntity**)Addr;
			return Value ? Value->GetName() : L"???";
		}
		case TYPE_Delegate:
		{
			return String::Format( L"0x%x", Addr );
		}
		default:
		{
			return String::Format( L"Bad property type %d", Type );
		}
	}
}


//
// Return true if this type require value destruction.
//
Bool CTypeInfo::NeedDestruction() const
{
	if( ArrayDim == -1 )
		return true;
	else if( Type == TYPE_String )
		return true;
	else if( Type == TYPE_Struct )
		return Struct->NeedDestruction();
	else
		return false;
}


//
// Resize dynamic array. Takes care about reallocation and destroying
// out-of-array elements.
//
void CTypeInfo::SetArrayLength( void* Addr, Int32 NewLength ) const
{
	assert(NewLength >= 0);
	assert(ArrayDim == -1);

	ArrayPOD* Array = (ArrayPOD*)Addr;
	CTypeInfo Elem( Type, 1, Inner );

	if( Elem.NeedDestruction() )
	{
		for( Int32 i=NewLength; i<Array->size; i++ )
			Elem.DestroyValue((UInt8*)Array->data + i*Elem.TypeSize(true));
	}

	// Resize.
	array::reallocate( Array->data, Array->size, NewLength, Elem.TypeSize(true) );
}


//
// Serialize property value.
//
void CTypeInfo::SerializeValue( void* Addr, CSerializer& S ) const
{
	// Handle dynamic array serialization.
	if( ArrayDim == -1 )
	{
		ArrayPOD* Array = (ArrayPOD*)Addr;
		if( S.GetMode() == SM_Load )
		{
			Int32 Length;
			Serialize( S, Length );
			SetArrayLength( Addr, Length );
		}
		else
		{
			Serialize( S, Array->size );
		}

		if( Array->size > 0 )
		{
			CTypeInfo Elems( Type, Array->size, Inner );
			Elems.SerializeValue( Array->data, S );
		}
		return;
	}

	// Handle simple types.
	switch( Type )
	{
		case TYPE_Byte:
		case TYPE_Bool:
		case TYPE_Integer:
		case TYPE_Float:
		case TYPE_Angle:
		case TYPE_Color:
		case TYPE_Vector:
		case TYPE_AABB:
			// Simple values.
			S.SerializeData( Addr, TypeSize(false) );
			break;

		case TYPE_Delegate:
			// List of delegates.
			for( Int32 i=0; i<ArrayDim; i++ )
				Serialize( S, ((TDelegate*)Addr)[i] );
			break;

		case TYPE_String:
			// List of strings.
			for( Int32 i=0; i<ArrayDim; i++ )
				Serialize( S, ((String*)Addr)[i] );
			break;

		case TYPE_Entity:
			// Entities.
			for( Int32 i=0; i<ArrayDim; i++ )
				Serialize( S, ((FEntity**)Addr)[i] );
			break;

		case TYPE_Resource:
			// Entities.
			for( Int32 i=0; i<ArrayDim; i++ )
				Serialize( S, ((FResource**)Addr)[i] );
			break;

		case TYPE_Struct:
			// Struct serialization.
			for( Int32 i=0; i<ArrayDim; i++ )
				Struct->SerializeValues( (UInt8*)Addr + Struct->Size*i, S );
			break;

		default:
			break;
	}
}


//
// Import a property values.
//
void CTypeInfo::ImportValue( void* Addr, CImporterBase& Im, String Prefix ) const
{
	// Handle dynamic array import.
	if( ArrayDim == -1 )
	{
		ArrayPOD* Array = (ArrayPOD*)Addr;
		Int32 RealLen = Im.ImportInteger( *(Prefix+L".Num") );
		SetArrayLength( Addr, RealLen );

		if( Array->size > 0 )
		{
			CTypeInfo Elems( Type, Array->size, Inner );
			Elems.ImportValue( Array->data, Im, Prefix );
		}
		return;
	}

	// Handle simple type import.
	switch( Type )
	{
		case TYPE_Byte:
			// Import byte.
			if( ArrayDim == 1 )
				*(UInt8*)Addr = Im.ImportByte( *Prefix );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					((UInt8*)Addr)[i] = Im.ImportByte( *String::Format( L"%s[%d]", *Prefix, i ) );
			break;

		case TYPE_Bool:
			// Import bool.
			if( ArrayDim == 1 )
				*(Bool*)Addr = Im.ImportBool( *Prefix );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					((Bool*)Addr)[i] = Im.ImportBool( *String::Format( L"%s[%d]", *Prefix, i ) );
			break;

		case TYPE_Integer:
			// Import integer.
			if( ArrayDim == 1 )
				*(Int32*)Addr = Im.ImportInteger( *Prefix );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					((Int32*)Addr)[i] = Im.ImportInteger( *String::Format( L"%s[%d]", *Prefix, i ) );
			break;

		case TYPE_Float:
			// Import float.
			if( ArrayDim == 1 )
				*(Float*)Addr = Im.ImportFloat( *Prefix );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					((Float*)Addr)[i] = Im.ImportFloat( *String::Format( L"%s[%d]", *Prefix, i ) );
			break;

		case TYPE_Angle:
			// Import angle.
			if( ArrayDim == 1 )
				*(math::Angle*)Addr = Im.ImportAngle( *Prefix );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					((math::Angle*)Addr)[i] = Im.ImportAngle( *String::Format( L"%s[%d]", *Prefix, i ) );
			break;

		case TYPE_Color:
			// Import color.
			if( ArrayDim == 1 )
				*(TColor*)Addr = Im.ImportColor( *Prefix );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					((TColor*)Addr)[i] = Im.ImportColor( *String::Format( L"%s[%d]", *Prefix, i ) );
			break;

		case TYPE_String:
			// Import string.
			if( ArrayDim == 1 )
				*(String*)Addr = Im.ImportString( *Prefix );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					((String*)Addr)[i] = Im.ImportString( *String::Format( L"%s[%d]", *Prefix, i ) );
			break;

		case TYPE_Vector:
			// Import vector.
			if( ArrayDim == 1 )
				*(math::Vector*)Addr = Im.ImportVector( *Prefix );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					((math::Vector*)Addr)[i] = Im.ImportVector( *String::Format( L"%s[%d]", *Prefix, i ) );
			break;

		case TYPE_AABB:
			// Import rect.
			if( ArrayDim == 1 )
				*(TRect*)Addr = Im.ImportAABB( *Prefix );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					((TRect*)Addr)[i] = Im.ImportAABB( *String::Format( L"%s[%d]", *Prefix, i ) );
			break;

		case TYPE_Resource:
		case TYPE_Entity:
			// Import object.
			if( ArrayDim == 1 )
				*(FObject**)Addr = Im.ImportObject( *Prefix );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					((FObject**)Addr)[i] = Im.ImportObject( *String::Format( L"%s[%d]", *Prefix, i ) );
			break;

		case TYPE_Delegate:
			// Import delegate.
			//assert(false && L"Not implemented");
			break;

		case TYPE_Struct:
			// Export struct.
			if( ArrayDim == 1 )
				Struct->ImportValues( Addr, Im, Prefix );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					Struct->ImportValues
					(
						(UInt8*)Addr + Struct->Size * i,
						Im,
						String::Format( L"%s[%d]", *Prefix, i )
					);
			break;


	}
}


//
// Export a property values.
//
void CTypeInfo::ExportValue( const void* Addr, CExporterBase& Ex, String Prefix ) const
{
	// Handle dynamic array export.
	if( ArrayDim == -1 )
	{
		ArrayPOD* Array = (ArrayPOD*)Addr;
		Ex.ExportInteger( *(Prefix+L".Num"), Array->size );
		
		if( Array->size > 0 )
		{
			CTypeInfo Elems( Type, Array->size, Inner );
			Elems.ExportValue( Array->data, Ex, Prefix );
		}
		return;
	}

	// Handle simple type export.
	switch( Type )
	{
		case TYPE_Byte:
			// Export byte.
			if( ArrayDim == 1 )
				Ex.ExportByte( *Prefix, *(UInt8*)Addr );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					Ex.ExportByte( *String::Format( L"%s[%d]", *Prefix, i ), ((UInt8*)Addr)[i] );
			break;

		case TYPE_Bool:
			// Export bool.
			if( ArrayDim == 1 )
				Ex.ExportBool( *Prefix, *(Bool*)Addr );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					Ex.ExportBool( *String::Format( L"%s[%d]", *Prefix, i ), ((Bool*)Addr)[i] );
			break;

		case TYPE_Integer:
			// Export integer.
			if( ArrayDim == 1 )
				Ex.ExportInteger( *Prefix, *(Int32*)Addr );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					Ex.ExportInteger( *String::Format( L"%s[%d]", *Prefix, i ), ((Int32*)Addr)[i] );
			break;

		case TYPE_Float:
			// Export float.
			if( ArrayDim == 1 )
				Ex.ExportFloat( *Prefix, *(Float*)Addr );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					Ex.ExportFloat( *String::Format( L"%s[%d]", *Prefix, i ), ((Float*)Addr)[i] );
			break;

		case TYPE_Angle:
			// Export angle.
			if( ArrayDim == 1 )
				Ex.ExportAngle( *Prefix, *(math::Angle*)Addr );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					Ex.ExportAngle( *String::Format( L"%s[%d]", *Prefix, i ), ((math::Angle*)Addr)[i] );
			break;

		case TYPE_Color:
			// Export color.
			if( ArrayDim == 1 )
				Ex.ExportColor( *Prefix, *(TColor*)Addr );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					Ex.ExportColor( *String::Format( L"%s[%d]", *Prefix, i ), ((TColor*)Addr)[i] );
			break;

		case TYPE_String:
			// Export string.
			if( ArrayDim == 1 )
				Ex.ExportString( *Prefix, *(String*)Addr );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					Ex.ExportString( *String::Format( L"%s[%d]", *Prefix, i ), ((String*)Addr)[i] );
			break;

		case TYPE_Vector:
			// Export vector.
			if( ArrayDim == 1 )
				Ex.ExportVector( *Prefix, *(math::Vector*)Addr );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					Ex.ExportVector( *String::Format( L"%s[%d]", *Prefix, i ), ((math::Vector*)Addr)[i] );
			break;

		case TYPE_AABB:
			// Export rect.
			if( ArrayDim == 1 )
				Ex.ExportAABB( *Prefix, *(TRect*)Addr );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					Ex.ExportAABB( *String::Format( L"%s[%d]", *Prefix, i ), ((TRect*)Addr)[i] );
			break;

		case TYPE_Resource:
		case TYPE_Entity:
			// Export object.
			if( ArrayDim == 1 )
				Ex.ExportObject( *Prefix, *(FObject**)Addr );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					Ex.ExportObject( *String::Format( L"%s[%d]", *Prefix, i ), ((FObject**)Addr)[i] );
			break;

		case TYPE_Delegate:
			// Export delegate.
			//assert(false && L"Not implemented");
			break;

		case TYPE_Struct:
			// Export struct.
			if( ArrayDim == 1 )
				Struct->ExportValues( Addr, Ex, Prefix );
			else
				for( Int32 i=0; i<ArrayDim; i++ )
					Struct->ExportValues
					(
						(UInt8*)Addr + Struct->Size * i,
						Ex,
						String::Format( L"%s[%d]", *Prefix, i )
					);
			break;
	}
}


//
// Return true, if type is trivial.
//
Bool CTypeInfo::IsSimpleType() const
{
	if( ArrayDim == -1 || Type == TYPE_Struct )
		return false;

	return true;
}


/*-----------------------------------------------------------------------------
    CEnum implementation.
-----------------------------------------------------------------------------*/

//
// Enumeration native constructor.
//
CEnum::CEnum( const Char* InName, UInt32 InFlags )
	:	Name( InName ),
		Flags( InFlags ),
		Elements()
{
}


//
// Enumeration script constructor.
//
CEnum::CEnum( const Char* InName, UInt32 InFlags, FScript* Script )
	:	Name( InName ),
		Flags( InFlags ),
		Elements()
{
}


//
// Enumeration destructor.
//
CEnum::~CEnum()
{
	Elements.empty();
}


//
// Add new element to enumeration.
//
Int32 CEnum::AddElement( String NewElem )
{
	assert(Elements.find(NewElem)==-1);

	// Add to list.
	Elements.push(NewElem);
	return Elements.size()-1;
}


//
// Find element in the enum and return it index.
// If element not found returns -1.
//
Int32 CEnum::FindElem( String TestName )
{
	return Elements.find(TestName);
}


//
// Return user-friendly
// alias name of i'th element.
//
String CEnum::GetAliasOf( Int32 i )
{
	assert(i>=0 && i<Elements.size());

#if USE_ALIASES
	// Parse name.
	String SourceName = Elements[i];
	if( !(Flags & ENUM_NoAliases) && String::Pos( L"_", SourceName ) != -1 )
	{
		// Regular enum name.
		Int32 i = 0;
		Char Buffer[64] = {}, *Walk = Buffer;
		assert(SourceName.Len() < arraySize(Buffer));
		while( SourceName(i) != '_' )
			i++;
		i++;
		Char PrevChar = '\0';
		while( i < SourceName.Len() )
		{
			Char ThisChar = SourceName(i);
			if( (ThisChar>='A'&&ThisChar<='Z')&&(PrevChar>='a'&&PrevChar<='z') )
				*Walk++ = ' ';
			*Walk++ = ThisChar;
			PrevChar = ThisChar;
			i++;
		}
		return Buffer;
	}
	else
	{
		// Something strange.
		return Elements[i];
	}
#else
	// No alias.
	return Elements[i];
#endif
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/