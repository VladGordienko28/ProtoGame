//-----------------------------------------------------------------------------
//	DxUtils.h: A Fluorine DirectX 11 utils
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace dx11
{
	/**
	 *	Convert an Fluorine format to DirectX format
	 */
	inline DXGI_FORMAT fluorineFormatToDirectX( rend::EFormat format )
	{
		switch( format )
		{
			case rend::EFormat::R8_I:			return DXGI_FORMAT_R8_SINT;
			case rend::EFormat::R8_U:			return DXGI_FORMAT_R8_UINT;
			case rend::EFormat::R16_F:			return DXGI_FORMAT_R16_FLOAT;
			case rend::EFormat::R16_I:			return DXGI_FORMAT_R16_SINT;
			case rend::EFormat::R16_U:			return DXGI_FORMAT_R16_UINT;
			case rend::EFormat::R32_F:			return DXGI_FORMAT_R32_FLOAT;
			case rend::EFormat::R32_I:			return DXGI_FORMAT_R32_SINT;
			case rend::EFormat::R32_U:			return DXGI_FORMAT_R32_UINT;
			case rend::EFormat::RG8_I:			return DXGI_FORMAT_R8G8_SINT;
			case rend::EFormat::RG8_U:			return DXGI_FORMAT_R8G8_UINT;
			case rend::EFormat::RG16_F:			return DXGI_FORMAT_R16G16_FLOAT;
			case rend::EFormat::RG16_I:			return DXGI_FORMAT_R16G16_SINT;
			case rend::EFormat::RG16_U:			return DXGI_FORMAT_R16G16_UINT;
			case rend::EFormat::RG32_F:			return DXGI_FORMAT_R32G32_FLOAT;
			case rend::EFormat::RG32_I:			return DXGI_FORMAT_R32G32_SINT;
			case rend::EFormat::RG32_U:			return DXGI_FORMAT_R32G32_UINT;
			case rend::EFormat::RGB32_F:		return DXGI_FORMAT_R32G32B32_FLOAT;
			case rend::EFormat::RGB32_I:		return DXGI_FORMAT_R32G32B32_SINT;
			case rend::EFormat::RGB32_U:		return DXGI_FORMAT_R32G32B32_UINT;
			case rend::EFormat::RGBA8_I:		return DXGI_FORMAT_R8G8B8A8_SINT;
			case rend::EFormat::RGBA8_U:		return DXGI_FORMAT_R8G8B8A8_UINT;
			case rend::EFormat::RGBA8_UNORM:	return DXGI_FORMAT_R8G8B8A8_UNORM;
			case rend::EFormat::RGBA32_F:		return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case rend::EFormat::RGBA32_I:		return DXGI_FORMAT_R32G32B32A32_SINT;
			case rend::EFormat::RGBA32_U:		return DXGI_FORMAT_R32G32B32A32_UINT;
			case rend::EFormat::D24S8:			return DXGI_FORMAT_D24_UNORM_S8_UINT;

			default:
				fatal( L"Unknown format %hs", rend::getFormatInfo( format ).name );
				return DXGI_FORMAT_UNKNOWN;
		}
	}

	/**
	 *	Convert an Fluorine usage flags to DirectX flags
	 */
	inline D3D11_USAGE fluorineUsageToDirectX( rend::EUsage usage )
	{
		switch( usage )
		{
			case rend::EUsage::Immutable:
				return D3D11_USAGE_IMMUTABLE;

			case rend::EUsage::Dynamic:
				return D3D11_USAGE_DYNAMIC;

			case rend::EUsage::GPUOnly:
				return D3D11_USAGE_DEFAULT;

			default:
				fatal( L"Unknown resource usage %d", usage );
				return D3D11_USAGE_DEFAULT;
		}
	}
}
}