//-----------------------------------------------------------------------------
//	FilesTracker.cpp: A helper class which track changes in files
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Resource.h"

namespace flu
{
namespace res
{
	FilesTracker::FilesTracker( String directory )
		:	m_trackedFiles(),
			m_directory( directory )
	{
		assert( fm::directoryExists( *directory ) );
	}

	FilesTracker::~FilesTracker()
	{
	}

	void FilesTracker::addResourceFile( ResourceId resourceId, const Array<String> resourceFiles )
	{
		for( auto it : resourceFiles )
		{
			File* trackedFile = m_trackedFiles.get( it );

			if( trackedFile )
			{
				// file already tracked
				trackedFile->resources.addUnique( resourceId );
			}
			else
			{
				// add new tracked file
				String absoluteFileName = m_directory + it;
				assert( fm::fileExists( *absoluteFileName ) );

				File newTrackedFile;
				newTrackedFile.lastModificationTime = fm::getFileModificationTime( *absoluteFileName );
				newTrackedFile.resources.push( resourceId );

				m_trackedFiles.put( it, newTrackedFile );
			}		
		}
	}
	
	void FilesTracker::removeResourceFiles( ResourceId resourceId )
	{
		Array<String> filesToUntrack;

		for( auto& file : m_trackedFiles )
		{
			file.value.resources.removeUnique( resourceId, false );

			if( file.value.resources.size() == 0 )
			{
				filesToUntrack.push( file.key );
			}
		}

		for( auto& it : filesToUntrack )
		{
			m_trackedFiles.remove( it );
		}
	}

	Array<ResourceId> FilesTracker::trackChanges()
	{
		Array<ResourceId> changedResources;
	
		for( auto& file : m_trackedFiles )
		{
			assert( file.value.resources.size() > 0 );

			String fileName = m_directory + file.key;

			if( fm::fileExists( *fileName ) )
			{
				Int64 fileTime = fm::getFileModificationTime( *fileName );

				if( fileTime > file.value.lastModificationTime )
				{
					for( auto& res : file.value.resources )
					{
						changedResources.addUnique( res );
					}
				
					file.value.lastModificationTime = fileTime;
				}
			}
			else
			{
				fatal( L"File \"%s\" removing is not allowed yet", *fileName );
			}
		}

		return changedResources;
	}
}
}