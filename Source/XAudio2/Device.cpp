//-----------------------------------------------------------------------------
//	Device.cpp: A XAudio2 audio device implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "XAudio2.h"

namespace flu
{
namespace xa2
{
	Device::Device()
		:	m_xAudio2( nullptr ),
			m_masteringVoice( nullptr ),
			m_masterVolume( 1.f ),
			m_musicVolume( 1.f ),
			m_sfxVolume( 1.f )
	{
		// create device
		HRESULT result = XAudio2Create( &m_xAudio2, 0 );

		if( !SUCCEEDED( result ) )
		{
			error( L"Unable to create XAudio2 device with error %d", result );
			return;
		}

		// enable debugging
#if FLU_DEBUG
		XAUDIO2_DEBUG_CONFIGURATION xAudioDebug;
		mem::zero( &xAudioDebug, sizeof( XAUDIO2_DEBUG_CONFIGURATION ) );

		xAudioDebug.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS | XAUDIO2_LOG_INFO;
		xAudioDebug.BreakMask = XAUDIO2_LOG_ERRORS;

		xAudioDebug.LogTiming = true;
		xAudioDebug.LogFunctionName = true;

		m_xAudio2->SetDebugConfiguration( &xAudioDebug );
#endif

		// create mastering voice
		result = m_xAudio2->CreateMasteringVoice( &m_masteringVoice );
		if( !SUCCEEDED( result ) )
		{
			error( L"Unable to create mastering voice for XAudio2 device with error %d", result );
			return;
		}

		// create pool of sound voices with various bits per channel
		for( Int32 i = 0; i < MAX_SOUND_VOICES; ++i )
		{
			m_soundVoices[i] = new SoundVoice();
			SoundVoice& soundVoice = *m_soundVoices[i];

			aud::ESoundFormat format = ( i % 2 ) == 0 ? aud::ESoundFormat::Mono8 : aud::ESoundFormat::Mono16;
			UInt32 frequency = 44100;

			WAVEFORMATEX waveFormat;
			mem::zero( &waveFormat, sizeof( WAVEFORMATEX ) );

			waveFormat.wFormatTag = WAVE_FORMAT_PCM;
			waveFormat.nChannels = 1;
			waveFormat.nSamplesPerSec = frequency; // will be changed later
			waveFormat.wBitsPerSample = format == aud::ESoundFormat::Mono8 ? 8 : 16;
			waveFormat.nBlockAlign = ( waveFormat.nChannels * waveFormat.wBitsPerSample ) / 8;
			waveFormat.nAvgBytesPerSec = ( waveFormat.nSamplesPerSec * waveFormat.nBlockAlign );
			waveFormat.cbSize = 0;

			IXAudio2SourceVoice* voice;

			result = m_xAudio2->CreateSourceVoice( &voice, &waveFormat, 0, 2.f, m_soundVoices[i].get() );

			if( FAILED(result) )
			{
				error( L"Unable to create SourceVoice with error %d", result );
				return;
			}

			soundVoice.init( format, frequency, voice );
		}

		info( L"XAudio2 device successfully created" );
	}

	Device::~Device()
	{
		m_xAudio2 = nullptr;
		info( L"XAudio2 device shutdown" );
	}

	Bool Device::isInitialized()
	{
		return m_xAudio2.hasObject();
	}

	void Device::tick( Float delta, const aud::Listener& listener )
	{
	}

	aud::SoundHandle Device::createSound( aud::ESoundFormat format, UInt32 frequency, UInt32 size, 
		const void* data, const Char* debugName )
	{
		aud::SoundHandle handle;

		m_sounds.emplaceElement( handle )->create( m_xAudio2.get(), format, frequency, size, 
			data, debugName );

		return handle;
	}

	void Device::destroySound( aud::SoundHandle handle )
	{
		// see if some voice is using this sound
		for( auto& it : m_soundVoices )
		{
			if( it->getSound() == handle )
			{
				it->stopAndFlush();
			}
		}

		XA2Sound& sound = m_sounds.get( handle );
		sound.destroy( m_xAudio2.get() );
		m_sounds.removeElement( handle );
	}

	aud::MusicHandle Device::createMusic( const void* data, SizeT size, const Char* debugName )
	{
		// not implemented yey
		return INVALID_HANDLE<aud::MusicHandle>();
	}

	void Device::destroyMusic( aud::MusicHandle handle )
	{
		// not implemented yet
	}

