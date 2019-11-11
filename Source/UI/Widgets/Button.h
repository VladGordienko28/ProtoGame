//-----------------------------------------------------------------------------
//	Button.h: A simple button
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
	/**
	 *	A button
	 */
	class Button: public Element
	{
	public:
		Button( String name, Root* root )
			:	Element( name, root )
		{
			m_tempFont = res::ResourceManager::get<fnt::Font>( L"Fonts.Verdana_14", res::EFailPolicy::FATAL );
			m_tempImage = res::ResourceManager::get<img::Image>( L"Experimental.UIBack", res::EFailPolicy::FATAL );
		}

		~Button()
		{
		}


		// experimental
		Bool eventMouseEnter() override
		{
			Element::eventMouseEnter();

			Animation anim;
			anim.setDuration( 0.1f );
			anim.setHeightScaling( 1.2f );
			anim.setWidthScaling( 1.2f );
			anim.setOpacityMult( 2.0f );

			m_animator.play( anim, EBlendType::ContinueInv );
			return false;
		}

		Bool eventMouseLeave() override
		{
			Element::eventMouseLeave();

			Animation anim;
			anim.setDuration( 0.1f );
			anim.setHeightScaling( 1.0f );
			anim.setWidthScaling( 1.0f );

			m_animator.play( anim, EBlendType::ContinueInv );
			return false;
		}


		Bool eventActivate() override
		{
			Element::eventMouseEnter();

			Animation anim;
			anim.setDuration( 0.1f );
			anim.setHeightScaling( 1.5f );
			anim.setWidthScaling( 1.5f );
			anim.setOpacityMult( 2.0f );
			anim.setYMovement( -30.f );

			m_animator.play( anim, EBlendType::ContinueInv );
			return false;
		}

		Bool eventDeactivate() override
		{
			Element::eventMouseLeave();

			Animation anim;
			anim.setDuration( 0.1f );
			anim.setHeightScaling( 1.0f );
			anim.setWidthScaling( 1.0f );

			m_animator.play( anim, EBlendType::ContinueInv );
			return false;
		}

		void draw( ICanvas& canvas ) override
		{
			math::Color drawColor = isFocused() ? math::colors::RED : math::colors::ORANGE;
			math::Color borderColor = math::colors::WHITE;
			drawColor.a = 255 * m_opacity;
			borderColor.a = 255 * m_opacity;

			canvas.drawImageRect( m_position, m_size, {0.f, 0.f}, {1.f, 1.f}, m_tempImage );
			//canvas.drawBorderRect( m_position, m_size, 2.f, drawColor, borderColor );

			const Float SCALE = 1.0f;

			canvas.drawTextLine( utils::getCenterTextPosition( m_position, m_size, m_tempFont, *m_name, m_name.len(), SCALE, SCALE ), 
				*m_name, m_name.len(), math::colors::WHITE, m_tempFont, SCALE, SCALE );

			//canvas.drawRect( m_position, m_size, math::colors::WHITE );
		}

	private:
		fnt::Font::Ptr m_tempFont;
		img::Image::Ptr m_tempImage;
	};
}
}