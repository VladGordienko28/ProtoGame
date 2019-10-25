//-----------------------------------------------------------------------------
//	Sound.cpp: A sound type implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Audio.h"

namespace flu
{
namespace aud
{
	Sound::Sound( String name )
		:	m_name( name ),
			m_handle( INVALID_HANDLE<SoundHandle>() ),
			m_frequency( 0 ),
			m_size( 0 ),
			m_format( ESoundFormat::Unknown )
	{
	}

	Sound::~Sound()
	{
	}

	Bool Sound::create( aud::Device* device, const res::CompiledResource& compiledResource )
	{
		assert( device );
		assert( compiledResource.isValid() );
		assert( m_handle == INVALID_HANDLE<SoundHandle>() );

		BufferReader reader( compiledResource.data );

		reader >> m_size;
		reader >> m_frequency;
		reader >> m_format;

		m_handle = device->createSound( m_format, m_frequency, m_size, 
			&compiledResource.data[reader.tell()], *m_name );

		return true;
	}

	void Sound::destroy( aud::Device* device )
	{
		assert( device );
		assert( m_handle != INVALID_HANDLE<SoundHandle>() );

		device->destroySound( m_handle );
		m_handle = INVALID_HANDLE<SoundHandle>();

		m_frequency = 0;
		m_size = 0;
		m_format = ESoundFormat::Unknown;
	}
}
}