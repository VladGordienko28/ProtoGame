//-----------------------------------------------------------------------------
//	ResourceProvider.cpp: A classes which provides compiled resources
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Resource.h"

namespace flu
{
namespace res
{
	PackageStorage::PackageStorage()
		:	m_packages(),
			m_resourceId2Package(),
			m_listener( nullptr )
	{
	}

	PackageStorage::~PackageStorage()
	{
		for( auto& package : m_packages )
		{
			delete package;
			package = nullptr;
		}

		m_packages.empty();
		m_resourceId2Package.empty();
	}

	void PackageStorage::setListener( IListener* listener )
	{
		m_listener = listener;
	}

	CompiledResource PackageStorage::requestCompiled( EResourceType type, String resourceName )
	{
		ResourceId resourceId( type, resourceName );

		return requestCompiled( resourceId );
	}

	CompiledResource PackageStorage::requestCompiled( ResourceId resourceId )
	{
		Package** packagePtr = m_resourceId2Package.get( resourceId );

		if( packagePtr && *packagePtr )
		{
			Package* package = *packagePtr;

			if( m_listener )
				m_listener->onInfo( resourceId.toString(), TXT( "loaded from package" ) );
			
			return package->getResource( resourceId );
		}
		else
		{
			if( m_listener )
				m_listener->onError( resourceId.toString(), TXT( "is not found" ) );

			return CompiledResource();
		}
	}

	String PackageStorage::resolveResourceId( ResourceId resourceId )
	{
		return m_namesResolver.getName( resourceId );
	}

	void PackageStorage::update( ResourceSystemList& systemList )
	{
	}

	Bool PackageStorage::loadAllPackages( String directory )
	{
		UInt32 numLoaded = 0;

		// remove last slash if has
		if( directory[directory.len() - 1] == TXT( '\\' ) )
		{
			directory = String::del( directory, directory.len() - 1, 1 );
		}

		Array<String> packageFiles = fm::findFiles( *directory, *String::format( L"*%s", Package::EXTENSION ) );

		for( auto& it : packageFiles )
		{
			const Bool loaded = loadPackage( it );

			if( loaded )
			{
				numLoaded++;
			}
		}

		return numLoaded > 0;
	}

	Bool PackageStorage::loadPackage( String fileName )
	{
		assert( fm::fileExists( *fileName ) );

		Package* package = new Package();

		if( package->load( fileName ) )
		{
			m_packages.push( package );

			// add info about all resources
			Array<ResourceId> packageResources = package->getResourceList();

			for( auto& resource : packageResources )
			{
				Bool isNew = m_resourceId2Package.put( resource, package );
				assert( isNew );
			}

			package->fillNamesResolver( m_namesResolver );

			info( TXT( "Package \"%s\" loaded" ), *package->getName() );
			return true;
		}
		else
		{
			error( TXT( "Unable to load package from \"%s\"" ), *fileName );

			delete package;
			return false;
		}
	}

	Array<String> PackageStorage::getPackageNames() const
	{
		Array<String> result;

		for( const auto& package : m_packages )
		{
			result.push( package->getName() );
		}

		return result;
	}

	static String getResourcePackageName( String resourceName )
	{
		Int32 dotPos = String::pos( TXT( "." ), resourceName, 0 );
		
		if( dotPos != -1 )
		{
			return String::copy( resourceName, 0, dotPos );
		}
		else
		{
			return TXT( "" );
		}
	}

	static String fileNameToResourceName( String fileName )
	{
		// remove extension
		Int32 dotPos = String::pos( TXT( "." ), fileName, 0 );
		if( dotPos == -1 )
		{
			return TXT( "" );
		}

		String resourceName = String::del( fileName, dotPos, fileName.len() - dotPos );

		// remove slashes
		for( SizeT i = 0; i < resourceName.len(); ++i )
		{
			if( resourceName[i] == TXT( '\\' ) || resourceName[i] == TXT( '/' ) )
			{
				resourceName[i] = TXT( '.' );
			}

			// check characters
			if( !cstr::isDigitLetter( resourceName[i] ) && 
				resourceName[i] != TXT( '.' ) )
			{
				return TXT( "" );
			}
		}

		return resourceName;
	}

