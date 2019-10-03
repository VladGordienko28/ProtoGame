//-----------------------------------------------------------------------------
//	ImageType.cpp: An image type
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Image.h"

namespace flu
{
namespace img
{
	Image::Image( String name )
		:	m_name( name ),
			m_handle( INVALID_HANDLE<rend::Texture2DHandle>() ),
			m_srv(),
			m_type( EImageType::MAX ),
			m_uSize( 0 ), m_vSize( 0 ),
			m_uBits( 0 ), m_vBits( 0 )
	{
	}

	Image::~Image()
	{
	}

	Bool Image::create( rend::Device* device, const res::CompiledResource& compiledResource )
	{
		assert( device );
		assert( compiledResource.isValid() );
		assert( m_handle == INVALID_HANDLE<rend::Texture2DHandle>() );

		UInt32 width = 0;
		UInt32 height = 0;

		LodePNGState state;
		lodepng_state_init( &state );

		UInt8* imageData;

		auto pngError = lodepng_decode( &imageData, &width, &height, &state, 
			&compiledResource.data[0], compiledResource.data.size() );

		if( pngError )
		{
			error( TEXT( "Unable to load image \"%s\" with error \"%s\"" ), *m_name, 
				lodepng_error_text( pngError ) );

			lodepng_state_cleanup( &state );
			return false;
		}

		//todo: only RGBA8 supported yet :(
		assert( state.info_raw.bitdepth == 8 );
		assert( state.info_raw.colortype == LCT_RGBA );

		m_handle = device->createTexture2D( rend::EFormat::RGBA8_UNORM, width, height, 1, rend::EUsage::Immutable, imageData, *wide2AnsiString( m_name ) );
		m_srv = device->getShaderResourceView( m_handle );

		m_uSize = width;
		m_vSize = height;

		m_type = EImageType::RGBA;

		m_uBits = intLog2( m_uSize );
		m_vBits = intLog2( m_vSize );

		mem::free( imageData );
		return true;
	}

	void Image::destroy( rend::Device* device )
	{
		assert( device );
		assert( m_handle != INVALID_HANDLE<rend::Texture2DHandle>() );

		device->destroyTexture2D( m_handle );
		m_handle = INVALID_HANDLE<rend::Texture2DHandle>();

		m_type = EImageType::MAX;

		m_uSize = m_vSize = 0;
		m_uBits = m_vBits = 0;
	}
}
}