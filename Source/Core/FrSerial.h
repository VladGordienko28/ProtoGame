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
	virtual void SerializeRef( class FObject*& Obj ) = 0;

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
inline void Serialize( CSerializer& S, UInt8& V )		{ S.SerializeData( &V, sizeof(UInt8)	); }
inline void Serialize( CSerializer& S, Int8& V )		{ S.SerializeData( &V, sizeof(Int8)		); }
inline void Serialize( CSerializer& S, Bool& V )		{ S.SerializeData( &V, sizeof(Bool)		); }
inline void Serialize( CSerializer& S, AnsiChar& V )	{ S.SerializeData( &V, sizeof(AnsiChar) ); }
inline void Serialize( CSerializer& S, Char& V )		{ S.SerializeData( &V, sizeof(Char)		); }
inline void Serialize( CSerializer& S, UInt16& V )		{ S.SerializeData( &V, sizeof(UInt16)	); }
inline void Serialize( CSerializer& S, Int16& V )		{ S.SerializeData( &V, sizeof(Int16)	); }
inline void Serialize( CSerializer& S, Int32& V )		{ S.SerializeData( &V, sizeof(Int32)	); }
inline void Serialize( CSerializer& S, UInt32& V )		{ S.SerializeData( &V, sizeof(UInt32)	); }
inline void Serialize( CSerializer& S, Float& V )		{ S.SerializeData( &V, sizeof(Float)	); }
inline void Serialize( CSerializer& S, UInt64& V )		{ S.SerializeData( &V, sizeof(UInt64)	); }

// Enumeration serialization.
template<class T> inline void SerializeEnum( CSerializer& S, T& Enum )	
{ 
	static_assert(std::is_enum<T>::value, "SerializeEnum accepts only enum types");
	S.SerializeData( &Enum, sizeof(UInt8) ); 
}

//
// Serialization utility.
//
#define CLEANUP_ARR_NULL(arr) arr.removeUnique(nullptr);


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/