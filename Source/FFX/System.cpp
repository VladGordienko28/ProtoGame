//-----------------------------------------------------------------------------
//	System.cpp: A FFX system implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "FFX.h"

namespace flu
{
namespace ffx
{
	System::System()
		:	m_device( nullptr )
	{
	}

	System::~System()
	{
	}

	void System::init( rend::Device* device, String shadersDirectory )
	{
		assert( device );
		assert( !m_device );
		assert( fm::directoryExists( *shadersDirectory ) );
		assert( fm::isAbsoluteFileName( *shadersDirectory ) );

		m_device = device;
		m_directory = shadersDirectory;
	}

	void System::shutdown()
	{
		m_device = nullptr;
		m_directory = L"";
	}

	Effect::Ptr System::getEffect( String effectName )
	{
		assert( m_device );

		// add cache here!!

		String fileName = m_directory + effectName + SHADER_EXTENSION;

		if( !fm::fileExists( *fileName ) )
		{
			error( L"Shader %s is not found", *fileName );
			return nullptr;
		}

		// load as text file
		Text::Ptr sourceText = fm::readTextFile( *fileName );
		assert( sourceText.hasObject() );



#if 0


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
#endif




	}



}
}