	void Device::playSFX( aud::SoundHandle soundHandle, Float gain, Float pitch )
	{
		XA2Sound& sound = m_sounds.get( soundHandle );
		Int32 voiceId = findAvailableSoundVoice( sound.m_format );

		if( voiceId != -1 )
		{
			SoundVoice::UPtr& voice = m_soundVoices[voiceId];

			voice->play( soundHandle, sound, gain, pitch );
		}
		else
		{
			warn( L"Unable to plat SFX, there is no avaiable voice" );
		}
	}

	void Device::setMasterVolume( Float volume )
	{
		m_masterVolume = clamp( volume, 0.f, 1.f );
		m_masteringVoice->SetVolume( m_masterVolume );
	}

	Float Device::getMasterVolume() const
	{
		return m_masterVolume;
	}

	void Device::setMusicVolume( Float volume )
	{
		// not implemented yet
	}

	Float Device::getMusicVolume() const
	{
		// not implemented yet
		return 1.f;
	}

	void Device::setSFXVolume( Float volume )
	{
		m_sfxVolume = clamp( volume, 0.f, 1.f );

		for( auto& it : m_soundVoices )
		{
			it->setVolume( m_sfxVolume );
		}
	}

	Float Device::getSFXVolume() const
	{
		return m_sfxVolume;
	}

	Int32 Device::findAvailableSoundVoice( aud::ESoundFormat format ) const
	{
		for( Int32 i = 0; i < MAX_SOUND_VOICES; ++i )
		{
			const SoundVoice::UPtr& soundVoice = m_soundVoices[i];

			if( soundVoice->getFormat() == format && soundVoice->isAvailable() )
			{
				return i;
			}
		}

		return -1;
	}


	void Device::SoundVoice::init( aud::ESoundFormat inFormat, UInt32 inFrequency, IXAudio2SourceVoice* inVoice )
	{
		assert( inFormat == aud::ESoundFormat::Mono8 || inFormat == aud::ESoundFormat::Mono16 );
		assert( inVoice );

		m_format = inFormat;
		m_frequency = inFrequency;
		m_voice = inVoice;
		m_volume = 1.f;
		m_sound = INVALID_HANDLE<aud::SoundHandle>();
	}

	void Device::SoundVoice::destroy()
	{
		if( m_sound != INVALID_HANDLE<aud::SoundHandle>() )
		{
			stopAndFlush();
		}
	}

	void Device::SoundVoice::stopAndFlush()
	{
		m_voice->Stop();
		m_voice->FlushSourceBuffers();
	}

	void Device::SoundVoice::setVolume( Float volume )
	{
		m_volume = volume;
		m_voice->SetVolume( m_volume * m_gain );
	}

	void Device::SoundVoice::play( aud::SoundHandle inHandle, const XA2Sound& inSound, Float gain, Float pitch )
	{
		assert( inHandle != INVALID_HANDLE<aud::SoundHandle>() );
		assert( m_sound == INVALID_HANDLE<aud::SoundHandle>() );
	
		// ensure frequency
		if( inSound.m_frequency != m_frequency )
		{
			m_frequency = inSound.m_frequency;
			m_voice->SetSourceSampleRate( m_frequency );
		}

		// update gain if required
		gain = clamp( gain, 0.f, 1.f );
		if( gain != m_gain )
		{
			m_gain = gain;
			setVolume( m_volume );
		}

		// update pitch if required
		pitch = clamp( pitch, 0.125f, 8.f );
		if( pitch != m_pitch )
		{
			m_pitch = pitch;
			m_voice->SetFrequencyRatio( pitch );
		}

		XAUDIO2_BUFFER xaBuffer;
		mem::zero( &xaBuffer, sizeof( XAUDIO2_BUFFER ) );
		xaBuffer.AudioBytes = inSound.m_waveData.size();
		xaBuffer.pAudioData = &inSound.m_waveData[0];
		xaBuffer.Flags = XAUDIO2_END_OF_STREAM;

		m_voice->SubmitSourceBuffer( &xaBuffer );
		if( SUCCEEDED( m_voice->Start() ) )
		{
			m_sound = inHandle;
		}
	}

	void Device::SoundVoice::OnVoiceProcessingPassStart( UINT32 BytesRequired )
	{
	}

	void Device::SoundVoice::OnVoiceProcessingPassEnd()
	{
	}

	void Device::SoundVoice::OnStreamEnd()
	{
	}

	void Device::SoundVoice::OnBufferStart( void* pBufferContext )
	{
	}

	void Device::SoundVoice::OnBufferEnd( void* pBufferContext )
	{
		// mark as available
		m_sound = INVALID_HANDLE<aud::SoundHandle>();
	}

	void Device::SoundVoice::OnLoopEnd( void* pBufferContext )
	{
	}

	void Device::SoundVoice::OnVoiceError( void* pBufferContext, HRESULT Error )
	{
	}
}
}