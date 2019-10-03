//-----------------------------------------------------------------------------
//	System.cpp: A FFX system implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "FFX.h"

namespace flu
{
namespace ffx
{

	rend::VertexDeclaration* s_vertexDeclaration = nullptr;


	System::System( rend::Device* device )
		:	m_effects(),
			m_device( device )
	{
		assert( m_device );
	}

	System::~System()
	{
		assert( m_effects.isEmpty() );
	}

	res::Resource* System::createResource( String resourceName, res::ResourceId resourceId,
		const res::CompiledResource& compiledResource )
	{
		assert( !m_effects.hasKey( resourceId ) );
		assert( compiledResource.isValid() );

		Effect* effect = new Effect( resourceName, *s_vertexDeclaration, m_device );
		effect->reload( compiledResource );

		effect->deleter( this, []( void* context, ReferenceCount* refCounter )->void
		{
			System* system = reinterpret_cast<System*>( context );
			Effect* effect = dynamic_cast<Effect*>( refCounter );
			assert( system && effect );

			system->destroyEffect( effect );
			delete effect;
		} );

		m_effects.put( resourceId, effect );

		return effect;
	}

	Bool System::allowHotReloading() const
	{
		return true;
	}

	void System::reloadResource( res::ResourceId resourceId, const res::CompiledResource& compiledResource )
	{
		assert( compiledResource.isValid() );

		Effect*& effect = m_effects.getRef( resourceId );
		effect->reload( compiledResource );
	}

	Bool System::hasResource( res::ResourceId resourceId ) const
	{
		return m_effects.hasKey( resourceId );
	}

	res::Resource* System::getResource( res::ResourceId resourceId ) const
	{
		Effect* const* effect = m_effects.get( resourceId );
		return effect ? *effect : nullptr;
	}

	void System::destroyEffect( Effect* effect )
	{
		assert( effect );

		res::ResourceId resourceId( res::EResourceType::Effect, effect->getName() );

		Bool removed = m_effects.remove( resourceId );
		assert( removed );
	}
}
}