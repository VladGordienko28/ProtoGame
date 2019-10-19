//-----------------------------------------------------------------------------
//	ConfigManager.cpp: A master of all ini files
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Core.h"

namespace flu
{
	class ConfigManagerImpl final: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<ConfigManagerImpl>;

		ConfigManagerImpl( String directory, String appName );
		~ConfigManagerImpl();

		void flushAll();

		Bool readBool( EConfigFile file, const Char* section, const Char* key, Bool _default );
		Int32 readInt( EConfigFile file, const Char* section, const Char* key, Int32 _default );
		Float readFloat( EConfigFile file, const Char* section, const Char* key, Float _default );
		String readString( EConfigFile file, const Char* section, const Char* key, String _default );

		void writeBool( EConfigFile file, const Char* section, const Char* key, Bool value );
		void writeInt( EConfigFile file, const Char* section, const Char* key, Int32 value );
		void writeFloat( EConfigFile file, const Char* section, const Char* key, Float value  );
		void writeString( EConfigFile file, const Char* section, const Char* key, String value );

		String getApplicationName() const
		{
			return m_appName;
		}

	private:
		static const constexpr Char EXTENSION[] = TXT("*.ini");
		static const constexpr Char SYSTEM_FILE[] = TXT("System");
		static const constexpr Char USER_FILE[] = TXT("User");

		struct Value
		{
		public:
			String text;
			Bool useQuotes = false;
		};

		struct Section
		{
		public:
			Map<String, Value> pairs;
		};

		struct File
		{
		public:
			String fileName;
			Map<String, Section> sections;
			Bool isDirty = false;
		};

		Map<String, File> m_files;
		String m_directory;
		String m_appName;

		Bool loadFile( String fileName );
		Bool saveFile( File& file );

		String readValue( EConfigFile file, const Char* section, const Char* key );
		void writeValue( EConfigFile file, const Char* section, const Char* key, String value );

