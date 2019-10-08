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

		GridDrawer( math::Color color, UInt32 size );
		~GridDrawer();

		void render( const ViewInfo& viewInfo );

	private:
		Float				m_gridSize;
		math::FloatColor	m_color;
		ffx::Effect::Ptr	m_effect;

		GridDrawer() = delete;
	};
}
}