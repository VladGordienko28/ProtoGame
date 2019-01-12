/*=============================================================================
	FrIni.cpp: Configuration manager.
	Created by Vlad Gordienko, Mar. 2018.
=============================================================================*/

#include "../Engine.h"

/*-----------------------------------------------------------------------------
	CConfigManager implementation.
-----------------------------------------------------------------------------*/

//
// Manager constructor.
//
CConfigManager::CConfigManager( String WorkingDirectory )
	:	Directory( WorkingDirectory ),
		Files()
{
	info(L"ConfigMan: Directory: '%s'", *Directory);

	TArray<String> FileList = GPlat->FindFiles( Directory, L"*.ini" );
	info(L"ConfigMan: %d configuration files found", FileList.Num());

	for( Integer iFile=0; iFile<FileList.Num(); iFile++ )
	{
		CTextReader Reader(FileList[iFile]);
		
		TIniFile IniFile;
		IniFile.bDirty = false;
		IniFile.FileName = FileList[iFile];

		IniFile.Name = String::Copy
		( 
			IniFile.FileName, 
			Directory.Len()+1, 
			IniFile.FileName.Len()-Directory.Len()-5 
		);

		TSection* Section = nullptr;
		while( !Reader.IsEOF() )
		{
			String Line = Reader.ReadLine();
			Char* Walk = *Line;

			// Skip initial whitespace.
			while( *Walk == ' ' && *Walk )
				Walk++;

			if( *Walk == '[' )
			{
				// Read a new section.
				Walk++;
				Char Buffer[256] = {}, *BuffWalk = Buffer;
				while( *Walk && *Walk != ']' )
					*BuffWalk++ = *Walk++;

				TSection NewSection;
				NewSection.Name = Buffer;
				
				IniFile.Sections.Push(NewSection);
				Section = &IniFile.Sections.Last();
			}
			else if( *Walk != L';' && *Walk )
			{
				// Read a pair.
				Char KeyBuffer[256]={}, ValueBuffer[1024]={}, *KeyWalk=KeyBuffer, *ValueWalk=ValueBuffer;

				// Parse key.
				while( *Walk && *Walk != '=' )
					*KeyWalk++ = *Walk++;

				// Parse value if any.
				if( *Walk && *Walk == '=' )
				{
					Walk++;
					while( *Walk && *Walk != ';' )
						*ValueWalk++ = *Walk++;

					if( Section )
						Section->Pairs.Add( KeyBuffer, ValueBuffer );
				}
			}
		}

		Files.Push(IniFile);
	}

	info(L"Configuration manager initialized");
}


//
// Configuration manager destructor.
//
CConfigManager::~CConfigManager()
{
	Flush();
	info(L"Configuration manager shutdown");
}


//
// Flush all dirty files.
//
void CConfigManager::Flush()
{
	for( Integer i=0; i<Files.Num(); i++ )
	{
		TIniFile& File = Files[i];
		if( File.bDirty )
		{
			CTextWriter Writer(File.FileName);

			for( Integer j=0; j<File.Sections.Num(); j++ )
			{
				TSection& Section = File.Sections[j];
				Writer.WriteString(String::Format(L"[%s]", *Section.Name));

				for( Integer k=0; k<Section.Pairs.Entries.Num(); k++ )
				{
					TMap<String, String>::TEntry& Entry = Section.Pairs.Entries[k];
					Writer.WriteString(String::Format(L"%s=%s", *Entry.Key, *Entry.Value ));
				}
				Writer.WriteString(L"");
			}
		}
	}
}


//
// Write bool value to config.
//
void CConfigManager::WriteBool( const Char* File, const Char* Section, const Char* Key, Bool Value )
{
	WriteString( File, Section, Key, Value ? L"True" : L"False" );
}


//
// Write integer value to config.
//
void CConfigManager::WriteInteger( const Char* File, const Char* Section, const Char* Key, Integer Value )
{
	WriteString( File, Section, Key, String::FromInteger(Value) );
}


//
// Write float value to config.
//
void CConfigManager::WriteFloat( const Char* File, const Char* Section, const Char* Key, Float Value )
{
	WriteString( File, Section, Key, String::FromFloat(Value) );
}


//
// Write string value to config.
//
void CConfigManager::WriteString( const Char* File, const Char* Section, const Char* Key, String Value )
{
	TIniFile* IniFile = FindFile(File);
	if( IniFile )
	{
		TSection* FSection = FindSection( File, Section );

		if( Section )
		{
			// Just update values.
			FSection->Pairs.Put( Key, Value );
		}
		else
		{
			// Section not found, so create new one.
			TSection NewSection;
			NewSection.Name = Section;
			NewSection.Pairs.Add( Key, Value );

			IniFile->Sections.Push(NewSection);
		}

		IniFile->bDirty = true;
	}
	else
	{
		// File not found, so create new one.
		TSection NewSection;
		NewSection.Name = Section;
		NewSection.Pairs.Add( Key, Value );

		TIniFile NewFile;
		NewFile.bDirty = true;
		NewFile.Name = File;
		NewFile.FileName = String::Format(L"%s\\%s.ini", *Directory, File);
		NewFile.Sections.Push(NewSection);

		Files.Push(NewFile);
	}
}


//
// Read a bool value from config.
//
Bool CConfigManager::ReadBool( const Char* File, const Char* Section, const Char* Key, Bool Default )
{
	String Value = ReadString( File, Section, Key, L"False" );
	Bool Result = Default;
	Integer TempInt;

	if( Value == L"True" )
	{
		Result = true;
	}
	else if( Value.ToInteger(TempInt, 0) )
	{
		Result = TempInt == 0;
	}

	return Result;
}



//
// Read a integer value from config.
//
Integer CConfigManager::ReadInteger( const Char* File, const Char* Section, const Char* Key, Integer Default )
{
	String Value = ReadString( File, Section, Key );
	Integer Result;

	Value.ToInteger( Result, Default );

	return Result;
}


//
// Read a float value from config.
//
Float CConfigManager::ReadFloat( const Char* File, const Char* Section, const Char* Key, Float Default )
{
	String Value = ReadString( File, Section, Key );
	Float Result;

	Value.ToFloat( Result, Default );

	return Result;
}


//
// Read a float value from config.
//
String CConfigManager::ReadString( const Char* File, const Char* Section, const Char* Key, String Default )
{
	String* Value = FindValue( File, Section, Key );
	return Value ? *Value : Default;
}


//
// Find a pair.
//
String* CConfigManager::FindValue( const Char* File, const Char* Section, const Char* Key )
{
	TSection* Found = FindSection( File, Section );
	return Found ? Found->Pairs.Get(Key) : nullptr;
}


//
// Find a section.
//
CConfigManager::TSection* CConfigManager::FindSection( const Char* File, const Char* Section )
{
	TIniFile* IniFile = FindFile(File);
	if( !IniFile )
		return nullptr;

	for( Integer j=0; j<IniFile->Sections.Num(); j++ )
		if( IniFile->Sections[j].Name == Section )
			return &IniFile->Sections[j];

	return nullptr;
}


//
// Find ini file.
//
CConfigManager::TIniFile* CConfigManager::FindFile( const Char* File )
{
	for( Integer i=0; i<Files.Num(); i++ )
		if( Files[i].Name == File )
		{
			return &Files[i];
		}

	return nullptr;
}


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/