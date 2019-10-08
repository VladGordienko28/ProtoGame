//-----------------------------------------------------------------------------
//	TextDrawer.h: A helper class, which helps draw text :)
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace gfx
{
	/**
	 *	An UI text drawer
	 */
	class TextDrawer: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<TextDrawer>;

		TextDrawer();
		~TextDrawer();

		void batchText( const Char* text, Int32 len, fnt::Font::Ptr font, math::Color color, 
			const math::Vector& from, Float xScale = 1.f, Float yScale = 1.f );

		void batchText( String text, fnt::Font::Ptr font, math::Color color, 
			const math::Vector& from, Float xScale = 1.f, Float yScale = 1.f )
		{
			batchText( *text, text.len(), font, color, from, xScale, yScale );
		}

		Float textHeight( fnt::Font::Ptr font, Float yScale ) const;
		Float textWidth( const Char* text, Int32 len, fnt::Font::Ptr font, Float xScale ) const;

		Float textWidth( String text, fnt::Font::Ptr font, Float xScale ) const
		{
			return textWidth( *text, text.len(), font, xScale );
		}

		void flush();

	private:
		struct Vertex
		{
			math::Vector	pos;
			math::Vector	tc;
			math::Color		color;
		};

		GrowOnlyVB<Vertex, 1024> m_vertexBuffer;
		GrowOnlyIB<UInt16, 1024> m_indexBuffer;

		fnt::Font::Ptr m_currentFont;

		ffx::Effect::Ptr m_effect;

		rend::SamplerStateId m_samplerState;
		rend::BlendStateId m_blendState;
	};
}
}