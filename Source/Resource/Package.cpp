//-----------------------------------------------------------------------------
//	Package.cpp: A resource package
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Resource.h"

namespace flu
{
namespace res
{
	Package::Package()
		:	m_name(),
			m_entries(),
			m_loader( nullptr )
	{
	}

	Package::~Package()
	{
		m_name = TXT( "" );
		m_entries.empty();
		m_loader = nullptr;
	}

	Bool Package::load( String fileName )
	{
		assert( !isLoaded() );

		if( !fm::fileExists( *fileName ) )
		{
			error( TXT( "Unable to load package: file \"%s\" is not found" ), *fileName );
			return false;
		}

		m_loader = fm::readBinaryFile( *fileName ).get();
		if( !m_loader.hasObject() )
		{
			error( TXT( "Unable to open package file \"%s\"" ), *fileName );
			return false;
		}

		// read and validate header
		PackageHeader header;
		*m_loader >> header;

		if( header.magic != PackageHeader::MAGIC )
		{
			error( TXT( "Unable to load package: file \"%s\" is not a package file" ), 
				*fileName );

			return false;
		}

		if( header.version != PackageHeader::VERSION )
		{
			error( TXT( "Unable to load package \"%s\", version mismatch" ), 
				*fileName );

			return false;
		}

		// load resources info
		m_name = header.name;
		assert( header.size > 0 );

		for( UInt32 i = 0; i < header.size; ++i )
		{
			ResourceId resourceId;	
			ResourceEntry entry;

			*m_loader >> resourceId;
			*m_loader >> entry.resourceName;
			*m_loader >> entry.checksum;
			*m_loader >> entry.dataOffset;

			Bool isNewEntry = m_entries.put( resourceId, entry );
			assert( isNewEntry );
		}

		info( TXT( "Package \"%s\" is loaded with %d resources" ), *m_name, m_entries.size() );
		return true;
	}

	Array<ResourceId> Package::getResourceList() const
	{
		assert( isLoaded() );
		return m_entries.keys();
	}

	CompiledResource Package::getResource( ResourceId resourceId )
	{
		assert( isLoaded() );

		ResourceEntry* entry = m_entries.get( resourceId );
		if( !entry )
		{
			// resource is not found in package
			return CompiledResource();
		}

		CompiledResource compiledResource;

		m_loader->seek( entry->dataOffset );
		*m_loader >> compiledResource;

		assert( compiledResource.isValid() );
		return compiledResource;
	}

	Bool Package::fillNamesResolver( NamesResolver& resolver ) const
	{
		assert( isLoaded() );

		for( const auto& it : m_entries )
		{
			resolver.addName( it.key, it.value.resourceName );
		}

		return false;
	}
}
}