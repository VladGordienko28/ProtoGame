//-----------------------------------------------------------------------------
//	FilesTracker.h: A helper class which track changes in files
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace res
{
	/**
	 *	FilesTracker
	 */
	class FilesTracker final: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<FilesTracker>;

		FilesTracker( String directory );
		~FilesTracker();

		void addResourceFile( ResourceId resourceId, const Array<String> resourceFiles );
		void removeResourceFiles( ResourceId resourceId );

		Array<ResourceId> trackChanges();

	private:
		static const UInt64 UNKNOWN_MODIFICATION_TIME = 0;

		// single tracked file
		struct File
		{
			Int64 lastModificationTime;
			Array<ResourceId> resources;
		};

		Map<String, File> m_trackedFiles;
		String m_directory;

		FilesTracker() = delete;
	};
}
}