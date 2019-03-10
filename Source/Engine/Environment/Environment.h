//-----------------------------------------------------------------------------
//	Environment.h: Environment set
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace envi
{
	/**
	 *	This class represents the Environment
	 */
	class Environment final
	{
	public:
		Satellite m_sun;
		Satellite m_moon;

		Environment();
		~Environment();

		void render( CCanvas* canvas, const EnvironmentContext* context );
		void renderInfo( CCanvas* canvas );

		// legacy
		friend void Serialize( CSerializer& s, Environment& v )
		{
			Serialize( s, v.m_sun );
			Serialize( s, v.m_moon );
		}
	};

} // namespace envi
} // namespace flu