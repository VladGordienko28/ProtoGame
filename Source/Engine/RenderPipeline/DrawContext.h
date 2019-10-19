//-----------------------------------------------------------------------------
//	DrawContext.h:
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace gfx
{
	/**
	 *	A draw context
	 */
	class DrawContext: public NonCopyable
	{
	public:
		DrawContext( rend::Device* device, gfx::SharedConstants& sharedConstants );
		~DrawContext();

		void pushViewInfo( const ViewInfo& info );
		void popViewInfo();
		const ViewInfo& getViewInfo() const;

		void setScissorArea( const rend::ScissorArea& area );
		const rend::ScissorArea& getScissorArea() const;

		Int32 backbufferWidth() const
		{
			return m_device->getBackbufferWidth();
		}

		Int32 backbufferHeight() const
		{
			return m_device->getBackbufferHeight();
		}

	private:
		static const SizeT VIEWINFO_STACK_SIZE = 8;

		FixedStack<ViewInfo, VIEWINFO_STACK_SIZE> m_viewInfoStack;
		rend::ScissorArea m_scissorArea;

		rend::Device* m_device;
		gfx::SharedConstants& m_sharedConstants;

		DrawContext() = delete;

		void submitViewInfo( const ViewInfo& info );
	};
}
}