//-----------------------------------------------------------------------------
//	DxAdapter.cpp: An experimental DirectX render integration into legacy interface
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Editor/Editor.h"

namespace flu
{

	CDirectX11Render::CDirectX11Render( rend::Device* inDevice, gfx::DrawContext& drawContext )
	{
		m_device = inDevice;
		m_canvas = new CDirectX11Canvas( this, m_device, drawContext );



		info( L"DirectX11: DirectX11 render initialized" );
	}

	CCanvas* CDirectX11Render::Lock()
	{
		m_device->beginFrame();
		m_device->clearRenderTarget( INVALID_HANDLE<rend::RenderTargetHandle>(), math::colors::BLACK );
	



		return m_canvas.get();
	}

	void CDirectX11Render::Unlock()
	{
		//m_canvas->m_effectSystem->update();

		profile_zone( EProfilerGroup::Render, Present );
		m_device->endFrame();
	}

	CDirectX11Render::~CDirectX11Render()
	{
		m_canvas = nullptr;
		m_device = nullptr;
	}


	CDirectX11Canvas::CDirectX11Canvas( CDirectX11Render* render, rend::Device* device, gfx::DrawContext& drawContext )
		:	CCanvas( drawContext ),
			m_render( render ),
			m_device( device )
	{

		// allocate required vertex buffers
		m_quadVB_XY = m_device->createVertexBuffer( sizeof( math::Vector ), 4, rend::EUsage::Dynamic, nullptr, "QuadVB_XY" );
		m_quadVB_XYUV = m_device->createVertexBuffer( sizeof( math::Vector )*2, 4, rend::EUsage::Dynamic, nullptr, "QuadVB_XYUV" );

		m_polyVB_XY = m_device->createVertexBuffer( sizeof( math::Vector ), 16, rend::EUsage::Dynamic, nullptr, "PolyVB_XY" );
		m_polyVB_XYUV = m_device->createVertexBuffer( sizeof( math::Vector )*2, 16, rend::EUsage::Dynamic, nullptr, "PolyVB_XYUV" );

		m_samplerNearest = m_device->getSamplerState( { rend::ESamplerFilter::Point, rend::ESamplerAddressMode::Wrap } );
		m_samplerLinear = m_device->getSamplerState( { rend::ESamplerFilter::Linear, rend::ESamplerAddressMode::Wrap } );

		m_normalBlendState = rend::BlendState::INVALID;
		m_alphaBlendState = m_device->getBlendState( { rend::EBlendFactor::SrcAlpha, rend::EBlendFactor::InvSrcAlpha, rend::EBlendOp::Add, rend::EBlendFactor::SrcAlpha, rend::EBlendFactor::InvSrcAlpha, rend::EBlendOp::Add } );
		m_translucentBlendState = m_device->getBlendState( { rend::EBlendFactor::One, rend::EBlendFactor::InvSrcColor, rend::EBlendOp::Add, rend::EBlendFactor::One, rend::EBlendFactor::InvSrcAlpha, rend::EBlendOp::Add } );
/*
		m_blendStates[BLEND_Regular] = -1;
		m_blendStates[BLEND_Masked] = -1;
		m_blendStates[BLEND_Translucent] = m_device->getBlendState( { rend::EBlendFactor::One, rend::EBlendFactor::InvSrcColor, rend::EBlendOp::Add, rend::EBlendFactor::One, rend::EBlendFactor::InvSrcAlpha, rend::EBlendOp::Add } );
		m_blendStates[BLEND_Modulated] = m_device->getBlendState( { rend::EBlendFactor::DestColor, rend::EBlendFactor::SrcColor, rend::EBlendOp::Add, rend::EBlendFactor::DestAlpha, rend::EBlendFactor::SrcAlpha, rend::EBlendOp::Add } );
		m_blendStates[BLEND_Alpha] = m_device->getBlendState( { rend::EBlendFactor::SrcAlpha, rend::EBlendFactor::InvSrcAlpha, rend::EBlendOp::Add, rend::EBlendFactor::SrcAlpha, rend::EBlendFactor::InvSrcAlpha, rend::EBlendOp::Add } );
		m_blendStates[BLEND_Darken] = m_device->getBlendState( { rend::EBlendFactor::Zero, rend::EBlendFactor::InvSrcColor, rend::EBlendOp::Add, rend::EBlendFactor::Zero, rend::EBlendFactor::InvSrcAlpha, rend::EBlendOp::Add } );
		m_blendStates[BLEND_Brighten] = m_device->getBlendState( { rend::EBlendFactor::SrcAlpha, rend::EBlendFactor::One, rend::EBlendOp::Add, rend::EBlendFactor::SrcAlpha, rend::EBlendFactor::One, rend::EBlendOp::Add } );
		m_blendStates[BLEND_FastOpaque] = m_device->getBlendState( { rend::EBlendFactor::One, rend::EBlendFactor::InvSrcColor, rend::EBlendOp::Add, rend::EBlendFactor::One, rend::EBlendFactor::InvSrcAlpha, rend::EBlendOp::Add } );
		*/

		UInt16 polyIds[16 * 3];

		for( Int32 i = 0; i < 16; ++i )
		{
			polyIds[i*3 + 0] = 0;
			polyIds[i*3 + 1] = i + 1;
			polyIds[i*3 + 2] = i + 2;
		}


		m_polyIB = m_device->createIndexBuffer( rend::EFormat::R16_U, 16 * 3, rend::EUsage::Immutable, polyIds, "Poly index buffer" );
	

		m_coloredEffect = res::ResourceManager::get<ffx::Effect>( L"System.Shaders.Colored", res::EFailPolicy::FATAL );
		m_texturedEffect = res::ResourceManager::get<ffx::Effect>( L"System.Shaders.Textured", res::EFailPolicy::FATAL );

		m_solidTech = m_coloredEffect->getTechnique( L"Solid" );
		m_stippleTech = m_coloredEffect->getTechnique( L"Stipple" );
	}

