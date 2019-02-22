/*=============================================================================
	FrCode.h: Script execution support structures.
	Created by Vlad Gordienko, Jul. 2016.
	Reimplemented by Vlad, Jan. 2018.
=============================================================================*/

/*-----------------------------------------------------------------------------
	Code execution structures.
-----------------------------------------------------------------------------*/

//
// A virtual machine register.
//
class TRegister
{
public:
	// Variables.
	struct
	{
		union 
		{
			UInt8	Value[32];
			void*	Addr;
		};
		String	StrValue;
	};

	// Constructor.
	TRegister()
		:	Addr( nullptr ),
			StrValue()
	{}
};


/*-----------------------------------------------------------------------------
    TFrame.
-----------------------------------------------------------------------------*/

//
// Constants.
//
#define MAX_RECURSION_DEPTH		32
#define MAX_ITERATIONS			1000000


//
// A code execution local frame.
//
class CFrame
{
public:
	// Constants.
	enum{ NUM_REGS = 24 };

	// Variables.
	FEntity*		This;
	FScript*		Script;
	CBytecode*		Bytecode;
	TRegister		Regs[NUM_REGS];	

	// Friends.
	friend CThreadFrame;
	friend FEntity;
	friend FScript;

	// Shared stack memory 512 kB.
	static CStaticPool<512*1024> GLocalsMem;

	// CFrame interface.
	CFrame( FEntity* InThis, CFunction* InFunction, Int32 InDepth = 1, CFrame* InPrevFrame = nullptr );
	CFrame( FEntity* InThis, CThreadCode* InThread );
	CFrame( FScript* InScript, CFunction* InStatic, Int32 InDepth = 1, CFrame* InPrevFrame = nullptr );
	~CFrame();
	void ScriptError( Char* Fmt, ... );

private:
	// An information about foreach loop.
	class TForeach
	{
	public:
		// Variables.
		Array<FEntity*>	Collection;
		Int32			i;

		// Constructor.
		TForeach()
			:	Collection(),
				i( 0 )
		{}
	};

	// Frame internal.
	CFrame*			PrevFrame;
	Int32			Depth;	
	UInt8*			Code;
	UInt8*			Locals;
	TForeach		Foreach;

	// Opcodes execution.
	void ProcessCode( TRegister* Result );
	void ExecuteNative( FEntity* Context, EOpCode Code );

	// Misc.
	String StackTrace();

public:
	// Constants readers.
	inline UInt8 ReadByte();
	inline UInt16 ReadWord();
	inline Bool ReadBool();
	inline Int32 ReadInteger();
	inline Float ReadFloat();
	inline math::Angle ReadAngle();
	inline TColor ReadColor();
	inline String ReadString();
	inline math::Vector ReadVector();
	inline math::Rect ReadAABB();
	inline FResource* ReadResource();
	inline FEntity* ReadEntity();
	inline FScript* ReadScript();
	inline FScript* ReadScriptSafe();
	inline EPropType ReadPropType();

	// Accessors.
	inline CFunction* Function()
	{
		return Bytecode == Script->Thread ? nullptr : (CFunction*)Bytecode;
	}
	inline CThreadCode* ThreadCode()
	{
		return Bytecode == Script->Thread ? Script->Thread : nullptr;
	}
};


/*-----------------------------------------------------------------------------
    CThreadFrame.
-----------------------------------------------------------------------------*/

//
// A current tread status.
//
enum EThreadStatus
{
	THR_Run,		// Thread executed well.
	THR_Stopped,	// Thread are stopped.
	THR_Sleep,		// Thread are sleep for now.
	THR_Wait		// Thread are waiting.
};


//
// An entity thread to process scenario.
//
class CThreadFrame
{
public:
	// Variables.
	EThreadStatus		Status;
	FEntity*			Entity;
	CFrame				Frame;
	Int32				LabelId;
	union
	{
		Float			SleepTime;	// THR_Sleep.
		UInt8*			WaitExpr;	// THR_Wait.
	};

	// CThreadFrame interface.
	CThreadFrame( FEntity* InEntity, CThreadCode* InThread );
	~CThreadFrame();
	void Tick( Float Delta );
};


/*-----------------------------------------------------------------------------
    CFrame implementation.
-----------------------------------------------------------------------------*/

inline UInt8 CFrame::ReadByte()
{
	UInt8 R = *Code;
	Code += sizeof(UInt8);
	return R;
}

inline Bool CFrame::ReadBool()
{
	Bool R = *(Bool*)Code;
	Code += sizeof(Bool);
	return R;
}

inline Int32 CFrame::ReadInteger()
{
	Int32 R = *(Int32*)Code;
	Code += sizeof(Int32);
	return R;
}

inline Float CFrame::ReadFloat()
{
	Float R = *(Float*)Code;
	Code += sizeof(Float);
	return R;
}

