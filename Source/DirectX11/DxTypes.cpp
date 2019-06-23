//-----------------------------------------------------------------------------
//	DxTypes.cpp: A DirectX 11 types implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "DirectX11.h"

namespace flu
{
namespace dx11
{
	DxTexture1D::~DxTexture1D()
	{
		assert( !m_texture.hasObject() );
		assert( !m_srv.hasObject() );
	}

	Bool DxTexture1D::create( ID3D11Device* device, rend::EFormat format, Int32 width, Int32 mips, 
		rend::EUsage usage, const void* initialData, const AnsiChar* debugName )
	{
		assert( device );
		assert( format != rend::EFormat::Unknown );
		assert( width > 0 );
		assert( isPowerOfTwo( width ) );
		assert( mips == 1 && "Mips chain is not supported yet" );

		D3D11_TEXTURE1D_DESC description;
		mem::zero( &description, sizeof( D3D11_TEXTURE2D_DESC ) );

		description.Width = m_width = width;
		description.MipLevels = m_levels = mips;
		description.ArraySize = 1;
		description.Format = fluorineFormatToDirectX( m_format = format );
		description.Usage = fluorineUsageToDirectX( m_usage = usage );
		description.BindFlags = D3D11_BIND_SHADER_RESOURCE; 
		description.CPUAccessFlags = usage == rend::EUsage::Dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
		description.MiscFlags = 0;

		HRESULT result;

		if( initialData )
		{
			auto formatInfo = rend::getFormatInfo( m_format );

			D3D11_SUBRESOURCE_DATA subresourceData;
			subresourceData.pSysMem = initialData;
			subresourceData.SysMemPitch = m_width * formatInfo.blockBytes / formatInfo.blockSizeX;
			subresourceData.SysMemSlicePitch = subresourceData.SysMemPitch / formatInfo.blockSizeY;

			result = device->CreateTexture1D( &description, &subresourceData, &m_texture );
			assert( SUCCEEDED( result ) );	
		}
		else
		{
			result = device->CreateTexture1D( &description, nullptr, &m_texture );
			assert( SUCCEEDED( result ) );
		}

		result = device->CreateShaderResourceView( m_texture.get(), nullptr, &m_srv );
		assert( SUCCEEDED( result ) );

		if( debugName )
		{
			m_texture->SetPrivateData( WKPDID_D3DDebugObjectName, UINT( cstr::length( debugName ) ), debugName );
			m_srv->SetPrivateData( WKPDID_D3DDebugObjectName, UINT( cstr::length( debugName ) ), debugName );
		}

		return true;
	}

	void DxTexture1D::destroy( ID3D11Device* device )
	{
		m_texture = nullptr;
		m_srv = nullptr;
	}

	DxTexture2D::~DxTexture2D()
	{
		assert( !m_texture.hasObject() );
		assert( !m_srv.hasObject() );
	}

	Bool DxTexture2D::create( ID3D11Device* device, rend::EFormat format, Int32 width, Int32 height, Int32 mips, 
		rend::EUsage usage, const void* initialData, const AnsiChar* debugName )
	{
		assert( device );
		assert( format != rend::EFormat::Unknown );
		assert( width > 0 && height > 0 );
		assert( isPowerOfTwo( width ) && isPowerOfTwo( height ) );
		assert( mips == 1 && "Mips chain is not supported yet" );

		D3D11_TEXTURE2D_DESC description;
		mem::zero( &description, sizeof( D3D11_TEXTURE2D_DESC ) );

		description.Width = m_width = width;
		description.Height = m_height = height;
		description.MipLevels = m_levels = mips;
		description.ArraySize = 1;
		description.Format = fluorineFormatToDirectX( m_format = format );
		description.SampleDesc.Count = 1;
		description.SampleDesc.Quality = 0;
		description.Usage = fluorineUsageToDirectX( m_usage = usage );
		description.BindFlags = D3D11_BIND_SHADER_RESOURCE; 
		description.CPUAccessFlags = usage == rend::EUsage::Dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
		description.MiscFlags = 0;

		HRESULT result;

		if( initialData )
		{
			auto formatInfo = rend::getFormatInfo( m_format );

			D3D11_SUBRESOURCE_DATA subresourceData;
			subresourceData.pSysMem = initialData;
			subresourceData.SysMemPitch = m_width * formatInfo.blockBytes / formatInfo.blockSizeX;
			subresourceData.SysMemSlicePitch = subresourceData.SysMemPitch / formatInfo.blockSizeY;

			result = device->CreateTexture2D( &description, &subresourceData, &m_texture );
			assert( SUCCEEDED( result ) );	
		}
		else
		{
			result = device->CreateTexture2D( &description, nullptr, &m_texture );
			assert( SUCCEEDED( result ) );
		}

		result = device->CreateShaderResourceView( m_texture.get(), nullptr, &m_srv );
		assert( SUCCEEDED( result ) );

		if( debugName )
		{
			m_texture->SetPrivateData( WKPDID_D3DDebugObjectName, UINT( cstr::length( debugName ) ), debugName );
			m_srv->SetPrivateData( WKPDID_D3DDebugObjectName, UINT( cstr::length( debugName ) ), debugName );
		}

		return true;
	}

