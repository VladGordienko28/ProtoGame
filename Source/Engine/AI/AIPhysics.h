//-----------------------------------------------------------------------------
//	AIPhysics.h: AI physics utils
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ai
{
	/**
	 *	Computes a maximum jump height using an initial speed and
	 *	gravity
	 */
	inline Float suggestJumpHeight( Float jumpSpeed, Float gravity )
	{
		const Float time = jumpSpeed / gravity;
		return jumpSpeed * time - 0.5f * time * time * gravity;
	}

	/**
	 *	Computes a jump's vertical speed to reach desiredHeight
	 */
	inline Float suggestJumpSpeed( Float desiredHeight, Float gravity )
	{
		return math::sqrt( 2.f * gravity * desiredHeight );
	}

	/**
	 *	Computes a vertical speed for jumping from the 'from' point to
	 *	the 'to' point.
	 */
	inline Float verticalSpeedForJumping( const math::Vector& from, const math::Vector& to, Float xSpeed, Float gravity )
	{
		const Float xTime = abs( to.x - from.x ) / xSpeed;
		const Float ySpeed = ( to.y + 0.5f * xTime * xTime * gravity - from.y ) / xTime;

		return max( ySpeed, 0.f );
	}

	/**
	 *	Return true, if jumping from 'from' point to 'to' point with givin xSpeed, gravity
	 *	maxJumpHeight is possible. Optionally returns suggested jump speed
	 */
	inline Bool canMakeJumpTo( const math::Vector& from, const math::Vector& to, Float xSpeed, 
		Float maxJumpHeight, Float gravity, Float* suggestedSpeed )
	{
		Float requiredSpeed = verticalSpeedForJumping( from, to, xSpeed, gravity );

		if( requiredSpeed < suggestJumpSpeed( maxJumpHeight, gravity ) )
		{
			if( suggestedSpeed )
				*suggestedSpeed = requiredSpeed;
			return true;
		}
		else
		{
			return false;
		}
	}
}
}