inline math::Angle CFrame::ReadAngle()
{
	math::Angle R = *(math::Angle*)Code;
	Code += sizeof(math::Angle);
	return R;
}

inline TColor CFrame::ReadColor()
{
	TColor R = *(TColor*)Code;
	Code += sizeof(TColor);
	return R;
}

inline String CFrame::ReadString()
{
	Int32 Len = ReadInteger();
	Char Buffer[STRING_ARRAY_MAX+1];

	if( Len < 0 )
	{
		// Ansi string.
		Int32 RealLen = -Len;
		for( Int32 i=0; i<RealLen; i++ )
			Buffer[i] = ((AnsiChar*)Code)[i];
		Code += RealLen * sizeof(AnsiChar);
		Buffer[RealLen] = '\0';
	}
	else
	{
		// Wide string.
		if( Len > 0 )
			mem::copy( Buffer, Code, Len*sizeof(Char) );
		Code += Len * sizeof(Char);
		Buffer[Len] = '\0';
	}
	return Buffer;
}

inline math::Vector CFrame::ReadVector()
{
	math::Vector R = *(math::Vector*)Code;
	Code += sizeof(math::Vector);
	return R;
}

inline math::Rect CFrame::ReadAABB()
{
	math::Rect R = *(math::Rect*)Code;
	Code += sizeof(math::Rect);
	return R;
}

inline UInt16 CFrame::ReadWord()
{
	UInt16 R = *(UInt16*)Code;
	Code += sizeof(UInt16);
	return R;
}

inline FResource* CFrame::ReadResource()
{
	UInt8 iRes = ReadByte();
	return iRes != 0xff ? Script->ResTable[iRes] : nullptr;
}

inline FEntity* CFrame::ReadEntity()
{
	Int32 iEntity = ReadInteger();
	return iEntity != -1 ? (FEntity*)GObjectDatabase->GObjects[iEntity] : nullptr;	
}

inline FScript* CFrame::ReadScript()
{
	FObject* Obj = GObjectDatabase->GObjects[ReadInteger()];
	assert(Obj->IsA(FScript::MetaClass));
	return (FScript*)Obj;
}

inline FScript* CFrame::ReadScriptSafe()
{
	Int32 i = ReadInteger();
	return i != -1 ? As<FScript>(GObjectDatabase->GObjects[i]) : nullptr;
}

inline EPropType CFrame::ReadPropType()
{
	EPropType R = (EPropType)*Code;
	Code += sizeof(UInt8);
	return R;
}


/*-----------------------------------------------------------------------------
    Stack macro.
-----------------------------------------------------------------------------*/

// Parameters grabbers.
#define POP_BYTE			(*(UInt8*)(Frame.Regs[Frame.ReadByte()].Value))
#define POP_BOOL			(*(Bool*)(Frame.Regs[Frame.ReadByte()].Value))		
#define POP_INTEGER			(*(Int32*)(Frame.Regs[Frame.ReadByte()].Value))
#define POP_FLOAT			(*(Float*)(Frame.Regs[Frame.ReadByte()].Value))
#define POP_ANGLE			(*(math::Angle*)(Frame.Regs[Frame.ReadByte()].Value))
#define POP_COLOR			(*(TColor*)(Frame.Regs[Frame.ReadByte()].Value))
#define POP_STRING			(Frame.Regs[Frame.ReadByte()].StrValue)
#define POP_VECTOR			(*(math::Vector*)(Frame.Regs[Frame.ReadByte()].Value))
#define POP_AABB			(*(math::Rect*)(Frame.Regs[Frame.ReadByte()].Value))
#define POP_RESOURCE		(*(FResource**)(Frame.Regs[Frame.ReadByte()].Value))
#define POP_ENTITY			(*(FEntity**)(Frame.Regs[Frame.ReadByte()].Value))

#define POPA_BYTE			((Byte*)(Frame.Regs[Frame.ReadByte()].Value))
#define POPA_BOOL			((Bool*)(Frame.Regs[Frame.ReadByte()].Value))		
#define POPA_INTEGER		((Int32*)(Frame.Regs[Frame.ReadByte()].Value))
#define POPA_FLOAT			((Float*)(Frame.Regs[Frame.ReadByte()].Value))
#define POPA_ANGLE			((math::Angle*)(Frame.Regs[Frame.ReadByte()].Value))
#define POPA_COLOR			((TColor*)(Frame.Regs[Frame.ReadByte()].Value))
#define POPA_STRING			(&Frame.Regs[Frame.ReadByte()].StrValue)
#define POPA_VECTOR			((math::Vector*)(Frame.Regs[Frame.ReadByte()].Value))
#define POPA_AABB			((math::Rect*)(Frame.Regs[Frame.ReadByte()].Value))
#define POPA_RESOURCE		((FResource**)(Frame.Regs[Frame.ReadByte()].Value))
#define POPA_ENTITY			((FEntity**)(Frame.Regs[Frame.ReadByte()].Value))


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/