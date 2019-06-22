//-----------------------------------------------------------------------------
//	DxAdapter.cpp: An experimental DirectX render integration into legacy interface
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Editor/Editor.h"

namespace flu
{
	CDirectX11Render::CDirectX11Render( HWND hwnd )
	{
		m_hwnd = hwnd;

		m_device = new dx11::Device( hwnd, 800, 600, false );
		m_canvas = new CDirectX11Canvas( this, m_device.get() );

		info( L"DirectX11: DirectX11 render initialized" );
	}

	void CDirectX11Render::Resize( Int32 newWidth, Int32 newHeight )
	{
		newWidth = clamp( newWidth,  1, 3000 );
		newHeight = clamp( newHeight, 1, 3000 );

		m_device->resize( newWidth, newHeight, false );
	}

	CCanvas* CDirectX11Render::Lock()
	{
		m_device->beginFrame();
		m_device->clearRenderTarget( INVALID_HANDLE<rend::RenderTargetHandle>(), math::colors::BLACK );
	
		m_canvas->ScreenWidth = m_device->getBackbufferWidth();
		m_canvas->ScreenHeight = m_device->getBackbufferHeight();
		m_canvas->m_lockTime = math::fMod( GPlat->Now(), 1000.f * 2.f * math::PI );

		return m_canvas.get();
	}

	void CDirectX11Render::Unlock()
	{
		m_canvas->m_effectSystem->update();

		profile_zone( EProfilerGroup::Render, Present );
		m_device->endFrame();
	}

	CDirectX11Render::~CDirectX11Render()
	{
		m_canvas = nullptr;
		m_device = nullptr;
	}


	void CDirectX11Render::Flush()
	{
	}

	void CDirectX11Render::RenderLevel( CCanvas* canvas, FLevel* level, Int32 x, Int32 y, Int32 w, Int32 h )
	{







	}


	CDirectX11Canvas::CDirectX11Canvas( CDirectX11Render* render, rend::Device* device )
		:	m_render( render ),
			m_device( device ),
			m_lockTime( 0.f )
	{
		m_effectSystem = new ffx::System();
		m_effectSystem->init( m_device, fm::getCurrentDirectory() + TEXT( "\\Shaders\\" ) );

			rend::VertexDeclaration declXY( L"VertexDecl_XY" );
			declXY.addElement( { rend::EFormat::RG32_F, rend::EVertexElementUsage::Position, 0, 0 } );

		m_colorEffect = m_effectSystem->getEffect( L"Colored", declXY );


			rend::VertexDeclaration declXYUV( L"VertexDecl_XYUV" );
			declXYUV.addElement( { rend::EFormat::RG32_F, rend::EVertexElementUsage::Position, 0, 0 } );
			declXYUV.addElement( { rend::EFormat::RG32_F, rend::EVertexElementUsage::TexCoord, 0, 8 } );

		m_texturedEffect = m_effectSystem->getEffect( L"Textured", declXYUV );


		// allocate required vertex buffers
		m_quadVB_XY = m_device->createVertexBuffer( sizeof( math::Vector ), 4, rend::EUsage::Dynamic, nullptr, "QuadVB_XY" );
		m_quadVB_XYUV = m_device->createVertexBuffer( sizeof( math::Vector )*2, 4, rend::EUsage::Dynamic, nullptr, "QuadVB_XYUV" );

		m_samplerNearest = m_device->getSamplerState( { rend::ESamplerFilter::Point, rend::ESamplerAddressMode::Wrap } );
		m_samplerLinear = m_device->getSamplerState( { rend::ESamplerFilter::Linear, rend::ESamplerAddressMode::Wrap } );

		m_blendStates[BLEND_Regular] = -1;
		m_blendStates[BLEND_Masked] = -1;
		m_blendStates[BLEND_Translucent] = m_device->getBlendState( { rend::EBlendFactor::One, rend::EBlendFactor::InvSrcColor, rend::EBlendOp::Add, rend::EBlendFactor::One, rend::EBlendFactor::InvSrcAlpha, rend::EBlendOp::Add } );
		m_blendStates[BLEND_Modulated] = m_device->getBlendState( { rend::EBlendFactor::DestColor, rend::EBlendFactor::SrcColor, rend::EBlendOp::Add, rend::EBlendFactor::DestAlpha, rend::EBlendFactor::SrcAlpha, rend::EBlendOp::Add } );
		m_blendStates[BLEND_Alpha] = m_device->getBlendState( { rend::EBlendFactor::SrcAlpha, rend::EBlendFactor::InvSrcAlpha, rend::EBlendOp::Add, rend::EBlendFactor::SrcAlpha, rend::EBlendFactor::InvSrcAlpha, rend::EBlendOp::Add } );
		m_blendStates[BLEND_Darken] = m_device->getBlendState( { rend::EBlendFactor::Zero, rend::EBlendFactor::InvSrcColor, rend::EBlendOp::Add, rend::EBlendFactor::Zero, rend::EBlendFactor::InvSrcAlpha, rend::EBlendOp::Add } );
		m_blendStates[BLEND_Brighten] = m_device->getBlendState( { rend::EBlendFactor::SrcAlpha, rend::EBlendFactor::One, rend::EBlendOp::Add, rend::EBlendFactor::SrcAlpha, rend::EBlendFactor::One, rend::EBlendOp::Add } );
		m_blendStates[BLEND_FastOpaque] = m_device->getBlendState( { rend::EBlendFactor::One, rend::EBlendFactor::InvSrcColor, rend::EBlendOp::Add, rend::EBlendFactor::One, rend::EBlendFactor::InvSrcAlpha, rend::EBlendOp::Add } );
	}

