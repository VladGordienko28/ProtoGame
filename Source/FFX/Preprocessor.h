//-----------------------------------------------------------------------------
//	Preprocessor.h: A ffx compiler preprocessor
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ffx
{
	/**
	 *	An abstract include provider
	 */
	class IIncludeProvider: public NonCopyable
	{
	public:
		virtual Text::Ptr getInclude( String relativeFileName ) const = 0;
	};

	/**
	 *	A preprocessor input
	 */
	struct PreprocessorInput
	{
	public:
		String relativeFileName;
		Map<String, String> defines;
		IIncludeProvider* includeProvider = nullptr;
		Bool emitLines = true;
	};

	/**
	 *	A preprocessor output
	 */
	struct PreprocessorOutput
	{
		Text::Ptr source;
		Array<String> dependencies;
	};

	/**
	 *	A FFX preprocessor
	 */
	extern Bool preprocess( const PreprocessorInput& input, PreprocessorOutput& output, String* error = nullptr );
}
}