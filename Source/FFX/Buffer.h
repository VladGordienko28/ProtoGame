//-----------------------------------------------------------------------------
//	Buffer.h: A ffx constant buffer
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ffx
{
	/**
	 * A constant buffer
	 */
	class Buffer final: public NonCopyable
	{
	public:




		void writeData( const void* data, UInt32 offset, UInt32 size );
		void submit( rend::Device* device, UInt32 slot, Bool force = false );

	private:
		UInt32 m_size;
		Array<UInt8> m_cpuData;

		rend::ConstantBufferHandle m_gpuBuffer;
		UInt32 m_gpuChecksum;
	};
}
}