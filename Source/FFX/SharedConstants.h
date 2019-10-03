//-----------------------------------------------------------------------------
//	SharedConstants.h: A class which control shared constants across shaders
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ffx
{
	/**
	 *	A shaders shared constants
	 */
	class SharedConstants: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<SharedConstants>;

		SharedConstants( UInt32 perFrameBufferSize, UInt32 perViewBufferSize, rend::Device* device );
		~SharedConstants();

		void bindToPipeline();

		void updatePerFrameBuffer( const void* data );
		void updatePerViewBuffer( const void* data );

	private:
		struct Buffer
		{
			rend::ConstantBufferHandle handle;
			UInt32 size;
		};

		Buffer m_perFrameCB;
		Buffer m_perViewCB;

		rend::Device* m_device;

		SharedConstants() = delete;
	};
}
}