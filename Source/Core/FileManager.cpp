//-----------------------------------------------------------------------------
//	FileManager.cpp: File functions
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

#include "Core.h"

#include "shlwapi.h"
#pragma comment( lib, "shlwapi.lib" ) 

namespace flu 
{
namespace fm
{
	static const Char* PATH_BASE_TEMP = L"Temp";
	static const Char* PATH_BASE_PACKAGES = L"..\\Packages";

	class BinaryFileReader: public IBinaryFileReader
	{
	public:
		BinaryFileReader( String inFileName,  FILE* inFile )
			:	m_fileName( inFileName ),
				m_file( inFile )
		{
			assert( m_file );
			m_hasError = false;
		}

		~BinaryFileReader()
		{
			fclose( m_file );
		}

		SizeT readData( void* buffer, SizeT size ) override
		{
			assert( buffer );
			const SizeT bytesRead = fread( buffer, 1, size, m_file );

			if( bytesRead != size )
			{
				m_hasError = true;
			}

			return bytesRead;
		}

		SizeT totalSize() override
		{
			SizeT storedPosition = ftell( m_file );
			fseek( m_file, 0, SEEK_END );

			SizeT fileSize = ftell( m_file );
			fseek( m_file, storedPosition, SEEK_SET );
			
			return fileSize;
		}

		void seek( SizeT newPosition ) override
		{
			if( fseek( m_file, newPosition, SEEK_SET ) )
			{
				m_hasError = true;
				fatal( L"Seek to position %d failed in file \"%s\"", newPosition, *m_fileName );
			}
		}

		SizeT tell() override
		{
			return ftell( m_file );
		}

		Bool isEof() const override
		{
			return feof( m_file );
		}

		String fileName() const override
		{
			return m_fileName;
		}

	private:
		String m_fileName;
		FILE* m_file;
	};

	IBinaryFileReader::Ptr readBinaryFile( const Char* fileName )
	{
		FILE* file;
		auto errorCode = _wfopen_s( &file, fileName, L"rb" );

		if( file )
		{
			return new BinaryFileReader( fileName, file );
		}
		else
		{
			error( L"Unable to open file \"%s\" with error %d", fileName, errorCode );
			return nullptr;
		}
	}

	class BinaryFileWriter: public IBinaryFileWriter
	{
	public:
		BinaryFileWriter( String inFileName,  FILE* inFile )
			:	m_fileName( inFileName ),
				m_file( inFile )
		{
			assert( m_file );
		}

		~BinaryFileWriter()
		{
			fclose( m_file );
		}

		void writeData( const void* buffer, SizeT size ) override
		{
			assert( buffer );
			fwrite( buffer, 1, size, m_file );
		}

		void* reserveData( SizeT numBytes ) override
		{
			assert( false && "BinaryFileWriter::reserveData is not allowed" );
			return nullptr;
		}

		SizeT size() const override
		{
			return ftell( m_file );
		}

		SizeT tell() const override
		{
			return ftell( m_file );
		}

		void seek( SizeT offset ) override
		{
			if( fseek( m_file, offset, SEEK_SET ) )
			{
				fatal( L"Seek to position %d failed in file \"%s\"", offset, *m_fileName );
			}
		}

		String fileName() const override
		{
			return m_fileName;
		}

	private:
		FILE* m_file;
		String m_fileName;
	};

	IBinaryFileWriter::Ptr writeBinaryFile( const Char* fileName )
	{
		FILE* file;
		auto errorCode = _wfopen_s( &file, fileName, L"wb" );

		if( file )
		{
			return new BinaryFileWriter( fileName, file );
		}
		else
		{
			error( L"Unable to create file \"%s\" with error %d", fileName, errorCode );
			return nullptr;
		}
	}

	Text::Ptr readTextFile( const Char* fileName )
	{
		FILE* file;
		auto errorCode = _wfopen_s( &file, fileName, L"r" );

		if( file )
		{
			Text::Ptr text = new Text();

			while( feof( file ) == 0 )
			{
				Char buffer[2048] = {};

				if( fgetws( buffer, arraySize( buffer ), file ) )
				{
					text->appendLine( buffer );
				}
				else
				{
					break;
				}
			}

			fclose( file );
			return text;
		}
		else
		{
			error( L"Unable to open file \"%s\" with error %d", fileName, errorCode );
			return nullptr;
		}
	}

	Bool writeTextFile( const Char* fileName, Text::Ptr text )
	{
		assert( text.hasObject() );

		FILE* file;
		auto errorCode = _wfopen_s( &file, fileName, L"w" );

		if( file )
		{
			for( Int32 i = 0; i < text->size(); ++i )
			{
				fwprintf( file, L"%s\n", *(*text)[i] );
			}

			fclose( file );
			return true;
		}
		else
		{
			error( L"Unable to create file \"%s\" with error %d", fileName, errorCode );
			return false;
		}
	}

	String getFilePath( const Char* fileName )
	{
		const Char* lastSlashPtr = cstr::findRevChar( fileName, '\\' );

		if( lastSlashPtr && lastSlashPtr != fileName )
		{
			Char buffer[MAX_FILE_NAME_SIZE] = {};
			SizeT numChars = ( reinterpret_cast<SizeT>( lastSlashPtr ) - 
				reinterpret_cast<SizeT>( fileName ) ) / sizeof(Char);
			
			return String( cstr::copy( buffer, MAX_FILE_NAME_SIZE, fileName, numChars ) );
		}
		else
		{
			return L"";
		}
	}

