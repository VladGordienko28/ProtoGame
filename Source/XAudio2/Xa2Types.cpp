//-----------------------------------------------------------------------------
//	Xa2Types.cpp: A XAudio2 types wrappers implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "XAudio2.h"

namespace flu
{
namespace xa2
{
	XA2Sound::~XA2Sound()
	{
		assert( m_waveData.size() == 0 );
	}

	Bool XA2Sound::create( IXAudio2* device, aud::ESoundFormat format, UInt32 frequency, UInt32 size, 
		const void* data, const Char* debugName )
	{
		assert( device );
		assert( format != aud::ESoundFormat::Unknown );
		assert( frequency > 0 );
		assert( size > 0 && data != nullptr );

		m_format = format;
		m_frequency = frequency;

		m_waveData.setSize( size );
		mem::copy( &m_waveData[0], data, size );

		return true;
	}

	void XA2Sound::destroy( IXAudio2* device )
	{
		m_waveData.empty();
	}

	SizeT XA2Sound::memoryUsage() const
	{
		return m_waveData.size();
	}
}
}