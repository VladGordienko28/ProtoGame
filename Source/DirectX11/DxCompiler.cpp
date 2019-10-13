//-----------------------------------------------------------------------------
//	DxCompiler.cpp: A DirectX shaders compiler
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "DirectX11.h"

#include <d3dcompiler.h>

namespace flu
{
namespace dx11
{
	ShaderCompiler::ShaderCompiler()
	{
	}

	ShaderCompiler::~ShaderCompiler()
	{	
	}

	rend::CompiledShader ShaderCompiler::compile( rend::EShaderType shaderType, Text::Ptr shaderText, const Char* entryPoint,
		String* warnings, String* errors )
	{
		assert( shaderType < rend::EShaderType::ST_MAX );
		assert( shaderText.hasObject() );
		assert( entryPoint && *entryPoint );

		// Dx compiler wants ansi charset
		AnsiString sourceText = wide2AnsiString( shaderText->toString() );

		const AnsiChar* shaderTarget = nullptr;
		switch( shaderType )
		{
			case rend::EShaderType::ST_Vertex:	shaderTarget = "vs_4_1"; break;
			case rend::EShaderType::ST_Pixel:	shaderTarget = "ps_4_1"; break;
			case rend::EShaderType::ST_Compute:	shaderTarget = "cs_4_1"; break;

			default:
				fatal( L"Unknown shader type %d", shaderType );
				break;
		}

		UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG; // todo: not good for production

		DxRef<ID3DBlob> shaderBlob;
		DxRef<ID3DBlob> errorBlob;

		HRESULT result = D3DCompile
		(
			*sourceText,
			sizeof(AnsiChar) * ( sourceText.len() + 1 ),
			nullptr,
			nullptr,
			nullptr,
			*wide2AnsiString( entryPoint ),
			shaderTarget,
			shaderFlags,
			0,
			&shaderBlob,
			&errorBlob
		);

		if( SUCCEEDED( result ) )
		{
			Array<UInt8> shaderData( shaderBlob->GetBufferSize() );
			mem::copy( &shaderData[0], shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize() );

			return rend::CompiledShader( shaderType, shaderData );
		}
		else
		{
			if( errors )
			{
				AnsiString ansiErrors = reinterpret_cast<AnsiChar*>( errorBlob->GetBufferPointer() );
				*errors = ansi2WideString( ansiErrors );
			}

			return rend::CompiledShader( rend::EShaderType::ST_MAX, Array<UInt8>() );
		}
	}
}
}