	void DxTexture2D::destroy( ID3D11Device* device )
	{
		m_texture = nullptr;
		m_srv = nullptr;
	}

	DxRenderTarget::~DxRenderTarget()
	{
		assert( !m_texture.hasObject() );
		assert( !m_srv.hasObject() );
		assert( !m_rtv.hasObject() );
	}

	Bool DxRenderTarget::create( ID3D11Device* device, rend::EFormat format, Int32 width, Int32 height, const AnsiChar* debugName )
	{
		assert( device );
		assert( format != rend::EFormat::Unknown );
		assert( width > 0 && height > 0 );

		D3D11_TEXTURE2D_DESC description;
		mem::zero( &description, sizeof( D3D11_TEXTURE2D_DESC ) );

		description.Width = m_width = width;
		description.Height = m_height = height;
		description.MipLevels = 1;
		description.ArraySize = 1;
		description.Format = fluorineFormatToDirectX( m_format = format );
		description.SampleDesc.Count = 1;
		description.SampleDesc.Quality = 0;
		description.Usage = D3D11_USAGE_DEFAULT;
		description.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET; 
		description.CPUAccessFlags = 0;
		description.MiscFlags = 0;

		HRESULT result;

		result = device->CreateTexture2D( &description, nullptr, &m_texture );
		assert( SUCCEEDED( result ) );

		result = device->CreateShaderResourceView( m_texture.get(), nullptr, &m_srv );
		assert( SUCCEEDED( result ) );

		result = device->CreateRenderTargetView( m_texture.get(), nullptr, &m_rtv );
		assert( SUCCEEDED( result ) );

		if( debugName )
		{
			m_texture->SetPrivateData( WKPDID_D3DDebugObjectName, UINT( cstr::length( debugName ) ), debugName );
			m_srv->SetPrivateData( WKPDID_D3DDebugObjectName, UINT( cstr::length( debugName ) ), debugName );
			m_rtv->SetPrivateData( WKPDID_D3DDebugObjectName, UINT( cstr::length( debugName ) ), debugName );
		}

		return true;
	}

	void DxRenderTarget::destroy( ID3D11Device* device )
	{
		m_rtv = nullptr;
		m_srv = nullptr;
		m_texture = nullptr;
	}

	DxPixelShader::~DxPixelShader()
	{
		assert( !m_shader.hasObject() );
	}

	Bool DxPixelShader::create( ID3D11Device* device, const rend::CompiledShader& shader, const AnsiChar* debugName )
	{
		assert( device );
		assert( shader.getType() == rend::EShaderType::Pixel );
		assert( shader.isValid() );

		HRESULT result = device->CreatePixelShader( shader.code(), shader.codeLength(), nullptr, &m_shader );
		assert( SUCCEEDED( result ) );

		if( debugName )
		{
			m_shader->SetPrivateData( WKPDID_D3DDebugObjectName, UINT( cstr::length( debugName ) ), debugName );
		}

		return true;
	}

	void DxPixelShader::destroy( ID3D11Device* device )
	{
		m_shader = nullptr;
	}

	DxVertexShader::~DxVertexShader()
	{
		assert( !m_shader.hasObject() );
	}

	Bool DxVertexShader::create( ID3D11Device* device, const rend::CompiledShader& shader, UInt32 vertexDeclaration,
		const AnsiChar* debugName )
	{
		assert( device );
		assert( shader.getType() == rend::EShaderType::Vertex );
		assert( shader.isValid() );
		assert( vertexDeclaration != 0 );

		m_vertexDeclaration = vertexDeclaration;

		HRESULT result = device->CreateVertexShader( shader.code(), shader.codeLength(), nullptr, &m_shader );
		assert( SUCCEEDED( result ) );

		if( debugName )
		{
			m_shader->SetPrivateData( WKPDID_D3DDebugObjectName, UINT( cstr::length( debugName ) ), debugName );
		}

		return true;
	}

	void DxVertexShader::destroy( ID3D11Device* device )
	{
		m_shader = nullptr;
	}

	DxVertexBuffer::~DxVertexBuffer()
	{
		assert( !m_buffer.hasObject() );
	}

