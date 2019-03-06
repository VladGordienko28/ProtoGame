/*=============================================================================
    FrFile.h: ANCI C style binary and text files loading & saving.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
    CFileLoader.
-----------------------------------------------------------------------------*/

//
// Serializer to load data from disc.
//
// Legacy, and will be removed soon
//
class FileLoaderAdapter: public CSerializer
{
public:
	FileLoaderAdapter( fm::IBinaryFileReader::Ptr inFile )
	{
		m_file = inFile;
		Mode = SM_Load;

		if( !m_file )
		{
			fatal( L"File not found" );
		}
	}

	void SerializeData( void* Mem, SizeT Count ) override
	{
		m_file->readData( Mem, Count );
	}

	void SerializeRef( FObject*& Obj ) override
	{
		Int32 Id;
		Serialize( *this, Id );
		Obj	= Id != -1 ? GObjectDatabase->GObjects[Id] : nullptr;
	}

	SizeT TotalSize() override
	{
		return m_file->totalSize();
	}

	void Seek( SizeT NewPos ) override
	{
		m_file->seek( NewPos );
	}

	SizeT Tell() override
	{
		return m_file->tell();
	}

private:
	fm::IBinaryFileReader::Ptr m_file;

	FileLoaderAdapter() = delete;
	FileLoaderAdapter( const FileLoaderAdapter& ) = delete;
	FileLoaderAdapter( FileLoaderAdapter&& ) = delete;
};


/*-----------------------------------------------------------------------------
    CFileSaver.
-----------------------------------------------------------------------------*/

//
// Serializer to write data to disc.
//
// Legacy, and will be removed soon
//
class FileSaverAdapter: public CSerializer
{
public:
	FileSaverAdapter( fm::IBinaryFileWriter::Ptr inFile )
	{
		m_file = inFile;
		Mode = SM_Save;

		if( !m_file )
		{
			fatal( L"File not found" );
		}
	}

	void SerializeData( void* Mem, SizeT Count ) override
	{
		m_file->writeData( Mem, Count );
	}

	void SerializeRef( FObject*& Obj ) override
	{
		Int32 Id	= Obj ? Obj->GetId() : -1;
		Serialize( *this, Id );
	}

	SizeT TotalSize()
	{
		assert( false && "Is not implemented" );
		return 0;
	}

	void Seek( SizeT NewPos ) override
	{
		assert( false && "Is not implemented" );
	}

	SizeT Tell() override
	{
		return m_file->tell();
	}

private:
	fm::IBinaryFileWriter::Ptr m_file;

	FileSaverAdapter() = delete;
	FileSaverAdapter( const FileSaverAdapter& ) = delete;
	FileSaverAdapter( FileSaverAdapter&& ) = delete;
};


/*-----------------------------------------------------------------------------
    CTextReader.
-----------------------------------------------------------------------------*/

//
// Text file reader.
//
class CTextReader
{
public:
	String	FileName;
	FILE*	File;

	// CTextReader interface.
	CTextReader( String InFileName )
		: FileName( InFileName )
	{
		_wfopen_s( &File, *FileName, L"r" );
		if( !File )
			fatal( L"File \"%s\" not found", *FileName );
	}
	~CTextReader()
	{
		fclose( File );
	}
	String ReadLine()
	{
		Char Buffer[2048] = {};
		
		if( fgetws( Buffer, 2048, File ) )
		{
			// Very ugly.
			for( Int32 i=0; i<2048; i++ )
				if( Buffer[i] == L'\n' )
				{
					Buffer[i] = L'\0';
					break;
				}

			return Buffer;
		}
		else
			return L"";
	}
	Bool IsEOF()
	{
		return feof( File ) != 0;
	}
};


/*-----------------------------------------------------------------------------
    CTextWriter.
-----------------------------------------------------------------------------*/

//
// Text file writer.
//
class CTextWriter
{
public:
	String	FileName;
	FILE*	File;

	// CTextWriter interface.
	CTextWriter( String InFileName )
		: FileName( InFileName )
	{
		_wfopen_s( &File, *FileName, L"w" );
		if( !File )
			fatal( L"File \"%s\" not found", *FileName );
	}
	~CTextWriter()
	{
		fclose( File );
	}
	void WriteString( String Line )
	{
		fwprintf( File, L"%s\n", *Line );
	}
	Bool IsEOF()
	{
		return feof( File ) != 0;
	}
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/