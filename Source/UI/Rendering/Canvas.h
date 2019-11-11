//-----------------------------------------------------------------------------
//	Canvas.h: An UI layer canvas
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
namespace rendering
{
	/**
	 *	An UI layer canvas
	 */
	class Canvas: public ICanvas, public NonCopyable
	{
	public:
		Canvas();
		~Canvas();

		void drawRect( const Position& pos, const Size& size, math::Color color ) override;

		void drawPatternRect( const Position& pos, const Size& size, 
			math::Color color, EPattern pattern ) override;

		void drawBorderRect( const Position& pos, const Size& size, 
			Float borderThickness, math::Color color, math::Color borderColor ) override;

		void drawPatternBorderRect( const Position& pos, const Size& size, 
			Float borderThickness, math::Color color, math::Color borderColor, EPattern pattern ) override;

		void drawImageRect( const Position& pos, const Size& size,
			const math::Vector& tc0, const math::Vector& tc1, img::Image::Ptr image ) override;

		void drawImageRect( const Position& pos, const Size& size,
			const math::Vector& tc0, const math::Vector& tc1, rend::Texture2DHandle texture ) override;

		void drawImageRect( const Position& pos, const Size& size,
			const math::Vector& tc0, const math::Vector& tc1, rend::ShaderResourceView srv ) override;

		void drawTextLine( const Position& pos, const Char* text, Int32 len, math::Color color,
			fnt::Font::Ptr font, Float xScale = 1.f, Float yScale = 1.f ) override;

		void drawLine( const Position& from, const Position& to, math::Color color ) override;

		void pushClipArea( const Position& pos, const Size& size ) override;
		void popClipArea() override;

		void clearOps();
		FlatShadeOp* getFlatShadeOps( UInt32& count );
		TextOp* getTextOps( UInt32& count );

		void setOrigin( const Position& newOrigin )
		{
			m_origin = newOrigin;
		}

		Position getOrigin() const
		{
			return m_origin;
		}

	private:
		enum EFlatShadeOp
		{
			FSO_Solid = 0,
			FSO_Alpha,
			FSO_MAX
		};

		StaticArray<FlatShadeOp, FSO_MAX> m_flatShadeOps;
		Array<TextOp> m_textOps; // todo: replace with GrowOnlyArray

		TextOp& findOrCreateTextOp( rend::ShaderResourceView srv );

	private:
		Position m_origin;
	};
}
}
}