//-----------------------------------------------------------------------------
//	Xa2Types.h: A XAudio2 types wrappers
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace xa2
{
	/**
	 *	A XAudio2 Sound
	 */
	struct XA2Sound
	{
	public:
		Array<UInt8> m_waveData;
		UInt32 m_frequency = 0;
		aud::ESoundFormat m_format = aud::ESoundFormat::Unknown;

		XA2Sound() = default;
		~XA2Sound();

		Bool create( IXAudio2* device, aud::ESoundFormat format, UInt32 frequency, UInt32 size, 
			const void* data, const Char* debugName );

		void destroy( IXAudio2* device );

		SizeT memoryUsage() const;
	};
}
}