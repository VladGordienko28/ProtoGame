//-----------------------------------------------------------------------------
//	Evaluator.h: An universal expression evaluator
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace eval
{
	/**
	 *	A collection of operators for any supported types
	 */
	template<typename T> class TypeInfo
	{
	public:
		using TYPE = T;

		struct Unary
		{
			using Function = T(*)( T );
			Function function; 
		};

		struct Binary
		{
			using Function = T(*)( T, T );
			Int32 priority;
			Function function;
		};

		virtual ~TypeInfo()
		{
		}

		virtual Bool parseValue( const lexer::Token& token, T& outValue ) const = 0;

		const Unary* getUnary( String name ) const
		{
			return m_unaryOperators.get( name );
		}

		const Binary* getBinary( String name ) const
		{
			return m_binaryOperators.get( name );
		}

		const Array<String>& getDualSymbols() const
		{
			return m_dualSymbols;
		}

	protected:
		Map<String, Unary> m_unaryOperators;
		Map<String, Binary> m_binaryOperators;
		Array<String> m_dualSymbols;
	};

	/**
	 *	An amazing expression evalutor
	 */
	template<typename TYPE_INFO> class Evaluator final
	{
	public:
		using TYPE = typename TYPE_INFO::TYPE;
		using VARIABLES = Map<String, TYPE>;

		static Bool evaluate( String expression, const VARIABLES& variables, TYPE& result, String* outError = nullptr )
		{
			static TYPE_INFO typeInfo;

			// todo: add proper support of dual symbols
			lexer::Lexer lexer( new Text( expression ), { typeInfo.getDualSymbols(), TEXT( '\"' ) } );
			String error;

			Bool status = evaluateImpl( lexer, variables, typeInfo, result, 0, error );

			if( outError )
			{
				*outError = error;
			}

			return status;
		}

	private:
		static Bool evaluateImpl( lexer::Lexer& lexer, const VARIABLES& variables, const TYPE_INFO& typeInfo, TYPE& result, Int32 priority, String& error )
		{
			TYPE value;
			lexer::Token token;
			lexer.getToken( token, true );

			if( token.getText() == TEXT( "(" ) )
			{
				if( !evaluateImpl( lexer, variables, typeInfo, value, 0, error ) )
				{
					return false;
				}

				if( !lexer.matchSymbol( TEXT( ")" ) ) )
				{
					error = TEXT( "No enclosing bracket found" );
					return false;
				}
			}
			else if( token.getType() == lexer::ETokenType::Identifier )
			{
				const TYPE* variable = variables.get( token.getText() );

				if( variable )
				{
					value = *variable;
				}
				else
				{
					error = String::format( TEXT( "Variable \"%s\" is not found" ), *token.getText() );
					return false;
				}
			}
			else if( auto unary = typeInfo.getUnary( token.getText() ) )
			{
				if( !evaluateImpl( lexer, variables, typeInfo, value, 999, error ) )
				{
					return false;
				}

				value = unary->function( value );
			}
			else if( typeInfo.parseValue( token, value ) )
			{
			}
			else
			{
				error = TEXT( "Missing sub-expression" );
				return false;
			}

		OperatorTest:;

			if( !lexer.getToken( token, false ) )
			{
				result = value;
				return true;
			}
			else if( auto binary = typeInfo.getBinary( token.getText() ) )
			{
				if( priority >= binary->priority )
				{
					result = value;
					lexer.gotoToken( token );
					return true;
				}
				else
				{
					TYPE other;

					if( evaluateImpl( lexer, variables, typeInfo, other, binary->priority, error ) )
					{
						value = binary->function( value, other );
						goto OperatorTest;
					}
					else
					{
						return false;
					}
				}
			}
			else
			{
				result = value;
				lexer.gotoToken( token );
				return true;
			}
		}
	};

	/**
	 *	An integer type info
	 */
	class IntegerTypeInfo: public TypeInfo<Int32>
	{
	public:
		IntegerTypeInfo()
		{
			m_unaryOperators.put( TEXT( "-" ), { []( Int32 a )->Int32{ return -a; } } );
			m_unaryOperators.put( TEXT( "+" ), { []( Int32 a )->Int32{ return +a; } } );
			m_unaryOperators.put( TEXT( "~" ), { []( Int32 a )->Int32{ return ~a; } } );
			m_unaryOperators.put( TEXT( "!" ), { []( Int32 a )->Int32{ return !a; } } );

			m_binaryOperators.put( TEXT( "+"  ), { 5, []( Int32 a, Int32 b )->Int32{ return a + b;  } } );
			m_binaryOperators.put( TEXT( "-"  ), { 5, []( Int32 a, Int32 b )->Int32{ return a - b;  } } );
			m_binaryOperators.put( TEXT( "*"  ), { 6, []( Int32 a, Int32 b )->Int32{ return a * b;  } } );
			m_binaryOperators.put( TEXT( "/"  ), { 6, []( Int32 a, Int32 b )->Int32{ return a / b;  } } );
			m_binaryOperators.put( TEXT( "%"  ), { 6, []( Int32 a, Int32 b )->Int32{ return a % b;  } } );

			m_binaryOperators.put( TEXT( "<"  ), { 4, []( Int32 a, Int32 b )->Int32{ return a < b;  } } );
			m_binaryOperators.put( TEXT( ">"  ), { 4, []( Int32 a, Int32 b )->Int32{ return a > b;  } } );
			m_binaryOperators.put( TEXT( "<=" ), { 4, []( Int32 a, Int32 b )->Int32{ return a <= b; } } );
			m_binaryOperators.put( TEXT( ">=" ), { 4, []( Int32 a, Int32 b )->Int32{ return a >= b; } } );

			m_binaryOperators.put( TEXT( "==" ), { 3, []( Int32 a, Int32 b )->Int32{ return a == b; } } );
			m_binaryOperators.put( TEXT( "!=" ), { 3, []( Int32 a, Int32 b )->Int32{ return a != b; } } );

			m_dualSymbols.push( TEXT( "<=" ) );
			m_dualSymbols.push( TEXT( ">=" ) );
			m_dualSymbols.push( TEXT( "!=" ) );
			m_dualSymbols.push( TEXT( "==" ) );
		}

		Bool parseValue( const lexer::Token& token, Int32& outValue ) const override
		{
			if( token.getType() == lexer::ETokenType::Integer )
			{
				outValue = token.getIntConst();
				return true;
			}
			else
			{
				return false;
			}
		}
	};

	/**
	 *	An float type info
	 */
	class FloatTypeInfo: public TypeInfo<Float>
	{
	public:
		FloatTypeInfo()
		{
			m_unaryOperators.put( TEXT( "-" ), { []( Float a )->Float{ return -a; } } );
			m_unaryOperators.put( TEXT( "+" ), { []( Float a )->Float{ return +a; } } );

			m_binaryOperators.put( TEXT( "+"  ), { 5, []( Float a, Float b )->Float{ return a + b;  } } );
			m_binaryOperators.put( TEXT( "-"  ), { 5, []( Float a, Float b )->Float{ return a - b;  } } );
			m_binaryOperators.put( TEXT( "*"  ), { 6, []( Float a, Float b )->Float{ return a * b;  } } );
			m_binaryOperators.put( TEXT( "/"  ), { 6, []( Float a, Float b )->Float{ return a / b;  } } );
		}

		Bool parseValue( const lexer::Token& token, Float& outValue ) const override
		{
			if( token.getType() == lexer::ETokenType::Integer || token.getType() == lexer::ETokenType::Float )
			{
				outValue = token.getFloatConst();
				return true;
			}
			else
			{
				return false;
			}
		}
	};
}
}