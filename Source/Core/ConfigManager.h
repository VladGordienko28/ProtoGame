//-----------------------------------------------------------------------------
//	ConfigManager.h: A master of all ini files
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 *	A configuration file type
	 */
	enum class EConfigFile
	{
		System,
		Application,
		User,
		Any
	};

	/**
	 *	A global configuration manager
	 */
	class ConfigManager final
	{
	public:
		static void create( String directory, String appName );
		static void destroy();
		static void flushAll();

		static Bool readBool( EConfigFile file, const Char* section, const Char* key, Bool _default = false );
		static Int32 readInt( EConfigFile file, const Char* section, const Char* key, Int32 _default = 0 );
		static Float readFloat( EConfigFile file, const Char* section, const Char* key, Float _default = 0.f );
		static String readString( EConfigFile file, const Char* section, const Char* key, String _default = TEXT("") );

		static void writeBool( EConfigFile file, const Char* section, const Char* key, Bool value );
		static void writeInt( EConfigFile file, const Char* section, const Char* key, Int32 value );
		static void writeFloat( EConfigFile file, const Char* section, const Char* key, Float value  );
		static void writeString( EConfigFile file, const Char* section, const Char* key, String value );
	};
}