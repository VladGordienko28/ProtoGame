/*=============================================================================
	FrCode.cpp: Flu script virtual machine.
	Created by Vlad Gordienko, Jul. 2016.
	Reimplemented by Vlad, Jan. 2018.
=============================================================================*/

#include "Engine.h"

/*-----------------------------------------------------------------------------
    CFrame implementation.
-----------------------------------------------------------------------------*/

//
// Static memory for script calling stack.
// 512 kB should be enough.
//
CStaticPool<512*1024> CFrame::GLocalsMem;


//
// Initialize frame for the function call.
//
CFrame::CFrame( FEntity* InThis, CFunction* InFunction, Int32 InDepth, CFrame* InPrevFrame )
	:	This( InThis ),
		Script( InThis->Script ),
		Bytecode( InFunction ),
		PrevFrame( InPrevFrame ),
		Depth( InDepth ),
		Code( &InFunction->Code[0] ),
		Locals( nullptr )
{
	// Test recursion depth.
	if( InDepth > MAX_RECURSION_DEPTH )
		ScriptError( L"Stack overflow" );

	// Allocate locals.
	Locals	= (UInt8*)GLocalsMem.Push0( InFunction->FrameSize );
}


//
// Initialize frame for the static function call.
//
CFrame::CFrame( FScript* InScript, CFunction* InStatic, Int32 InDepth, CFrame* InPrevFrame )
	:	This( nullptr ),
		Script( InScript ),
		Bytecode( InStatic ),
		PrevFrame( InPrevFrame ),
		Depth( InDepth ),
		Code( &InStatic->Code[0] ),
		Locals( nullptr )
{
	// Test recursion depth.
	if( InDepth > MAX_RECURSION_DEPTH )
		ScriptError( L"Stack overflow" );

	// Allocate locals.
	Locals	= (UInt8*)GLocalsMem.Push0( InStatic->FrameSize );
}


//
// Initialize frame for
// the entity thread execution.
//
CFrame::CFrame( FEntity* InThis, CThreadCode* InThread )
	:	Depth( 0 ),
		Bytecode( InThread ),
		PrevFrame( nullptr ),
		Script( InThis->Script ),
		This( InThis ),
		Locals( nullptr ),
		Code( &InThread->Code[0] )
{
}


//
// Trace the script calling stack, start from
// the current frame.
//
String CFrame::StackTrace()
{
	String Result;
	CFrame*	Walk	= this;

	while( Walk )
	{
		Result += String::Format
							( 
								L"<- %s::%s", 
								*Walk->Script->GetName(),
								Walk->Bytecode == Walk->Script->Thread ?
													L"Thread" :
													*((CFunction*)Walk->Bytecode)->Name
							);

		Walk = Walk->PrevFrame;
	}

	return Result;
}


//
// Raise a script exception.
//
void CFrame::ScriptError( Char* Fmt, ... )
{
	Char Dest[2048] = {};
	va_list ArgPtr;
	va_start( ArgPtr, Fmt );
	_vsnwprintf_s( Dest, 1024, Fmt, ArgPtr );
	va_end( ArgPtr );

	LogManager::instance().handleFatalScriptMessage
		( 
			L"Runtime error at %s(%s::%s). Message='%s'. History: %s.", 
			*This->GetFullName(),
			*Script->GetName(),
			Script->Thread == Bytecode ? L"Thread" : *((CFunction*)Bytecode)->Name,
			Dest,
			*StackTrace()
		);

	// Interrupt execution.
	throw nullptr;
}


//
// Frame destructor.
//
CFrame::~CFrame()
{
	if( Locals )
	{
		CFunction* Func = (CFunction*)Bytecode;

		for( Int32 i=0; i<Func->Locals.size(); i++ )
		{
			CProperty* L = Func->Locals[i];
			L->DestroyValue( Locals + L->Offset );
		}

		GLocalsMem.Pop(Locals);
	}  
}


/*-----------------------------------------------------------------------------
    FEntity implementation.
-----------------------------------------------------------------------------*/

//
// Call a script event.
//
void FEntity::CallFunction( String FuncName, const CVariant& A1, const CVariant& A2, const CVariant& A3, const CVariant& A4 )
{
	// Don't call if no text.
	if( !Script->IsScriptable() || Script->Methods.size()==0 )
		return;

	// Try to find event.
	CFunction* Function = Script->FindMethod(FuncName);
	if( !Function )
	{
		info(L"FEntity::CallFunction: Function '%s' not found", *FuncName);
		return;
	}

	// Execute code!
	try
	{
		// Create new frame frame.
		CFrame Frame( this, Function, 1, nullptr );

		// Parse parameters.
		const CVariant* Args[4] = { &A1, &A2, &A3, &A4 };
		for( Int32 iArg=0; iArg<4 && iArg<Function->ParmsCount; iArg++ )
		{
			// Everything parsed.
			if( Args[iArg]->Type == TYPE_None )
				break;

			CProperty* Parm = Function->Locals[iArg];

			// Match types.
			if( Parm->Type != Args[iArg]->Type )
			{
				// Mismatched.
				LogManager::instance().handleFatalScriptMessage
				(
					L"Function call %s(%s::%s) from C++ failed. Argument %d types mismatched '%s' and '%s'",
					*GetFullName(),
					*Script->GetName(),
					*FuncName,
					iArg+1,
					*Parm->TypeName(),
					*CTypeInfo(Args[iArg]->Type).TypeName()
				);
				throw nullptr;
			}

			// Copy arguments.
			if( Parm->Type != TYPE_String )
				Parm->CopyValue( &Frame.Locals[Parm->Offset], Args[iArg]->Value );
			else
				*(String*)&Frame.Locals[Parm->Offset] = Args[iArg]->StringValue;
		}

		// Execute the code!
		Frame.ProcessCode( nullptr );
	}
	catch( ... )
	{
		// Script crashes :(
		// But abort only current frame.
		LogManager::instance().handleFatalScriptMessage( L"CallFunction: Runtime error" );
	}
}


/*-----------------------------------------------------------------------------
	FScript implementation.
-----------------------------------------------------------------------------*/

