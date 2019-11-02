//-----------------------------------------------------------------------------
//	System.cpp: A UI layout system implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "UI.h"

namespace flu
{
namespace ui
{
	System::System()
		:	m_layouts()
	{
	}

	System::~System()
	{
		assert( m_layouts.isEmpty() );
	}

	res::Resource* System::createResource( String resourceName, res::ResourceId resourceId,
		const res::CompiledResource& compiledResource )
	{
		assert( !m_layouts.hasKey( resourceId ) );
		assert( compiledResource.isValid() );

		Layout* layout = new Layout( resourceName );
		layout->create( compiledResource );
		
		layout->deleter( this, []( void* context, ReferenceCount* refCounter )->void
		{
			System* system = reinterpret_cast<System*>( context );
			Layout* layout = dynamic_cast<Layout*>( refCounter );
			assert( system && layout );

			system->destroyLayout( layout );
			delete layout;
		} );

		m_layouts.put( resourceId, layout );

		return layout;
	}

	Bool System::allowHotReloading() const
	{
		return true;
	}

	void System::reloadResource( res::ResourceId resourceId, const res::CompiledResource& compiledResource )
	{
		assert( compiledResource.isValid() );

		Layout*& layout = m_layouts.getRef( resourceId );
		layout->destroy();
		layout->create( compiledResource );
	}

	Bool System::hasResource( res::ResourceId resourceId ) const
	{
		return m_layouts.hasKey( resourceId );
	}

	res::Resource* System::getResource( res::ResourceId resourceId ) const
	{
		Layout* const* layout = m_layouts.get( resourceId );
		return layout ? *layout : nullptr;
	}

	void System::destroyLayout( Layout* layout )
	{
		assert( layout );

		res::ResourceId resourceId( res::EResourceType::Layout, layout->getName() );

		Bool removed = m_layouts.remove( resourceId );
		assert( removed );

		layout->destroy();
	}
}
}