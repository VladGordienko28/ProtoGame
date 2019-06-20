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
		struct CompilationError
		{
			String error;

		};



	private:



		void throwError( const Char* fmt, ... );
		void throwWarning( const Char* fmt, ... );

	
	};
}
}