	String getFileExt( const Char* fileName )
	{
		const Char* dotPtr = cstr::findRevChar( fileName, '.' );
		return dotPtr ? String( dotPtr + 1 ) : L"";
	}

	String getFileName( const Char* fileName )
	{
		const Char* dotPtr = cstr::findRevChar( fileName, '.' );
		const Char* lastSlashPtr = cstr::findRevChar( fileName, '\\' );

		if( dotPtr )
		{
			const Char* fromPtr = lastSlashPtr ? lastSlashPtr + 1 : fileName;
			assert( dotPtr > fromPtr );

			Char buffer[MAX_FILE_NAME_SIZE] = {};
			SizeT numChars = ( reinterpret_cast<SizeT>( dotPtr ) - reinterpret_cast<SizeT>( fromPtr ) ) / sizeof(Char);
			
			return String( cstr::copy( buffer, MAX_FILE_NAME_SIZE, fromPtr, numChars ) );
		}
		else
		{
			return lastSlashPtr ? L"" : fileName;
		}
	}

	Bool isAbsoluteFileName( const Char* fileName )
	{ 
		return cstr::findChar( fileName, ':' ) != nullptr;
	}

	String normalizeFileName( const Char* fileName )
	{
		Char buffer[MAX_FILE_NAME_SIZE] = {};
		cstr::copy( buffer, MAX_FILE_NAME_SIZE, fileName, cstr::length( fileName ) );

		for( Char* ptr = buffer; *ptr; ptr++ )
		{
			if( *ptr == '/' )
			{
				*ptr = '\\';
			}
		}

		return buffer; 
	}

	String resolveFileName( const Char* relFileName, EPathBase pathBase )
	{ 
		String solvedPath;

		if( isAbsoluteFileName( relFileName ) || pathBase == EPathBase::Absolute )
		{
			solvedPath = relFileName;
		}
		else if( pathBase == EPathBase::Exe )
		{
			solvedPath = String::format( L"%s\\%s", *getCurrentDirectory(), relFileName );
		}
		else if( pathBase == EPathBase::Temp )
		{
			solvedPath = String::format( L"%s\\%s\\%s", *getCurrentDirectory(), PATH_BASE_TEMP, relFileName );
		}
		else if( pathBase == EPathBase::Packages )
		{
			solvedPath = String::format( L"%s\\%s\\%s", *getCurrentDirectory(), PATH_BASE_PACKAGES, relFileName );
		}
		else
		{
			fatal( L"Unknown EPathBase::%d", pathBase );
		}

		// make sure path is correct
		Char buffer[MAX_FILE_NAME_SIZE] = {};
		solvedPath = normalizeFileName( *solvedPath );

		if( PathCanonicalize( buffer, *solvedPath ) )
		{
			return buffer;
		}
		else
		{
			return L"";
		}
	}

	Bool directoryExists( const Char* dirName )
	{ 
		UInt32 Type	= GetFileAttributes( dirName );
		return Type & FILE_ATTRIBUTE_DIRECTORY;
	}

	Bool fileExists( const Char* fileName )
	{ 
		FILE* file;
		auto errorCode = _wfopen_s( &file, fileName, L"rb" );

		if( file )
		{
			fclose( file );
			return true;
		}
		else
		{
			return false;
		}
	}

	Array<String> findFiles( const Char* directory, const Char* wildcard )
	{
		Array<String> result;
		WIN32_FIND_DATA findData;

		HANDLE hFind = FindFirstFile( *String::format( L"%s\\%s", directory, wildcard ), &findData );

		if( hFind != INVALID_HANDLE_VALUE )
		{
			do
			{
				result.push( String::format( L"%s\\%s", directory, findData.cFileName ) );

			} while( FindNextFile( hFind, &findData ) );


			FindClose( hFind );
		}

		return std::move( result );
	}

	String getCurrentDirectory()
	{ 
		Char buffer[MAX_FILE_NAME_SIZE] = {};
		DWORD result = GetCurrentDirectory( MAX_FILE_NAME_SIZE, buffer );

		if( result == 0 )
		{
			fatal( L"GetCurrentDirectory is failed with %d error", GetLastError() );
		}

		return buffer; 
	}

	void setCurrentDirectory( const Char* newDirectory )
	{
		info( L"Current directory changed to \"%s\"", newDirectory );
		BOOL result = SetCurrentDirectory( newDirectory );

		if( !result )
		{
			fatal( L"Unable to change current working directory with error %d", GetLastError() );
		}
	}

	Int64 getFileModificationTime( const Char* fileName )
	{
		WIN32_FILE_ATTRIBUTE_DATA fileData;

		if( GetFileAttributesEx( fileName, GetFileExInfoStandard, &fileData ) )
		{
			return *reinterpret_cast<Int64*>( &fileData.ftLastWriteTime );
		}
		else
		{
			return 0;
		}
	}

} /* namespace fm */
} /* namespace flu */