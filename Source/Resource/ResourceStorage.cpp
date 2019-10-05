//-----------------------------------------------------------------------------
//	ResourceProvider.cpp: A classes which provides compiled resources
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Resource.h"

namespace flu
{
namespace res
{
	LocalStorage::LocalStorage( String packagesPath, String cachePath, Bool useCache )
	{
		m_useCache = useCache;
		m_packagesPath = fm::resolveFileName( *packagesPath, fm::EPathBase::Exe );
		m_cachePath = fm::resolveFileName( *cachePath, fm::EPathBase::Exe );

		if( !fm::directoryExists( *m_packagesPath ) )
		{
			fatal( L"Packages directory \"%s\" is not found", *m_packagesPath );
		}

		if( m_useCache && !fm::directoryExists( *m_cachePath ) )
		{
			if( !fm::createDirectory( *m_cachePath ) )
			{
				fatal( L"Unable to create cache directory \"%s\"", *m_cachePath );
			}
		}

		m_filesTracker = new FilesTracker( m_packagesPath );
	}

	LocalStorage::~LocalStorage()
	{
		m_filesTracker = nullptr;

		for( auto& it : m_compilers )
		{
			it = nullptr;
		}
	}

	void LocalStorage::registerCompiler( EResourceType type, IResourceCompiler* compiler )
	{
		assert( compiler );
		SizeT resTypeId = static_cast< SizeT >( type );

		assert( !m_compilers[resTypeId].hasObject() );
		m_compilers[resTypeId] = compiler;
	}

	CompiledResource LocalStorage::requestCompiled( ResourceId resourceId,
		IListener& listener, Bool allowCached )
	{
		// try to resolve ResourceId
		String resourceName = m_namesResolver.getName( resourceId );

		if( resourceName )
		{
			return requestCompiled( resourceId.getType(), resourceName, 
				listener, allowCached );
		}
		else
		{
			fatal( TEXT( "LocalStorage: unable to resolve ResourceId \"%s\"" ), *resourceId.toString() );
			return CompiledResource();
		}
	}

	CompiledResource LocalStorage::requestCompiled( EResourceType type, String resourceName, 
		IListener& listener, Bool allowCached )
	{
		ResourceId resourceId( type, resourceName );	

		// attempt to find in cache
		if( m_useCache && allowCached )
		{
			CompiledResource cachedResource = loadFromCache( resourceId, resourceName, listener );

			if( cachedResource.isValid() )
			{
				return cachedResource;
			}
		}

		// compilation need
		CompilationOutput output = compile( resourceId, resourceName, listener );

		if( !output.hasError() )
		{
			if( m_useCache )
			{
				saveToCache( resourceId, resourceName, output, listener );
			}

			return output.compiledResource;
		}
		else
		{
			return CompiledResource();
		}
	}

	String LocalStorage::resolveResourceId( ResourceId resourceId ) const
	{
		return m_namesResolver.getName( resourceId );
	}

	void LocalStorage::reloadChanged( ResourceSystemList& systems, IListener& listener )
	{
		Array<ResourceId> changedResources = m_filesTracker->trackChanges();

		for( auto& it : changedResources )
		{
			IResourceSystem* system = systems[ static_cast<SizeT>( it.getType() ) ].get();
			assert( system );

			if( system->allowHotReloading() )
			{
				String resourceName = m_namesResolver.getName( it );
				listener.onInfo( resourceName, TEXT( "changed from the outside, reloading..." ) );

				CompiledResource recompiled = requestCompiled( it.getType(), resourceName, listener, false );

				if( recompiled.isValid() )
				{
					system->reloadResource( it, recompiled );
				}
				else
				{
					listener.onError( resourceName, TEXT( "unable to reload changed resource" ) );
				}
			}
		}
	}

	CompiledResource LocalStorage::loadFromCache( ResourceId resourceId, String resourceName, IListener& listener )
	{
		assert( m_useCache && m_cachePath );

		String cacheFileName = m_cachePath + resourceId.toString() + CACHE_EXTENSION;

		if( fm::fileExists( *cacheFileName ) )
		{
			auto fileReader = fm::readBinaryFile( *cacheFileName );

			if( fileReader.hasObject() )
			{
				Array<String> dependencyFiles;
				UInt64 lastModificationTime;
				Map<ResourceId, String> references;
				CompiledResource compiledResource;
			
				*fileReader >> dependencyFiles;
				*fileReader >> lastModificationTime;
				*fileReader >> references;
				*fileReader >> compiledResource;

				// validate cache
				Bool isValidCache = true;
				for( auto& it : dependencyFiles )
				{
					String itFileName = m_packagesPath + it;

					if( fm::fileExists( *itFileName ) )
					{
						UInt64 modificationTime = fm::getFileModificationTime( *itFileName );
						if( modificationTime > lastModificationTime )
						{
							isValidCache = false;
							break;
						}
					}
					else
					{
						isValidCache = false;
						break;
					}
				}

				// start tracking dependency files
				if( isValidCache )
				{
					m_filesTracker->removeResourceFiles( resourceId );
					m_filesTracker->addResourceFile( resourceId, dependencyFiles );

					m_namesResolver.addName( resourceId, resourceName );

					// store references info
					for( const auto& it : references )
					{
						m_namesResolver.addName( it.key, it.value );
					}

					listener.onInfo( resourceName, TEXT( "found in cache" ) );
					return compiledResource;				
				}
				else
				{
					listener.onInfo( resourceName, TEXT( "cache is out of date" ) );
				}
			}
		}

		// not found in cache
		return CompiledResource();
	}