	CDirectX11Canvas::~CDirectX11Canvas()
	{

		m_coloredEffect = nullptr;
		m_texturedEffect = nullptr;
		

		m_device->destroyVertexBuffer( m_quadVB_XY );
		m_device->destroyVertexBuffer( m_quadVB_XYUV );
		m_device->destroyVertexBuffer( m_polyVB_XY );
		m_device->destroyVertexBuffer( m_polyVB_XYUV );

		m_device->destroyIndexBuffer( m_polyIB );
	}

	void CDirectX11Canvas::SetClip( const TClipArea& area )
	{
		if( CLIP_NONE != area )
		{
			rend::ScissorArea scissorArea;
			scissorArea.left = area.X;
			scissorArea.top = area.Y;
			scissorArea.right = area.X + area.Width;
			scissorArea.bottom = area.Y + area.Height;

			m_device->setScissorArea( scissorArea );		
		}
		else
		{
			m_device->setScissorArea( rend::ScissorArea::NULL_AREA() );
		}
	}

	void CDirectX11Canvas::DrawPoly( const TRenderPoly& poly )
	{
		if( poly.Flags & POLY_FlatShade )
		{
			m_coloredEffect->setColor( 0, poly.Color );

			if( poly.Flags & (POLY_StippleI | POLY_StippleII) )
				m_coloredEffect->setTechnique( m_stippleTech );
			else
				m_coloredEffect->setTechnique( m_solidTech );

			m_coloredEffect->setBlendState( m_normalBlendState );

			if( poly.Flags & POLY_Ghost )
				m_coloredEffect->setBlendState( m_translucentBlendState );

			if( poly.Flags & POLY_AlphaGhost )
				m_coloredEffect->setBlendState( m_alphaBlendState );

			m_coloredEffect->apply();

			m_device->updateVertexBuffer( m_polyVB_XY, poly.Vertices, sizeof( math::Vector ) * poly.NumVerts );
			m_device->setVertexBuffer( m_polyVB_XY );



		}
		else if( poly.Image != INVALID_HANDLE<rend::Texture2DHandle>() )
		{
			math::Vector verts[16][2];

			for( Int32 i = 0; i < poly.NumVerts; ++i )
			{
				verts[i][0] = poly.Vertices[i];
				verts[i][1] = poly.TexCoords[i];
			}

			m_texturedEffect->setBlendState( m_normalBlendState );

			if( poly.Flags & POLY_Ghost )
				m_texturedEffect->setBlendState( m_translucentBlendState );

			if( poly.Flags & POLY_AlphaGhost )
				m_texturedEffect->setBlendState( m_alphaBlendState );

			m_texturedEffect->setColor( 0, poly.Color );
			m_texturedEffect->setSRV( 0, m_device->getShaderResourceView( poly.Image ) );

			m_texturedEffect->setSamplerState( 0, m_samplerNearest );
			//m_texturedEffect->setBlendState( m_blendStates[As<FBitmap>(poly.Texture)->BlendMode] );

			m_texturedEffect->apply();

			m_device->updateVertexBuffer( m_polyVB_XYUV, verts, sizeof( verts ) );
			m_device->setVertexBuffer( m_polyVB_XYUV );
		}
		else
			return;

		m_device->setIndexBuffer( m_polyIB );
		m_device->setTopology( rend::EPrimitiveTopology::TriangleList );
		m_device->drawIndexed( 3*(poly.NumVerts - 2), 0, 0 );
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
			m_coloredEffect->setColor( 0, rect.Color );

			if( rect.Flags & (POLY_StippleI | POLY_StippleII) )
				m_coloredEffect->setTechnique( m_stippleTech );
			else
				m_coloredEffect->setTechnique( m_solidTech );

			m_coloredEffect->setBlendState( m_normalBlendState );

			if( rect.Flags & POLY_Ghost )
				m_coloredEffect->setBlendState( m_translucentBlendState );

			if( rect.Flags & POLY_AlphaGhost )
				m_coloredEffect->setBlendState( m_alphaBlendState );


			m_coloredEffect->apply();

			m_device->updateVertexBuffer( m_quadVB_XY, Verts, sizeof( Verts ) );
			m_device->setVertexBuffer( m_quadVB_XY );
		}
		else if( rect.Image != INVALID_HANDLE<rend::Texture2DHandle>() )
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

