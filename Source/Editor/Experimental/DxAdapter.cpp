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

		gfx::api::initialize( m_device.get() );

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

		gfx::SharedConstants::PerFrameData perFrameData;
		perFrameData.gameTime = m_canvas->m_lockTime;

		m_canvas->m_sharedConstants->setPerFrameData( perFrameData );
		m_canvas->m_sharedConstants->bindToPipeline();


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
		gfx::api::finalize();

		m_canvas = nullptr;
		m_device = nullptr;
	}


	img::Image::Ptr g_coolImage;
	rend::Device* g_device;

		ffx::Effect::Ptr g_colorEffect;
		ffx::Effect::Ptr g_texturedEffect;

	void renderLoadEffects()
	{
		g_colorEffect = res::ResourceManager::get<ffx::Effect>( L"System.Shaders.Colored" );
		g_texturedEffect = res::ResourceManager::get<ffx::Effect>( L"System.Shaders.Textured" );;
	}

	void renderDestroyEffects()
	{
		g_colorEffect = nullptr;
		g_texturedEffect = nullptr;
	}


	CDirectX11Canvas::CDirectX11Canvas( CDirectX11Render* render, rend::Device* device )
		:	m_render( render ),
			m_device( device ),
			m_lockTime( 0.f )
	{
		StackTop = 0;
		g_device = device;


	
		//res::ResourceManager::loadAllPackages();
		//res::ResourceManager::generatePackages();/////////////////////////////////////////////////////////////////////////

		//g_coolImage = res::ResourceManager::get< img::Image >( L"Bitmaps.BForestTiles", res::EFailPolicy::FATAL );



		m_sharedConstants = new gfx::SharedConstants( m_device );





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
	}

	CDirectX11Canvas::~CDirectX11Canvas()
	{

		g_colorEffect = nullptr;
		g_texturedEffect = nullptr;

		m_sharedConstants = nullptr;
		

		m_device->destroyVertexBuffer( m_quadVB_XY );
		m_device->destroyVertexBuffer( m_quadVB_XYUV );
		m_device->destroyVertexBuffer( m_polyVB_XY );
		m_device->destroyVertexBuffer( m_polyVB_XYUV );

		m_device->destroyIndexBuffer( m_polyIB );
	}

	void CDirectX11Canvas::SetTransform( const gfx::ViewInfo& info )
	{
		gfx::SharedConstants::PerViewData perViewData;
		info.viewProjectionMatrix( m_device->getBackbufferWidth(), m_device->getBackbufferHeight(), perViewData.viewProjectionMatrix );
		perViewData.worldCamera = math::Vector4( info.coords.origin, info.bounds.sizeX(), info.bounds.sizeY() );

		m_sharedConstants->setPerViewData( perViewData );

		// Store this coords system.
		View			= info;
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

	void CDirectX11Canvas::DrawPoint( const math::Vector& p, Float size, math::Color color )
	{
	}

	void CDirectX11Canvas::DrawLine( const math::Vector& a, const math::Vector& b, math::Color color, Bool stipple )
	{
		const math::Vector Verts[2] = { a, b };

		g_colorEffect->setColor( 0, color );
		g_colorEffect->setBool( 16, stipple );

		g_colorEffect->apply();

		m_device->updateVertexBuffer( m_quadVB_XY, Verts, sizeof( Verts ) );
		m_device->setVertexBuffer( m_quadVB_XY );

		m_device->setTopology( rend::EPrimitiveTopology::LineStrip );
		m_device->draw( 2 );

	}

	void CDirectX11Canvas::DrawPoly( const TRenderPoly& poly )
	{
		if( poly.Flags & POLY_FlatShade )
		{
			g_colorEffect->setColor( 0, poly.Color );
			g_colorEffect->setBool( 16, poly.Flags & (POLY_StippleI | POLY_StippleII) );

			g_colorEffect->setBlendState( m_normalBlendState );

			if( poly.Flags & POLY_Ghost )
				g_colorEffect->setBlendState( m_translucentBlendState );

			if( poly.Flags & POLY_AlphaGhost )
				g_colorEffect->setBlendState( m_alphaBlendState );

			g_colorEffect->apply();

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

			g_texturedEffect->setBlendState( m_normalBlendState );

			if( poly.Flags & POLY_Ghost )
				g_texturedEffect->setBlendState( m_translucentBlendState );

			if( poly.Flags & POLY_AlphaGhost )
				g_texturedEffect->setBlendState( m_alphaBlendState );

			g_texturedEffect->setColor( 0, poly.Color );
			g_texturedEffect->setSRV( 0, m_device->getShaderResourceView( poly.Image ) );

			g_texturedEffect->setSamplerState( 0, m_samplerNearest );
			//m_texturedEffect->setBlendState( m_blendStates[As<FBitmap>(poly.Texture)->BlendMode] );

			g_texturedEffect->apply();

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
			g_colorEffect->setColor( 0, rect.Color );
			g_colorEffect->setBool( 16, rect.Flags & (POLY_StippleI | POLY_StippleII) );

			g_colorEffect->setBlendState( m_normalBlendState );

			if( rect.Flags & POLY_Ghost )
				g_colorEffect->setBlendState( m_translucentBlendState );

			if( rect.Flags & POLY_AlphaGhost )
				g_colorEffect->setBlendState( m_alphaBlendState );


			g_colorEffect->apply();

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

			g_texturedEffect->setBlendState( m_normalBlendState );

			if( rect.Flags & POLY_Ghost )
				g_texturedEffect->setBlendState( m_translucentBlendState );

			if( rect.Flags & POLY_AlphaGhost )
				g_texturedEffect->setBlendState( m_alphaBlendState );

			g_texturedEffect->setColor( 0, rect.Color );
			g_texturedEffect->setSRV( 0, m_device->getShaderResourceView( rect.Image ) );

			g_texturedEffect->setSamplerState( 0, m_samplerNearest );
			//m_texturedEffect->setBlendState( m_blendStates[As<FBitmap>(rect.Texture)->BlendMode] );

			g_texturedEffect->apply();

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




//
// Render objects comparison.
//
Bool RenderObjectCmp( FComponent*const &A, FComponent*const &B )
{
	return A->GetLayer() < B->GetLayer();
}

#if 0
	void CDirectX11Render::RenderLevel( CCanvas* canvas, FLevel* level, Int32 x, Int32 y, Int32 w, Int32 h )
	{
		// Check pointers.
		assert( level );
		assert( canvas == m_canvas.get() );

		// Sort render objects according to it layer.
		if( GFrameStamp & 31 )
			level->RenderObjects.sort(RenderObjectCmp);

		// Clamp level scrolling when we play.
		if( level->bIsPlaying )
		{
			TCamera& Camera = level->Camera;

			Camera.Location.x	= clamp
			(
				Camera.Location.x,
				Camera.ScrollBound.min.x + Camera.FOV.x*0.5f,
				Camera.ScrollBound.max.x - Camera.FOV.x*0.5f
			);

			Camera.Location.y	= clamp
			(
				Camera.Location.y,
				Camera.ScrollBound.min.y + Camera.FOV.y*0.5f,
				Camera.ScrollBound.max.y - Camera.FOV.y*0.5f
			);
		}

		// Compute master view.
		gfx::ViewInfo MasterView	= gfx::ViewInfo
		(
			level->Camera.Location,
			level->Camera.Rotation,
			level->Camera.GetFitFOV( w, h ),
			level->Camera.Zoom,
			false,
			x,
			y, 
			w, 
			h
		);


		canvas->PushTransform( MasterView );
		{
			//g_grid.render( canvas->View );


			// Render all objects in master view.
			for( Int32 i=0; i<level->RenderObjects.size(); i++ )
				level->RenderObjects[i]->Render( canvas );

			// Draw debug stuff.
			// !!todo: add special flag for level.
			CDebugDrawHelper::Instance().Render( canvas );

			// Draw path if need
			if( level->RndFlags & RND_Paths )
				level->m_navigator.draw( canvas );


			/*
			// Temp Image stuff
			{
				math::Vector Verts[4];

				Verts[1] = { -10, -10 };
				Verts[0] = { -10, +10 };
				Verts[2] = { +10, +10 };
				Verts[3] = { +10, -10 };
			
				math::Vector T1	= { 0.f, 0.f };
				math::Vector T2	= { 1.f, 1.f };

				math::Vector myVerts [4][2] = 
				{
					Verts[0], { T1.x, T2.y },
					Verts[1], { T1.x, T1.y },
					Verts[2], { T2.x, T2.y },
					Verts[3], { T2.x, T1.y },
				};

				g_texturedEffect->setColor( 0, math::colors::WHITE );
				g_texturedEffect->setSRV( 0, g_coolImage->getSRV() );

				g_texturedEffect->setSamplerState( 0, m_canvas->m_samplerNearest );
				g_texturedEffect->setBlendState( m_canvas->m_blendStates[EBitmapBlend::BLEND_Masked] );

				g_texturedEffect->apply();

				m_device->updateVertexBuffer( m_canvas->m_quadVB_XYUV, myVerts, sizeof( myVerts ) );
				m_device->setVertexBuffer(  m_canvas->m_quadVB_XYUV );

				m_device->setTopology( rend::EPrimitiveTopology::TriangleStrip );
				m_device->draw( 4 );
			
			}
			*/


		}
		canvas->PopTransform();


	// Render HUD in-game only.
	if( level->bIsPlaying && (level->RndFlags & RND_HUD) )
	{
		// Set screen coords.
		canvas->PushTransform
		(
			gfx::ViewInfo
			( 
				canvas->Clip.X, 
				canvas->Clip.Y, 
				canvas->Clip.Width, 
				canvas->Clip.Height 
			)
		); 
		{
			// Draw each element.
			for( FPainterComponent* P=level->FirstPainter; P; P=P->NextPainter )
				P->RenderHUD( canvas );
		}
		canvas->PopTransform();
	}


#if 0


	
	// Update shader time.
	Canvas->EnableShader( &Canvas->FluShader );
	Canvas->FluShader.SetModeComplex();
	Canvas->FluShader.SetValue1f( Canvas->FluShader.idGameTime, Canvas->LockTime );
	Canvas->FluShader.SetAmbientLight( math::colors::BLACK );



	// Setup clipping area. In game only.
	if( Level->bIsPlaying )
	{
		math::Vector RealFOV	= MasterView.FOV;
		math::Vector CamFOV		= Level->Camera.FOV;

		Canvas->SetClip
		(
			TClipArea
			(
				X,
				Y+H*((RealFOV.y-CamFOV.y)/2.f)/RealFOV.y,
				W,
				H*(CamFOV.y/RealFOV.y)
			)
		);
	}

	// Draw scene.
	Canvas->PushTransform( MasterView );
	{

		// Set ambient light in level.
		if( Level->RndFlags & RND_Lighting )
			//Canvas->FluShader.SetAmbientLight(Level->AmbientLight);
			Canvas->FluShader.SetAmbientLight( Level->m_ambientColors.sampleLinear( Level->m_environmentContext.getCurrentTime().toPercent(), math::colors::BLACK ) );

		/*
		// Render sky zone if any.
		if( Level->Sky && (Level->RndFlags & RND_Backdrop) )
			drawSkyZone( Canvas, Level, MasterView );
			*/

		// Temporary render sky
		if( Level->RndFlags & RND_Backdrop )
		{
			Canvas->PushTransform( TViewInfo( math::Vector(0, 0), 0, math::Vector(1, 1), 1, true, X,
				Y, 
				W, 
				H ) );

			TRenderRect rect;
			rect.Bounds = math::Rect( math::Vector(0.f, 0.f), 1.f );

			rect.Flags = POLY_Unlit | POLY_AlphaGhost;
			rect.TexCoords.min = math::Vector( 1, 1 );
			rect.TexCoords.max = math::Vector( 0, 0 );
			rect.Texture = Level->m_duskBitmap;


			Float progress = Level->m_environmentContext.getCurrentTime().toPercent();
			FBitmap* first = nullptr;
			FBitmap* second = nullptr;
			Float alpha = 0.5f;

			Float midnightTime = envi::TimeOfDay( 0, 0 ).toPercent();
			Float dawnTime = envi::TimeOfDay( 8, 0 ).toPercent();
			Float noonTime = envi::TimeOfDay( 15, 0 ).toPercent();
			Float duskTime = envi::TimeOfDay( 21, 0 ).toPercent();

			if( inRange( progress, midnightTime, dawnTime ) )
			{
				first = Level->m_midnightBitmap;
				second = Level->m_dawnBitmap;
				alpha = (dawnTime - progress) / (dawnTime - midnightTime);
			}
			else if ( inRange( progress, dawnTime, noonTime ) )
			{
				first = Level->m_dawnBitmap;
				second = Level->m_noonBitmap;
				alpha = (noonTime - progress) / (noonTime - dawnTime);
			}
			else if( inRange( progress, noonTime, duskTime ) )
			{
				first = Level->m_noonBitmap;
				second = Level->m_duskBitmap;
				alpha = (duskTime - progress) / (duskTime - noonTime);
			}
			else 
			{
				first = Level->m_duskBitmap;
				second = Level->m_midnightBitmap;
				alpha = (1.f - progress) / (1.f - duskTime);
			}


			rect.Texture = first;
			rect.Color = math::Color( 255, 255, 255, 255 );
			Canvas->DrawRect( rect );

			rect.Texture = second;
			rect.Color = math::Color( 255, 255, 255, 255 * (1-alpha) );
			Canvas->DrawRect( rect );

			Canvas->PopTransform();
		}

		if( !Level->bIsPlaying )
			Level->m_environment.renderInfo( Canvas );
			
		Level->m_environment.render( Canvas, &Level->m_environmentContext );


		// Draw editor grid.
		if( Level->RndFlags & RND_Grid )
			drawGrid( Canvas );	



		// Handle all portals.
		if( Level->RndFlags & RND_Portals )
		{
			drawHalfPlaneMirror( Canvas, Level, MasterView );
			drawHalfPlaneWarp( Canvas, Level, MasterView );
		}

		// Reset lights. And prepare for their collection.
		if( Level->RndFlags & RND_Lighting )
			Canvas->FluShader.ResetLights();

		// Collect all light sources from master view.
		if( Level->RndFlags & RND_Lighting )
			for( FLightComponent* Light=Level->FirstLight; Light; Light=Light->NextLight )
			{
				if( Canvas->View.Bounds.isOverlap( Light->GetLightRect() ) )
					Canvas->FluShader.AddLight
					(
						Light,
						Light->Base->Location,
						Light->Base->Rotation
					);
			}

		// Render all objects in master view.
		for( Int32 i=0; i<Level->RenderObjects.size(); i++ )
			Level->RenderObjects[i]->Render( Canvas );

		// Draw addition-lighting lightmap.
		if( Level->RndFlags & RND_Lighting )
			Canvas->RenderLightmap();

		// Draw debug stuff.
		// !!todo: add special flag for level.
		CDebugDrawHelper::Instance().Render(Canvas);

		// Draw path if need
		if( Level->RndFlags & RND_Paths )
			Level->m_navigator.draw( Canvas );

		// Draw safe frame area.
		if( !Level->bIsPlaying )
			drawSafeFrame( Canvas, Level->Camera );
	}
	Canvas->PopTransform();



	// post-processing
	{
		profile_zone( EProfilerGroup::Render, PostFX );

		// Turn off scene rendering stuff.
		Canvas->SetClip( CLIP_NONE );
		Canvas->SetBlend( BLEND_Regular );
		glPushMatrix();
		glLoadIdentity();

		//Canvas->MasterFBO->Unbind();

		// blur stage
		if( Level->BlurIntensity > math::EPSILON && Level->RndFlags & RND_Effects )
		{
			// apply horiz blur
			Canvas->BlurFBO->Bind( Canvas->ScreenWidth, Canvas->ScreenHeight );
			Canvas->HorizBlurShader.SetTargetWidth( Canvas->ScreenWidth );
			Canvas->HorizBlurShader.SetIntensity( Level->BlurIntensity );

			glBindTexture( GL_TEXTURE_2D, Canvas->MasterFBO->GetTextureId() );

			Canvas->EnableShader(&Canvas->HorizBlurShader);
			{
				drawFullScreenRect();
			}
			Canvas->DisableShader();

			// apply vert blur
			Canvas->MasterFBO->Bind( Canvas->ScreenWidth, Canvas->ScreenHeight );
			Canvas->VertBlurShader.SetTargetHeight( Canvas->ScreenHeight );
			Canvas->VertBlurShader.SetIntensity( Level->BlurIntensity );

			glBindTexture( GL_TEXTURE_2D, Canvas->BlurFBO->GetTextureId() );

			Canvas->EnableShader(&Canvas->VertBlurShader);
			{
				drawFullScreenRect();
			}
			Canvas->DisableShader();
		}

		// color correction stage (final)
		Canvas->MasterFBO->Unbind();

		// Select master scene effect.
		if( !(Level->RndFlags & RND_Effects) )
		{
			// Post-effects are turned off.
			Canvas->FinalShader.SetPostEffect(GNullEffect);
		}
		else if( Level->bIsPlaying )
		{
			// Set effect in game, used interpolator.
			Canvas->FinalShader.SetPostEffect(Level->GFXManager->GetResult());
		}
		else
		{
			// Default for editor
			Canvas->FinalShader.SetPostEffect(Level->Effect);
		}

		Canvas->FinalShader.setAberrationIntensity( Level->AberrationIntensity );
		Canvas->FinalShader.setVignette( Level->m_vignette );
		Canvas->FinalShader.setEnableFXAA( Level->m_enableFXAA );
		Canvas->FinalShader.setRTSize( Canvas->ScreenWidth, Canvas->ScreenHeight );

		// Render FBO to screen.
		glBindTexture( GL_TEXTURE_2D, Canvas->MasterFBO->GetTextureId() );
		Canvas->OldBitmap = nullptr;

		Canvas->EnableShader(&Canvas->FinalShader);
		{
			drawFullScreenRect();
		}
		Canvas->DisableShader();

		glPopMatrix();
	}
#endif



	}
#endif
}