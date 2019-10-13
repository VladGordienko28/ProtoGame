//-----------------------------------------------------------------------------
//	FxCompiler.h: A ffx compiler
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ffx
{
	/**
	 * A FFX shaders compiler
	 */
	class Compiler final: public res::IResourceCompiler
	{
	public:
		Compiler( rend::Device* device );
		~Compiler();

		Bool compile( String relativePath, res::IDependencyProvider& dependencyProvider, 
			res::CompilationOutput& output ) const override;

		Bool isSupportedFile( String relativePath ) const override;

	private:
		rend::Device* m_device;

	private:
		// compiler settings
		static const Bool EMIT_LINES = true;
		static const Int32 MAX_INCLUSION_DEPTH = 8;

		struct TechniqueInfo
		{
		public:
			String name;
			String entries[rend::EShaderType::ST_MAX];
		};

		struct Context
		{
		public:
			res::CompilationOutput& output;

			TextWriter writer;
			Array<String> inclusionStack;

			rend::VertexDeclaration vertexDecl;
			Array<TechniqueInfo> techniques;

			Context() = delete;
			
			Context( res::CompilationOutput& inOutput )
				:	output( inOutput ),
					inclusionStack( MAX_INCLUSION_DEPTH )
			{
			}
		};

		Bool parseFile( String relativePath, 
			res::IDependencyProvider& dependencyProvider, Context& context ) const;

		Bool parseDirective( lexer::Lexer& lexer, 
			res::IDependencyProvider& dependencyProvider, Context& context ) const;

		Bool compileVertexDecl( lexer::Lexer& lexer, Context& context ) const;
		Bool compileTechnique( lexer::Lexer& lexer, Context& context ) const;
	};
}
}