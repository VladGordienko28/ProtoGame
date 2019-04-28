//-----------------------------------------------------------------------------
//	Token.h: Lexer's generated token
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu
{
namespace lexer
{
	/**
	 * A token type
	 */
	enum class ETokenType
	{
		Unknown = 0,
		Symbol,
		Identifier,
		String,
		Integer,
		Float
	};

	/**
	 * A cursor position in the text
	 */
	struct Position
	{
		Int32 line;
		Int32 pos;
	};

	/**
	 * A just parsed token(lexeme)
	 */
	struct Token
	{
	public:
		Token( ETokenType inType, String inText, const Position& inPosition = { 0, 0 } )
			:	type( inType ),
				text( inText ),
				position( inPosition )
		{
			assert( inType == ETokenType::Symbol || inType == ETokenType::Identifier );
		}

		Token( Float inFloat, String inText, const Position& inPosition = { 0, 0 } )
			:	type( ETokenType::Float ),
				text( inText ),
				position( inPosition ),
				constFloat( inFloat )
		{
		}

		Token( Int32 inInteger, String inText, const Position& inPosition = { 0, 0 } )
			:	type( ETokenType::Integer ),
				text( inText ),
				position( inPosition ),
				constInteger( inInteger )
		{
		}

		Token( String inLiteral, String inText, const Position& inPosition = { 0, 0 } )
			:	type( ETokenType::String ),
				text( inText ),
				position( inPosition ),
				constString( inLiteral )
		{
		}

		Token() = default;

		ETokenType getType() const
		{
			return type;
		}

		String getText() const
		{
			return text;
		}

		String getStringConst() const
		{
			assert( type == ETokenType::String );
			return constString;
		}

		Int32 getIntConst() const
		{
			assert( type == ETokenType::Integer );
			return constInteger;
		}

		Float getFloatConst() const
		{
			assert( type == ETokenType::Float || type == ETokenType::Integer );
			return type == ETokenType::Float ? constFloat : constInteger;
		}

		Int32 getLine() const
		{
			return position.line;
		}

		Position getPosition() const
		{
			return position;
		}

	private:
		ETokenType type = ETokenType::Unknown;
		String text = L"";
		Position position = { 0, 0 };

		union
		{
			Int32 constInteger;
			Float constFloat;
		};
		String constString;
	};
}
}