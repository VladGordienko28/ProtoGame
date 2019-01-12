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
	Integer ReadInteger( const Char* File, const Char* Section, const Char* Key, Integer Default=0 );
	Float ReadFloat( const Char* File, const Char* Section, const Char* Key, Float Default=0.f );
	String ReadString( const Char* File, const Char* Section, const Char* Key, String Default=L"" );

	// Writing.
	void WriteBool( const Char* File, const Char* Section, const Char* Key, Bool Value );
	void WriteInteger( const Char* File, const Char* Section, const Char* Key, Integer Value );
	void WriteFloat( const Char* File, const Char* Section, const Char* Key, Float Value );
	void WriteString( const Char* File, const Char* Section, const Char* Key, String Value );

private:
	// Structures.
	struct TSection
	{
		String					Name;
		TMap<String, String>	Pairs;
	};
	struct TIniFile
	{
		Bool					bDirty;
		TArray<TSection>		Sections;
		String					FileName;
		String					Name;
	};

	// Variables.
	TArray<TIniFile> Files;
	String Directory;

	// Helpers.
	String* FindValue( const Char* File, const Char* Section, const Char* Key );
	TSection* FindSection( const Char* File, const Char* Section );
	TIniFile* FindFile( const Char* File );
};


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/