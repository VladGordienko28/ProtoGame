#include "pch.h"
#include "App.h"

#include <ppltasks.h>

using namespace XBoxGame;

//using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

namespace flu
{

	class CDirectX11Canvas: public CCanvas
	{
	public:
		// COpenGLCanvas interface.
		CDirectX11Canvas( rend::Device* device, gfx::DrawContext& drawContext )
		:	CCanvas( drawContext ),
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

		~CDirectX11Canvas()
		{
		m_coloredEffect = nullptr;
		m_texturedEffect = nullptr;
		

		m_device->destroyVertexBuffer( m_quadVB_XY );
		m_device->destroyVertexBuffer( m_quadVB_XYUV );
		m_device->destroyVertexBuffer( m_polyVB_XY );
		m_device->destroyVertexBuffer( m_polyVB_XYUV );

		m_device->destroyIndexBuffer( m_polyIB );
		}

		// CCanvas interface.
	void DrawPoly( const TRenderPoly& poly )
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

	void DrawRect( const TRenderRect& rect )
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

	void DrawList( const TRenderList& list )
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


		ffx::Effect::Ptr m_coloredEffect;
		ffx::Effect::Ptr m_texturedEffect;

		ffx::TechniqueId m_solidTech;
		ffx::TechniqueId m_stippleTech;

	private:

		// not good, but well for now
		rend::Device* m_device;


		rend::VertexBufferHandle m_quadVB_XY;
		rend::VertexBufferHandle m_quadVB_XYUV;

		rend::VertexBufferHandle m_polyVB_XY;
		rend::VertexBufferHandle m_polyVB_XYUV;

		rend::IndexBufferHandle m_polyIB;

		rend::SamplerStateId m_samplerNearest;
		rend::SamplerStateId m_samplerLinear;

		rend::BlendStateId m_normalBlendState;
		rend::BlendStateId m_alphaBlendState;
		rend::BlendStateId m_translucentBlendState;
	};
}



// The main function is only used to initialize our IFrameworkView class.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
	return ref new App();
}

App::App() :
	m_windowClosed(false),
	m_windowVisible(true)
{
}

// The first method called when the IFrameworkView is being created.
void App::Initialize(CoreApplicationView^ applicationView)
{
	// Register event handlers for app lifecycle. This example includes Activated, so that we
	// can make the CoreWindow active and start rendering on the window.
	applicationView->Activated +=
		ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

	CoreApplication::Suspending +=
		ref new EventHandler<SuspendingEventArgs^>(this, &App::OnSuspending);

	CoreApplication::Resuming +=
		ref new EventHandler<Platform::Object^>(this, &App::OnResuming);

	// At this point we have access to the device. 
	// We can create the device-dependent resources.
	//m_deviceResources = std::make_shared<DX::DeviceResources>();//////////////////////////////

	// Initial logging
	//flu::LogManager::instance().addCallback( new flu::LogCallbackFile( L"XBox.log" ) );

#if FLU_DEBUG
	flu::LogManager::instance().addCallback( new flu::LogCallbackDebug( true ) );

	if( !IsDebuggerPresent() )
	{
		flu::LogManager::instance().addCallback( new flu::LogCallbackConsole() );
	}
#endif

	// Say hello to user.
	info( L"=========================" );
	info( L"=    Fluorine Engine    =" );
	info( L"=      %s        =", FLU_VERSION );
	info( L"=========================" );
	info( L"" );

	flu::ConfigManager::create( fm::getCurrentDirectory(), L"XBox" );
	m_inputDevice = new flu::xb::XInputDevice();

}

// Convert from dip to pixels
void dipsToPixels( float inW, float inH, int& outW, int& outH )
{
	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
/*
	float dpi = currentDisplayInformation->LogicalDpi;

	static const float dipsPerInch = 96.0f;

	outW = flu::math::floor( inW * dpi / dipsPerInch + 0.5f );
	outH = flu::math::floor( inH * dpi / dipsPerInch + 0.5f );*/

	outW = currentDisplayInformation->ScreenWidthInRawPixels;
	outH = currentDisplayInformation->ScreenHeightInRawPixels;
}


class MyTestInt: public in::InputClient
{
public:
	MyTestInt( aud::Device* dev, String sndName )
	{
		device = dev;
		sound = res::ResourceManager::get<aud::Sound>( L"Experimental.Bola", res::EFailPolicy::FATAL );
	}


	Bool onGamepadDown( in::GamepadId id, in::EGamepadButton button )
	{
		if( button == in::EGamepadButton::GB_A )
		{
			device->playSFX( sound->getHandle(), 1.f, 1.5f );
			return true;
		}
	}

private:
	aud::Device* device;
	aud::Sound::Ptr sound;
};

MyTestInt* g_testInt;


// Called when the CoreWindow object is created (or re-created).
void App::SetWindow(CoreWindow^ window)
{
	window->SizeChanged += 
		ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &App::OnWindowSizeChanged);

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);

	window->Closed += 
		ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDisplayContentsInvalidated);

	//----------------------------------------------
	assert( !m_renderDevice.hasObject() );
	assert( !m_audioDevice.hasObject() );

	int realW, realH;
	dipsToPixels( window->Bounds.Width,  window->Bounds.Height, realW, realH );

	m_renderDevice = new flu::dx11::Device( reinterpret_cast<IUnknown*>( window ), realW, realH, false );
	m_audioDevice = new flu::xa2::Device();

	m_world = new World( m_renderDevice.get(), m_audioDevice.get(), m_inputDevice.get() );

	g_testInt = new MyTestInt( m_audioDevice.get(), L"" );
	m_inputDevice->addClient( g_testInt );

	m_grid = new flu::gfx::GridDrawer( flu::math::colors::GREEN, 515 );
	m_textDrawer = new flu::gfx::TextDrawer();

	m_font = res::ResourceManager::get<flu::fnt::Font>( L"Fonts.Consolas_9", flu::res::EFailPolicy::FATAL );
	//m_deviceResources->SetWindow(window);

	m_canvas = new CDirectX11Canvas( m_renderDevice.get(), m_world->drawContext() );



}

