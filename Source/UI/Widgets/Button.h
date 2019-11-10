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
			math::Color borderColor = math::colors::BLACK;
			drawColor.a = 255 * m_opacity;
			borderColor.a = 255 * m_opacity;

			canvas.drawBorderRect( m_position, m_size, 2.f, drawColor, borderColor );
			//canvas.drawRect( m_position, m_size, math::colors::WHITE );
		}
	};
}
}