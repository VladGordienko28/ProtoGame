//-----------------------------------------------------------------------------
//	Device.h: A XAudio2 audio device
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace xa2
{
	/**
	 *	A XAudio2 audio device
	 */
	class Device: public aud::Device
	{
	public:
		Device();
		~Device();
		Bool isInitialized() override;

		void tick( Float delta, const aud::Listener& listener ) override;

		aud::SoundHandle createSound( aud::ESoundFormat format, UInt32 frequency, UInt32 size, 
			const void* data, const Char* debugName ) override;

		void destroySound( aud::SoundHandle handle ) override;

		aud::MusicHandle createMusic( const void* data, SizeT size, const Char* debugName ) override;
		void destroyMusic( aud::MusicHandle handle ) override;

		void playSFX( aud::SoundHandle soundHandle, Float gain, Float pitch ) override;

		void setMasterVolume( Float volume ) override;
		Float getMasterVolume() const override;

		void setMusicVolume( Float volume ) override;
		Float getMusicVolume() const override;

		void setSFXVolume( Float volume ) override;
		Float getSFXVolume() const override;

	private:
		DxRef<IXAudio2> m_xAudio2;
		IXAudio2MasteringVoice* m_masteringVoice;

		Float m_masterVolume;
		Float m_musicVolume;
		Float m_sfxVolume;

	private:
		static const SizeT MAX_SOUNDS = 256;

		HandleArray<aud::SoundHandle, XA2Sound, MAX_SOUNDS> m_sounds;

	private:
		class SoundVoice final: public IXAudio2VoiceCallback
		{
		public:
			using UPtr = UniquePtr<SoundVoice>;

			SoundVoice() = default;
			~SoundVoice() = default;

			STDMETHOD_( void, OnVoiceProcessingPassStart )( UINT32 ) override;
			STDMETHOD_( void, OnVoiceProcessingPassEnd )() override;
			STDMETHOD_( void, OnStreamEnd )() override;
			STDMETHOD_( void, OnBufferStart )( void* ) override;
			STDMETHOD_( void, OnBufferEnd )( void* context ) override;
			STDMETHOD_( void, OnLoopEnd )( void* ) override;
			STDMETHOD_( void, OnVoiceError )( void*, HRESULT ) override;

			void init( aud::ESoundFormat inFormat, UInt32 inFrequency, IXAudio2SourceVoice* inVoice );
			void destroy();

			void play( aud::SoundHandle inHandle, const XA2Sound& inSound, Float gain, Float pitch );
			void stopAndFlush();
			void setVolume( Float volume );

			aud::ESoundFormat getFormat() const
			{
				return m_format;
			}

			UInt32 getFrequency() const
			{
				return m_frequency;
			}

			aud::SoundHandle getSound() const
			{
				return m_sound;
			}

			Bool isAvailable() const
			{
				return m_sound == INVALID_HANDLE<aud::SoundHandle>();
			}

		private:
			aud::ESoundFormat m_format;
			UInt32 m_frequency;

			aud::SoundHandle m_sound;
			Float m_gain;
			Float m_pitch;

			Float m_volume;
			IXAudio2SourceVoice* m_voice;
		};

		static const SizeT MAX_SOUND_VOICES = 16;
		StaticArray<SoundVoice::UPtr, MAX_SOUND_VOICES> m_soundVoices;

		Int32 findAvailableSoundVoice( aud::ESoundFormat format ) const;
	};
}
}