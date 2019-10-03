//-----------------------------------------------------------------------------
//	Glyph.h: A font glyph definition
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace fnt
{
	/**
	 *	A single glyph in font
	 */
	struct Glyph
	{
	public:
		Float x;
		Float y;
		Float width;
		Float height;

		friend IOutputStream& operator<<( IOutputStream& stream, const Glyph& glyph )
		{
			stream << glyph.x << glyph.y << glyph.width << glyph.height;
			return stream;
		}

		friend IInputStream& operator>>( IInputStream& stream, Glyph& glyph )
		{
			stream >> glyph.x >> glyph.y >> glyph.width >> glyph.height;
			return stream;
		}
	};
}
}