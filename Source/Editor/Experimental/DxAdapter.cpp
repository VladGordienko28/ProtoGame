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
		newWidth = clamp( newWidth,  1, 1920 );
		newHeight = clamp( newHeight, 1, 1080 );

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
	{/*
		math::FloatColor drawColor = math::Color::hsl2rgb( GPlat->Now()*3, 255, 128, 255 );

		m_device->updateConstantBuffer( m_canvas->m_colorCB, &drawColor, sizeof( drawColor ) );

		m_device->setVertexShader( m_canvas->m_coloredVs );
		m_device->setPixelShader( m_canvas->m_coloredPs );

		math::Vector points[4] =
		{
			{ +0.25f, -0.25f },
			{ -0.25f, -0.25f },

			{ +0.25f, +0.25f },
			{ -0.25f, +0.25f },
		};

		m_device->updateVertexBuffer( m_canvas->m_quadVB_XY, points, sizeof( points ) );

		m_device->setVertexBuffer( m_canvas->m_quadVB_XY );
		m_device->setConstantBuffers( rend::EShaderType::Vertex, 0, 1, &m_canvas->m_transformCB );
		m_device->setConstantBuffers( rend::EShaderType::Pixel, 1, 1, &m_canvas->m_colorCB );

		m_device->setTopology( rend::EPrimitiveTopology::TriangleStrip );
		m_device->draw( 4 );

		*/
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




#if 0
	rend::Device::UPtr m_renderDevice;

	rend::VertexBufferHandle vertexBuffer;
	rend::ShaderHandle vertexShader;
	rend::ShaderHandle pixelShader;
	rend::ConstantBufferHandle shadeParamsCB;
	rend::ConstantBufferHandle transformCB;
	rend::Texture2DHandle texture;
#endif



#if 0

	m_renderDevice = new dx11::Device( hWnd, 800, 600, false );

	math::Color texData[32][32];

	for( int i = 0; i < 32; ++i )
		for( int j = 0; j < 32; ++j )
		{
			texData[i][j].r =
				texData[i][j].g =
				texData[i][j].b = ((i ^ j) % 2) != 0 ? 255 : 128;
			texData[i][j].a = 255;
		}


	texture = m_renderDevice->createTexture2D( rend::EFormat::RGBA8_UNORM, 32, 32, 1, rend::EUsage::Immutable,
		texData, "MyCheckerTexture" );


	// --------------------------------------------------------------------------------------
	String shaderText =
L"//-----------------------------------------------------------------------------\n"
"//	FlatShader.cpp: Flat color shading shader\n"
"//	Created by Vlad Gordienko, 2018\n"
"//-----------------------------------------------------------------------------\n"
"\n"
"cbuffer Transforms\n"
"{\n"
"	matrix projectionMatrix;\n"
"}\n"
"\n"
"cbuffer ShadeParams\n"
"{\n"
"	float4 color;\n"
"}\n"
"\n"
"// See: flu::render::VertexXY\n"
"struct VS_INPUT\n"
"{\n"
"	float2 position		: POSITION;\n"
"	float2 texCoord		: TEXCOORD0;\n"
"};\n"
"\n"
"struct VS_OUTPUT\n"
"{\n"
"	float4 position		: SV_Position;\n"
"	float2 texCoord		: TEXCOORD0;\n"
"	float4 color		: COLOR;\n"
"};\n"
"\n"
"/**\n"
" *	Vertex Shader entry point\n"
" */\n"
"VS_OUTPUT vs_main( in VS_INPUT input )\n"
"{\n"
"	VS_OUTPUT output;\n"
"\n"
"	output.position = mul( float4( input.position, 0.f, 1.f ), projectionMatrix );\n"
"	output.color = color;\n"
"	output.texCoord = input.position * 2.5 + 0.0 * input.texCoord;\n"
"\n"
"	return output;\n"
"}\n"
"Texture2D albedo;\n"
"SamplerState albedoSampler;\n"
"\n"
"/**\n"
" *	Pixel Shader entry point\n"
" */\n"
"float4 ps_main( in VS_OUTPUT input ) : SV_Target\n"
"{\n"
"	return input.color * albedo.Sample( albedoSampler, input.texCoord );\n"
"}";
	
	rend::ShaderCompiler::UPtr compiler = new dx11::ShaderCompiler();
	Text::Ptr text = new Text( shaderText );

	String Error;

	auto result1 = compiler->compile( rend::EShaderType::Vertex, text, L"vs_main", nullptr, &Error );
	auto result2 = compiler->compile( rend::EShaderType::Pixel, text, L"ps_main", nullptr, &Error );	

	assert( result1.isValid() && result2.isValid() );

	rend::VertexDeclaration decl( L"TrololoVertexDeclaration" );
	decl.addElement( { rend::EFormat::RG32_F, rend::EVertexElementUsage::Position, 0, 0 } );
	decl.addElement( { rend::EFormat::RG32_F, rend::EVertexElementUsage::TexCoord, 0, 8 } );

	vertexShader = m_renderDevice->createVertexShader( result1, decl, "FlatVertexShader" );
	pixelShader = m_renderDevice->createPixelShader( result2, "FlatPixelShader" );

	math::FloatColor drawColor = math::colors::CORAL;
	shadeParamsCB = m_renderDevice->createConstantBuffer( sizeof(math::FloatColor), rend::EUsage::Dynamic, 
		&drawColor, "ShaderParams Constant Buffer" );


	Float matrix[16] = 
	{
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};

	transformCB = m_renderDevice->createConstantBuffer( sizeof(matrix), rend::EUsage::Immutable, 
		matrix, "Transform Constant buffer" );


	math::Vector vertexBufferData[6] =
	{
		{ 0.f, 0.f },	{ 0.f, 0.f },
		{ 0.5f, 0.5f },	{ 1.f, 0.f },
		{ 1.f, 0.5f },	{ 1.f, 1.f }
	};

	vertexBuffer = m_renderDevice->createVertexBuffer( sizeof(math::Vector) * 2, 3,
		rend::EUsage::Dynamic, vertexBufferData, "My Amazing VertexBuffer" );

/*
		EFormat format = EFormat::Unknown;
		EVertexElementUsage usage = EVertexElementUsage::Position;
		UInt32 usageIndex = 0;
		UInt32 offset = 0;
*/


	// --------------------------------------------------------------------------------------



#endif

#if 0
	m_renderDevice->destroyVertexBuffer( vertexBuffer );
	m_renderDevice->destroyVertexShader( vertexShader );
	m_renderDevice->destroyPixelShader( pixelShader );
	m_renderDevice->destroyConstantBuffer( shadeParamsCB );
	m_renderDevice->destroyConstantBuffer( transformCB );
	m_renderDevice->destroyTexture2D( texture );



	m_renderDevice = nullptr;
#endif


#if 0
				m_renderDevice->beginFrame();


				///////////////////////////////////////////////////////////////////////////

					m_renderDevice->setVertexShader( vertexShader );
					m_renderDevice->setPixelShader( pixelShader );
					m_renderDevice->setVertexBuffer( vertexBuffer );

					m_renderDevice->setConstantBuffers( rend::EShaderType::Vertex, 0, 1, &transformCB );
					m_renderDevice->setConstantBuffers( rend::EShaderType::Vertex, 1, 1, &shadeParamsCB );

					m_renderDevice->setTopology( rend::EPrimitiveTopology::TriangleList );


	rend::SamplerStateId mySampler = m_renderDevice->getSamplerState( { rend::ESamplerFilter::Point, rend::ESamplerAddressMode::Clamp } );
	m_renderDevice->setSamplerStates( rend::EShaderType::Pixel, 0, 1, &mySampler );


	auto resourceView = m_renderDevice->getShaderResourceView( texture );
	m_renderDevice->setSRVs( rend::EShaderType::Pixel, 0, 1, &resourceView );

					math::FloatColor drawColor =
					{
						math::sin( GPlat->Now() * 2.7f ) * 0.5f + 0.5f,
						math::sin( GPlat->Now() * 2.1f ) * 0.5f + 0.5f,
						math::sin( GPlat->Now() * 0.9f ) * 0.5f + 0.5f,
						1.f
					};

					m_renderDevice->updateConstantBuffer( shadeParamsCB, &drawColor, sizeof( math::FloatColor ) );

					m_renderDevice->draw( 3 );

				m_renderDevice->endFrame( false );
#endif


	CDirectX11Canvas::CDirectX11Canvas( CDirectX11Render* render, rend::Device* device )
		:	m_render( render ),
			m_device( device ),
			m_lockTime( 0.f )
	{
		// load all shaders
		{
			Text::Ptr shaderText = fm::readTextFile( L"Shaders\\Colored.hlsl" );
			assert( shaderText.hasObject() );

			String errorMsg;
			String warnMsg;
			rend::ShaderCompiler::UPtr compiler = new dx11::ShaderCompiler();

			auto compiledPS = compiler->compile( rend::EShaderType::Pixel, shaderText, L"psMain", &warnMsg, &errorMsg );
			if( !compiledPS.isValid() )
				fatal( L"Unable to compile hlsl shader with error: \n%s", *errorMsg );

			auto compiledVS = compiler->compile( rend::EShaderType::Vertex, shaderText, L"vsMain", &warnMsg, &errorMsg );
			if( !compiledVS.isValid() )
				fatal( L"Unable to compile hlsl shader with error: \n%s", *errorMsg );

			rend::VertexDeclaration declXY( L"VertexDecl_XY" );
			declXY.addElement( { rend::EFormat::RG32_F, rend::EVertexElementUsage::Position, 0, 0 } );

			m_coloredVs = m_device->createVertexShader( compiledVS, declXY, "Colored.hlsl" );
			m_coloredPs = m_device->createPixelShader( compiledPS, "Colored.hlsl" );
		}



		// allocate required constant buffers
		m_transformCB = m_device->createConstantBuffer( 16 * sizeof( Float ), rend::EUsage::Dynamic, nullptr, "TransformCB" );
		m_colorCB = m_device->createConstantBuffer( sizeof( math::FloatColor ), rend::EUsage::Dynamic, nullptr, "ColorCB" );


		// allocate required vertex buffers
		m_quadVB_XY = m_device->createVertexBuffer( sizeof( math::Vector ), 4, rend::EUsage::Dynamic, nullptr, "QuadVB_XY" );


	}

	CDirectX11Canvas::~CDirectX11Canvas()
	{
		m_device->destroyVertexShader( m_coloredVs );
		m_device->destroyPixelShader( m_coloredPs );
		
		m_device->destroyVertexBuffer( m_quadVB_XY );

		m_device->destroyConstantBuffer( m_transformCB );
		m_device->destroyConstantBuffer( m_colorCB );
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

		m_device->updateConstantBuffer( m_transformCB, matrix, sizeof( matrix ) );
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

		const math::FloatColor drawColor = color;
			
		m_device->updateConstantBuffer( m_colorCB, &drawColor, sizeof( drawColor ) );

		m_device->setVertexShader( m_coloredVs );
		m_device->setPixelShader( m_coloredPs );

		m_device->updateVertexBuffer( m_quadVB_XY, Verts, sizeof( Verts ) );

		m_device->setVertexBuffer( m_quadVB_XY );
		m_device->setConstantBuffers( rend::EShaderType::Vertex, 0, 1, &m_transformCB );
		m_device->setConstantBuffers( rend::EShaderType::Pixel, 1, 1, &m_colorCB );

		m_device->setTopology( rend::EPrimitiveTopology::LineStrip );
		m_device->draw( 2 );

	}

	void CDirectX11Canvas::DrawPoly( const TRenderPoly& poly )
	{
	}

	void CDirectX11Canvas::DrawRect( const TRenderRect& rect )
	{
		if( rect.Flags & POLY_FlatShade )
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

			const math::FloatColor drawColor = rect.Color;
			
			m_device->updateConstantBuffer( m_colorCB, &drawColor, sizeof( drawColor ) );

			m_device->setVertexShader( m_coloredVs );
			m_device->setPixelShader( m_coloredPs );

			m_device->updateVertexBuffer( m_quadVB_XY, Verts, sizeof( Verts ) );

			m_device->setVertexBuffer( m_quadVB_XY );
			m_device->setConstantBuffers( rend::EShaderType::Vertex, 0, 1, &m_transformCB );
			m_device->setConstantBuffers( rend::EShaderType::Pixel, 1, 1, &m_colorCB );

			m_device->setTopology( rend::EPrimitiveTopology::TriangleStrip );
			m_device->draw( 4 );
		}
	}

	void CDirectX11Canvas::DrawList( const TRenderList& list )
	{
	}
}