	CDirectX11Canvas::~CDirectX11Canvas()
	{
		m_effectSystem->shutdown();
		m_effectSystem = nullptr;
		
		for( auto& it : m_textures )
		{
			m_device->destroyTexture2D( it );
		}

		m_textures.empty();
		m_srvs.empty();

		m_device->destroyVertexBuffer( m_quadVB_XY );
		m_device->destroyVertexBuffer( m_quadVB_XYUV );
	}

	void CDirectX11Canvas::SetTransform( const TViewInfo& info )
	{
		const Float xFov2 = 2.f / ( info.FOV.x * info.Zoom );
		const Float yFov2 = 2.f / ( info.FOV.y * info.Zoom );

		const Float backbufferWidth = m_device->getBackbufferWidth();
		const Float backbufferHeight = m_device->getBackbufferHeight();

		math::Vector sScale, sOffset;

		sScale.x = info.Width / backbufferWidth;
		sScale.y = info.Height / backbufferHeight;

		sOffset.x = ( 2.f / backbufferWidth ) * ( info.X + ( info.Width / 2.f ) ) - 1.f;
		sOffset.y = 1.f - ( 2.f / backbufferHeight ) * ( info.Y + ( info.Height / 2.f ) );

		Float matrix[4][4];

		matrix[0][0] = xFov2 * +info.Coords.xAxis.x * sScale.x;
		matrix[1][0] = yFov2 * -info.Coords.xAxis.y * sScale.y;
		matrix[2][0] = 0.f;
		matrix[3][0] = 0.f;

		matrix[0][1] = xFov2 * -info.Coords.yAxis.x * sScale.x;
		matrix[1][1] = yFov2 * +info.Coords.yAxis.y * sScale.y;
		matrix[2][1] = 0.f;
		matrix[3][1] = 0.f;

		matrix[0][2] = 0.f;
		matrix[1][2] = 0.f;
		matrix[2][2] = 1.f;
		matrix[3][2] = 0.f;

		matrix[0][3] = -( info.Coords.origin.x * matrix[0][0] + info.Coords.origin.y * matrix[0][1] ) + sOffset.x;
		matrix[1][3] = -( info.Coords.origin.x * matrix[1][0] + info.Coords.origin.y * matrix[1][1] ) + sOffset.y;
		matrix[2][3] = 1.f;
		matrix[3][3] = 1.f;

		m_colorEffect->setData( matrix, sizeof( matrix ), 16 );
		m_texturedEffect->setData( matrix, sizeof( matrix ), 16 );
	}

	void CDirectX11Canvas::SetClip( const TClipArea& area )
	{
	}

	void CDirectX11Canvas::DrawPoint( const math::Vector& p, Float size, math::Color color )
	{
	}

	void CDirectX11Canvas::DrawLine( const math::Vector& a, const math::Vector& b, math::Color color, Bool stipple )
	{
		const math::Vector Verts[2] = { a, b };

		m_colorEffect->setColor( 0, color );

		m_colorEffect->apply();

		m_device->updateVertexBuffer( m_quadVB_XY, Verts, sizeof( Verts ) );
		m_device->setVertexBuffer( m_quadVB_XY );

		m_device->setTopology( rend::EPrimitiveTopology::LineStrip );
		m_device->draw( 2 );

	}

	void CDirectX11Canvas::DrawPoly( const TRenderPoly& poly )
	{
	}

