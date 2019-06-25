//-----------------------------------------------------------------------------
//	FxCompiler.cpp: A ffx compiler implementation
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "FFX.h"

namespace flu
{
namespace ffx
{
	/**
	 * A helper class for code generation
	 */
	class CodeEmitter: public IOutputStream
	{
	public:
		CodeEmitter( Array<UInt8>& blob )
			:	m_blob( blob ),
				m_position( 0 )
		{
			m_blob.empty();
		}

		SizeT size() const override
		{
			return m_blob.size();
		}

		SizeT tell() const override
		{
			return m_position;
		}

		void* reserveData( SizeT numBytes ) override
		{
			if( m_position + numBytes > SizeT( m_blob.size() ) )
			{
				m_blob.setSize( static_cast<Int32>( m_position + numBytes ) );
			}

			void* result = &m_blob[static_cast<Int32>( m_position )];
			m_position += numBytes;

			return result;
		}

		void writeData( const void* buffer, SizeT numBytes ) override
		{
			if( m_position + numBytes > SizeT( m_blob.size() ) )
			{
				m_blob.setSize( static_cast<Int32>( m_position + numBytes ) );
			}

			mem::copy( &m_blob[m_position], buffer, numBytes );
			m_position += numBytes;
		}

		void seek( SizeT offset ) override
		{
			assert( offset > 0 && offset < SizeT( m_blob.size() ) );
			m_position = offset;
		}

	private:
		Array<UInt8>& m_blob;
		SizeT m_position;

		CodeEmitter() = delete;
	};

	Compiler::Compiler( rend::Device* device )
		:	m_device( device )
	{
		assert( m_device );
	}

	Compiler::~Compiler()
	{
	}

	Bool Compiler::compile( String relativeFileName, IIncludeProvider* includeProvider, Compiler::Output& output )
	{
		assert( includeProvider );
		assert( relativeFileName );

		rend::ShaderCompiler::UPtr apiCompiler = m_device->createCompiler();
		assert( apiCompiler );

		CodeEmitter emitter( output.effectBlob );

		// emit header
		emitter << String( FFX_VERSION );
		emitter << apiCompiler->compilerMark();

		// preprocess
		PreprocessorInput preprocessorInput;
		preprocessorInput.relativeFileName = relativeFileName;
		preprocessorInput.includeProvider = includeProvider;
		preprocessorInput.emitLines = true;

		PreprocessorOutput preprocessorOutput;

		if( !preprocess( preprocessorInput, preprocessorOutput, &output.errorMsg ) )
		{
			error( L"Unable to preprocess effect \"%s\" with error: \n%s", *relativeFileName, *output.errorMsg );
			return false;
		}

		// pixel shader compilation
		rend::CompiledShader compiledPS = apiCompiler->compile( rend::EShaderType::Pixel, preprocessorOutput.source, Effect::PS_ENTRY, nullptr, &output.errorMsg );
		if( !compiledPS.isValid() )
		{
			error( L"Unable to compile api pixel shader with error: \n%s", *output.errorMsg );
			return false;
		}

		emitter << compiledPS;

		// vertex shader compilation
		rend::CompiledShader compiledVS = apiCompiler->compile( rend::EShaderType::Vertex, preprocessorOutput.source, Effect::VS_ENTRY, nullptr, &output.errorMsg );
		if( !compiledVS.isValid() )
		{
			error( L"Unable to compile api vertex shader with error: \n%s", *output.errorMsg );
			return false;
		}

		emitter << compiledVS;

		output.dependencies = preprocessorOutput.dependencies;
		return true;
	}
}
}