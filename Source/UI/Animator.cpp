//-----------------------------------------------------------------------------
//	Animator.cpp: An UI element animator implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "UI.h"

namespace flu
{
namespace ui
{
	Animator::Animator( Element* element )
		:	m_element( element ),
			m_targetKey(),
			m_currentKey(),
			m_startKey(),
			m_finishCallback( nullptr ),
			m_delayRemainTime( 0.f ),
			m_animDuration( 0.f ),
			m_animRemainTime( 0.f ),
			m_playing( false )
	{
		assert( m_element );
	}

	Animator::~Animator()
	{
	}

	void Animator::update( Float delta )
	{
		if( !m_playing )
		{
			// nothing to play
			return;
		}

		// wait for delay
		m_delayRemainTime -= delta;
		
		if( m_delayRemainTime > 0.f )
		{
			return;
		}

		m_animRemainTime -= delta;
		
		if( m_animRemainTime > 0.f )
		{
			// do animation
			const Float alpha = 1.f - m_animRemainTime / m_animDuration;

			// interpolation
			m_currentKey.position.x = lerp( m_startKey.position.x, m_targetKey.position.x, alpha );
			m_currentKey.position.y = lerp( m_startKey.position.y, m_targetKey.position.y, alpha );

			m_currentKey.size.width = lerp( m_startKey.size.width, m_targetKey.size.width, alpha );
			m_currentKey.size.height = lerp( m_startKey.size.height, m_targetKey.size.height, alpha );

			m_currentKey.opacity = lerp( m_startKey.opacity, m_targetKey.opacity, alpha );
		}
		else
		{
			// end of animation
			m_playing = false;
			m_currentKey = m_targetKey;

			if( m_finishCallback )
			{
				m_finishCallback( m_element );
			}
		}

		// apply key
		m_element->m_position = m_currentKey.position;
		m_element->m_size = m_currentKey.size;
		m_element->m_opacity = m_currentKey.opacity;
	}

	void Animator::play( const Animation& animation, EBlendType blendType, Callback callback )
	{
		if( m_playing )
		{
			// blend required
			switch( blendType )
			{
				case EBlendType::Override:
					m_animDuration = animation.getDuration();
					m_startKey = m_currentKey;
					break;

				case EBlendType::Continue:
					m_animDuration = m_animRemainTime;
					m_startKey = m_currentKey;
					break;

				case EBlendType::ContinueInv:
					m_animDuration = m_animDuration - m_animRemainTime;
					m_startKey = m_currentKey;
					break;

				case EBlendType::Reset:
					m_animDuration = animation.getDuration();
					m_startKey = m_refKey;
					break;
			}
		}
		else
		{
			// no blending required
			m_animDuration = animation.getDuration();
			m_startKey = m_currentKey;
		}

		animation.getTargetKey( m_refKey, m_targetKey );
		m_animRemainTime = m_animDuration;
		m_finishCallback = callback;
		m_delayRemainTime = animation.getDelay();
		m_playing = true;
	}

	void Animator::setRefPose( const Position& position, const Size& size, Float opacity )
	{
		m_refKey.position = position;
		m_refKey.size = size;
		m_refKey.opacity = opacity;

		m_startKey = m_refKey;
		m_currentKey = m_refKey;
	}
}
}