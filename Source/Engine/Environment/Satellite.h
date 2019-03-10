//-----------------------------------------------------------------------------
//	Satellite.h: An orbital object in the sky: sun, moon or whatever
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace envi
{
	/**
	 *	An orbital object in the sky
	 */
	class Satellite final
	{
	public:
		// sprite
		FBitmap* m_bitmap;
		math::Vector m_size;

		// orbit
		TimeOfDay m_zenithTime;
		math::Vector m_orbitCenter;
		Float m_orbitWidth;
		Float m_orbitHeight;

		Satellite();
		~Satellite();

		void render( CCanvas* canvas, TimeOfDay dayTime );
		void renderOrbit( CCanvas* canvas );

		// legacy
		friend void Serialize( CSerializer& s, Satellite& v );
	};

} // namespace envi
} // namespace flu