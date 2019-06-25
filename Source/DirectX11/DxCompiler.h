//-----------------------------------------------------------------------------
//	DxCompiler.h: A DirectX shaders compiler
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace dx11
{
	/**
	 *	A DirectX shaders compiler
	 */
	class ShaderCompiler final: public rend::ShaderCompiler
	{
	public:
		static const constexpr Char* COMPILER_MARK = TEXT( "hlsl_4.1" );

		ShaderCompiler();
		~ShaderCompiler();

		rend::CompiledShader compile( rend::EShaderType shaderType, Text::Ptr shaderText, const Char* entryPoint,
			String* warnings = nullptr, String* errors = nullptr ) override;

		String compilerMark() const override
		{
			return COMPILER_MARK;
		}
	};
}
}