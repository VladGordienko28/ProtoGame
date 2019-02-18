//-----------------------------------------------------------------------------
//	PhysicsUtils.h: Physics relative utils and functions
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace phys
{
	/**
	 *	Return whether surface is good for walking on
	 */
	inline Bool isWalkableSurface( const math::Vector& floorNormal )
	{
		return ( floorNormal.x <= +0.7f ) && ( floorNormal.x >= -0.7f ) && 
			( floorNormal.y > 0.f );
	}
}
}