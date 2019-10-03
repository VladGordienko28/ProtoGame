//-----------------------------------------------------------------------------
//	SharedConstants.h: A class which control shared constants across shaders
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "FFX.h"

namespace flu
{
namespace ffx
{
	SharedConstants::SharedConstants( UInt32 perFrameBufferSize, UInt32 perViewBufferSize, rend::Device* device )
		:	m_device( device )
	{
		assert( device );
		assert( perFrameBufferSize > 0 && perViewBufferSize > 0 );

		m_perFrameCB.handle = device->createConstantBuffer( perFrameBufferSize, rend::EUsage::Dynamic, nullptr, "PerFrame constants" );
		m_perFrameCB.size = perFrameBufferSize;

		m_perViewCB.handle = device->createConstantBuffer( perViewBufferSize, rend::EUsage::Dynamic, nullptr, "PerView constants" );
		m_perViewCB.size = perViewBufferSize;
	}

	SharedConstants::~SharedConstants()
	{
		m_device->destroyConstantBuffer( m_perFrameCB.handle );
		m_perFrameCB.handle = INVALID_HANDLE<rend::ConstantBufferHandle>();

		m_device->destroyConstantBuffer( m_perViewCB.handle );
		m_perViewCB.handle = INVALID_HANDLE<rend::ConstantBufferHandle>();
	}

	void SharedConstants::bindToPipeline()
	{
		m_device->setConstantBuffers( rend::EShaderType::Vertex, EConstantBufferType::CBT_PerFrame, 1, &m_perFrameCB.handle );
		m_device->setConstantBuffers( rend::EShaderType::Pixel, EConstantBufferType::CBT_PerFrame, 1, &m_perFrameCB.handle );

		m_device->setConstantBuffers( rend::EShaderType::Vertex, EConstantBufferType::CBT_PerView, 1, &m_perViewCB.handle );
		m_device->setConstantBuffers( rend::EShaderType::Pixel, EConstantBufferType::CBT_PerView, 1, &m_perViewCB.handle );
	}

	void SharedConstants::updatePerFrameBuffer( const void* data )
	{
		assert( data && m_perFrameCB.handle != INVALID_HANDLE<rend::ConstantBufferHandle>() );
		m_device->updateConstantBuffer( m_perFrameCB.handle, data, m_perFrameCB.size );
	}

	void SharedConstants::updatePerViewBuffer( const void* data )
	{
		assert( data && m_perViewCB.handle != INVALID_HANDLE<rend::ConstantBufferHandle>() );
		m_device->updateConstantBuffer( m_perViewCB.handle, data, m_perViewCB.size );
	}
}
}