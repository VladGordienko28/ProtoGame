//-----------------------------------------------------------------------------
//	SharedConstants.h: A class which control shared constants across shaders
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "FFX.h"

namespace flu
{
namespace ffx
{
	SharedConstants::SharedConstants( rend::Device* device )
		:	m_device( device )
	{
		assert( m_device != nullptr );
	}

	SharedConstants::~SharedConstants()
	{
		if( m_perFrameCB.handle != INVALID_HANDLE<rend::ConstantBufferHandle>() )
		{
			m_device->destroyConstantBuffer( m_perFrameCB.handle );
			m_perFrameCB.handle = INVALID_HANDLE<rend::ConstantBufferHandle>();
		}

		if( m_perViewCB.handle != INVALID_HANDLE<rend::ConstantBufferHandle>() )
		{
			m_device->destroyConstantBuffer( m_perViewCB.handle );
			m_perViewCB.handle = INVALID_HANDLE<rend::ConstantBufferHandle>();
		}
	}

	void SharedConstants::bindToPipeline()
	{
		assert( m_perFrameCB.handle != INVALID_HANDLE<rend::ConstantBufferHandle>() );
		assert( m_perViewCB.handle != INVALID_HANDLE<rend::ConstantBufferHandle>() );

		m_device->setConstantBuffers( rend::EShaderType::Vertex, EConstantBufferType::CBT_PerFrame, 1, &m_perFrameCB.handle );
		m_device->setConstantBuffers( rend::EShaderType::Pixel, EConstantBufferType::CBT_PerFrame, 1, &m_perFrameCB.handle );

		m_device->setConstantBuffers( rend::EShaderType::Vertex, EConstantBufferType::CBT_PerView, 1, &m_perViewCB.handle );
		m_device->setConstantBuffers( rend::EShaderType::Pixel, EConstantBufferType::CBT_PerView, 1, &m_perViewCB.handle );
	}

	void SharedConstants::initPerFrameBuffer( UInt32 bufferSize )
	{
		assert( bufferSize > 0 );
		assert( m_perFrameCB.handle == INVALID_HANDLE<rend::ConstantBufferHandle>() );

		m_perFrameCB.size = bufferSize;
		m_perFrameCB.handle = m_device->createConstantBuffer( bufferSize, 
			rend::EUsage::Dynamic, nullptr, "PerFrame Constants" );
	}

	void SharedConstants::initPerViewBuffer( UInt32 bufferSize )
	{
		assert( bufferSize > 0 );
		assert( m_perViewCB.handle == INVALID_HANDLE<rend::ConstantBufferHandle>() );

		m_perViewCB.size = bufferSize;
		m_perViewCB.handle = m_device->createConstantBuffer( bufferSize, 
			rend::EUsage::Dynamic, nullptr, "PerView Constants" );
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