	void LocalStorage::saveToCache( ResourceId resourceId, String resourceName, const CompilationOutput& output, IListener& listener )
	{
		assert( m_useCache && m_cachePath );
		assert( !output.hasError() && output.compiledResource.isValid() );

		String cacheFileName = m_cachePath + resourceId.toString() + CACHE_EXTENSION;
	
		auto fileWriter = fm::writeBinaryFile( *cacheFileName );

		if( fileWriter.hasObject() )
		{
			*fileWriter << output.dependencyFiles;
			*fileWriter << output.lastModificationTime;
			*fileWriter << output.references;
			*fileWriter << output.compiledResource;

			listener.onInfo( resourceName, String::format( TEXT( "Cache saved to \"%s\"" ), *cacheFileName ) );
		}
		else
		{
			listener.onWarning( resourceName, 
				String::format( TEXT( "Unable save resource cache to \"%s\"" ), *cacheFileName ) );
		}
	}

	CompilationOutput LocalStorage::compile( ResourceId resourceId, String resourceName, IListener& listener )
	{
		UInt64 startupTime = time::cycles64();
		listener.onInfo( resourceName, TEXT( "compilation started..." ) );

		assert( resourceName );
		assert( getCompiler( resourceId.getType() ) );

		CompilationOutput output;

		EResourceType fileType;
		String resourceFileName = findResourceFile( resourceName, &fileType );
	
		if( !resourceFileName )
		{
			listener.onError( resourceName, TEXT( "is not found on disk" ) );
			return output;
		}

		assert( resourceId.getType() == fileType );

		class DependencyProvider: public IDependencyProvider
		{
		public:
			DependencyProvider( String resourcePath, String resourceRelativePath, CompilationOutput& compilationOutput )
				:	m_resourcePath( resourcePath ),
					m_resourceRelativePath( resourceRelativePath ),
					m_compilationOutput( compilationOutput )
			{
			}

			Text::Ptr getTextFile( String relativeFileName ) const override
			{
				if( !fm::isAbsoluteFileName( *relativeFileName ) )
				{
					String absoluteFileName = m_resourcePath + relativeFileName;
					Text::Ptr textFile = fm::readTextFile( *absoluteFileName );

					if( textFile.hasObject() )
					{
						m_compilationOutput.dependencyFiles.addUnique( m_resourceRelativePath + L"\\" + relativeFileName );

						UInt64 modificationTime = fm::getFileModificationTime( *absoluteFileName );
						if( modificationTime > m_compilationOutput.lastModificationTime )
						{
							m_compilationOutput.lastModificationTime = modificationTime;
						}

						return textFile;
					}
				}

				return nullptr;
			}

			IInputStream::Ptr getBinaryFile( String relativeFileName ) const override
			{
				if( !fm::isAbsoluteFileName( *relativeFileName ) )
				{
					String absoluteFileName = m_resourcePath + relativeFileName;
					fm::IBinaryFileReader::Ptr binFile = fm::readBinaryFile( *absoluteFileName );

					if( binFile.hasObject() )
					{
						m_compilationOutput.dependencyFiles.addUnique( m_resourceRelativePath + L"\\" + relativeFileName );

						UInt64 modificationTime = fm::getFileModificationTime( *absoluteFileName );
						if( modificationTime > m_compilationOutput.lastModificationTime )
						{
							m_compilationOutput.lastModificationTime = modificationTime;
						}

						return binFile.get();
					}
				}

				return nullptr;
			}

		private:
			String m_resourcePath;
			String m_resourceRelativePath;
			CompilationOutput& m_compilationOutput;
		};

		String resourceRelativePath = fm::getFilePath( *resourceFileName );
		String resourcePath = m_packagesPath + resourceRelativePath + TEXT( "\\" );

		DependencyProvider dependencyProvider( resourcePath, resourceRelativePath, output );

		getCompiler( fileType )->compile( fm::getFileNameExt( *resourceFileName ), dependencyProvider, output );

		// output all warnings
		for( auto& it : output.warningsMsg )
		{
			listener.onWarning( resourceName, it );
		}

		if( !output.hasError() )
		{
			listener.onInfo( resourceName, String::format( L"compiled successfully in %.4f sec", 
				time::elapsedSecFrom( startupTime ) ) );

			m_namesResolver.addName( resourceId, resourceName );

			// untrack old dependencies and track new
			m_filesTracker->removeResourceFiles( resourceId );
			m_filesTracker->addResourceFile( resourceId, output.dependencyFiles );
		}
		else
		{
			// compilation failed
			listener.onError( resourceName, output.errorMsg );
		}

		return output;
	}

