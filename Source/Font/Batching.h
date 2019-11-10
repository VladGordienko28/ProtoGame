//-----------------------------------------------------------------------------
//	Batching.h: A helping text batching functions
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace fnt
{
	/**
	 *	A text vertex to draw
	 */
	struct TextVertex
	{
	public:
		math::Vector	pos;
		math::Vector	tc;
		math::Color		color;
	};

	/**
	 *	Fill vertex buffer and index buffer with text data,
	 *	assumed it enoght space for data
	 */
	extern void batchLine( const Char* text, Int32 len, Font::Ptr font, math::Color color, 
		TextVertex* vb, UInt32& vbSize, UInt16* ib, UInt32& ibSize,
		const math::Vector& from, Float xScale = 1.f, Float yScale = 1.f );

	/**
	 *	Returns the height of the line of the text to draw
	 */
	extern Float getLineHeight( fnt::Font::Ptr font, Float yScale );

	/**
	 *	Returns the width of the line of the text to draw
	 */
	extern Float getLineWidth( const Char* text, Int32 len, Font::Ptr font, Float xScale );
	extern Float getLineWidth( const Char* text, Font::Ptr font, Float xScale );

	/**
	 *	Returns the number vertices required to draw text
	 */
	extern UInt32 getBatchVertexCount( Int32 textLength );

	/**
	 *	Returns the number indices required to draw text
	 */
	extern UInt32 getBatchIndexCount( Int32 textLength );
}
}