	Bool DxVertexBuffer::create( ID3D11Device* device, UInt32 vertexSize, UInt32 numVerts, 
		rend::EUsage usage, const void* initialData, const AnsiChar* debugName )
	{
		assert( device );
		assert( vertexSize != 0 && numVerts != 0 );
		
		m_vertexSize = vertexSize;
		m_numVerts = numVerts;

		D3D11_BUFFER_DESC description;
		description.ByteWidth = vertexSize * numVerts;
		description.Usage = fluorineUsageToDirectX( m_usage = usage );
		description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		description.CPUAccessFlags = usage == rend::EUsage::Dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
		description.MiscFlags = 0;
		description.StructureByteStride = 0;

		HRESULT result;

		if( initialData )
		{
			D3D11_SUBRESOURCE_DATA subresourceData;
			subresourceData.pSysMem = initialData;
			subresourceData.SysMemPitch = subresourceData.SysMemSlicePitch = 0;

			result = device->CreateBuffer( &description, &subresourceData, &m_buffer );
			assert( SUCCEEDED( result ) );
		}
		else
		{
			result = device->CreateBuffer( &description, nullptr, &m_buffer );
			assert( SUCCEEDED( result ) );
		}

		if( debugName )
		{
			m_buffer->SetPrivateData( WKPDID_D3DDebugObjectName, UINT( cstr::length( debugName ) ), debugName );		
		}

		return true;
	}

	void DxVertexBuffer::destroy( ID3D11Device* device )
	{
		m_buffer = nullptr;
	}

	DxIndexBuffer::~DxIndexBuffer()
	{
		assert( !m_buffer.hasObject() );
	}

	Bool DxIndexBuffer::create( ID3D11Device* device, rend::EFormat format, UInt32 numIndexes,
		rend::EUsage usage, const void* initialData, const AnsiChar* debugName )
	{
		assert( device );
		assert( format == rend::EFormat::R16_U || format == rend::EFormat::R32_U );
		assert( numIndexes );

		m_format = fluorineFormatToDirectX( format );
		m_numIndexes = numIndexes;

		D3D11_BUFFER_DESC description;
		description.ByteWidth = numIndexes * rend::getFormatInfo( format ).blockBytes;
		description.Usage = fluorineUsageToDirectX( m_usage = usage );
		description.BindFlags = D3D11_BIND_INDEX_BUFFER;
		description.CPUAccessFlags = usage == rend::EUsage::Dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
		description.MiscFlags = 0;
		description.StructureByteStride = 0;

		HRESULT result;

		if( initialData )
		{
			D3D11_SUBRESOURCE_DATA subresourceData;
			subresourceData.pSysMem = initialData;
			subresourceData.SysMemPitch = subresourceData.SysMemSlicePitch = 0;

			result = device->CreateBuffer( &description, &subresourceData, &m_buffer );
			assert( SUCCEEDED( result ) );
		}
		else
		{
			result = device->CreateBuffer( &description, nullptr, &m_buffer );
			assert( SUCCEEDED( result ) );
		}

		if( debugName )
		{
			m_buffer->SetPrivateData( WKPDID_D3DDebugObjectName, UINT( cstr::length( debugName ) ), debugName );		
		}

		return true;
	}

	void DxIndexBuffer::destroy( ID3D11Device* device )
	{
		m_buffer = nullptr;
	}

	DxConstantBuffer::~DxConstantBuffer()
	{
		assert( !m_buffer.hasObject() );
	}

	Bool DxConstantBuffer::create( ID3D11Device* device, UInt32 bufferSize,
		rend::EUsage usage, const void* initialData, const AnsiChar* debugName )
	{
		assert( device );
		assert( bufferSize > 0 );

		D3D11_BUFFER_DESC description;
		description.ByteWidth = m_size = bufferSize;
		description.Usage = fluorineUsageToDirectX( m_usage = usage );
		description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		description.CPUAccessFlags = usage == rend::EUsage::Dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
		description.MiscFlags = 0;
		description.StructureByteStride = 0;

		HRESULT result;

		if( initialData )
		{
			D3D11_SUBRESOURCE_DATA subresourceData;
			subresourceData.pSysMem = initialData;
			subresourceData.SysMemPitch = subresourceData.SysMemSlicePitch = 0;

			result = device->CreateBuffer( &description, &subresourceData, &m_buffer );
			assert( SUCCEEDED( result ) );
		}
		else
		{
			result = device->CreateBuffer( &description, nullptr, &m_buffer );
			assert( SUCCEEDED( result ) );
		}

		if( debugName )
		{
			m_buffer->SetPrivateData( WKPDID_D3DDebugObjectName, UINT( cstr::length( debugName ) ), debugName );		
		}

		return true;
	}

	void DxConstantBuffer::destroy( ID3D11Device* device )
	{
		m_buffer = nullptr;
	}
}
}