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

		World( rend::Device* inRenderDevice );
		~World();

		virtual void onUpdate();
		virtual void onResize( UInt32 newWidth, UInt32 newHeight, Bool fullScreen );

		// foooooooooooooooooooooooooooooooooooooooooooooo
		gfx::DrawContext& drawContext() { return m_drawContext; };

	protected:
		// rendering systems
		rend::Device* m_renderDevice;
		gfx::DrawContext m_drawContext;

		// timing

		World() = delete;
	};
}