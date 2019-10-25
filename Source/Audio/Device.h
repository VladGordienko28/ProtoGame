//-----------------------------------------------------------------------------
//	Device.h: An abstract audio device
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace aud
{
	/**
	 *	An abstract audio device
	 */
	class Device
	{
	public:
		using UPtr = UniquePtr<Device>;

		Device() = default;
		virtual ~Device() = default;
		virtual Bool isInitialized() = 0;

		virtual void tick( Float delta, const Listener& listener ) = 0;

		virtual SoundHandle createSound( ESoundFormat format, UInt32 frequency, UInt32 size, 
			const void* data, const Char* debugName ) = 0;

		virtual void destroySound( SoundHandle handle ) = 0;

		virtual MusicHandle createMusic( const void* data, SizeT size, const Char* debugName ) = 0;
		virtual void destroyMusic( MusicHandle handle ) = 0;

		// todo: add spatial stuff here


		virtual void playSFX( SoundHandle soundHandle, Float gain, Float pitch ) = 0;

		virtual void setMasterVolume( Float volume ) = 0;
		virtual Float getMasterVolume() const = 0;

		virtual void setMusicVolume( Float volume ) = 0;
		virtual Float getMusicVolume() const = 0;

		virtual void setSFXVolume( Float volume ) = 0;
		virtual Float getSFXVolume() const = 0;

		// todo: add audio stats here
	};
}
}