// Initializes scene resources, or loads a previously saved app state.
void App::Load(Platform::String^ entryPoint)
{
	//if (m_main == nullptr)
	{
		//m_main = std::unique_ptr<XBoxGameMain>(new XBoxGameMain(m_deviceResources));
	}
}

static UInt64 g_lastTickTime;

// This method is called after the window becomes active.
void App::Run()
{
	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

			UInt64 thisTickTime = flu::time::cycles64();


			//m_main->Update();

			//m_renderDevice->beginFrame();

		profile_begin_frame();
			using namespace flu;

			// hack for profile switching
			if( flu::profile::isDefaultProfiler() )
			{
				m_inputDevice->update( 0.f ); // real dt!
			}
			else
			{
				profile_zone( EProfilerGroup::General, Input );
				m_inputDevice->update( 0.f ); // real dt!	
			}


			static math::Vector pos = {0, 0};
			static math::Angle rot = 0;



		//if( dwResult == ERROR_SUCCESS  )
		{
				pos += m_inputDevice->getGamepadStick( 0, 0 );

				rot += m_inputDevice->getGamepadTrigger( 0, 1 ) * 0.1f;
				rot -= m_inputDevice->getGamepadTrigger( 0, 0 ) * 0.1f;


		}



			gfx::ViewInfo view( pos, rot, {64, 32}, 1, false, 0, 0, m_world->drawContext().backbufferWidth(), m_world->drawContext().backbufferHeight() );

			m_world->onBeginUpdate();

			m_world->drawContext().pushViewInfo( view );
/*
		ViewInfo( const math::Vector& inLocation, math::Angle inRotation, const math::Vector& inFov, Float inZoom, 
			Bool inMirage, Float inX, Float inY, Float inWidth, Float inHeight );
*/


			m_grid->render( view );

			m_world->drawContext().popViewInfo();

			gfx::ViewInfo sview( 0, 0, m_world->drawContext().backbufferWidth(), m_world->drawContext().backbufferHeight() );


			m_world->drawContext().pushViewInfo( sview );

			m_textDrawer->batchText( L"Azazazaazaza lalki, Vlad the Best!", m_font, math::colors::WHITE, {25, 50} );

			double time = flu::time::cyclesToMs( thisTickTime - g_lastTickTime );
			int fps = int(1000.0 / time);
			m_textDrawer->batchText( String::format( L"FPS: %d, %.4f ms", fps, time ), m_font, math::colors::WHITE, {25, 75} );

			m_textDrawer->flush();

			m_world->drawContext().popViewInfo();

			//m_renderDevice->endFrame( true );
			m_world->onEndUpdate(m_canvas.get());

			{
				profile_zone( EProfilerGroup::General, ResourceManager );
				res::ResourceManager::update(); // todo: add timeout
			}

			/*
			if (m_main->Render())
			{
				m_deviceResources->Present();
			}*/

			threading::sleep( 5 );
			profile_end_frame();

			g_lastTickTime = thisTickTime;
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

// Required for IFrameworkView.
// Terminate events do not cause Uninitialize to be called. It will be called if your IFrameworkView
// class is torn down while the app is in the foreground.
void App::Uninitialize()
{
}

// Application lifecycle event handlers.

void App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	// Run() won't start until the CoreWindow is activated.
	CoreWindow::GetForCurrentThread()->Activate();
}

void App::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	// Save app state asynchronously after requesting a deferral. Holding a deferral
	// indicates that the application is busy performing suspending operations. Be
	// aware that a deferral may not be held indefinitely. After about five seconds,
	// the app will be forced to exit.
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	::concurrency::create_task([this, deferral]()
	{
       // m_deviceResources->Trim();

		// Insert your code here.

		deferral->Complete();
	});
}

void App::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	// Restore any data or state that was unloaded on suspend. By default, data
	// and state are persisted when resuming from suspend. Note that this event
	// does not occur if the app was previously terminated.

	// Insert your code here.
}

// Window event handlers.

void App::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{

	int realW, realH;
	dipsToPixels( sender->Bounds.Width,  sender->Bounds.Height, realW, realH );

	m_renderDevice->resize( realW, realH );
	m_world->onResize( realW, realH, false );

	//m_deviceResources->SetLogicalSize(Size());
	//m_main->CreateWindowSizeDependentResources();
}

void App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}

void App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

// DisplayInformation event handlers.

void App::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	// Note: The value for LogicalDpi retrieved here may not match the effective DPI of the app
	// if it is being scaled for high resolution devices. Once the DPI is set on DeviceResources,
	// you should always retrieve it using the GetDpi method.
	// See DeviceResources.cpp for more details.
	//m_deviceResources->SetDpi(sender->LogicalDpi);
	//m_main->CreateWindowSizeDependentResources();
}

void App::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	//m_deviceResources->SetCurrentOrientation(sender->CurrentOrientation);
	//m_main->CreateWindowSizeDependentResources();
}

void App::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	//m_deviceResources->ValidateDevice();
}