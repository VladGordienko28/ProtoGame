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
	CBufferWriter( TArray<Byte>& InData )
		:	Data(InData)
	{
		Mode = SM_Save;
	}

	// CSerializer interface.
	void SerializeData( void* Mem, SizeT Count )
	{
		Integer i = Data.Num();
		Data.SetNum(i + Count);
		MemCopy( &Data[i], Mem, Count );
	}
	void SerializeRef( FObject*& Obj )
	{
		Integer iObj = Obj ? Obj->GetId() : -1;
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
	TArray<Byte>&	Data;
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
	CBufferReader( TArray<Byte>& InData )
		:	Data(InData),
			Pos(0)
	{
		Mode = SM_Load;
	}

	// CSerializer interface.
	void SerializeData( void* Mem, SizeT Count )
	{
		assert(Pos+Count <= Data.Num());
		MemCopy( Mem, &Data[Pos], Count );
		Pos += Count;
	}
	void SerializeRef( FObject*& Obj )
	{
		Integer iObj;
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
	TArray<Byte>&	Data;
	SizeT	Pos;
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/