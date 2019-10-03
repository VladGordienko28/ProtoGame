//-----------------------------------------------------------------------------
//	System.cpp: A Font system implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Font.h"

namespace flu
{
namespace fnt
{
	System::System()
		:	m_fonts()
	{
	}

	System::~System()
	{
		assert( m_fonts.isEmpty() );
	}

	res::Resource* System::createResource( String resourceName, res::ResourceId resourceId,
		const res::CompiledResource& compiledResource )
	{
		assert( !m_fonts.hasKey( resourceId ) );
		assert( compiledResource.isValid() );

		Font* font = new Font( resourceName );
		font->create( compiledResource );
		
		font->deleter( this, []( void* context, ReferenceCount* refCounter )->void
		{
			System* system = reinterpret_cast<System*>( context );
			Font* font = dynamic_cast<Font*>( refCounter );
			assert( system && font );

			system->destroyFont( font );
			delete font;
		} );

		m_fonts.put( resourceId, font );

		return font;
	}

	Bool System::allowHotReloading() const
	{
		return true;
	}

	void System::reloadResource( res::ResourceId resourceId, const res::CompiledResource& compiledResource )
	{
		assert( compiledResource.isValid() );

		Font*& font = m_fonts.getRef( resourceId );
		font->destroy();
		font->create( compiledResource );
	}

	Bool System::hasResource( res::ResourceId resourceId ) const
	{
		return m_fonts.hasKey( resourceId );
	}

	res::Resource* System::getResource( res::ResourceId resourceId ) const
	{
		Font* const* font = m_fonts.get( resourceId );
		return font ? *font : nullptr;
	}

	void System::destroyFont( Font* font )
	{
		assert( font );

		const res::ResourceId resourceId( res::EResourceType::Font, font->getName() );

		const Bool removed = m_fonts.remove( resourceId );
		assert( removed );

		font->destroy();
	}
}
}