	void CDirectX11Canvas::DrawRect( const TRenderRect& rect )
	{
		math::Vector Verts[4];

		// Compute sprite vertices.
		if( !rect.Rotation )
		{
			// No rotation.
			Verts[1] = rect.Bounds.min;
			Verts[0] = math::Vector( rect.Bounds.min.x, rect.Bounds.max.y );
			Verts[2] = rect.Bounds.max;
			Verts[3] = math::Vector( rect.Bounds.max.x, rect.Bounds.min.y );
		}
		else
		{
			// Rotation.
			math::Vector Center	= rect.Bounds.center();
			math::Vector Size2	= rect.Bounds.size() * 0.5f;
			math::Coords Coords	= math::Coords( Center, rect.Rotation );

			math::Vector XAxis = Coords.xAxis * Size2.x,
					YAxis = Coords.yAxis * Size2.y;

			// World coords.
			Verts[1] = Center - YAxis - XAxis;
			Verts[0] = Center + YAxis - XAxis;
			Verts[2] = Center + YAxis + XAxis;
			Verts[3] = Center - YAxis + XAxis;
		}


		if( rect.Flags & POLY_FlatShade )
		{
			m_colorEffect->setColor( 0, rect.Color );
			m_colorEffect->apply();

			m_device->updateVertexBuffer( m_quadVB_XY, Verts, sizeof( Verts ) );
			m_device->setVertexBuffer( m_quadVB_XY );
		}
		else if( rect.Texture && rect.Texture->IsA(FBitmap::MetaClass) )
		{
			math::Vector T1	= rect.TexCoords.min;
			math::Vector T2	= rect.TexCoords.max;

			math::Vector myVerts [4][2] = 
			{
				Verts[0], { T1.x, T2.y },
				Verts[1], { T1.x, T1.y },
				Verts[2], { T2.x, T2.y },
				Verts[3], { T2.x, T1.y },
			};


			m_texturedEffect->setColor( 0, rect.Color );
			m_texturedEffect->setSRV( 0, getSrvOf(As<FBitmap>(rect.Texture)) );

			m_texturedEffect->setSamplerState( 0, As<FBitmap>(rect.Texture)->Filter == BFILTER_Nearest ? m_samplerNearest : m_samplerLinear );
			m_texturedEffect->setBlendState( m_blendStates[As<FBitmap>(rect.Texture)->BlendMode] );

			m_texturedEffect->apply();

			m_device->updateVertexBuffer( m_quadVB_XYUV, myVerts, sizeof( myVerts ) );
			m_device->setVertexBuffer( m_quadVB_XYUV );
		}


		m_device->setTopology( rend::EPrimitiveTopology::TriangleStrip );
		m_device->draw( 4 );
	}

	void CDirectX11Canvas::DrawList( const TRenderList& list )
	{
	}


static void* Palette8ToRGBA( UInt8* SourceData, math::Color* Palette, Int32 USize, Int32 VSize )
{
	static math::Color Buffer512[512*512];
	Int32 i, n;

    // Doesn't allow palette image with dimension > 512.
	if( USize*VSize > 512*512 )
		return nullptr;
	
	i	= USize * VSize;
	while( i-- > 0 )
	{
		Buffer512[i] = Palette[SourceData[i]];
	}

	return Buffer512;
}


	rend::ShaderResourceView CDirectX11Canvas::getSrvOf( FBitmap* bitmap )
	{
		if( bitmap )
		{
			if( bitmap->RenderInfo == -1 )
			{
				rend::Texture2DHandle newTexture = m_device->createTexture2D( rend::EFormat::RGBA8_UNORM, bitmap->USize, bitmap->VSize, 1,
					bitmap->bDynamic ? rend::EUsage::Dynamic : rend::EUsage::Immutable, bitmap->Format == BF_Palette8 ? Palette8ToRGBA
											( 
												(UInt8*)bitmap->GetData(), 
												&bitmap->Palette.Colors[0], 
												bitmap->USize, 
												bitmap->VSize 
											) : bitmap->GetData(), *wide2AnsiString(bitmap->GetName()) );

				// todo: add dynamic textures update!!!

				bitmap->RenderInfo = m_textures.push( newTexture );
				m_srvs.push( m_device->getShaderResourceView( newTexture ) );
			}


			if( bitmap->bDynamic && bitmap->bRedrawn )
			{
				m_device->updateTexture2D( m_textures[bitmap->RenderInfo], bitmap->Format == BF_Palette8 ? Palette8ToRGBA
											( 
												(UInt8*)bitmap->GetData(), 
												&bitmap->Palette.Colors[0], 
												bitmap->USize, 
												bitmap->VSize 
											) : bitmap->GetData() );

				bitmap->bRedrawn = false;
			}

			bitmap->Tick();

			return m_srvs[bitmap->RenderInfo];
		}
		else
		{
			return { nullptr };
		}


	}
}