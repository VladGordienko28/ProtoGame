//-----------------------------------------------------------------------------
//	ViewInfo.h: Viewport stuff
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace gfx
{
	/**
	 *	An information about view
	 */
	struct ViewInfo
	{
	public:
		// world info
		math::Coords coords;
		math::Coords unCoords;
		math::Rect bounds;
		math::Vector fov;
		Float zoom;

		// screen info
		Float x;
		Float y;
		Float width;
		Float height;

		// other
		Bool isMirage;

		/**
		 *	Default constructor
		 */
		ViewInfo();

		/**
		 *	Screen or part of it constructor
		 */
		ViewInfo( Float inX, Float inY, Float inWidth, Float inHeight );

		/**
		 *	World or part of it constructor
		 */
		ViewInfo( const math::Vector& inLocation, math::Angle inRotation, const math::Vector& inFov, Float inZoom, 
			Bool inMirage, Float inX, Float inY, Float inWidth, Float inHeight );

		/**
		 *	Project a point from the world's coords to the screen
		 *	coords system.
		 */
		void project( const math::Vector& point, Float& outX, Float& outY ) const;

		/**
		 *	Project a point from the screen's coords to the world
		 *	coords system.
		 */
		math::Vector deproject( Float pixelX, Float pixelY ) const;

		math::Vector deproject( const math::Vector pixelPos ) const
		{
			return deproject( pixelPos.x, pixelPos.y );
		}

		math::Vector location() const
		{
			return coords.origin;
		}

		math::Vector upVector() const
		{
			return coords.yAxis;
		}

		math::Vector rightVector() const
		{
			return coords.xAxis;
		}
	};
}
}