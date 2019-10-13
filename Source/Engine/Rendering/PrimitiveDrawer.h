//-----------------------------------------------------------------------------
//	PrimitiveDrawer.h: A helper class, which draw some editor or debug stuff
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace gfx
{
	/**
	 *	An editor or debug primitives drawer
	 */
	class PrimitiveDrawer: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<PrimitiveDrawer>;

		PrimitiveDrawer();
		~PrimitiveDrawer();

		void batchPoint( const math::Vector& pos, Float z, Float size, math::Color color );
		void batchCoolPoint( const math::Vector& pos, Float z, Float size, math::Color color );

		void batchLine( const math::Vector& from, const math::Vector& to, Float z, math::Color color );
		void batchSmoothLine( const math::Vector& from, const math::Vector& to, Float z, math::Color color, UInt32 detail = 16 );
		void batchLineStrip( const math::Vector* list, UInt32 numVerts, Float z, math::Color color );

		void batchCircle( const math::Vector& pos, Float radius, Float z, math::Color color, UInt32 detail = 32 );
		void batchEllipse( const math::Vector& pos, Float xSize, Float ySize, Float z, math::Color color, UInt32 detail = 32 );

		void batchLineStar( const math::Vector& pos, math::Angle rot, Float size, Float z, math::Color color );
		void batchLineRect( const math::Vector& pos, Float xSize, Float ySize, math::Angle rot, Float z, math::Color color );

		void flush();

	private:
		struct Vertex
		{
			math::Vector3 pos;
			math::Color color;
		};

		GrowOnlyVB<Vertex, 512> m_vertexBuffer;
		ffx::Effect::Ptr m_effect;
	};
}
}