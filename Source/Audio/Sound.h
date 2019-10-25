//-----------------------------------------------------------------------------
//	Sound.h: A sound type
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace aud
{
	/**
	 *	A sound
	 */
	class Sound: public res::Resource
	{
	public:
		DECLARE_RESOURCE( Sound, SoundSystem, SoundCompiler );

		~Sound();

		String getName() const { return m_name; }
		
		ESoundFormat getFormat() const { return m_format; }

		UInt32 getFrequency() const { return m_frequency; };
		UInt32 getSize() const { return m_size; };

		aud::SoundHandle getHandle() const { return m_handle; }

		// todo: add duration computation

	private:
		String m_name;

		aud::SoundHandle m_handle;

		UInt32 m_frequency;
		UInt32 m_size;
		ESoundFormat m_format;

		Sound() = delete;
		Sound( String name );

		Bool create( aud::Device* device, const res::CompiledResource& compiledResource );
		void destroy( aud::Device* device );

		friend class SoundSystem;
	};
}
}