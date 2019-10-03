//-----------------------------------------------------------------------------
//	GridDrawer.h: Editor grid drawer
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace gfx
{
	/**
	 *	An editor grid drawer
	 */
	class GridDrawer: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<GridDrawer>;

		GridDrawer();
		~GridDrawer();

		void create( math::Color color, UInt32 size );
		void destroy();

		void render( const ViewInfo& viewInfo );

	private:
		Float				m_gridSize;
		math::FloatColor	m_color;
		ffx::Effect::Ptr	m_effect;
	};
}
}