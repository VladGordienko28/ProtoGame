//-----------------------------------------------------------------------------
//	Compiler.h: An UI layout compiler
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
	/**
	 * An UI layout compiler
	 */
	class Compiler final: public res::IResourceCompiler
	{
	public:
		Compiler();
		~Compiler();

		Bool compile( String relativePath, res::IDependencyProvider& dependencyProvider, 
			res::CompilationOutput& output ) const override;

		Bool isSupportedFile( String relativePath ) const override
		{
			const String ext = fm::getFileExt( *relativePath );

			return ext == TXT( "layout" );
		}
	};
}
}