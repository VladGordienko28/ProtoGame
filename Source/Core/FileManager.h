//-----------------------------------------------------------------------------
//	FileManager.h: File manager
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu 
{
namespace fm
{
	static const SizeT MAX_FILE_NAME_SIZE = 256;

	enum class EPathBase: Int32
	{
		Absolute,
		Exe,
		Temp,
		Packages,
		MAX
	};

	/**
	 *	An abstract binary file reader
	 */
	class IBinaryFileReader: public IInputStream
	{
	public:
		using Ptr = SharedPtr<IBinaryFileReader>;

		virtual String fileName() const = 0;
	};

	/**
	 *	An abstract binary file writer
	 */
	class IBinaryFileWriter: public IOutputStream
	{
	public:
		using Ptr = SharedPtr<IBinaryFileWriter>;

		virtual String fileName() const = 0;
	};

	/**
	 *	Read a binary file. Return null if file is not found.
	 */
	extern IBinaryFileReader::Ptr readBinaryFile( const Char* fileName );

	/**
	 *	Write a binary file. Return null if file creation failed.
	 */
	extern IBinaryFileWriter::Ptr writeBinaryFile( const Char* fileName );

	/**
	 *	Read a text file. Return null if file is not found.
	 */
	extern Text::Ptr readTextFile( const Char* fileName );

	/**
	 *	Write a text file. Return false if file wasn't created
	 */
	extern Bool writeTextFile( const Char* fileName, Text::Ptr text );

	/**
	 *	Returns the file path.
	 *	For example:
	 *		"F:\Amazing Fluorine\Res\HeroesAndMagic\Bitmaps\BMidnightSky.png" -> "F:\Amazing Fluorine\Res\HeroesAndMagic\Bitmaps"
	 *		"FlappyPork.fluproj" -> ""
	 */
	extern String getFilePath( const Char* fileName );

	/**
	 *	Returns the file extension.
	 *	For example:
	 *		"F:\Amazing Fluorine\Res\HeroesAndMagic\Bitmaps\BMidnightSky.png" -> "png"
	 *		"FlappyPork.fluproj" -> "fluproj"
	 */
	extern String getFileExt( const Char* fileName );
	
	/**
	 *	Returns the file name only.
	 *	For example:
	 *		"F:\Amazing Fluorine\Res\HeroesAndMagic\Bitmaps\BMidnightSky.png" -> "BMidnightSky"
	 *		"FlappyPork.fluproj" -> "FlappyPork"
	 */
	extern String getFileName( const Char* fileName );

	/**
	 *	Returns the file name with extension.
	 *	For example:
	 *		"F:\Amazing Fluorine\Res\HeroesAndMagic\Bitmaps\BMidnightSky.png" -> "BMidnightSky.png"
	 *		"FlappyPork.fluproj" -> "FlappyPork.fluproj"
	 */
	extern String getFileNameExt( const Char* fileName );
	
	/**
	 *	Return true, if file name is absolute, i.e. contains colon.
	 */
	extern Bool isAbsoluteFileName( const Char* fileName );

	/**
	 *	Replace all wrong slashes("/") to correct("\") in the path.
	 */
	extern String normalizeFileName( const Char* fileName );

	/**
	 *	Tries to resolve any path to absolute.
	 *		* EPathBase::Absolute - try to remove all control symbols and fix slashes
	 *		* EPathBase::Exe - same as above, but uses exe path as a base
	 *		* EPathBase::Temp - same as above, but uses some temporal path as a base
	 *		* EPathBase::Packages - same as above, but uses packages path as a base
	 */
	extern String resolveFileName( const Char* relFileName, EPathBase pathBase = EPathBase::Exe );

	/**
	 *	Return whether directory exists
	 */
	extern Bool directoryExists( const Char* dirName );

	/**
	 *	Creates a new directory, only last folder will be
	 *	created.
	 */
	extern Bool createDirectory( const Char* dirPath );

	/**
	 *	Return whether file exists
	 */
	extern Bool fileExists( const Char* fileName );

	/**
	 *	Traverse in diectory and return absolute file names for all files
	 *	event in nested directories.
	 *	Note: assumed directory without a last slash
	 */
	extern Bool traverseDirectory( const Char* directory, Array<String>& outFiles );

	/**
	 *	Find files in the specified directory with wildcard
	 */
	extern Array<String> findFiles( const Char* directory, const Char* wildcard );

	/**
	 *	Return the current working directory, should be only EXE path
	 */
	extern String getCurrentDirectory();

	/**
	 *	Change the current working directory, make sure, you know what are doing
	 */
	extern void setCurrentDirectory( const Char* newDirectory );

	/**
	 *	Return last file modification time
	 */
	extern Int64 getFileModificationTime( const Char* fileName );

} /* namespace fm */
} /* namespace flu */