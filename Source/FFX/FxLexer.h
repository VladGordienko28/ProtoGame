//-----------------------------------------------------------------------------
//	FxLexer.h: A ffx source lexer
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ffx
{
	/**
	 * A fluorine effect lexer
	 */
	class Lexer: public lexer::Lexer
	{
	public:
		Lexer( Text::Ptr text );
		~Lexer();

	private:
		Char getChar() override;

		static const lexer::LexerConfig getDefaultConfig();
	};
}
}