		Section* findSection( EConfigFile file, const Char* section, Bool createIfNotFound = false );
	};

	ConfigManagerImpl::ConfigManagerImpl( String directory, String appName )
		:	m_files(),
			m_directory( directory ),
			m_appName( appName )
	{
		assert( fm::directoryExists( *directory ) );
		assert( appName );

		Array<String> iniFileList = fm::findFiles( *directory, EXTENSION );

		for( const auto& fileName : iniFileList )
		{
			if( !loadFile( fileName ) )
			{
				error( L"Unable to parse config file \"%s\"", *fileName );
			}
		}

		info( L"ConfigManager initialized with %d/%d config files", m_files.size(), 
			iniFileList.size() );
	}

	ConfigManagerImpl::~ConfigManagerImpl()
	{
		flushAll();
		m_files.empty();

		info( L"ConfigManager shutdown" );
	}

	void ConfigManagerImpl::flushAll()
	{
		UInt32 numFlushed = 0;

		for( auto& file : m_files )
		{
			if( file.value.isDirty )
			{
				if( !saveFile( file.value ) )
				{
					error( L"Unable to save config file \"%s\"", *file.value.fileName );
				}

				numFlushed++;
			}
		}

		info( L"ConfigManager flushed %d files", numFlushed );
	}

	Bool ConfigManagerImpl::readBool( EConfigFile file, const Char* section, const Char* key, Bool _default )
	{
		String value = readValue( file, section, key );
		Int32 tempInt;

		if( String::insensitiveCompare( TXT("True"), value ) == 0 )
		{
			return true;
		}
		else if( String::insensitiveCompare( TXT("False"), value ) == 0 )
		{
			return false;
		}
		else if( value.toInteger( tempInt, 0 ) )
		{
			return tempInt == 0;
		}
		else
		{
			return _default;
		}
	}

	Int32 ConfigManagerImpl::readInt( EConfigFile file, const Char* section, const Char* key, Int32 _default )
	{
		String value = readValue( file, section, key );
		Int32 result = _default;

		value.toInteger( result, _default );

		return result;
	}

	Float ConfigManagerImpl::readFloat( EConfigFile file, const Char* section, const Char* key, Float _default )
	{
		String value = readValue( file, section, key );
		Float result = _default;

		value.toFloat( result, _default );

		return result;
	}

	String ConfigManagerImpl::readString( EConfigFile file, const Char* section, const Char* key, String _default )
	{
		String value = readValue( file, section, key );
		return value ? value : _default;
	}

	void ConfigManagerImpl::writeBool( EConfigFile file, const Char* section, const Char* key, Bool value )
	{
		writeValue( file, section, key, value ? TXT("True") : TXT("False") );
	}

	void ConfigManagerImpl::writeInt( EConfigFile file, const Char* section, const Char* key, Int32 value )
	{
		writeValue( file, section, key, String::fromInteger( value ) );
	}

	void ConfigManagerImpl::writeFloat( EConfigFile file, const Char* section, const Char* key, Float value  )
	{
		writeValue( file, section, key, String::fromFloat( value ) );
	}

	void ConfigManagerImpl::writeString( EConfigFile file, const Char* section, const Char* key, String value )
	{
		writeValue( file, section, key, value );
	}

	Bool ConfigManagerImpl::loadFile( String fileName )
	{
		class IniLexer: public lexer::Lexer
		{
		public:
			IniLexer( Text::Ptr text )
				:	Lexer( text, lexer::LexerConfig() )
			{
			}

			~IniLexer()
			{
			}

		private:
			Char getChar() override
			{
			Loop:
				Char c = getRawChar();

				if( c == TXT(';') || c == TXT('#') )
				{
					m_position.line++;
					m_position.pos = 0;
					goto Loop;
				}

				return c;
			}
		};	

		Text::Ptr text = fm::readTextFile( *fileName );
		if( !text.hasObject() )
		{
			return false;
		}

		File file;
		file.fileName = fileName;
		file.isDirty= false;

		IniLexer lexer( text );
		Section* section = nullptr;

		while( !lexer.isEof() )
		{
			lexer::Token token;
			if( !lexer.getToken( token, true ) )
			{
				break;
			}

			if( token.getType() == lexer::ETokenType::Symbol && token.getText() == TXT("[") )
			{
				// section declaration
				String sectionName = lexer.getIdentifier();

				if( sectionName && !file.sections.hasKey( sectionName ) && 
					lexer.matchSymbol( TXT("]") ) )
				{
					file.sections.put( sectionName, {} );	
					
					section = file.sections.get( sectionName );
					assert( section );
				}
				else
				{
					// missing section name, section is duplicated or
					// no enclosing bracket
					return false;
				}
			}
			else if( token.getType() == lexer::ETokenType::Identifier )
			{
				// pair declaration
				String key = token.getText();
				lexer::Token valueToken;

				if( section && !section->pairs.hasKey( key ) && 
					lexer.matchSymbol( TXT("=") ) && lexer.getToken( valueToken, true ) )
				{
					Value value;

					if( valueToken.getType() == lexer::ETokenType::Float || 
						valueToken.getType() == lexer::ETokenType::Integer ||
						valueToken.getType() == lexer::ETokenType::String ||
						valueToken.getType() == lexer::ETokenType::Identifier )
					{
						value.useQuotes = valueToken.getType() == lexer::ETokenType::String;
						value.text = value.useQuotes ? valueToken.getStringConst() : valueToken.getText();

						section->pairs.put( key, value );					
					}
					else
					{
						// unrecognized value type
						return false;
					}
				}
				else
				{
					// out of section, duplicated, missing assignment or value
					return false;
				}
			}
			else
			{
				// unrecognized token
				return false;
			}
		}

		String name = fm::getFileName( *fileName );

		assert( name && !m_files.hasKey( name ) );
		m_files.put( name, file );
		return true;
	}

	Bool ConfigManagerImpl::saveFile( File& file )
	{
		assert( file.isDirty );

		TextWriter writer;

		for( const auto& section : file.sections )
		{
			writer << String::format( TXT("[%s]\n"), *section.key );

			for( const auto& pair : section.value.pairs )
			{
				auto fmtStr = pair.value.useQuotes ? TXT("%s=\"%s\"\n") : TXT("%s=%s\n");
				writer << String::format( fmtStr, *pair.key, *pair.value.text );
			}

			writer << TXT("\n");
		}

		if( fm::writeTextFile( *file.fileName, writer.getText() ) )
		{
			file.isDirty = false;
			return true;
		}
		else
		{
			return false;
		}
	}

	String ConfigManagerImpl::readValue( EConfigFile file, const Char* section, const Char* key )
	{
		if( Section* sect = findSection( file, section, false ) )
		{
			Value* value = sect->pairs.get( key );

			return value ? value->text : TXT("");
		}
		else
		{
			return TXT("");
		}
	}

	void ConfigManagerImpl::writeValue( EConfigFile file, const Char* section, const Char* key, String value )
	{
		Section* sect = findSection( file, section, true );
		assert( sect );

		Value val;
		val.text = value;
		val.useQuotes = cstr::findChar( *value, TXT(' ') ) != nullptr;

		sect->pairs.put( key, val );
	}

	ConfigManagerImpl::Section* ConfigManagerImpl::findSection( EConfigFile file, const Char* section, Bool createIfNotFound )
	{
		String fileName;

		// find file to write
		switch ( file )
		{
			case EConfigFile::System:
			{
				fileName = SYSTEM_FILE;
				break;
			}
			case EConfigFile::Application:
			{
				fileName = m_appName;
				break;
			}
			case EConfigFile::User:
			{
				fileName = USER_FILE;
				break;
			}
			case EConfigFile::Any:
			{
				for( auto& itFile : m_files )
				{
					for( auto& itSection : itFile.value.sections )
					{
						if( itSection.key == section )
						{
							// found
							return &itSection.value;
						}
					}
				}
				break;
			}
			default:
			{
				fatal( L"Unknown config file type %d", file );
				break;
			}
		}

		File* configFile = m_files.get( fileName );

		if( !configFile )
		{
			if( createIfNotFound )
			{
				assert( file != EConfigFile::Any && fileName );

				File newFile;
				newFile.fileName = String::format( TXT("%s\\%s.ini"), *m_directory, *fileName );
				newFile.isDirty = true;

				m_files.put( fileName, newFile );
				configFile = m_files.get( fileName );
			}
			else
			{
				return nullptr;
			}
		}

		Section* configSect = configFile->sections.get( section );

		if( !configSect && createIfNotFound )
		{
			configFile->sections.put( section, {} );
			return configFile->sections.get( section );
		}
		else
		{
			return configSect;
		}
	}

	ConfigManagerImpl::UPtr g_configManager;

	void ConfigManager::create( String directory, String appName )
	{
		assert( !g_configManager.hasObject() );

		g_configManager = new ConfigManagerImpl( directory, appName );
	}

	void ConfigManager::destroy()
	{
		assert( g_configManager.hasObject() );

		g_configManager = nullptr;
	}

	void ConfigManager::flushAll()
	{
		g_configManager->flushAll();
	}

	Bool ConfigManager::readBool( EConfigFile file, const Char* section, const Char* key, Bool _default )
	{
		return g_configManager->readBool( file, section, key, _default );
	}

	Int32 ConfigManager::readInt( EConfigFile file, const Char* section, const Char* key, Int32 _default )
	{
		return g_configManager->readInt( file, section, key, _default );
	}

	Float ConfigManager::readFloat( EConfigFile file, const Char* section, const Char* key, Float _default )
	{
		return g_configManager->readFloat( file, section, key, _default );
	}

	String ConfigManager::readString( EConfigFile file, const Char* section, const Char* key, String _default )
	{
		return g_configManager->readString( file, section, key, _default );
	}

	void ConfigManager::writeBool( EConfigFile file, const Char* section, const Char* key, Bool value )
	{
		g_configManager->writeBool( file, section, key, value );
	}

	void ConfigManager::writeInt( EConfigFile file, const Char* section, const Char* key, Int32 value )
	{
		g_configManager->writeInt( file, section, key, value );
	}

	void ConfigManager::writeFloat( EConfigFile file, const Char* section, const Char* key, Float value  )
	{
		g_configManager->writeFloat( file, section, key, value );
	}

	void ConfigManager::writeString( EConfigFile file, const Char* section, const Char* key, String value )
	{
		g_configManager->writeString( file, section, key, value );
	}

	String ConfigManager::getApplicationName()
	{
		return g_configManager->getApplicationName();
	}
}