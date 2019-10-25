//-----------------------------------------------------------------------------
//	SoundSystem.cpp: A sound system implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Audio.h"

namespace flu
{
namespace aud
{
	SoundSystem::SoundSystem( aud::Device* device )
		:	m_device( device ),
			m_sounds()
	{
		assert( m_device );
	}

	SoundSystem::~SoundSystem()
	{
		assert( m_sounds.isEmpty() );
	}

	res::Resource* SoundSystem::createResource( String resourceName, res::ResourceId resourceId,
		const res::CompiledResource& compiledResource )
	{
		assert( !m_sounds.hasKey( resourceId ) );
		assert( compiledResource.isValid() );

		Sound* sound = new Sound( resourceName );
		sound->create( m_device, compiledResource );
		
		sound->deleter( this, []( void* context, ReferenceCount* refCounter )->void
		{
			SoundSystem* system = reinterpret_cast<SoundSystem*>( context );
			Sound* sound = dynamic_cast<Sound*>( refCounter );
			assert( system && sound );

			system->destroySound( sound );
			delete sound;
		} );

		m_sounds.put( resourceId, sound );

		return sound;
	}

	void SoundSystem::reloadResource( res::ResourceId resourceId, const res::CompiledResource& compiledResource )
	{
		assert( compiledResource.isValid() );

		Sound*& sound = m_sounds.getRef( resourceId );
		sound->destroy( m_device );
		sound->create( m_device, compiledResource );
	}

	Bool SoundSystem::allowHotReloading() const
	{
		return true;
	}

	Bool SoundSystem::hasResource( res::ResourceId resourceId ) const
	{
		return m_sounds.hasKey( resourceId );
	}

	res::Resource* SoundSystem::getResource( res::ResourceId resourceId ) const
	{
		Sound* const* sound = m_sounds.get( resourceId );
		return sound ? *sound : nullptr;
	}

	void SoundSystem::destroySound( Sound* sound )
	{
		assert( sound );

		res::ResourceId resourceId( res::EResourceType::Sound, sound->getName() );

		Bool removed = m_sounds.remove( resourceId );
		assert( removed );

		sound->destroy( m_device );
	}
}
}