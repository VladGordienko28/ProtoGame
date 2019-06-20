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
		ShaderCompiler();
		~ShaderCompiler();

		rend::CompiledShader compile( rend::EShaderType shaderType, Text::Ptr shaderText, const Char* entryPoint,
			String* warnings = nullptr, String* errors = nullptr ) override;
	};
}
}