//
// Call a static function.
//
void FScript::CallStaticFunction( String FuncName, const CVariant& A1, const CVariant& A2, const CVariant& A3, const CVariant& A4 )
{
	// Don't call if no text.
	if( !IsScriptable() || StaticFunctions.size()==0 )
		return;

	// Try to find event.
	CFunction* Function = FindStaticFunction(FuncName);
	if( !Function )
	{
		warn(L"FScript::CallStaticFunction: Function '%s' not found", *FuncName);
		return;
	}

	// Execute code!
	try
	{
		// Create new frame frame.
		CFrame Frame( this, Function, 1, nullptr );

		// Parse parameters.
		const CVariant* Args[4] = { &A1, &A2, &A3, &A4 };
		for( Int32 iArg=0; iArg<4 && iArg<Function->ParmsCount; iArg++ )
		{
			// Everything parsed.
			if( Args[iArg]->Type == TYPE_None )
				break;

			CProperty* Parm = Function->Locals[iArg];

			// Match types.
			if( Parm->Type != Args[iArg]->Type )
			{
				// Mismatched.
				LogManager::instance().handleFatalScriptMessage
				(
					L"Static call %s::%s from C++ failed. Argument %d types mismatched '%s' and '%s'",
					*GetName(),
					*FuncName,
					iArg+1,
					*Parm->TypeName(),
					*CTypeInfo(Args[iArg]->Type).TypeName()
				);
				throw nullptr;
			}

			// Copy arguments.
			if( Parm->Type != TYPE_String )
				Parm->CopyValue( &Frame.Locals[Parm->Offset], Args[iArg]->Value );
			else
				*(String*)&Frame.Locals[Parm->Offset] = Args[iArg]->StringValue;
		}

		// Execute the code!
		Frame.ProcessCode( nullptr );
	}
	catch( ... )
	{
		// Script crashes :(
		// But abort only current frame.
		LogManager::instance().handleFatalScriptMessage( L"CallFunction: Runtime error" );
	}
}


/*-----------------------------------------------------------------------------
    Script execution.
-----------------------------------------------------------------------------*/

