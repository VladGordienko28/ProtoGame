//-----------------------------------------------------------------------------
//	Stats.h: GPU statistics counter
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace rend
{
	/**
	 *	A GPU memory statistics
	 */
	struct MemoryStats
	{
	public:
		SizeT m_texureBytes = 0;
		SizeT m_vertexBufferBytes = 0;
		SizeT m_indexBufferBytes = 0;
		SizeT m_constantBufferBytes = 0;

		SizeT totalBytes() const
		{
			return m_texureBytes + m_vertexBufferBytes + m_indexBufferBytes +
				m_constantBufferBytes;
		}
	};

	/**
	 *	A GPU draws statistics
	 */
	struct DrawStats
	{
	public:
		Int32 m_drawCalls = 0;

		Int32 m_blendStateSwitches = 0;
		Int32 m_renderStateSwitches = 0;
	};
}
}