	UInt32 PackageStorage::generatePackages( LocalStorage* localStorage, String outputPath )
	{
		assert( localStorage );
		assert( outputPath );
		assert( fm::directoryExists( *localStorage->getPackagesPath() ) );
		assert( fm::directoryExists( *outputPath ) );
		assert( outputPath[outputPath.len() - 1] == TXT( '\\' )  );

		// remove last slash if has
		String packagesPath = localStorage->getPackagesPath();
		if( packagesPath[packagesPath.len() - 1] == TXT( '\\' ) )
		{
			packagesPath = String::del( packagesPath, packagesPath.len() - 1, 1 );
		}

		Array<String> allFiles;

		// collect all files
		if( !fm::traverseDirectory( *packagesPath, allFiles ) )
		{
			error( L"Unable to generate packages: directory \"%s\" is empty", *packagesPath );
			return 0;
		}

		// make path relative for all resources
		for( auto& it : allFiles )
		{
			assert( it.len() > packagesPath.len() );

			it = String::del( it, 0, packagesPath.len() + 1 );
		}

		struct ResourceInfo
		{
			String fileName;
			String resourceName;
			EResourceType resourceType;
		};

		Map<String, Array<ResourceInfo>> packagesInfo;

		for( auto& file : allFiles )
		{
			// check whether file compilable
			EResourceType resourceType = localStorage->getFileResourceType( file );

			if( resourceType != EResourceType::MAX )
			{
				ResourceInfo resInfo;
				resInfo.fileName = file;
				resInfo.resourceName = fileNameToResourceName( file );
				resInfo.resourceType = resourceType;

				assert( resInfo.resourceName );

				String packageName = getResourcePackageName( resInfo.resourceName );
				assert( packageName );

				if( packagesInfo.hasKey( packageName ) )
				{
					packagesInfo.getRef( packageName ).push( resInfo );
				}
				else
				{
					Array<ResourceInfo> resInfos;
					resInfos.push( resInfo );
					packagesInfo.put( packageName, resInfos );
				}	
			}
		}

		// generate package by package
		for( auto& package : packagesInfo )
		{
			assert( package.value.size() > 0 );
		
			String packageFileName = outputPath + package.key + Package::EXTENSION;

			fm::IBinaryFileWriter::Ptr writer = fm::writeBinaryFile( *packageFileName );
			if( !writer.hasObject() )
			{
				error( TXT( "Unable to save package \"%s\"" ), *packageFileName );
				return 0;
			}

			PackageHeader header;
			header.magic = PackageHeader::MAGIC;
			header.version = PackageHeader::VERSION;
			header.name = package.key;
			header.size = package.value.size();

			*writer << header;

			Array<SizeT> resourceChecksumOffsets;
			Array<SizeT> resourceDataOffsets;

			// writer resource infos with stubs
			for( Int32 i = 0; i < package.value.size(); ++i )
			{
				const ResourceInfo& info = package.value[i];
				ResourceId resourceId( info.resourceType, info.resourceName );

				*writer << resourceId;
				*writer << info.resourceName;

				UInt32 unused = 0xdeadbeaf;

				resourceChecksumOffsets.push( writer->tell() );
				*writer << unused;

				resourceDataOffsets.push( writer->tell() );
				*writer << unused;
			}

			// compile each resource
			for( Int32 i = 0; i < package.value.size(); ++i )
			{
				const ResourceInfo& resInfo = package.value[i];

				ListenerList listener;
				CompiledResource compiledResource = localStorage->requestCompiled( resInfo.resourceType, resInfo.resourceName );

				if( !compiledResource.isValid() )
				{
					error( TXT( "Unable to compile resource \"%s\"" ), *resInfo.resourceName );
					return 0;
				}

				UInt32 checksum = compiledResource.getChecksum();
				UInt32 offset = writer->tell();

				writer->seek( resourceChecksumOffsets[i] );
				*writer << checksum;
				writer->seek( resourceDataOffsets[i] );
				*writer << offset;

				writer->seek( offset );

				*writer << compiledResource;
			}
		}

		return packagesInfo.size();
	}
}
}