			m_texturedEffect->setBlendState( m_normalBlendState );

			if( rect.Flags & POLY_Ghost )
				m_texturedEffect->setBlendState( m_translucentBlendState );

			if( rect.Flags & POLY_AlphaGhost )
				m_texturedEffect->setBlendState( m_alphaBlendState );

			m_texturedEffect->setColor( 0, rect.Color );
			m_texturedEffect->setSRV( 0, m_device->getShaderResourceView( rect.Image ) );

			m_texturedEffect->setSamplerState( 0, m_samplerNearest );
			//m_texturedEffect->setBlendState( m_blendStates[As<FBitmap>(rect.Texture)->BlendMode] );

			m_texturedEffect->apply();

			m_device->updateVertexBuffer( m_quadVB_XYUV, myVerts, sizeof( myVerts ) );
			m_device->setVertexBuffer( m_quadVB_XYUV );
		}


		m_device->setTopology( rend::EPrimitiveTopology::TriangleStrip );
		m_device->draw( 4 );
	}

	void CDirectX11Canvas::DrawList( const TRenderList& list )
	{
		for( Int32 i = 0; i < list.NumRects; ++i )
		{
			TRenderPoly poly;

			poly.Color = list.Colors ? math::colors::WHITE : list.DrawColor;
			poly.Flags = list.Flags;
			poly.NumVerts = 4;
			poly.Image = list.Image;
			mem::copy( poly.Vertices, &list.Vertices[i*4], 4 * sizeof(math::Vector) );
			mem::copy( poly.TexCoords, &list.TexCoords[i*4], 4 * sizeof(math::Vector) );

			DrawPoly( poly );
		}
	}

}