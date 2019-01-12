/*=============================================================================
    FrSerial.h: An abstract serializer.
    Copyright Jun.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    CSerializer.
-----------------------------------------------------------------------------*/

//
// Serializer mode.
//
enum ESerMode
{
	SM_None,			// Bad mode. 
	SM_Load,			// Loading.
	SM_Save,			// Saving.
	SM_Undefined		// Otherwise.
};


//
// An abstract serializer.
//
class CSerializer
{
public:
	// Functions.
	virtual void SerializeData( void* Mem, SizeT Count ) = 0;
	virtual void SerializeRef( FObject*& Obj ) = 0;

	virtual SizeT TotalSize()
	{
		return 0;
	}
	virtual void Seek( SizeT NewPos )
	{
	}
	virtual SizeT Tell()
	{
		return 0;
	}

	// Accessors.
	inline ESerMode GetMode()
	{
		return Mode;
	}

protected:
	// Variables.
	ESerMode	Mode;
};


//
// Basic types serialization.
//
inline void Serialize( CSerializer& S, Byte& V )		{ S.SerializeData( &V, sizeof(Byte)		); }
inline void Serialize( CSerializer& S, SByte& V )		{ S.SerializeData( &V, sizeof(SByte)	); }
inline void Serialize( CSerializer& S, Bool& V )		{ S.SerializeData( &V, sizeof(Bool)		); }
inline void Serialize( CSerializer& S, AnsiChar& V )	{ S.SerializeData( &V, sizeof(AnsiChar) ); }
inline void Serialize( CSerializer& S, Char& V )		{ S.SerializeData( &V, sizeof(Char)		); }
inline void Serialize( CSerializer& S, Word& V )		{ S.SerializeData( &V, sizeof(Word)		); }
inline void Serialize( CSerializer& S, SWord& V )		{ S.SerializeData( &V, sizeof(SWord)	); }
inline void Serialize( CSerializer& S, Integer& V )		{ S.SerializeData( &V, sizeof(Integer)	); }
inline void Serialize( CSerializer& S, DWord& V )		{ S.SerializeData( &V, sizeof(DWord)	); }
inline void Serialize( CSerializer& S, Float& V )		{ S.SerializeData( &V, sizeof(Float)	); }
inline void Serialize( CSerializer& S, QWord& V )		{ S.SerializeData( &V, sizeof(QWord)	); }

// Enumeration serialization.
template<class T> inline void SerializeEnum( CSerializer& S, T& Enum )	
{ 
	static_assert(std::is_enum<T>::value, "SerializeEnum accepts only enum types");
	S.SerializeData( &Enum, sizeof(Byte) ); 
}

//
// Serialization utility.
//
#define CLEANUP_ARR_NULL(arr) arr.RemoveUnique(nullptr);


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/