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
	class Compiler final: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<Compiler>;

		struct Output
		{
		public:
			Array<UInt8> effectBlob;
			Array<String> dependencies;

			String errorMsg;
			Array<String> warningsMsg;
		};

		Compiler( rend::Device* device );
		~Compiler();

		Bool compile( String relativeFileName, IIncludeProvider* includeProvider, Output& output );

	private:
		rend::Device* m_device;

		Compiler() = delete;
	};
}
}