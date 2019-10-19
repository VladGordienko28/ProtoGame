/*=============================================================================
    FrGUIRender.cpp: Editor GUI render.
    Copyright Jul.2016 Vlad Gordienko.
=============================================================================*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
    CGUIRender implementation.
-----------------------------------------------------------------------------*/

//
// GUI render constructor.
//
CGUIRender::CGUIRender()
	:	m_brightness( 1.f )
{
	m_coloredEffect = res::ResourceManager::get<ffx::Effect>( L"System.Shaders.Colored", res::EFailPolicy::FATAL );
	m_texturedEffect = res::ResourceManager::get<ffx::Effect>( L"System.Shaders.Textured", res::EFailPolicy::FATAL );	

	m_solidTech = m_coloredEffect->getTechnique( L"Solid" );
	m_stippleTech = m_coloredEffect->getTechnique( L"Stipple" );

	m_coloredVB = gfx::api::createVertexBuffer( sizeof(math::Vector), 4, rend::EUsage::Dynamic, nullptr, "GUI_Colored_VB" );
	m_texturedVB = gfx::api::createVertexBuffer( sizeof(math::Vector4), 4, rend::EUsage::Dynamic, nullptr, "GUI_Textured_VB" );

	auto samplerState = gfx::api::getSamplerState( { rend::ESamplerFilter::Linear, rend::ESamplerAddressMode::Wrap } );
	m_texturedEffect->setSamplerState( 0, samplerState );
}


//
// GUI render destructor.
//
CGUIRender::~CGUIRender()
{
	m_coloredEffect = nullptr;
	m_texturedEffect = nullptr;

	gfx::api::destroyVertexBuffer( m_coloredVB );
	gfx::api::destroyVertexBuffer( m_texturedVB );
}


//
// Prepare for GUI rendering.
//
void CGUIRender::BeginPaint( gfx::DrawContext& drawContext )
{
	// Set window coords system.
	drawContext.pushViewInfo( 
							gfx::ViewInfo
							( 
								0.f, 
								0.f, 
								drawContext.backbufferWidth(), 
								drawContext.backbufferHeight() 
							) 
						);

	m_drawContext = &drawContext;
}


//
// End GUI rendering.
//
void CGUIRender::EndPaint( gfx::DrawContext& drawContext )
{
	drawContext.popViewInfo();
	drawContext.setScissorArea( rend::ScissorArea::NULL_AREA() );
	m_drawContext = nullptr;
}


//
// Draw a rectangle.
//
void CGUIRender::DrawRegion( TPoint P, TSize S, math::Color Color, math::Color BorderColor, EBrushPattern Pattern )
{
	math::Vector verts[4] = 
	{
		{ Float(P.X), Float(P.Y + S.Height) },
		{ Float(P.X), Float(P.Y)  },
		{ Float(P.X + S.Width), Float(P.Y + S.Height) },
		{ Float(P.X + S.Width), Float(P.Y) }
	};


	m_coloredEffect->setBlendState( rend::BlendState::INVALID );

	// Apply color rescale.
	if( m_brightness != 1.f )
	{
		Color		*= m_brightness;
		BorderColor	*= m_brightness;
	}

	if( Color == BorderColor && Pattern == BPAT_Solid )
	{
		//
		// Case 1: Draw fully solid rect.
		//
		m_coloredEffect->setColor( 0, Color );
		m_coloredEffect->setTechnique( m_solidTech );
		m_coloredEffect->apply();

		gfx::api::updateVertexBuffer( m_coloredVB, verts, sizeof( verts ) );
		gfx::api::setVertexBuffer( m_coloredVB );

		gfx::api::setTopology( rend::EPrimitiveTopology::TriangleStrip );
		gfx::api::draw( 4, 0 );
	}
	else if( Pattern == BPAT_None )
	{
		//
		// Case 2: Just draw border.
		//
/*
		math::Rect Rect;
		Rect.min	= math::Vector( P.X, P.Y );
		Rect.max	= math::Vector( P.X + S.Width, P.Y + S.Height );

		Canvas->DrawLineRect
		( 
			Rect.center(), 
			Rect.size(), 
			0, 
			BorderColor, 
			false
		);*/
	}
	else if( Pattern == BPAT_Solid )
	{
		//
		// Case 3: Draw two overlap rectangles.
		//
		math::Vector borderVerts[4] = 
		{
			{ Float(P.X+1), Float(P.Y + S.Height-1) },
			{ Float(P.X+1), Float(P.Y+1)  },
			{ Float(P.X + S.Width-1), Float(P.Y + S.Height-1) },
			{ Float(P.X + S.Width-1), Float(P.Y+1) }
		};

		// border
		m_coloredEffect->setColor( 0, BorderColor );
		m_coloredEffect->setTechnique( m_solidTech );
		m_coloredEffect->apply();

		gfx::api::updateVertexBuffer( m_coloredVB, verts, sizeof( verts ) );
		gfx::api::setVertexBuffer( m_coloredVB );

		gfx::api::setTopology( rend::EPrimitiveTopology::TriangleStrip );
		gfx::api::draw( 4, 0 );

		// rect
		m_coloredEffect->setColor( 0, Color );
		m_coloredEffect->apply();

		gfx::api::updateVertexBuffer( m_coloredVB, borderVerts, sizeof( borderVerts ) );

		gfx::api::draw( 4, 0 );
	}
	else
	{
		//
		// Case 4: Draw pattern.
		//
		m_coloredEffect->setColor( 0, Color );
		m_coloredEffect->setTechnique( m_stippleTech );
		m_coloredEffect->apply();

		gfx::api::updateVertexBuffer( m_coloredVB, verts, sizeof( verts ) );
		gfx::api::setVertexBuffer( m_coloredVB );

		gfx::api::setTopology( rend::EPrimitiveTopology::TriangleStrip );
		gfx::api::draw( 4, 0 );


/*		Canvas->DrawLineRect
		( 
			Rect.Bounds.center(), 
			Rect.Bounds.size(), 
			0, 
			BorderColor, 
			false
		);*/
	}
}


