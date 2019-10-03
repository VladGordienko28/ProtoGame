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
		Compiler( rend::Device* device )
			:	m_device( device )
		{
			assert( device );
		}

		~Compiler()
		{
		
		}

		Bool compile( String relativePath, res::IDependencyProvider& dependencyProvider, 
			res::CompilationOutput& output ) const override
;

		Bool isSupportedFile( String relativePath ) const override
		{
			String ext = fm::getFileExt( *relativePath );
			return ext == TEXT("ffx");
		}

	private:
		rend::Device* m_device;


/*
		using UPtr = UniquePtr<Compiler>;

		struct Output
		{
		public:
			Array<UInt8> effectBlob;
			Array<String> dependencies;

			String errorMsg;
			Array<String> warningsMsg;
		};



		Bool compile( String relativeFileName, IIncludeProvider* includeProvider, Output& output );

	private:
		rend::Device* m_device;

		Compiler() = delete;*/
	};
}
}