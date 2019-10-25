//-----------------------------------------------------------------------------
//	World.h: An engine world
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 *	A world
	 */
	class World: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<World>;

		World( rend::Device* renderDevice, aud::Device* audioDevice, in::Device* inputDevice );
		~World();

		//virtual void onUpdate();
		virtual void onResize( UInt32 newWidth, UInt32 newHeight, Bool fullScreen );

		// todo: merge into one function
		virtual void onBeginUpdate();
		virtual void onEndUpdate( CCanvas* canvas );


		// foooooooooooooooooooooooooooooooooooooooooooooo
		gfx::DrawContext& drawContext() { return m_drawContext; };

	protected:
		// rendering systems
		rend::Device* m_renderDevice;
		gfx::DrawContext m_drawContext;
		gfx::SharedConstants m_sharedConstants;

		// input system
		in::Device* m_inputDevice;

		// audio system
		aud::Device* m_audioDevice;

		EngineChart::UPtr m_engineChart;

		// timing

		World() = delete;

		virtual void updateMetrics();
	};
}