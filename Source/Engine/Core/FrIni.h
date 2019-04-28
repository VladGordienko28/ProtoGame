/*=============================================================================
    FrIni.h: Config files support class.
    Copyright Oct.2016 Vlad Gordienko.
=============================================================================*/

/*-----------------------------------------------------------------------------
	CConfigManager.
-----------------------------------------------------------------------------*/

//
// An ini-files based configuration manager.
//
class CConfigManager
{
public:
	// CConfigManager interface.
	CConfigManager( String WorkingDirectory );
	~CConfigManager();
	void Flush();

	// Reading.
	Bool ReadBool( const Char* File, const Char* Section, const Char* Key, Bool Default=false );
	Int32 ReadInteger( const Char* File, const Char* Section, const Char* Key, Int32 Default=0 );
	Float ReadFloat( const Char* File, const Char* Section, const Char* Key, Float Default=0.f );
	String ReadString( const Char* File, const Char* Section, const Char* Key, String Default=L"" );

	// Writing.
	void WriteBool( const Char* File, const Char* Section, const Char* Key, Bool Value );
	void WriteInteger( const Char* File, const Char* Section, const Char* Key, Int32 Value );
	void WriteFloat( const Char* File, const Char* Section, const Char* Key, Float Value );
	void WriteString( const Char* File, const Char* Section, const Char* Key, String Value );

private:
	// Structures.
	struct TSection
	{
		String					Name;
		Map<String, String>		Pairs;
	};
	struct TIniFile
	{
		Bool				bDirty;
		Array<TSection>		Sections;
		String				FileName;
		String				Name;
	};

	// Variables.
	Array<TIniFile> Files;
	String Directory;

	// Helpers.
	String* FindValue( const Char* File, const Char* Section, const Char* Key );
	TSection* FindSection( const Char* File, const Char* Section );
	TIniFile* FindFile( const Char* File );
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/