/*=============================================================================
    FrNative.cpp: Native script functions execution.
    Copyright Sep.2016 Vlad Gordienko.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    CFrame implementation.
-----------------------------------------------------------------------------*/

//
// Execute a native function.
//
void CFrame::ExecuteNative( FEntity* Context, EOpCode Code )
{
	CFrame& Frame = *this;
	switch(	Code )	
	{
		//
		// Engine functions.
		//
		case OP_PlaySoundFX:
		{
			FSound* S		= (FSound*)POP_RESOURCE;
			Float	Gain	= POP_FLOAT;
			Float	Pitch	= POP_FLOAT;
			if( S )
				GApp->GAudio->PlayFX( S, Gain, Pitch );
			break;
		}
		case OP_PlayMusic:
		{
			FMusic*		M		= (FMusic*)POP_RESOURCE;
			Float	FadeTime	= POP_FLOAT;
			GApp->GAudio->PlayMusic( M, FadeTime );
			break;
		}
		case OP_KeyIsPressed:
		{
			Int32 iKey	= POP_INTEGER;
			*POPA_BOOL		= GApp->GInput->KeyIsPressed(iKey);
			break;
		}
		case OP_GetScreenCursor:
		{
			*POPA_VECTOR	= math::Vector( GApp->GInput->MouseX, GApp->GInput->MouseY );
			break;
		}
		case OP_GetWorldCursor:
		{
			*POPA_VECTOR	= GApp->GInput->WorldCursor;
			break;
		}
		case OP_Localize:
		{
			String	File	= POP_STRING;
			String	Section	= POP_STRING;
			String	Key		= POP_STRING;
			*POPA_STRING	= GApp->Config->ReadString( *File, *Section, *Key );
			break;
		}
		case OP_GetScript:
		{
			FEntity* Entity	= POP_ENTITY;
			*POPA_RESOURCE	= Entity ? Entity->Script : nullptr;
			break;
		}
		case OP_StaticPush:
		{
			String	Key		= POP_STRING,
					Value	= POP_STRING;
			GStaticBuffer.Put( Key, Value );
			break;
		}
		case OP_StaticPop:
		{
			String	Key		= POP_STRING,
					Default	= POP_STRING;
			String*	Value	= GStaticBuffer.Get( Key );
			*POPA_STRING	= Value ? *Value : Default;
			break;
		}
		case OP_TravelTo:
		{
			FLevel*	Level	= As<FLevel>(POP_RESOURCE);
			Bool	bCopy	= POP_BOOL;
			if( Level )
			{
				GIncomingLevel.Destination	= Level;
				GIncomingLevel.bCopy		= bCopy;
				GIncomingLevel.Teleportee	= This;
			}
			else
				ScriptError( L"An attempt to travel into oblivion." );
			break;
		}
		case OP_FindEntity:
		{
			String ObjName = POP_STRING;
			if( This )
			{
				FObject* Obj	= GObjectDatabase->FindObject( ObjName, FEntity::MetaClass, This->Level );
				*POPA_ENTITY	= As<FEntity>(Obj);
			}
			else
			{
				*POPA_ENTITY	= nullptr;
			}
			break;
		}
		case OP_MatchKeyCombo:
		{
			String Combo = POP_STRING;
			*POPA_BOOL = GApp->GInput->MatchKeyCombo(Combo);
			break;
		}


		//
		// Math functions.
		//
		case OP_Abs:
		{
			Float A = POP_FLOAT;
			*POPA_FLOAT = abs( A );
			break;
		}
		case OP_ArcTan:
		{
			Float A = POP_FLOAT;
			*POPA_FLOAT = math::arcTan( A );
			break;
		}
		case OP_ArcTan2:
		{
			Float Y = POP_FLOAT;
			Float X = POP_FLOAT;
			*POPA_FLOAT = math::arcTan2( Y, X );
			break;
		}
		case OP_Cos:
		{
			Float A = POP_FLOAT;
			*POPA_FLOAT = math::cos( A );
			break;
		}
		case OP_Sin:
		{
			Float A = POP_FLOAT;
			*POPA_FLOAT = math::sin( A );
			break;
		}
		case OP_Sqrt:
		{
			Float A = POP_FLOAT;
			if( A < 0.f ) ScriptError( L"Negative X in 'sqrt'" );
			*POPA_FLOAT = math::sqrt( A );
			break;
		}
		case OP_Distance:
		{
			math::Vector A = POP_VECTOR;
			math::Vector B = POP_VECTOR;
			*POPA_FLOAT = math::distance( A, B );
			break;
		}
		case OP_Exp:
		{
			Float A = POP_FLOAT;
			*POPA_FLOAT = exp( A );
			break;
		}
		case OP_Ln:
		{
			Float A = POP_FLOAT;
			*POPA_FLOAT = math::ln( A );
			break;
		}
		case OP_Frac:
		{
			Float A = POP_FLOAT;
			*POPA_FLOAT = math::frac( A );
			break;
		}
		case OP_Round:
		{
			Float A = POP_FLOAT;
			*POPA_INTEGER = math::round( A );
			break;
		}
		case OP_Normalize:
		{
			math::Vector A = POP_VECTOR;
			A.normalize();
			*POPA_VECTOR = A;
			break;
		}
		case OP_Random:
		{
			Int32 A = POP_INTEGER;
			*POPA_INTEGER = Random( A );
			break;
		}
		case OP_RandomF:
		{
			*POPA_FLOAT = RandomF();
			break;
		}
		case OP_VectorSize:
		{
			math::Vector A = POP_VECTOR;
			*POPA_FLOAT = A.size();
			break;
		}
		case OP_RGBA:
		{
			UInt8	R	= POP_BYTE;
			UInt8	G	= POP_BYTE;
			UInt8	B	= POP_BYTE;
			UInt8	A	= POP_BYTE;
			*POPA_COLOR	= TColor( R, G, B, A );
			break;
		}
		case OP_CharAt:
		{
			String	S = POP_STRING;
			Int32 i = clamp( POP_INTEGER, 0, S.Len()-1 );
			Char Tmp[2] = { S[i], 0 };
			*POPA_STRING	= String(Tmp);
			break;
		}
		case OP_IndexOf:
		{
			String Needle	= POP_STRING;
			String HayStack	= POP_STRING;
			*POPA_INTEGER = String::Pos( Needle, HayStack );
			break;
		}
		case OP_Execute:
		{
			GApp->ConsoleExecute(POP_STRING);
			break;
		}
		case OP_Now:
		{
			*POPA_FLOAT	= GPlat->Now();
			break;
		}


		//
		// Iterators.
		//
		case IT_AllEntities:
		{
			FScript*	Script	= As<FScript>(POP_RESOURCE);
			FLevel*		Level	= This->Level;
			Foreach.Collection.empty();
			if( Script )
			{
				for( Int32 i=0; i<Level->Entities.size(); i++ )
					if( Level->Entities[i]->Script == Script && !Level->Entities[i]->Base->bDestroyed )
						Foreach.Collection.push(Level->Entities[i]);
			}
			else
			{
				for( Int32 i=0; i<Level->Entities.size(); i++ )
					if( !Level->Entities[i]->Base->bDestroyed )
						Foreach.Collection.push(Level->Entities[i]);
			}
			break;
		}
		case IT_RectEntities:
		{
			FScript*		Script		= As<FScript>(POP_RESOURCE);
			math::Rect		Area		= POP_AABB;
			FLevel*			Level		= This->Level;
			Int32			NumBases	= 0;
			FBaseComponent*	Bases[MAX_COLL_LIST_OBJS];
			Foreach.Collection.empty();		
			if( Script )
				Level->CollHash->GetOverlappedByScript( Area, Script, NumBases, Bases );
			else
				Level->CollHash->GetOverlapped( Area, NumBases, Bases );
			for( Int32 i=0; i<NumBases; i++ )
				Foreach.Collection.push(Bases[i]->Entity);
			break;
		}
		case IT_TouchedEntities:
		{
			Foreach.Collection.empty();
			if( This->Base->IsA(FPhysicComponent::MetaClass) )
			{
				FPhysicComponent* Phys = (FPhysicComponent*)This->Base;
				for( Int32 i=0; i<arraySize(Phys->Touched); i++ )
					if( Phys->Touched[i] )
						Foreach.Collection.push(Phys->Touched[i]);
			}
			break;
		}


		//
		// Simple binary operators.
		//
		#define CASE_BINARY( icode, op, a1, a2, r ) case icode:{ UInt8 iReg=ReadByte(); *(r*)(Regs[iReg].Value) = *(a1*)(Regs[iReg].Value) op *(a2*)(Regs[ReadByte()].Value); break;}
		CASE_BINARY( BIN_Mult_Integer,		*,	Int32,		Int32,		Int32		)
		CASE_BINARY( BIN_Mult_Float,		*,	Float,		Float,		Float		)
		CASE_BINARY( BIN_Mult_Color,		*,	TColor,		TColor,		TColor		)
		CASE_BINARY( BIN_Mult_Vector,		*,	math::Vector,	Float,		math::Vector		)
		CASE_BINARY( BIN_Div_Integer,		/,	Int32,		Int32,		Int32		)
		CASE_BINARY( BIN_Div_Float,			/,	Float,		Float,		Float		)
		CASE_BINARY( BIN_Mod_Integer,		%,	Int32,		Int32,		Int32		)
		CASE_BINARY( BIN_Add_Integer,		+,	Int32,		Int32,		Int32		)
		CASE_BINARY( BIN_Add_Float,			+,	Float,		Float,		Float		)
		CASE_BINARY( BIN_Add_Color,			+,	TColor,		TColor,		TColor		)
		CASE_BINARY( BIN_Add_Vector,		+,	math::Vector,	math::Vector,	math::Vector		)
		CASE_BINARY( BIN_Sub_Integer,		-,	Int32,		Int32,		Int32		)
		CASE_BINARY( BIN_Sub_Float,			-,	Float,		Float,		Float		)
		CASE_BINARY( BIN_Sub_Color,			-,	TColor,		TColor,		TColor		)
		CASE_BINARY( BIN_Sub_Vector,		-,	math::Vector,	math::Vector,	math::Vector		)
		CASE_BINARY( BIN_Shr_Integer,		>>,	Int32,		Int32,		Int32		)
		CASE_BINARY( BIN_Shl_Integer,		<<,	Int32,		Int32,		Int32		)
		CASE_BINARY( BIN_Less_Integer,		<,	Int32,		Int32,		Bool		)
		CASE_BINARY( BIN_Less_Float,		<,	Float,		Float,		Bool		)
		CASE_BINARY( BIN_LessEq_Integer,	<=,	Int32,		Int32,		Bool		)
		CASE_BINARY( BIN_LessEq_Float,		<=,	Float,		Float,		Bool		)
		CASE_BINARY( BIN_Greater_Integer,	>,	Int32,		Int32,		Bool		)
		CASE_BINARY( BIN_Greater_Float,		>,	Float,		Float,		Bool		)
		CASE_BINARY( BIN_GreaterEq_Integer,	>=,	Int32,		Int32,		Bool		)
		CASE_BINARY( BIN_GreaterEq_Float,	>=,	Float,		Float,		Bool		)
		CASE_BINARY( BIN_And_Integer,		&,	Int32,		Int32,		Int32		)
		CASE_BINARY( BIN_Xor_Integer,		^,	Int32,		Int32,		Int32		)
		CASE_BINARY( BIN_Cross_Vector,		/,	math::Vector,	math::Vector,	Float		)
		CASE_BINARY( BIN_Or_Integer,		|,	Int32,		Int32,		Int32		)
		CASE_BINARY( BIN_Dot_Vector,		*,	math::Vector,	math::Vector,	Float		)
		#undef CASE_BINARY

		//
		// Assignment binary operators.
		//
		#define CASE_ASSIGN( icode, op, a1, a2, r ) case icode: { UInt8 iReg=ReadByte(); *(r*)Regs[iReg].Addr op *(a2*)Regs[ReadByte()].Value; break;}
		CASE_ASSIGN( BIN_AddEqual_Integer,	+=,		Int32,		Int32,		Int32 )
		CASE_ASSIGN( BIN_AddEqual_Float,	+=,		Float,		Float,		Float )
		CASE_ASSIGN( BIN_AddEqual_Vector,	+=,		math::Vector,	math::Vector,	math::Vector )
		CASE_ASSIGN( BIN_AddEqual_Color,	+=,		TColor,		TColor,		TColor )
		CASE_ASSIGN( BIN_SubEqual_Integer,	-=,		Int32,		Int32,		Int32 )
		CASE_ASSIGN( BIN_SubEqual_Float,	-=,		Float,		Float,		Float )
		CASE_ASSIGN( BIN_SubEqual_Vector,	-=,		math::Vector,	math::Vector,	math::Vector )
		CASE_ASSIGN( BIN_SubEqual_Color,	-=,		TColor,		TColor,		TColor )
		CASE_ASSIGN( BIN_MulEqual_Integer,	*=,		Int32,		Int32,		Int32 )
		CASE_ASSIGN( BIN_MulEqual_Float,	*=,		Float,		Float,		Float )
		CASE_ASSIGN( BIN_MulEqual_Color,	*=,		TColor,		TColor,		TColor )
		CASE_ASSIGN( BIN_DivEqual_Integer,	/=,		Int32,		Int32,		Int32 )
		CASE_ASSIGN( BIN_DivEqual_Float,	/=,		Float,		Float,		Float )
		CASE_ASSIGN( BIN_ModEqual_Integer,	%=,		Int32,		Int32,		Int32 )
		CASE_ASSIGN( BIN_ShlEqual_Integer,	<<=,	Int32,		Int32,		Int32 )
		CASE_ASSIGN( BIN_ShrEqual_Integer,	>>=,	Int32,		Int32,		Int32 )
		CASE_ASSIGN( BIN_AndEqual_Integer,	&=,		Int32,		Int32,		Int32 )
		CASE_ASSIGN( BIN_XorEqual_Integer,	^=,		Int32,		Int32,		Int32 )
		CASE_ASSIGN( BIN_OrEqual_Integer,	|=,		Int32,		Int32,		Int32 )
		#undef CASE_ASSIGN

		//
		// Simple unary operators.
		//
		#define CASE_UNARY( icode, op, type ) case icode:{UInt8 iReg=ReadByte(); *(type*)(Regs[iReg].Value) = op *(type*)(Regs[iReg].Value); break;}
		CASE_UNARY( UN_Plus_Integer,		+,		Int32 );
		CASE_UNARY( UN_Plus_Float,			+,		Float );
		CASE_UNARY( UN_Plus_Vector,			+,		math::Vector );
		CASE_UNARY( UN_Plus_Color,			+,		TColor );
		CASE_UNARY( UN_Minus_Integer,		-,		Int32 );
		CASE_UNARY( UN_Minus_Float,			-,		Float );
		CASE_UNARY( UN_Minus_Vector,		-,		math::Vector );
		CASE_UNARY( UN_Minus_Color,			-,		TColor );
		CASE_UNARY( UN_Not_Bool,			!,		Bool );
		CASE_UNARY( UN_Not_Integer,			~,		Int32 );
		#undef CASE_UNARY

		//
		// Suffix / prefix operators.
		//
		#define CASE_PREFIX( icode, op, type ) case icode:{(*(type*)(Regs[ReadByte()].Addr))op; break;}
		CASE_PREFIX( UN_Inc_Integer,		++,		Int32 );
		CASE_PREFIX( UN_Inc_Float,			++,		Float );
		CASE_PREFIX( UN_Dec_Integer,		--,		Int32 );
		CASE_PREFIX( UN_Dec_Float,			--,		Float );
		#undef CASE_PREFIX

		//
		// Ugly string operators.
		//
		case BIN_AddEqual_String:
		{
			UInt8 iReg = ReadByte();
			*(String*)Regs[iReg].Addr += Regs[ReadByte()].StrValue;
			break;
		}
		case BIN_Add_String:
		{
			UInt8 iReg=ReadByte();
			Regs[iReg].StrValue += Regs[ReadByte()].StrValue;
			break;
		}

		//
		// Invalid opcode.
		//
		default:
		{
			// Bad instruction.
			ScriptError( L"Unknown instruction '%i'", Code );
			break;
		}
	}
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/