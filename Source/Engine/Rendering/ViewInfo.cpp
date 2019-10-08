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

	void ViewInfo::viewProjectionMatrix( UInt32 clientWidth, UInt32 clientHeight, Float* matrix ) const
	{
		const Float xFov2 = 2.f / ( fov.x * zoom );
		const Float yFov2 = 2.f / ( fov.y * zoom );

		const math::Vector sScale = 
		{
			width / clientWidth,
			height / clientHeight
		};

		const math::Vector sOffset =
		{
			( 2.f / clientWidth ) * ( x + ( width / 2.f ) ) - 1.f,
			1.f - ( 2.f / clientHeight ) * ( y + ( height / 2.f ) )
		};

		matrix[0 * 4 + 0] = xFov2 * +coords.xAxis.x * sScale.x;
		matrix[1 * 4 + 0] = yFov2 * -coords.xAxis.y * sScale.y;
		matrix[2 * 4 + 0] = 0.f;
		matrix[3 * 4 + 0] = 0.f;

		matrix[0 * 4 + 1] = xFov2 * -coords.yAxis.x * sScale.x;
		matrix[1 * 4 + 1] = yFov2 * +coords.yAxis.y * sScale.y;
		matrix[2 * 4 + 1] = 0.f;
		matrix[3 * 4 + 1] = 0.f;

		matrix[0 * 4 + 2] = 0.f;
		matrix[1 * 4 + 2] = 0.f;
		matrix[2 * 4 + 2] = 1.f;
		matrix[3 * 4 + 2] = 0.f;

		matrix[0 * 4 + 3] = -( coords.origin.x * matrix[0 * 4 + 0] + coords.origin.y * matrix[0 * 4 + 1] ) + sOffset.x;
		matrix[1 * 4 + 3] = -( coords.origin.x * matrix[1 * 4 + 0] + coords.origin.y * matrix[1 * 4 + 1] ) + sOffset.y;
		matrix[2 * 4 + 3] = 1.f;
		matrix[3 * 4 + 3] = 1.f;
	}
}
}