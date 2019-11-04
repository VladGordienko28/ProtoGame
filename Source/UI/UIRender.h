//-----------------------------------------------------------------------------
//	UIRender.h: An UI render
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
	/**
	 *	An UI render
	 */
	class Render: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<Render>;

		~Render();

		void prepareBatches( Container* treeRoot );
		void flushBatches();

	private:
		rend::Device* m_device;

		// todo: wrap to some kind of queue
			struct Vertex
			{
				math::Vector pos;
				math::Color color;
			};

			ffx::Effect::Ptr m_effect;
			rend::VertexBufferHandle m_vb;
			UInt32 m_vbSize;
			Array<Vertex> m_cpuBuffer;

			rend::BlendStateId m_blendStateId;


//////////////////////////////////////////////////////////
		Render() = delete;
		Render( rend::Device* device );

		friend class Root;
	};
}
}