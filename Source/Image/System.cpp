//-----------------------------------------------------------------------------
//	System.cpp: A Image system
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Image.h"

namespace flu
{
namespace img
{
	System::System( rend::Device* device )
		:	m_device( device ),
			m_images()
	{
		assert( m_device );
	}

	System::~System()
	{
		assert( m_images.isEmpty() );
	}

	res::Resource* System::createResource( String resourceName, res::ResourceId resourceId,
		const res::CompiledResource& compiledResource )
	{
		assert( !m_images.hasKey( resourceId ) );
		assert( compiledResource.isValid() );

		Image* image = new Image( resourceName );
		image->create( m_device, compiledResource );
		
		image->deleter( this, []( void* context, ReferenceCount* refCounter )->void
		{
			System* system = reinterpret_cast<System*>( context );
			Image* image = dynamic_cast<Image*>( refCounter );
			assert( system && image );

			system->destroyImage( image );
			delete image;
		} );

		m_images.put( resourceId, image );

		return image;
	}

	void System::reloadResource( res::ResourceId resourceId, const res::CompiledResource& compiledResource )
	{
		assert( compiledResource.isValid() );

		Image*& image = m_images.getRef( resourceId );
		image->destroy( m_device );
		image->create( m_device, compiledResource );
	}

	Bool System::allowHotReloading() const
	{
		return true;
	}

	Bool System::hasResource( res::ResourceId resourceId ) const
	{
		return m_images.hasKey( resourceId );
	}

	res::Resource* System::getResource( res::ResourceId resourceId ) const
	{
		Image* const* image = m_images.get( resourceId );
		return image ? *image : nullptr;
	}

	void System::destroyImage( Image* image )
	{
		assert( image );

		res::ResourceId resourceId( res::EResourceType::Image, image->getName() );

		Bool removed = m_images.remove( resourceId );
		assert( removed );

		image->destroy( m_device );
	}
}
}

// todo: make it safe and avoid some strange stuff
void* lodepng_malloc( size_t size )
{
	if( size > 0 )
	{
		return flu::mem::malloc( size );
	}
	else
	{
		return nullptr;
	}
}

void* lodepng_realloc( void* ptr, size_t new_size )
{
	if( ptr )
	{
		return flu::mem::realloc( ptr, new_size );	
	}
	else
	{
		return flu::mem::malloc( new_size );
	}
}

void lodepng_free( void* ptr )
{
	if( ptr )
	{
		flu::mem::free( ptr );	
	}
}