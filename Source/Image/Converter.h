//-----------------------------------------------------------------------------
//	Converter.h: An image files converter(compiler)
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace img
{
	/**
	 * An images converter(compiler)
	 */
	class Converter final: public res::IResourceCompiler
	{
	public:
		Converter();
		~Converter();

		Bool compile( String relativePath, res::IDependencyProvider& dependencyProvider, 
			res::CompilationOutput& output ) const override
;
		Bool isSupportedFile( String relativePath ) const override
		{
			const String ext = fm::getFileExt( *relativePath );

			return ext == TEXT( "bmp" ) || ext == TEXT( "png" ) ||
				ext == TEXT( "tga" );
		}
	};
}
}