void CGUIRender::DrawImage( TPoint P, TSize S, TPoint BP, TSize BS, img::Image::Ptr image )
{
	DrawTexture( P, S, BP, BS, image->getHandle(), image->getUSize(), image->getVSize() );
}

void CGUIRender::DrawTexture( TPoint P, TSize S, TPoint BP, TSize BS, rend::Texture2DHandle image, UInt32 width, UInt32 height )
{
	Float invUSize = 1.f / width;
	Float invVSize = 1.f / height;

	// Texture coords.
	math::Rect tc;
	tc.min.x	= BP.X * invUSize;
	tc.min.y	= BP.Y * invVSize;
	tc.max.x	= ( BP.X + BS.Width )  * invUSize;
	tc.max.y	= ( BP.Y + BS.Height ) * invVSize;

	math::Vector4 verts[4] = 
	{
		{ Float(P.X), Float(P.Y + S.Height), tc.min.x, tc.max.y },
		{ Float(P.X), Float(P.Y), tc.min.x, tc.min.y  },
		{ Float(P.X + S.Width), Float(P.Y + S.Height), tc.max.x, tc.max.y },
		{ Float(P.X + S.Width), Float(P.Y), tc.max.x, tc.min.y }
	};

	m_texturedEffect->setColor( 0, math::colors::WHITE*m_brightness );
	m_texturedEffect->setSRV( 0, gfx::srvOf( image ) );

	m_texturedEffect->apply();

	gfx::api::updateVertexBuffer( m_texturedVB, verts, sizeof( verts ) );
	gfx::api::setVertexBuffer( m_texturedVB );

	gfx::api::setTopology( rend::EPrimitiveTopology::TriangleStrip );
	gfx::api::draw( 4, 0 );
}


//
// Draw a GUI text.
//
void CGUIRender::DrawText( TPoint P, const Char* Text, Int32 Len, math::Color Color, fnt::Font::Ptr Font )
{
	if( m_brightness != 1.f )
		Color *= m_brightness;

	m_textDrawer.batchText( Text, Len, Font, Color, math::Vector(P.X, P.Y) );
	m_textDrawer.flush();
}


//
// Set render clipping area.
//
void CGUIRender::SetClipArea( TPoint P, TSize S )
{
	m_drawContext->setScissorArea( { P.Y, P.X, P.Y + S.Height, P.X + S.Width } );
}


//
// Set a brightness for gui elements
// rendering.
//
void CGUIRender::SetBrightness( Float Brig )
{
	m_brightness = clamp( Brig, 0.f, 1.f );
}


/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/