//
// Execute script function code.
//
void CFrame::ProcessCode( TRegister* Result )
{
	// Infinity loop detection variables.
	Int32 LoopCounter = 0;

	// Current execution context.
	FEntity* Context = This;

	// Execute it!
	while( *Code != CODE_EOC )
	{
		EOpCode Op = (EOpCode)*Code++;

		switch( Op )
		{
			case CODE_Jump:
			{
				// Immediately jump.
				Code = &Bytecode->Code[ReadWord()];
				if( ++LoopCounter >= MAX_ITERATIONS )
					ScriptError( L"Infinity loop" );
				break;
			}
			case CODE_JumpZero:
			{
				// Conditional jump.
				UInt16 Dst = ReadWord();
				if( !*(Bool*)(Regs[ReadByte()].Value) )
					Code = &Bytecode->Code[Dst];

				if( ++LoopCounter >= MAX_ITERATIONS )
					ScriptError( L"Infinity loop" );
				break;
			}
			case CODE_LToR:
			{
				// General purpose l to r.
				UInt8 iReg = ReadByte();
				mem::copy( Regs[iReg].Value, Regs[iReg].Addr, ReadByte() );
				break;
			}
			case CODE_LToRDWord:
			{
				// DWord l to r.
				UInt8 iReg = ReadByte();
				*((UInt32*)Regs[iReg].Value) = *(UInt32*)Regs[iReg].Addr;
				break;
			}
			case CODE_LToRString:
			{
				// String l to r.
				UInt8 iReg = ReadByte();
				Regs[iReg].StrValue = *(String*)Regs[iReg].Addr;
				break;
			}
			case CODE_Assign:
			{
				// General purpose assignment.
				UInt8 iDst = ReadByte();
				UInt8 iSrc = ReadByte();
				mem::copy( Regs[iDst].Addr, Regs[iSrc].Value, ReadByte() );
				break;
			}
			case CODE_AssignDWord:
			{
				// Assign DWord sized value.
				UInt8 iDst = ReadByte();
				*((UInt32*)Regs[iDst].Addr) = *((UInt32*)Regs[ReadByte()].Value);
				break;
			}
			case CODE_AssignString:
			{
				// String assignment.
				UInt8 iDst = ReadByte();
				*((String*)Regs[iDst].Addr) = Regs[ReadByte()].StrValue;
				break;
			}
			case CODE_LocalVar:
			{
				// Local variable.
				UInt8 iReg = ReadByte();
				Regs[iReg].Addr	= Locals + ReadWord();
				break;
			}
			case CODE_EntityProperty:
			{
				// Get an entity property.
				UInt8 iReg = ReadByte();
				Regs[iReg].Addr = &Context->InstanceBuffer->Data[ReadWord()];
				break;
			}
			case CODE_BaseProperty:
			{
				// Get an base component property.
				UInt8* Base = (UInt8*)Context->Base;
				UInt8 iReg = ReadByte();
				Regs[iReg].Addr = Base + ReadWord();
				break;
			}
			case CODE_ComponentProperty:
			{
				// Get an extra component property.
				UInt8*	Component =	(UInt8*)Context->Components[ReadByte()];
				UInt8 iReg = ReadByte();
				Regs[iReg].Addr = Component + ReadWord();
				break;
			}
			case CODE_ResourceProperty:
			{
				// Get an resource property.
				UInt8 iReg = ReadByte();
				FResource* Res = *(FResource**)Regs[iReg].Value;
				if( !Res )
					ScriptError( L"Access to null resource" );
				Regs[iReg].Addr	= (UInt8*)Res + ReadWord();
				break;
			}
			case CODE_ProtoProperty:
			{
				// Get a prototype property.
				FScript*	Prototype	= ReadScript();
				UInt8		iSource		= ReadByte();
				UInt8		iReg		= ReadByte();
				UInt8*		BaseAddr	=	iSource == 0xff ? (UInt8*)&Prototype->InstanceBuffer->Data[0] :
											iSource == 0xfe ? (UInt8*)Prototype->Base : (UInt8*)Prototype->Components[iSource];
				Regs[iReg].Addr	= BaseAddr + ReadWord();
				break;
			}
			case CODE_StaticProperty:
			{
				// Get a static script property.
				FScript* StaticScript = ReadScript();
				UInt8 iReg = ReadByte();

				Regs[iReg].Addr = &StaticScript->StaticsBuffer->Data[ReadWord()];
				break;
			}
			case CODE_ArrayElem:
			{
				// Get a static array element.
				UInt8 iReg = ReadByte();
				Int32 Index = *(Int32*)Regs[ReadByte()].Value;
				
				Regs[iReg].Addr = (UInt8*)Regs[iReg].Addr + Index * ReadByte();

				Int32 Max = ReadByte();
				if( Index < 0 || Index >= Max )
					ScriptError( L"Static array violates bounds %i/%i", Index, Max );
				break;
			}
			case CODE_DynArrayElem:
			{
				// Get a dynamic array element.
				UInt8 iReg = ReadByte();
				Int32 Index = *(Int32*)Regs[ReadByte()].Value;

				ArrayPOD* Array = (ArrayPOD*)Regs[iReg].Addr;
				if( Index < 0 || Index >= Array->size )
					ScriptError( L"Dynamic array violates bounds %i/%i", Index, Array->size );

				Regs[iReg].Addr = (UInt8*)Array->data + Index * ReadByte();
				break;
			}
			case CODE_LMember:
			{
				// Get an l-value member.
				UInt8 iReg = ReadByte();
				Regs[iReg].Addr = (UInt8*)Regs[iReg].Addr + ReadByte();
				break;
			}
			case CODE_RMember:
			{
				// Get an r-value member.
				UInt8 iReg = ReadByte();
				UInt8 Offset = ReadByte();
				mem::copy( &Regs[iReg].Value[0], &Regs[iReg].Value[Offset], arraySize(Regs[iReg].Value)-Offset );
				break;
			}
			case CODE_DynPop:
			{
				// Pops last dynamic array item.
				UInt8 iReg = ReadByte();
				UInt8 InnerSize = ReadByte();
				ArrayPOD* Array = (ArrayPOD*)Regs[iReg].Addr;

				if( Array->size <= 0 )
					ScriptError( L"Dynamic array underflow" );

				Int32 Index = Array->size - 1;

				if( InnerSize != 0 )
				{
					mem::copy( Regs[iReg].Value, (UInt8*)Array->data + Index*InnerSize, InnerSize );
					array::reallocate( Array->data, Array->size, Array->size-1, InnerSize );
				}
				else
				{
					String* StrPtr = (String*)((UInt8*)Array->data + Index*sizeof(String));
					Regs[iReg].StrValue = *StrPtr;
					StrPtr->~String();
					array::reallocate( Array->data, Array->size, Array->size-1, sizeof(String) );
				}
				break;
			}
			case CODE_DynPush:
			{
				// Dynamic array push.
				UInt8 iReg = ReadByte();
				UInt8 iElem = ReadByte();
				UInt8 InnerSize = ReadByte();
				ArrayPOD* Array = (ArrayPOD*)Regs[iReg].Addr;
				Int32 Index = Array->size;

				if( Index >= DYNAMIC_ARRAY_MAX )
					ScriptError( L"Dynamic array overflow" );

				if( InnerSize != 0 )
				{
					array::reallocate( Array->data, Array->size, Index+1, InnerSize );
					mem::copy( (UInt8*)Array->data + Index*InnerSize, Regs[iElem].Value, InnerSize );
				}
				else
				{
					array::reallocate( Array->data, Array->size, Index+1, sizeof(String) );
					*(String*)((UInt8*)Array->data + Index*sizeof(String)) = Regs[iElem].StrValue;
				}

				*(Int32*)(Regs[iReg].Value) = Index;
				break;
			}
			case CODE_DynRemove:
			{
				// Dynamic array remove item.
				ArrayPOD* Array = (ArrayPOD*)Regs[ReadByte()].Addr;
				Int32 Index = *(Int32*)(Regs[ReadByte()].Value);
				UInt8 InnerSize = ReadByte();

				if( Index < 0 || Index >= Array->size )
					ScriptError( L"Attempt to remove item out of dynamic array size %i/%i", Index, Array->size );

				if( InnerSize != 0 )
				{
					mem::swap
					( 
						(UInt8*)Array->data + Index*InnerSize, 
						(UInt8*)Array->data + (Array->size-1)*InnerSize, 
						InnerSize 
					);
					array::reallocate( Array->data, Array->size, Array->size-1, InnerSize );
				}
				else
				{
					((String*)(UInt8*)Array->data + Index*sizeof(String))->~String();
					mem::swap
					( 
						(UInt8*)Array->data + Index*sizeof(String), 
						(UInt8*)Array->data + (Array->size-1)*sizeof(String), 
						sizeof(String) 
					);
					array::reallocate( Array->data, Array->size, Array->size-1, sizeof(String) );
				}
				break;
			}
			case CODE_DynSetLength:
			{
				// Set dynamic array length.
				ArrayPOD* Array = (ArrayPOD*)Regs[ReadByte()].Addr;
				Int32 NewLength = *(Int32*)(Regs[ReadByte()].Value);
				UInt8 InnerSize = ReadByte();

				if( NewLength < 0 || NewLength >= DYNAMIC_ARRAY_MAX )
					ScriptError( L"Invalid dynamic array new length %i", NewLength );

				if( InnerSize != 0 )
				{
					array::reallocate( Array->data, Array->size, NewLength, InnerSize );
				}
				else
				{
					for( Int32 i=NewLength; i<Array->size; i++ )
						((String*)((UInt8*)Array->data + i*sizeof(String)))->~String();

					array::reallocate( Array->data, Array->size, NewLength, sizeof(String) );
				}
				break;
			}
			case CODE_DynGetLength:
			{
				// Get dynamic array length.
				UInt8 iReg = ReadByte();
				ArrayPOD* Array = (ArrayPOD*)Regs[iReg].Addr;
				*(Int32*)(Regs[iReg].Value) = Array->size;
				break;
			}
			case CODE_This:
			{
				// This reference.
				*(FEntity**)(Regs[ReadByte()].Value) = This;
				break;
			}
			case CODE_ConstByte:
			{
				// Byte constant.
				UInt8 Value = ReadByte();
				*(UInt8*)(Regs[ReadByte()].Value) = Value;
				break;
			}
			case CODE_ConstBool:
			{
				// Bool constant.
				Bool Value = ReadBool();
				*(Bool*)(Regs[ReadByte()].Value) = Value;
				break;
			}
			case CODE_ConstInteger:
			{
				// Integer constant.
				Int32 Value = ReadInteger();
				*(Int32*)(Regs[ReadByte()].Value) = Value;
				break;
			}
			case CODE_ConstFloat:
			{
				// Float constant.
				Float Value = ReadFloat();
				*(Float*)(Regs[ReadByte()].Value) = Value;
				break;
			}
			case CODE_ConstAngle:
			{
				// Angle constant.
				math::Angle Value = ReadAngle();
				*(math::Angle*)(Regs[ReadByte()].Value) = Value;
				break;
			}
			case CODE_ConstColor:
			{
				// Color constant.
				TColor Value = ReadColor();
				*(TColor*)(Regs[ReadByte()].Value) = Value;
				break;
			}
			case CODE_ConstString:
			{
				// String constant.
				String Value = ReadString();
				Regs[ReadByte()].StrValue = Value;
				break;
			}
			case CODE_ConstVector:
			{
				// Vector constant.
				math::Vector Value = ReadVector();
				*(math::Vector*)(Regs[ReadByte()].Value) = Value;
				break;
			}
			case CODE_ConstAABB:
			{
				// TRect constant.
				TRect Value = ReadAABB();
				*(TRect*)(Regs[ReadByte()].Value) = Value;
				break;
			}
			case CODE_ConstResource:
			{
				// FResource constant.
				FResource* Value = ReadResource();
				*(FResource**)(Regs[ReadByte()].Value) = Value;
				break;
			}
			case CODE_ConstEntity:
			{
				// FEntity constant.
				FEntity* Value = ReadEntity();
				*(FEntity**)(Regs[ReadByte()].Value) = Value;
				break;
			}
			case CODE_ConstDelegate:
			{
				// TDelegate constant.
				TDelegate Value;
				Value.iMethod = ReadInteger();
				Value.Script = ReadScript();
				Value.Context = ReadEntity();
				*(TDelegate*)(Regs[ReadByte()].Value) = Value;
				break;
			}
			case CODE_Assert:
			{
				// Assertion.
				UInt16 Line = ReadWord();
				if( !*(Bool*)(Regs[ReadByte()].Value) )
					ScriptError( L"Assertion failed in line %d", Line );
				break;
			}
			case CODE_Log:
			{
				// C-style output function, its really SLOW!!
				ELogLevel LogLevel = static_cast<ELogLevel>( ReadByte() );
				String Fmt = ReadString();
				Char Out[256], *Str = Out;

				for( Int32 i=0; i<Fmt.Len(); i++ )
				{
					if( Fmt[i] != L'%' )
					{
						*Str++ = Fmt[i];
					}
					else
					{
						CTypeInfo Info = CTypeInfo( ReadPropType() );
						UInt8 iReg = ReadByte();
						String Value = Info.Type == TYPE_String ? Regs[iReg].StrValue : Info.ToString( Regs[iReg].Value );

						for( Int32 j=0; j<Value.Len(); j++ )
							*Str++ = Value[j];
						i++;
					}
				}

				*Str = 0;
				LogManager::instance().handleScriptMessage( LogLevel, Out );
				break;
			}
			case CODE_EntityCast:
			{
				// Entity explicit cast.
				UInt8 iReg			= ReadByte();
				FScript* DstType	= ReadScript();
				FEntity* Value		= *(FEntity**)Regs[iReg].Value;
				if( Value && Value->Script != DstType )
					ScriptError
							( 
								L"Invalid script typecast '%s' to '%s'", 
								*Value->Script->GetName(), 
								*DstType->GetName() 
							);
				break;
			}
			case CODE_FamilyCast:
			{
				// Entity explicit family cast.
				UInt8	iReg		= ReadByte();
				Int32	iFamily		= ReadInteger();
				FEntity* Value		= *(FEntity**)Regs[iReg].Value;

				if( Value && Value->Script->iFamily != iFamily )
					ScriptError
							( 
								L"Invalid family typecast '%d' to '%d'", 
								Value->Script->iFamily, 
								iFamily
							);
				break;
			}
			case CODE_Length:
			{
				// String length.
				UInt8 iReg = ReadByte();
				*(Int32*)(Regs[iReg].Value) = Regs[iReg].StrValue.Len();
				break;
			}
			case CODE_New:
			{
				// Create a new entity.
				static String TempName = L"Temp";
				FLevel* Level		= This->Level;
				FScript* Script		= As<FScript>(*(FObject**)Regs[ReadByte()].Value);
				if( !Script )
					ScriptError( L"Failed create entity, meta-script is not specified" );

				*(FEntity**)(Regs[ReadByte()].Value) = Level->CreateEntity( Script, TempName, This->Base->Location );
				break;
			}
			case CODE_Delete:
			{
				// Delete an entity.
				FEntity* Poor = *(FEntity**)(Regs[ReadByte()].Value);
				if( Poor )
					Poor->Base->bDestroyed	= true;
				else
					ScriptError( L"An attempt to delete undefined entity" );
				break;
			}
			case CODE_VectorCnstr:
			{
				// Vector constructor.
				math::Vector* VectorPtr = (math::Vector*)Regs[ReadByte()].Value;
				VectorPtr->x = *(Float*)Regs[ReadByte()].Value;	
				VectorPtr->y = *(Float*)Regs[ReadByte()].Value;	
				break;
			}
			case CODE_DelegateCnstr:
			{
				// Delegate construction.
				//!! Need to be extended.
				TDelegate* DelegatePtr = (TDelegate*)Regs[ReadByte()].Addr;
				DelegatePtr->iMethod = ReadInteger();
				DelegatePtr->Script = Script;
				DelegatePtr->Context = This;
				break;
			}
			case CODE_Label:
			{
				// Current label id.
				*(Int32*)Regs[ReadByte()].Value = This->Thread->LabelId;
				break;
			}
			case CODE_Is:
			{
				// Test entity script.
				UInt8 iReg = ReadByte();
				FEntity* Entity = *(FEntity**)(Regs[iReg].Value);
				FScript* Test	= *(FScript**)(Regs[ReadByte()].Value);
				if( !Test )
					ScriptError( L"'is' failure, meta-script is null" );

				*(Bool*)(Regs[iReg].Value) = Entity ? Entity->Script == Test : false;
				break;
			}
			case CODE_In:
			{
				// Test entity family.
				UInt8 iReg = ReadByte();
				FEntity* Entity = *(FEntity**)(Regs[iReg].Value);
				Int32	iFamily	= ReadInteger();
				*(Bool*)(Regs[iReg].Value) = Entity ? Entity->Script->iFamily == iFamily : false;
				break;
			}
			case CODE_Equal:
			{
				// Comparison operator "==".
				UInt8 i1	= ReadByte();
				UInt8 i2 = ReadByte();	
				UInt8 Size = ReadByte();
				*(Bool*)(Regs[i1].Value) = Size != 0 ? mem::cmp( Regs[i1].Value, Regs[i2].Value, Size ) : Regs[i1].StrValue == Regs[i2].StrValue;
				break;
			}
			case CODE_NotEqual:
			{
				// Comparison operator "!=".
				UInt8 i1	= ReadByte();
				UInt8 i2 = ReadByte();	
				UInt8 Size = ReadByte();
				*(Bool*)(Regs[i1].Value) = Size != 0 ? !mem::cmp( Regs[i1].Value, Regs[i2].Value, Size ) : Regs[i1].StrValue != Regs[i2].StrValue;
				break;
			}
			case CODE_ConditionalOp:
			{
				// Ternary if.
				UInt8 iRes		= ReadByte();
				UInt8 iFirst		= ReadByte();
				UInt8 iSecond	= ReadByte();
				UInt8 Decision	= *(Bool*)(Regs[ReadByte()].Value) ? iFirst : iSecond;

				mem::copy( Regs[iRes].Value, Regs[Decision].Value, sizeof(TRegister::Value) );
				Regs[iRes].StrValue = Regs[Decision].StrValue;
				break;
			}
			case CAST_ByteToBool:
			{
				// Byte to bool cast.
				UInt8 iReg = ReadByte();
				*(Bool*)Regs[iReg].Value = *(UInt8*)Regs[iReg].Value;
				break;
			}
			case CAST_ByteToInteger:
			{
				// Byte to integer cast.
				UInt8 iReg = ReadByte();
				*(Int32*)Regs[iReg].Value = *(UInt8*)Regs[iReg].Value;
				break;
			}
			case CAST_ByteToFloat:
			{
				// Byte to float cast.
				UInt8 iReg = ReadByte();
				*(Float*)Regs[iReg].Value = *(UInt8*)Regs[iReg].Value;
				break;
			}
			case CAST_ByteToString:
			{
				// Byte to string cast.
				UInt8 iReg = ReadByte();
				Regs[iReg].StrValue = String::FromInteger(*(UInt8*)Regs[iReg].Value);
				break;
			}
			case CAST_BoolToInteger:
			{
				// Bool to integer cast.
				UInt8 iReg = ReadByte();
				*(Int32*)Regs[iReg].Value = *(Bool*)Regs[iReg].Value;
				break;
			}
			case CAST_BoolToString:
			{
				// Bool to string cast.
				UInt8 iReg = ReadByte();
				Regs[iReg].StrValue = *(Bool*)Regs[iReg].Value ? L"true" : L"false";
				break;
			}
			case CAST_IntegerToByte:
			{
				// Integer to byte cast.
				UInt8 iReg = ReadByte();
				*(UInt8*)Regs[iReg].Value = *(Int32*)Regs[iReg].Value;
				break;
			}
			case CAST_IntegerToBool:
			{
				// Integer to bool cast.
				UInt8 iReg = ReadByte();
				*(Bool*)Regs[iReg].Value = *(Int32*)Regs[iReg].Value;
				break;
			}
			case CAST_IntegerToFloat:
			{
				// Integer to float cast.
				UInt8 iReg = ReadByte();
				*(Float*)Regs[iReg].Value = *(Int32*)Regs[iReg].Value;
				break;
			}
			case CAST_IntegerToAngle:
			{
				// Integer to angle cast.
				UInt8 iReg = ReadByte();
				*(math::Angle*)Regs[iReg].Value = *(Int32*)Regs[iReg].Value;
				break;
			}
			case CAST_IntegerToColor:
			{
				// Integer to angle cast.
				UInt8 iReg = ReadByte();
				(*(TColor*)Regs[iReg].Value).D = *(UInt32*)Regs[iReg].Value;
				break;
			}
			case CAST_IntegerToString:
			{
				// Integer to string cast.
				UInt8 iReg = ReadByte();
				Regs[iReg].StrValue = String::FromInteger(*(Int32*)Regs[iReg].Value);
				break;
			}
			case CAST_FloatToByte:
			{
				// Float to byte cast.
				UInt8 iReg = ReadByte();
				*(UInt8*)Regs[iReg].Value = *(Float*)Regs[iReg].Value;
				break;
			}
			case CAST_FloatToBool:
			{
				// Float to bool cast.
				UInt8 iReg = ReadByte();
				*(Bool*)Regs[iReg].Value = *(Float*)Regs[iReg].Value != 0.f;
				break;
			}
			case CAST_FloatToInteger:
			{
				// Float to integer cast.
				UInt8 iReg = ReadByte();
				*(Int32*)Regs[iReg].Value = *(Float*)Regs[iReg].Value;
				break;
			}
			case CAST_FloatToAngle:
			{
				// Float to angle cast.
				UInt8 iReg = ReadByte();
				*(math::Angle*)Regs[iReg].Value = math::Angle::fromRads(*(Float*)Regs[iReg].Value);
				break;
			}
			case CAST_FloatToString:
			{
				// Float to string cast.
				UInt8 iReg = ReadByte();
				Regs[iReg].StrValue = String::FromFloat(*(Float*)Regs[iReg].Value);
				break;
			}
			case CAST_AngleToInteger:
			{
				// Angle to integer cast.
				UInt8 iReg = ReadByte();
				*(Int32*)Regs[iReg].Value = *(math::Angle*)Regs[iReg].Value;
				break;
			}
			case CAST_AngleToFloat:
			{
				// Angle to float cast.
				UInt8 iReg = ReadByte();
				*(Float*)Regs[iReg].Value = (*(math::Angle*)Regs[iReg].Value).toRads();
				break;
			}
			case CAST_AngleToString:
			{
				// Angle to string cast.
				UInt8 iReg = ReadByte();
				Regs[iReg].StrValue = String::FromFloat((*(math::Angle*)Regs[iReg].Value).toDegs());
				break;
			}
			case CAST_AngleToVector:
			{
				// Angle to vector cast.
				UInt8 iReg = ReadByte();
				*(math::Vector*)Regs[iReg].Value = math::angleToVector(*(math::Angle*)Regs[iReg].Value);
				break;
			}
			case CAST_ColorToInteger:
			{
				// Color to integer cast.
				UInt8 iReg = ReadByte();
				*(Int32*)Regs[iReg].Value = (*(TColor*)Regs[iReg].Value).D;
				break;
			}
			case CAST_ColorToString:
			{
				// Color to string cast.
				UInt8 iReg = ReadByte();
				TColor Value = *(TColor*)Regs[iReg].Value;
				Regs[iReg].StrValue = String::Format( L"#%02x%02x%02x", Value.R, Value.G, Value.B );
				break;
			}
			case CAST_StringToByte:
			{
				// Color to byte cast.
				UInt8 iReg = ReadByte();
				Int32 i;
				Regs[iReg].StrValue.ToInteger( i, 0 );
				*(UInt8*)Regs[iReg].Value = i;
				break;
			}
			case CAST_StringToBool:
			{
				// String to bool cast.
				UInt8 iReg = ReadByte();
				*(Bool*)Regs[iReg].Value = Regs[iReg].StrValue == L"true";
				break;
			}
			case CAST_StringToInteger:
			{
				// String to integer cast.
				UInt8 iReg = ReadByte();
				Regs[iReg].StrValue.ToInteger( *(Int32*)Regs[iReg].Value, 0 );
				break;
			}
			case CAST_StringToFloat:
			{
				// String to float cast.
				UInt8 iReg = ReadByte();
				Regs[iReg].StrValue.ToFloat( *(Float*)Regs[iReg].Value, 0.f );
				break;
			}
			case CAST_VectorToBool:
			{
				// Vector to bool cast.
				UInt8 iReg = ReadByte();
				*(Bool*)Regs[iReg].Value = (*(math::Vector*)Regs[iReg].Value).sizeSquared() < 0.01f;
				break;
			}
			case CAST_VectorToAngle:
			{
				// Vector to angle cast.
				UInt8 iReg = ReadByte();
				*(math::Angle*)Regs[iReg].Value = math::vectorToAngle(*(math::Vector*)Regs[iReg].Value);
				break;
			}
			case CAST_VectorToString:
			{
				// Vector to string cast.
				UInt8 iReg = ReadByte();
				math::Vector Value = *(math::Vector*)Regs[iReg].Value;
				Regs[iReg].StrValue = String::Format( L"[%.2f, %.2f]", Value.x, Value.y );
				break;
			}
			case CAST_AabbToBool:
			{
				// Aabb to bool cast.
				UInt8 iReg = ReadByte();
				TRect Value = *(TRect*)Regs[iReg].Value;
				*(Bool*)Regs[iReg].Value = Value.Min != Value.Max;
				break;
			}
			case CAST_AabbToStrnig:
			{
				// Aabb to string cast.
				UInt8 iReg = ReadByte();
				TRect Value = *(TRect*)Regs[iReg].Value;
				Regs[iReg].StrValue = String::Format( L"(%2.f, %2.f, %2.f, %2.f )", Value.Min.x, Value.Min.y, Value.Max.x, Value.Max.y );
				break;
			}
			case CAST_ResourceToBool:
			{
				// Resource to bool cast.
				UInt8 iReg = ReadByte();
				*(Bool*)Regs[iReg].Value = *(FResource**)Regs[iReg].Value != nullptr;
				break;
			}
			case CAST_ResourceToString:
			{
				// Resource to entity cast.
				UInt8 iReg = ReadByte();
				FResource* Res = *(FResource**)Regs[iReg].Value;
				Regs[iReg].StrValue = Res ? Res->GetName() : L"none";
				break;
			}
			case CAST_EntityToBool:
			{
				// Entity to bool cast.
				UInt8 iReg = ReadByte();
				*(Bool*)Regs[iReg].Value = *(FEntity**)Regs[iReg].Value != nullptr;
				break;
			}
			case CAST_EntityToString:
			{
				// Entity to entity cast.
				UInt8 iReg = ReadByte();
				FEntity* Entity = *(FEntity**)Regs[iReg].Value;
				Regs[iReg].StrValue = Entity ? Entity->GetName() : L"undefined";
				break;
			}
			case CAST_DelegateToBool:
			{
				// Delegate to bool cast.
				UInt8 iReg = ReadByte();
				*(Bool*)Regs[iReg].Value = *(TDelegate*)Regs[iReg].Value;
				break;
			}
			case CAST_DelegateToString:
			{
				// Delegate to string cast.
				UInt8 iReg = ReadByte();
				TDelegate Delegate = *(TDelegate*)Regs[iReg].Value;
				Regs[iReg].StrValue = Delegate ? String::Format(L"%s[%s]", *Delegate.Script->GetName(), *Delegate.Context->GetName()) : L"nowhere";
				break;
			}
			case CODE_Context:
			{
				// Change current context.
				FEntity* NewContext = *(FEntity**)Regs[ReadByte()].Value;
				if( !NewContext )
					ScriptError( L"Access to undefined entity" );
				Context	= NewContext;
				break;
			}
			case CODE_ThisContext:
			{
				// Set current this as context.
				Context = This;
				break;
			}
			case CODE_Switch:
			{
				// Perform switch statement.
				Int32 Expr = 0;
				UInt8 Size = ReadByte();
				mem::copy( &Expr, Regs[ReadByte()].Value, Size );
				UInt8* AddrDef = &Bytecode->Code[ReadWord()];
				Code = &Bytecode->Code[ReadWord()];
				Int32 NumLabs = ReadByte();

				for( Int32 i=0; i<NumLabs; i++ )
				{
					Int32 Label	= 0;
					mem::copy( &Label, Code, Size );
					Code += Size;
					UInt16 Addr		= ReadWord();

					if( Label == Expr )
					{
						Code		= &Bytecode->Code[Addr];
						goto LabFound;
					}
				}

				// No labels found, goto default.
				Code	= AddrDef;
				LabFound:
				break;
			}
			case CODE_Foreach:
			{
				// Perform foreach statement.
				FEntity**	Value	= (FEntity**)&Locals[ReadWord()];	
				FScript*	Type	= ReadScriptSafe();
				UInt16		EndAddr	= ReadWord();

				if( Foreach.i < Foreach.Collection.size() )
				{
					*Value		= Foreach.Collection[Foreach.i];
					Foreach.i++;

					if( Type && *Value && (*Value)->Script != Type )
						ScriptError( L"Iterator control's and value's scripts are mismatched" );
				}
				else
				{
					*Value		= nullptr;
					Code		= &Bytecode->Code[EndAddr];
					Foreach.i	= 0;
					Foreach.Collection.empty();
				}
				break;
			}
			case CODE_CallMethod:
			{
				// Call script method.
				CFunction* Func = Context->Script->Methods[ReadByte()];
				void* OutParms[16];
				assert(Func->ParmsCount<=arraySize(OutParms));
				CFrame NewFrame( Context, Func, Depth+1, this );

				for( Int32 i=0; i<Func->ParmsCount; i++ )
				{
					CProperty* Arg = Func->Locals[i];
					UInt8 iReg = ReadByte();
					if( Arg->Flags & PROP_OutParm )
					{
						// Out param.
						OutParms[i]	= Regs[iReg].Addr;
						Arg->CopyValue
									(	
										NewFrame.Locals + Arg->Offset,
										OutParms[i]
									);
					}
					else
					{
						// Regular param.
						Arg->CopyValue
									(	
										NewFrame.Locals + Arg->Offset,
										Arg->Type == TYPE_String ? (UInt8*)&Regs[iReg].StrValue : (UInt8*)Regs[iReg].Value
									);
					}
				}

				if( Func->ResultVar )
				{
					// With result.
					UInt8 iRes = ReadByte();
					NewFrame.ProcessCode( &Regs[iRes] );
				}
				else
				{
					// Without result.
					NewFrame.ProcessCode( nullptr );
				}

				// Copy out parameters back.
				for( Int32 i=0; i<Func->ParmsCount; i++ )
				{
					CProperty* Arg = Func->Locals[i];
					if( Arg->Flags & PROP_OutParm )
						Arg->CopyValue( OutParms[i], NewFrame.Locals + Arg->Offset );
				}

				break;
			}
			case CODE_CallDelegate:
			{
				// Call delegate.
				TDelegate Delegate = *((TDelegate*)Regs[ReadByte()].Value);

				if( !Delegate )
					ScriptError( L"Attempt to call 'nowhere' delegate" );

				CFunction * Func = Delegate.Script->Methods[Delegate.iMethod];
				void* OutParms[16];
				assert(Func->ParmsCount<=arraySize(OutParms));
				CFrame NewFrame( Context, Func, Depth+1, this );

				for( Int32 i=0; i<Func->ParmsCount; i++ )
				{
					CProperty* Arg = Func->Locals[i];
					UInt8 iReg = ReadByte();
					if( Arg->Flags & PROP_OutParm )
					{
						// Out param.
						OutParms[i]	= Regs[iReg].Addr;
						Arg->CopyValue
						(	
							NewFrame.Locals + Arg->Offset,
							OutParms[i]
						);
					}
					else
					{
						// Regular param.
						Arg->CopyValue
						(	
							NewFrame.Locals + Arg->Offset,
							Arg->Type == TYPE_String ? (UInt8*)&Regs[iReg].StrValue : (UInt8*)Regs[iReg].Value
						);
					}
				}

				if( Func->ResultVar )
				{
					// With result.
					UInt8 iRes = ReadByte();
					NewFrame.ProcessCode( &Regs[iRes] );
				}
				else
				{
					// Without result.
					NewFrame.ProcessCode( nullptr );
				}

				// Copy out parameters back.
				for( Int32 i=0; i<Func->ParmsCount; i++ )
				{
					CProperty* Arg = Func->Locals[i];
					if( Arg->Flags & PROP_OutParm )
						Arg->CopyValue( OutParms[i], NewFrame.Locals + Arg->Offset );
				}
				break;
			}
			case CODE_CallVF:
			{
				// Call virtual function.
				CFunction* Func = Context->Script->VFTable[ReadByte()];
				if( !Func )
					ScriptError( L"Attempt call abstract method from '%s'", *Context->Script->GetName() );		

				CFrame NewFrame( Context, Func, Depth+1, this );

				for( Int32 i=0; i<Func->ParmsCount; i++ )
				{
					CProperty* Arg = Func->Locals[i];
					UInt8 iReg = ReadByte();
					Arg->CopyValue
								(	
									NewFrame.Locals + Arg->Offset,
									Arg->Type == TYPE_String ? (UInt8*)&Regs[iReg].StrValue : (UInt8*)Regs[iReg].Value
								);
				}

				if( Func->ResultVar )
				{
					// With result.
					UInt8 iRes = ReadByte();
					NewFrame.ProcessCode( &Regs[iRes] );
				}
				else
				{
					// Without result.
					NewFrame.ProcessCode( nullptr );
				}
				break;
			}
			case CODE_BaseMethod:
			{
				// Base method call.
				CNativeFunction* Native = CClassDatabase::GFuncs[ReadWord()];
				((Context->Base)->*(Native->ptrMethod))( *this );
				break;
			}
			case CODE_ComponentMethod:
			{
				// Component method call.
				CNativeFunction* Native = CClassDatabase::GFuncs[ReadWord()];
				((Context->Components[ReadByte()])->*(Native->ptrMethod))( *this );
				break;
			}
			case CODE_ResourceMethod:
			{
				// Resource method call.
				UInt8 iReg = ReadByte();
				FResource* Res = *(FResource**)Regs[iReg].Value;
				if( !Res )
					ScriptError( L"Access to null resource" );

				CNativeFunction* Native = CClassDatabase::GFuncs[ReadWord()];
				((Res)->*(Native->ptrMethod))( *this );
				break;
			}
			case CODE_CallExtended:
			{
				// Call a simple C++ function.
				CNativeFunction* Native = CClassDatabase::GFuncs[ReadWord()];
				Native->ptrFunction( *this );
				break;
			}
			case CODE_CallStatic:
			{
				// Call a static script function.
				FScript* StaticScript = ReadScript();
				CFunction* Func = StaticScript->StaticFunctions[ReadByte()];
				void* OutParms[16];
				assert(Func->ParmsCount<=arraySize(OutParms));
				CFrame NewFrame( StaticScript, Func, Depth+1, this );				

				for( Int32 i=0; i<Func->ParmsCount; i++ )
				{
					CProperty* Arg = Func->Locals[i];
					UInt8 iReg = ReadByte();
					if( Arg->Flags & PROP_OutParm )
					{
						// Out param.
						OutParms[i]	= Regs[iReg].Addr;
						Arg->CopyValue
									(	
										NewFrame.Locals + Arg->Offset,
										OutParms[i]
									);
					}
					else
					{
						// Regular param.
						Arg->CopyValue
									(	
										NewFrame.Locals + Arg->Offset,
										Arg->Type == TYPE_String ? (UInt8*)&Regs[iReg].StrValue : (UInt8*)Regs[iReg].Value
									);
					}
				}

				if( Func->ResultVar )
				{
					// With result.
					UInt8 iRes = ReadByte();
					NewFrame.ProcessCode( &Regs[iRes] );
				}
				else
				{
					// Without result.
					NewFrame.ProcessCode( nullptr );
				}

				// Copy out parameters back.
				for( Int32 i=0; i<Func->ParmsCount; i++ )
				{
					CProperty* Arg = Func->Locals[i];
					if( Arg->Flags & PROP_OutParm )
						Arg->CopyValue( OutParms[i], NewFrame.Locals + Arg->Offset );
				}

				break;
			}
			case CODE_Stop:
			{
				// Stop thread execution.
				This->Thread->Status	= THR_Stopped;
				goto LeaveCode;
			}
			case CODE_Sleep:
			{
				// Make thread sleep.
				This->Thread->SleepTime	= *(Float*)(Regs[ReadByte()].Value);
				This->Thread->Status	= THR_Sleep;
				goto LeaveCode;
			}
			case CODE_Wait:
			{
				// Force the thread to wait.
				This->Thread->WaitExpr	= &Bytecode->Code[ReadWord()];

				if( *(Bool*)(Regs[ReadByte()].Value) )
				{
					// AWake.
					This->Thread->Status	= THR_Run;
					This->Thread->WaitExpr	= nullptr;
				}
				else
				{
					// Still wait.
					This->Thread->Status	= THR_Wait;
				}
				goto LeaveCode;
			}
			case CODE_Goto:
			{
				// Goto label in thread.
				Int32 iLabel = *(Int32*)(Regs[ReadByte()].Value);

				if( iLabel >= 0 && iLabel < Script->Thread->Labels.size() )
				{
					// Goto label and restart execution.
					UInt16 LabAddr				= Script->Thread->Labels[iLabel].Address;
					This->Thread->Frame.Code	= &Script->Thread->Code[LabAddr];
					This->Thread->Status		= THR_Run;
					This->Thread->LabelId		= iLabel;
				}
				else
					ScriptError( L"Bad label %d in 'goto'", iLabel );

				// Interrupt thread, if in running in thread.
				if( Bytecode == Script->Thread )
					goto LeaveCode;

				break;
			}
			case CODE_Interrupt:
			{
				// Interrupt thread execution.
				goto LeaveCode;
				break;
			}
			default:
			{
				// Delegate execution to native functions.
				ExecuteNative( Context, Op );
				break;
			}
		}
	}

LeaveCode:;

	// Copy result if required.
	if( Result && Function() && Function()->ResultVar )
	{
		CProperty* ResProp = Function()->ResultVar;
		ResProp->CopyValue
						( 
							ResProp->Type == TYPE_String ? (UInt8*)&Result->StrValue : Result->Value,
							Locals + ResProp->Offset
						);
	}
}


