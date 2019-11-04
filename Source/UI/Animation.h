//-----------------------------------------------------------------------------
//	Animation.h: An UI simple animation (action)
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
	/**
	 *	An animation key
	 */
	struct AnimKey
	{
	public:
		Position position;
		Size size;
		Float opacity;
	};

	/**
	 *	An UI animation
	 */
	class Animation final
	{
	public:
		Animation()
			:	m_delay( 0.f ),
				m_duration( 0.f ),
				m_flags( ANIM_None )
		{
		}

		~Animation()
		{
		}

		void setDelay( Float delay )
		{
			m_delay = max( 0.f, delay );
		}

		Float getDelay() const
		{
			return m_delay;
		}

		void setDuration( Float duration )
		{
			m_duration = max( 0.f, duration );
		}

		Float getDuration() const
		{
			return m_duration;
		}

		void setXMovement( Float xOffset )
		{
			m_flags |= ANIM_Move;
			m_xOffset = xOffset;
		}

		void setYMovement( Float yOffset )
		{
			m_flags |= ANIM_Move;
			m_yOffset = yOffset;
		}

		void setWidthScaling( Float widthScale )
		{
			m_flags |= ANIM_Scale;
			m_widthScale = widthScale;
		}

		void setHeightScaling( Float heightScale )
		{
			m_flags |= ANIM_Scale;
			m_heightScale = heightScale;
		}

		void setOpacityMult( Float opacityMult )
		{
			m_flags |= ANIM_Opacity;
			m_opacityMult = opacityMult;
		}

		void getTargetKey( const AnimKey& refKey, AnimKey& outKey ) const
		{
			outKey = refKey;

			if( m_flags & ANIM_Move )
			{
				outKey.position.x = refKey.position.x + m_xOffset;
				outKey.position.y = refKey.position.y + m_yOffset;
			}

			if( m_flags & ANIM_Scale )
			{
				outKey.size.width = refKey.size.width * m_widthScale;
				outKey.size.height = refKey.size.height * m_heightScale;

				outKey.position.x -= 0.5f * ( outKey.size.width - refKey.size.width );
				outKey.position.y -= 0.5f * ( outKey.size.height - refKey.size.height );
			}

			if( m_flags & ANIM_Opacity )
			{
				outKey.opacity = refKey.opacity * m_opacityMult;
			}
		}

	private:
		enum EFlags: UInt32
		{
			ANIM_None = 0,
			ANIM_Move = 1,
			ANIM_Scale = 2,
			ANIM_Opacity = 4
		};

		Float m_delay;
		Float m_duration;

		UInt32 m_flags;

		Float m_xOffset = 0.f;
		Float m_yOffset = 0.f;
		Float m_widthScale = 1.f;
		Float m_heightScale = 1.f;
		Float m_opacityMult = 1.f;
	};
}
}