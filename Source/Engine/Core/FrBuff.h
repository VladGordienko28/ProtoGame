/*=============================================================================
	FrBuff.h: Temporal buffer serialization.
	Created by Vlad Gordienko, Mar. 2018.
=============================================================================*/

/*-----------------------------------------------------------------------------
    CBufferWriter.
-----------------------------------------------------------------------------*/

//
// A Byte-buffer writer.
//
class CBufferWriter: public CSerializer
{
public:
	// CBufferWriter interface.
	CBufferWriter( TArray<UInt8>& InData )
		:	Data(InData)
	{
		Mode = SM_Save;
	}

	// CSerializer interface.
	void SerializeData( void* Mem, SizeT Count )
	{
		Int32 i = Data.Num();
		Data.SetNum(i + Count);
		mem::copy( &Data[i], Mem, Count );
	}
	void SerializeRef( FObject*& Obj )
	{
		Int32 iObj = Obj ? Obj->GetId() : -1;
		Serialize( *this, iObj );
	}
	SizeT TotalSize()
	{
		return Data.Num();
	}
	void Seek( SizeT NewPos )
	{
		error(L"CBufferWriter::Seek");
	}
	SizeT Tell()
	{
		return Data.Num();
	}

private:
	TArray<UInt8>&	Data;
};


/*-----------------------------------------------------------------------------
    CBufferReader.
-----------------------------------------------------------------------------*/

//
// A Byte-buffer reader.
//
class CBufferReader: public CSerializer
{
public:
	// CBufferReader interface.
	CBufferReader( TArray<UInt8>& InData )
		:	Data(InData),
			Pos(0)
	{
		Mode = SM_Load;
	}

	// CSerializer interface.
	void SerializeData( void* Mem, SizeT Count )
	{
		assert(Pos+Count <= Data.Num());
		mem::copy( Mem, &Data[Pos], Count );
		Pos += Count;
	}
	void SerializeRef( FObject*& Obj )
	{
		Int32 iObj;
		Serialize( *this, iObj );
		Obj = iObj != -1 ? GObjectDatabase->GObjects[iObj] : nullptr;
	}
	SizeT TotalSize()
	{
		return Data.Num();
	}
	void Seek( SizeT NewPos )
	{
		assert(NewPos>=0 && NewPos<Data.Num());
		Pos = NewPos;
	}
	SizeT Tell()
	{
		return Pos;
	}

private:
	TArray<UInt8>&	Data;
	SizeT	Pos;
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/