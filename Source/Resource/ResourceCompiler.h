//-----------------------------------------------------------------------------
//	ResourceCompiler.h: An abstract resource compiler
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace res
{
	/**
	 *	An abstract dependency provider
	 */
	class IDependencyProvider: public NonCopyable
	{
	public:
		virtual Text::Ptr getTextFile( String relativeFileName ) const = 0;
		virtual IInputStream::Ptr getBinaryFile( String relativeFileName ) const = 0;
	};

	/**
	 *	A compilation output
	 */
	struct CompilationOutput
	{
	public:
		CompiledResource compiledResource;
	
		Array<String> dependencyFiles;
		UInt64 lastModificationTime = 0;

		Map<ResourceId, String> references; // todo: maybe needs in CompileResource

		Array<String> warningsMsg;
		String errorMsg;

		Bool hasError() const
		{
			return errorMsg.len() > 0 || !compiledResource.isValid();
		}
	};

	/**
	 *	An abstract resource compiler
	 */
	class IResourceCompiler: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<IResourceCompiler>;

		virtual Bool compile( String relativePath, IDependencyProvider& dependencyProvider, 
			CompilationOutput& output ) const = 0;

		virtual Bool isSupportedFile( String relativePath ) const = 0;
	};
}
}