/*-----------------------------------------------------------------------------
    CThreadFrame implementation.
-----------------------------------------------------------------------------*/

//
// Thread constructor. Creates frame for execution.
// Warning: Use only while playing, since cause
// problems with GC and Undo/Redo.
//
CThreadFrame::CThreadFrame( FEntity* InEntity, CThreadCode* InThread )
	:	Frame( InEntity, InThread ),
		Status( THR_Run ),
		Entity( InEntity ),
		SleepTime( 0.f ),
		WaitExpr( nullptr ),
		LabelId( -1 )
{
}


//
// Entity thread destructor. Call only
// when entity die, or game over.
//
CThreadFrame::~CThreadFrame()
{
}


//
// Execute entity's thread code.
//
void CThreadFrame::Tick( Float Delta )
{
	// Execute code!
	try
	{
		switch( Status )
		{
			case THR_Run:
			{
				// Normally process the code.
				Frame.ProcessCode( nullptr );
				break;
			}
			case THR_Stopped:
			{
				// Thread are stopped.
				// Nothing to do.
				break;
			}
			case THR_Sleep:
			{
				// Sleep until time not passed.
				SleepTime -= Delta;

				if( SleepTime <= 0.f )
				{
					// Awake.
					SleepTime	= 0.f;
					Status		= THR_Run;
				}
				break;
			}
			case THR_Wait:
			{
				// Wait expression became true.
				assert(WaitExpr != nullptr);

				Frame.Code = WaitExpr;
				Frame.ProcessCode( nullptr );		
				break;
			}
			default:
				fatal( L"Bad thread '%s' status '%d'", *Entity->GetFullName(), (UInt8)Status );
		}
	}
	catch( ... )
	{
		// Damn, something horrible happened.
		LogManager::instance().handleFatalScriptMessage( L"Entity: Thread error in entity '%s'", *Entity->GetFullName() );

		// Stop this thread execution to prevent thread crash at next tick.
		Status = THR_Stopped;
		Frame.Code = &Frame.Bytecode->Code.last();
	}
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/