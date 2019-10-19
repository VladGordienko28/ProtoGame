//-----------------------------------------------------------------------------
//	LocalStorage.cpp: A local storage implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Resource.h"

namespace flu
{
namespace res
{
	LocalStorage::LocalStorage( String packagesPath, String cachePath, Bool useCache )
		:	m_listener( nullptr )
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

	void LocalStorage::setListener( IListener* listener )
	{
		m_listener = listener;
	}

	CompiledResource LocalStorage::requestCompiled( EResourceType type, String resourceName )
	{
		return requestCompiledImpl( type, resourceName, true );
	}

	CompiledResource LocalStorage::requestCompiled( ResourceId resourceId )
	{
		return requestCompiledImpl( resourceId, true );
	}

	CompiledResource LocalStorage::requestCompiledImpl( ResourceId resourceId, Bool allowCached )
	{
		// try to resolve ResourceId
		String resourceName = m_namesResolver.getName( resourceId );

		if( resourceName )
		{
			return requestCompiledImpl( resourceId.getType(), resourceName, allowCached );
		}
		else
		{
			fatal( TXT( "LocalStorage: unable to resolve ResourceId \"%s\"" ), *resourceId.toString() );
			return CompiledResource();
		}
	}

	CompiledResource LocalStorage::requestCompiledImpl( EResourceType type, String resourceName, Bool allowCached )
	{
		ResourceId resourceId( type, resourceName );	

		// attempt to find in cache
		if( m_useCache && allowCached )
		{
			CompiledResource cachedResource = loadFromCache( resourceId, resourceName );

			if( cachedResource.isValid() )
			{
				return cachedResource;
			}
		}

		// compilation need
		CompilationOutput output = compile( resourceId, resourceName );

		if( !output.hasError() )
		{
			if( m_useCache )
			{
				saveToCache( resourceId, resourceName, output );
			}

			return output.compiledResource;
		}
		else
		{
			return CompiledResource();
		}
	}

	String LocalStorage::resolveResourceId( ResourceId resourceId )
	{
		return m_namesResolver.getName( resourceId );
	}

	Array<ResourceId> LocalStorage::trackChanges()
	{
		return m_filesTracker->trackChanges();
	}

	void LocalStorage::update( ResourceSystemList& systemList )
	{
		Array<ResourceId> changedResources = trackChanges();

		for( auto& it : changedResources )
		{
			IResourceSystem* system = systemList[static_cast<SizeT>( it.getType() )].get();
			assert( system );

			if( system->allowHotReloading() )
			{
				String prettyName = m_namesResolver.getPrettyName( it );

				if( m_listener )
				{
					m_listener->onInfo( prettyName, TXT( "changed from the outside, reloading..." ) );
				}

				CompiledResource recompiled = requestCompiledImpl( it, false );

				if( recompiled.isValid() )
				{
					system->reloadResource( it, recompiled );
				}
				else
				{
					if( m_listener )
					{
						m_listener->onError( prettyName, TXT( "unable to reload changed resource" ) );
					}
				}
			}
		}
	}

	CompiledResource LocalStorage::loadFromCache( ResourceId resourceId, String resourceName )
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

					if( m_listener )
					{
						m_listener->onInfo( resourceName, TXT( "found in cache" ) );
					}

					return compiledResource;				
				}
				else
				{
					if( m_listener )
					{
						m_listener->onInfo( resourceName, TXT( "cache is out of date" ) );
					}
				}
			}
		}

		// not found in cache
		return CompiledResource();
	}

	void LocalStorage::saveToCache( ResourceId resourceId, String resourceName, const CompilationOutput& output )
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

			if( m_listener )
				m_listener->onInfo( resourceName, String::format( TXT( "Cache saved to \"%s\"" ), *cacheFileName ) );
		}
		else
		{
			if( m_listener )
				m_listener->onWarning( resourceName, 
					String::format( TXT( "Unable save resource cache to \"%s\"" ), *cacheFileName ) );
		}
	}

	CompilationOutput LocalStorage::compile( ResourceId resourceId, String resourceName )
	{
		UInt64 startupTime = time::cycles64();

		if( m_listener )
			m_listener->onInfo( resourceName, TXT( "compilation started..." ) );

		assert( resourceName );
		assert( getCompiler( resourceId.getType() ) );

		CompilationOutput output;

		EResourceType fileType;
		String resourceFileName = findResourceFile( resourceName, &fileType );
	
		if( !resourceFileName )
		{
			if( m_listener )
				m_listener->onError( resourceName, TXT( "is not found on disk" ) );

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
		String resourcePath = m_packagesPath + resourceRelativePath + TXT( "\\" );

		DependencyProvider dependencyProvider( resourcePath, resourceRelativePath, output );

		getCompiler( fileType )->compile( fm::getFileNameExt( *resourceFileName ), dependencyProvider, output );

		// output all warnings
		if( m_listener )
		{
			for( auto& it : output.warningsMsg )
			{
				m_listener->onWarning( resourceName, it );
			}		
		}

		if( !output.hasError() )
		{
			if( m_listener )
				m_listener->onInfo( resourceName, String::format( L"compiled successfully in %.4f sec", 
					time::elapsedSecFrom( startupTime ) ) );

			m_namesResolver.addName( resourceId, resourceName );

			// remember all references
			for( const auto& it : output.references )
			{
				m_namesResolver.addName( it.key, it.value );
			}

			// untrack old dependencies and track new
			m_filesTracker->removeResourceFiles( resourceId );
			m_filesTracker->addResourceFile( resourceId, output.dependencyFiles );
		}
		else
		{
			// compilation failed
			if( m_listener )
				m_listener->onError( resourceName, output.errorMsg );
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
			if( wildcard[i] == TXT( '.' ) )
			{
				wildcard[i] = TXT( '\\' );
			}
		}

		wildcard += TXT( ".*" );

		String directory = m_packagesPath + fm::getFilePath( *wildcard );
		String fileMask = fm::getFileName( *wildcard ) + TXT( "." ) + fm::getFileExt( *wildcard );

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
		return TXT( "" );
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

	String LocalStorage::prepareResourceFolder( String resourceName )
	{
		assert( resourceName );

		// check for package name at least
		if( String::pos( TXT("."), resourceName ) == -1 )
		{
			if( m_listener )
			{
				m_listener->onError( resourceName, TXT("missing package name") );
			}

			return TXT("");
		}

		// step by step create subfolders
		String result;
		Int32 folderNamePos = 0;
		Int32 dotPos = String::pos( TXT("."), resourceName );

		while( dotPos != -1 )
		{
			String folderName = String::copy( resourceName, folderNamePos, dotPos - folderNamePos );
			assert( folderName );

			result += folderName;
			String absolutePath = m_packagesPath + result;

			if( !fm::directoryExists( *absolutePath ) )
			{
				if( !fm::createDirectory( *absolutePath ) )
				{
					if( m_listener )
					{
						m_listener->onError( resourceName, 
							String::format( TXT("Unable to create directory \"%s\""), *absolutePath ) );
					}

					return TXT("");
				}
			}

			folderNamePos = dotPos + 1;
			dotPos = String::pos( TXT("."), resourceName, folderNamePos );
			result += TXT("\\");
		}

		return result;
	}
}
}