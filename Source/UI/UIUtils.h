//-----------------------------------------------------------------------------
//	UIUtils.h: An UI utils
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
namespace utils
{
	/**
	 *	Return centered text position inside an element
	 */
	inline Position getCenterTextPosition( const Position& position, const Size& size,
		fnt::Font::Ptr font, const Char* text, Int32 len, Float xScale = 1.f, Float yScale = 1.f )
	{
		Float xOffset = 0.5f * ( size.width - font->textWidth( text, len ) * xScale );
		Float yOffset = 0.5f * ( size.height - font->maxHeight() * yScale );

		return Position( position.x + xOffset, position.y + yOffset );
	}
}
}
}