//-----------------------------------------------------------------------------
//	SoundCompiler.h: A sound files compiler
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace aud
{
	/**
	 * A sounds compiler
	 */
	class SoundCompiler final: public res::IResourceCompiler
	{
	public:
		SoundCompiler();
		~SoundCompiler();

		Bool compile( String relativePath, res::IDependencyProvider& dependencyProvider, 
			res::CompilationOutput& output ) const override;

		Bool isSupportedFile( String relativePath ) const override
		{
			const String ext = fm::getFileExt( *relativePath );

			return ext == TXT( "wav" );
		}
	};
}
}