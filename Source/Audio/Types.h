//-----------------------------------------------------------------------------
//	Types.h: Basic audio types
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace aud
{
	/**
	 *	Audio resources handles
	 */
	using SoundHandle	= Handle<24, 8, 'ASND'>;
	using MusicHandle	= Handle<24, 8, 'AMUS'>;
	using SourceHandle	= Handle<24, 8, 'ASRC'>;

	/**
	 *	An audio listener
	 */
	struct Listener
	{
	public:
		math::Vector location;
		math::Angle rotation;
	};

	/**
	 *	A sound format
	 */
	enum class ESoundFormat
	{
		Unknown,
		Mono8,
		Mono16
	};
	ENUM_FOR_STREAM( ESoundFormat );
}
}