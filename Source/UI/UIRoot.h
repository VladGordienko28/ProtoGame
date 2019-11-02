//-----------------------------------------------------------------------------
//	UIRoot.h: An UI system Root
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
	/**
	 *	An UI system root
	 */
	class Root final: public Container, public in::InputClient
	{
	public:
		using UPtr = UniquePtr<Root>;

		Root();
		~Root();

		void resize( Float width, Float height );


		void update( Float delta );
		void prepareBatches();
		void flushBatches( rend::Device* device );

		void setUIScale( Float newScale );
		Float getUIScale() const;

	private:
		Float m_uiScale;



		// not here!!!!!!!!!!!!!!!!!!!!
		ffx::Effect::Ptr m_flatShadeEffect;
		rend::VertexBufferHandle m_vb;

	};
}
}