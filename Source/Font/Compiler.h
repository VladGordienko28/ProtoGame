//-----------------------------------------------------------------------------
//	Compiler.h: A font files compiler
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace fnt
{
	/**
	 * A fonts compiler
	 */
	class Compiler final: public res::IResourceCompiler
	{
	public:
		Compiler() = default;
		~Compiler() = default;

		Bool compile( String relativePath, res::IDependencyProvider& dependencyProvider, 
			res::CompilationOutput& output ) const override;
;
		Bool isSupportedFile( String relativePath ) const override
		{
			const String ext = fm::getFileExt( *relativePath );

			return ext == TXT( "ffnt" );
		}
	};
}
}