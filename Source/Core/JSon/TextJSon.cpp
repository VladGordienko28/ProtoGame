//-----------------------------------------------------------------------------
//	TextJSon.cpp: Text JSon managment functions
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Core/Core.h"

namespace flu
{
	static const Char* JSON_BOOL_TRUE = L"true";
	static const Char* JSON_BOOL_FALSE = L"false";

	static const Int32 JSON_INDENT_SIZE = 4;

	JSon::Ptr JSon::loadFromText( Text::Ptr text, String* error )
	{
		struct Loader
		{
		public:
			static JSon::Ptr loadJSonNode( lexer::Lexer& parser, String& error )
			{
				lexer::Token token;
				if( parser.getToken( token, true ) )
				{
					switch ( token.getType() )
					{
						case lexer::ETokenType::Symbol:
						{
							if( token.getText() == L"{" )
							{
								JSon::Ptr objectNode = JSon::createObjectNode();

								for( ; ; )
								{
									lexer::Token nextToken;

									if( parser.getToken( nextToken, false ) )
									{
										if( nextToken.getText() == L"}" )
										{
											return objectNode;
										}
										else if( nextToken.getType() == lexer::ETokenType::Identifier )
										{
											if( !parser.matchSymbol( L":" ) )
											{
												error = L"missing \":\"";
												return nullptr;
											}

											JSon::Ptr valueNode = loadJSonNode( parser, error );
											if( valueNode.hasObject() )
											{
												objectNode->addField( nextToken.getText(), valueNode );

												if( parser.matchSymbol( L"}" ) )
												{
													return objectNode;
												}
												else
												{
													if( !parser.matchSymbol( L"," ) )
													{
														error = L"missing \",\"";
														return nullptr;
													}
												}
											}
											else
											{
												return nullptr;
											}
										}
										else
										{
											error = String::format( L"unexpected token \"%s\"", *nextToken.getText() );
											return nullptr;
										}
									}
									else
									{
										error = L"unexpected end of text";
										return nullptr;
									}
								}
							}
							else if( token.getText() == L"[" )
							{					
								JSon::Ptr arrayNode = JSon::createArrayNode();

								for( ; ; )
								{							
									if( parser.peekSymbol() == L"]" )
									{
										return arrayNode;
									}
									else
									{
										JSon::Ptr valueNode = loadJSonNode( parser, error );
										if( valueNode.hasObject() )
										{
											arrayNode->insertElement( valueNode );

											if( parser.matchSymbol( L"]" ) )
											{
												return arrayNode;
											}
											else
											{
												if( !parser.matchSymbol( L"," ) )
												{
													error = L"missing \",\"";
													return nullptr;
												}
											}
										}
										else
										{
											return nullptr;
										}
									}
								}
							}
							else
							{
								error = String::format( L"unexpected symbol \"%s\"", *token.getText() );
								return nullptr;
							}
						}
						case lexer::ETokenType::Identifier:
						{
							if( cstr::insensitiveCompare( *token.getText(), JSON_BOOL_TRUE ) == 0 )
							{
								return JSon::createBoolNode( true );
							}
							else if( cstr::insensitiveCompare( *token.getText(), JSON_BOOL_FALSE ) == 0 )
							{
								return JSon::createBoolNode( false );
							}
							else
							{
								error = String::format( L"unexpected identifier \"%s\"", *token.getText() );
								return nullptr;
							}
						}
						case lexer::ETokenType::String:
						{
							return JSon::createStringNode( token.getStringConst() );
						}
						case lexer::ETokenType::Integer:
						{
							return JSon::createIntNode( token.getIntConst() );
						}
						case lexer::ETokenType::Float:
						{
							return JSon::createFloatNode( token.getFloatConst() );
						}
						case lexer::ETokenType::Unknown:
						default:
						{
							error = L"unexpected token";
							return nullptr;
						}
					}
				}
				else
				{
					error = L"unexpected end of text";
					return nullptr;
				}
			}
		};

		const lexer::LexerConfig jsonConfig =
		{
			{ },
			'\"'
		};

		String tempError;
		lexer::Lexer parser( text, jsonConfig );
		JSon::Ptr result = Loader::loadJSonNode( parser, tempError );

		if( !result && error )
		{
			*error = String::format( L"%s at line: %d", *tempError, parser.getPosition().line );
		}

		return result;
	}

	Text::Ptr JSon::saveToText( JSon::Ptr json, String* error )
	{
		struct Saver
		{
		public:
			static String saveJSonNode( JSon::Ptr json, String& error, Int32 indentLevel )
			{
				if( json.hasObject() )
				{
					switch ( json->getNodeType() )
					{
						case ENodeType::BOOL:
						{
							return json->asBool() ? JSON_BOOL_TRUE : JSON_BOOL_FALSE;
						}
						case ENodeType::INT:
						{
							return String::fromInteger( json->asInt() );
						}
						case ENodeType::FLOAT:
						{
							return String::fromFloat( json->asFloat() );
						}
						case ENodeType::STRING:
						{
							return String::format( L"\"%s\"", *json->asString() );
						}
						case ENodeType::OBJECT:
						{
							String indent = String::ofChar( ' ', ( indentLevel + 1 ) * JSON_INDENT_SIZE );
							String result = L"{\n";

							for( auto it = json->firstField(); it != json->endField(); it++ )
							{
								String fieldText = saveJSonNode( it->value, error, indentLevel + 1 );

								if( fieldText )
								{
									result += String::format( L"%s%s: %s%s\n", *indent, 
										*it->key, *fieldText, 
										it != ( json->endField() - 1 ) ? L"," : L"" );
								}
								else
								{
									return L"";
								}
							}

							return result + String::ofChar( ' ', indentLevel * JSON_INDENT_SIZE ) + L"}";	
						}
						case ENodeType::ARRAY:
						{
							String indent = String::ofChar( ' ', ( indentLevel + 1 ) * JSON_INDENT_SIZE );
							String result = L"[\n";

							for( Int32 i = 0; i < json->arraySize(); ++i )
							{
								String elementText = saveJSonNode( json->getElement( i, EMissingPolicy::USE_NULL ), error, indentLevel + 1 );

								if( elementText )
								{
									result += String::format( L"%s%s%s\n", *indent, 
										*elementText, 
										i != ( json->arraySize() - 1 ) ? L"," : L"" );
								}
								else
								{
									return L"";
								}
							}

							return result + String::ofChar( ' ', indentLevel * JSON_INDENT_SIZE ) + L"]";			
						}
						case ENodeType::STUB:
						default:
						{
							error = L"temporal node found in json";
							return L"";
						}
					}
				}
				else
				{
					error = L"null node found";
					return L"";
				}
			}
		};

		String tempError;
		String result = Saver::saveJSonNode( json, tempError, 0 );

		if( result == L"" && error )
		{
			*error = tempError;
		}

		return result ? new Text( result ) : nullptr;
	}
}