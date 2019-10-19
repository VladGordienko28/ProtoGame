//-----------------------------------------------------------------------------
//	Lexer.h: A cool universal lexer
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu
{
namespace lexer
{
	/**
	 * A lexer config
	 */
	struct LexerConfig
	{
	public:
		Array<String> m_dualSymbols = {}; // todo: replace with Set<>
		Char m_literalString = '\"';
	};

	/**
	 * A lexer
	 */
	class Lexer
	{
	public:
		Lexer( Text::Ptr text, const LexerConfig& config );
		virtual ~Lexer();

		void reset();

		/**
		 *	Grab the next lexeme from the text stream
		 *	@param outToken: result token
		 *	@param allowNegative: interprete '-' as part of numberic value or not
		 *	@return true if token just retrieved or false if eof reached
		 */
		Bool getToken( Token& outToken, Bool allowNegative = true, Bool allowFloat = true );

		/**
		 *	Peek a next token in the text
		 */
		Bool peekToken( Token& outToken, Bool allowNegative = true );

		/**
		 *	Goto to the token's position in the text
		 */
		void gotoToken( const Token& token );

		/**
		 *	Read a literal integer constant
		 */
		Bool readInt( Int32& result, Int32 _default = 0 );

		/**
		 *	Read a literal float constant
		 */
		Bool readFloat( Float& result, Float _default = 0.f );

		/**
		 *	Read a literal string constant
		 */
		Bool readString( String& result, const Char* _default = L"" );

		/**
		 *	Peek the next identifier and return to
		 *	the previous text position
		 */
		String peekIdentifier();

		/**
		 *	Peek the next symbol and return to
		 *	the previous text position
		 */
		String peekSymbol();

		/**
		 *	Read an identifier, and return it text. If lexeme
		 *	is not an identifier returns empty string
		 */
		String getIdentifier();

		/**
		 *	Match the identifier with the next lexeme (should be an identifier),
		 *	if they are matched return true
		 *	otherwise return false and get back to the previous text position.
		 */
		Bool matchIdentifier( const Char* identifier, Bool matchCase = true );

		/**
		 *	Match the symbol with a next lexeme (should be a symbol),
		 *	if they are matched return true
		 *	otherwise return false and unget lexer to the previous text position
		 */
		Bool matchSymbol( const Char* symbol );

		/**
		 *	Return true if EOF is reached
		 */
		Bool isEof();

		/**
		 *	Return current text position
		 */
		Position getPosition() const
		{
			return m_position;
		}

	protected:
		static const SizeT MAX_TOKEN_LENGHT = 256;

		Text::Ptr m_text;
		LexerConfig m_config;
		Position m_position;
		Position m_lastPosition;

		Lexer() = delete;
		Lexer( const Lexer& ) = delete;
		Lexer( Lexer&& ) = delete;
		Lexer& operator=( const Lexer& ) = delete;
		Lexer& operator=( Lexer&& ) = delete;

		Char getRawChar();
		void ungetChar();

	protected:
		// You can handle commentaries here
		// todo: it should be cool to use kind of template polymorphism
		virtual Char getChar()
		{
			return getRawChar();
		}
	};
}
}