//-----------------------------------------------------------------------------
//	ICanvas.h: An abstract UI canvas
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
	/**
	 *	An abstract canvas, which is used for UI elements
	 *	rendering
	 */
	class ICanvas
	{
	public:
		enum class EPattern
		{
			PolkaDot,
			Diagonal
		};

		virtual ~ICanvas()
		{
		}

		virtual void drawRect( const Position& pos, const Size& size, math::Color color ) = 0;

		virtual void drawPatternRect( const Position& pos, const Size& size, 
			math::Color color, EPattern pattern ) = 0;

		virtual void drawBorderRect( const Position& pos, const Size& size, 
			Float borderThickness, math::Color color, math::Color borderColor ) = 0;

		virtual void drawPatternBorderRect( const Position& pos, const Size& size, 
			Float borderThickness, math::Color color, math::Color borderColor, EPattern pattern ) = 0;

		virtual void drawImageRect( const Position& pos, const Size& size,
			const math::Vector& tc0, const math::Vector& tc1, img::Image::Ptr image ) = 0;

		virtual void drawImageRect( const Position& pos, const Size& size,
			const math::Vector& tc0, const math::Vector& tc1, rend::Texture2DHandle texture ) = 0;

		virtual void drawImageRect( const Position& pos, const Size& size,
			const math::Vector& tc0, const math::Vector& tc1, rend::ShaderResourceView srv ) = 0;

		virtual void drawTextLine( const Position& pos, const Char* text, Int32 len, math::Color color,
			fnt::Font::Ptr font, Float xScale = 1.f, Float yScale = 1.f ) = 0;

		virtual void drawLine( const Position& from, const Position& to, math::Color color ) = 0;

		virtual void pushClipArea( const Position& pos, const Size& size ) = 0;
		virtual void popClipArea() = 0;
	};
}
}