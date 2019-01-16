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
	CBufferWriter( Array<UInt8>& InData )
		:	Data(InData)
	{
		Mode = SM_Save;
	}

	// CSerializer interface.
	void SerializeData( void* Mem, SizeT Count )
	{
		Int32 i = Data.size();
		Data.setSize(i + Count);
		mem::copy( &Data[i], Mem, Count );
	}
	void SerializeRef( FObject*& Obj )
	{
		Int32 iObj = Obj ? Obj->GetId() : -1;
		Serialize( *this, iObj );
	}
	SizeT TotalSize()
	{
		return Data.size();
	}
	void Seek( SizeT NewPos )
	{
		fatal(L"CBufferWriter::Seek");
	}
	SizeT Tell()
	{
		return Data.size();
	}

private:
	Array<UInt8>&	Data;
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
	CBufferReader( Array<UInt8>& InData )
		:	Data(InData),
			Pos(0)
	{
		Mode = SM_Load;
	}

	// CSerializer interface.
	void SerializeData( void* Mem, SizeT Count )
	{
		assert(Pos+Count <= Data.size());
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
		return Data.size();
	}
	void Seek( SizeT NewPos )
	{
		assert(NewPos>=0 && NewPos<Data.size());
		Pos = NewPos;
	}
	SizeT Tell()
	{
		return Pos;
	}

private:
	Array<UInt8>&	Data;
	SizeT	Pos;
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/