	String LocalStorage::findResourceFile( String resourceName, EResourceType* outType ) const
	{
		assert( resourceName );

		// replace all '.' with '\\' in name
		String wildcard = resourceName;
		for( SizeT i = 0; i < wildcard.len(); ++i )
		{
			if( wildcard[i] == TEXT( '.' ) )
			{
				wildcard[i] = TEXT( '\\' );
			}
		}

		wildcard += TEXT( ".*" );

		String directory = m_packagesPath + fm::getFilePath( *wildcard );
		String fileMask = fm::getFileName( *wildcard ) + TEXT( "." ) + fm::getFileExt( *wildcard );

		Array<String> files = fm::findFiles( *directory, *fileMask );

		for( auto& file : files )
		{
			String relativeFileName = String::del( file, 0, m_packagesPath.len() );

			// find at least one compiler
			for( SizeT i = 0; i < m_compilers.size(); ++i )
			{
				const IResourceCompiler* compiler = m_compilers[i].get();

				if( compiler && compiler->isSupportedFile( relativeFileName ) )
				{
					if( outType )
					{
						*outType = static_cast< EResourceType >( i );
					}

					return relativeFileName;
				}
			}
		}

		// nothing found
		return TEXT( "" );
	}

	EResourceType LocalStorage::getFileResourceType( String fileName ) const
	{
		for( SizeT i = 0; i < m_compilers.size(); ++i )
		{
			const auto& compiler = m_compilers[i];

			if( compiler.hasObject() && compiler->isSupportedFile( fileName ) )
			{
				return static_cast<EResourceType>( i );
			}
		}

		return EResourceType::MAX;
	}

	PackageStorage::PackageStorage()
		:	m_packages(),
			m_resourceId2Package()
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

	CompiledResource PackageStorage::requestCompiled( EResourceType type, String resourceName, IListener& listener )
	{
		ResourceId resourceId( type, resourceName );

		return requestCompiled( resourceId, listener );
	}

	CompiledResource PackageStorage::requestCompiled( ResourceId resourceId, IListener& listener )
	{
		Package** packagePtr = m_resourceId2Package.get( resourceId );

		if( packagePtr && *packagePtr )
		{
			Package* package = *packagePtr;

			listener.onInfo( resourceId.toString(), TEXT( "loaded from package" ) );
			return package->getResource( resourceId );
		}
		else
		{
			listener.onError( resourceId.toString(), TEXT( "is not found" ) );
			return CompiledResource();
		}
	}

	String PackageStorage::resolveResourceId( ResourceId resourceId ) const
	{
		return m_namesResolver.getName( resourceId );
	}

	Bool PackageStorage::loadAllPackages( String directory )
	{
		UInt32 numLoaded = 0;

		// remove last slash if has
		if( directory[directory.len() - 1] == TEXT( '\\' ) )
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

			info( TEXT( "Package \"%s\" loaded" ), *package->getName() );
			return true;
		}
		else
		{
			error( TEXT( "Unable to load package from \"%s\"" ), *fileName );

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
		Int32 dotPos = String::pos( TEXT( "." ), resourceName, 0 );
		
		if( dotPos != -1 )
		{
			return String::copy( resourceName, 0, dotPos );
		}
		else
		{
			return TEXT( "" );
		}
	}

	static String fileNameToResourceName( String fileName )
	{
		// remove extension
		Int32 dotPos = String::pos( TEXT( "." ), fileName, 0 );
		if( dotPos == -1 )
		{
			return TEXT( "" );
		}

		String resourceName = String::del( fileName, dotPos, fileName.len() - dotPos );

		// remove slashes
		for( SizeT i = 0; i < resourceName.len(); ++i )
		{
			if( resourceName[i] == TEXT( '\\' ) || resourceName[i] == TEXT( '/' ) )
			{
				resourceName[i] = TEXT( '.' );
			}

			// check characters
			if( !cstr::isDigitLetter( resourceName[i] ) && 
				resourceName[i] != TEXT( '.' ) )
			{
				return TEXT( "" );
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
		assert( outputPath[outputPath.len() - 1] == TEXT( '\\' )  );

		// remove last slash if has
		String packagesPath = localStorage->getPackagesPath();
		if( packagesPath[packagesPath.len() - 1] == TEXT( '\\' ) )
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
				error( TEXT( "Unable to save package \"%s\"" ), *packageFileName );
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
				CompiledResource compiledResource = localStorage->requestCompiled( resInfo.resourceType, resInfo.resourceName, listener, true );

				if( !compiledResource.isValid() )
				{
					error( TEXT( "Unable to compile resource \"%s\"" ), *resInfo.resourceName );
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