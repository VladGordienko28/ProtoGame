//-----------------------------------------------------------------------------
//	ViewInfo.cpp: Viewport stuff
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Engine/Engine.h"

namespace flu
{
namespace gfx
{
	ViewInfo::ViewInfo()
		:	coords(),
			unCoords(),
			bounds(),
			fov( 0.f, 0.f ), zoom( 1.f ),
			x( 0.f ), y( 0.f ),	
			width( 0.f ), height( 0.f ),
			isMirage( false )
	{
	}

	ViewInfo::ViewInfo( Float inX, Float inY, Float inWidth, Float inHeight )
		:	x( inX ), y( inY ),
			width( inWidth ), height( inHeight ),
			isMirage( false ),
			coords( { inWidth * 0.5f, inHeight * 0.5f }, { 1.f, 0.f }, { 0.f, 1.f } ),
			unCoords( coords.transpose() ),
			bounds( { 0.f, 0.f }, inWidth, inHeight ),
			fov( inWidth, -inHeight ),
			zoom( 1.f )
	{
	}

	ViewInfo::ViewInfo( const math::Vector& inLocation, math::Angle inRotation, const math::Vector& inFov, Float inZoom, 
		Bool inMirage, Float inX, Float inY, Float inWidth, Float inHeight )
		:	x( inX ), y( inY ),
			width( inWidth ), height( inHeight ),
			coords( inLocation, inRotation ),
			unCoords( coords.transpose() ),
			fov( inFov ),
			zoom( inZoom ),
			isMirage( inMirage )
	{
		if( inRotation )
		{
			bounds = math::Rect( coords.origin, zoom * math::sqrt( sqr( fov.x ) + sqr( fov.y ) ) );
		}
		else
		{
			bounds = math::Rect( coords.origin, fov.x * zoom, fov.y * zoom );
		}
	}

	void ViewInfo::project( const math::Vector& point, Float& outX, Float& outY ) const
	{
		const math::Vector pr = math::transformPointBy( point, coords );

		const Float xRatio = width / fov.x;
		const Float yRatio = height / fov.y;

		outX = width * 0.5f + pr.x * xRatio / zoom;
		outY = height * 0.5f - pr.y * yRatio / zoom;
	}

	math::Vector ViewInfo::deproject( Float pixelX, Float pixelY ) const
	{
		const Float xRatio = fov.x / width;
		const Float yRatio = fov.y / height;

		math::Vector ur;
		ur.x = ( pixelX - width * 0.5f ) * xRatio * zoom;
		ur.y = ( height * 0.5f - pixelY ) * yRatio * zoom;

		return math::transformPointBy( ur, unCoords );
	}
}
}