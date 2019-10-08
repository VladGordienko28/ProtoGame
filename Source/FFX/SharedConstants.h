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
		SharedConstants( rend::Device* device );
		~SharedConstants();

		void bindToPipeline();

	protected:
		void initPerFrameBuffer( UInt32 bufferSize );
		void initPerViewBuffer( UInt32 bufferSize );

		void updatePerFrameBuffer( const void* data );
		void updatePerViewBuffer( const void* data );

	private:
		struct Buffer
		{
			rend::ConstantBufferHandle handle = INVALID_HANDLE<rend::ConstantBufferHandle>();
			UInt32 size = 0;
		};

		Buffer m_perFrameCB;
		Buffer m_perViewCB;

		rend::Device* m_device;

		SharedConstants() = delete;
	};
}
}