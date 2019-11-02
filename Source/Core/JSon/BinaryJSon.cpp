//-----------------------------------------------------------------------------
//	BinaryJSon.cpp: Binary JSon managment functions
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

#include "Core/Core.h"

namespace flu 
{
	JSon::Ptr JSon::loadFromStream( IInputStream& inputStream, String* error )
	{
		struct Deserializer
		{
		public:
			static JSon::Ptr deserializeJSonNode( IInputStream& inputStream, String& error )
			{
				UInt8 nodeTypeId;
				inputStream >> nodeTypeId;
				ENodeType nodeType = static_cast<ENodeType>( nodeTypeId );

				if( inputStream.hasError() )
				{
					error = L"unexpected end of stream";
					return nullptr;
				}

				switch( nodeType )
				{
					case ENodeType::BOOL:
					{
						Bool value;
						inputStream >> value;
						return JSon::createBoolNode( value );
					}
					case ENodeType::INT:
					{
						Int32 value;
						inputStream >> value;
						return JSon::createIntNode( value );
					}
					case ENodeType::FLOAT:
					{
						Float value;
						inputStream >> value;
						return JSon::createFloatNode( value );
					}
					case ENodeType::STRING:
					{
						String value;
						inputStream >> value;
						return JSon::createStringNode( value );
					}
					case ENodeType::ARRAY:
					{
						Int32 arraySize;
						inputStream >> arraySize;

						JSon::Ptr arrayNode = JSon::createArrayNode();

						for( Int32 i = 0; i < arraySize; ++i )
						{
							JSon::Ptr value = deserializeJSonNode( inputStream, error );

							if( value )
							{
								arrayNode->insertElement( value );
							}
							else
							{
								return nullptr;
							}
						}

						return arrayNode;
					}
					case ENodeType::OBJECT:
					{
						Int32 fieldsCount;
						inputStream >> fieldsCount;

						JSon::Ptr objectNode = JSon::createObjectNode();

						for( Int32 i = 0; i < fieldsCount; ++i )
						{
							String key;
							inputStream >> key;

							JSon::Ptr value = deserializeJSonNode( inputStream, error );

							if( value )
							{
								objectNode->addField( key, value );
							}
							else
							{
								return nullptr;
							}
						}

						return objectNode;
					}
					case ENodeType::STUB:
					default:
					{
						error = L"unknown JSon node type";
						return nullptr;
					}
				}
			}
		};

		String tempError;
		JSon::Ptr result = Deserializer::deserializeJSonNode( inputStream, tempError );

		if( !result && error )
		{
			*error = tempError;
		}

		return result;
	}

	Bool JSon::saveToStream( IOutputStream& outputStream, JSon::Ptr json, String* error )
	{
		struct Serializer
		{
		public:
			static Bool serializeJSonNode( IOutputStream& outputStream, JSon::Ptr json, String& error )
			{
				if( json.hasObject() )
				{
					const ENodeType nodeType = json->getNodeType();				
					outputStream << static_cast<UInt8>( nodeType );

					switch( nodeType )
					{
						case ENodeType::BOOL:
						{
							outputStream << json->asBool();
							return true;
						}
						case ENodeType::INT:
						{
							outputStream << json->asInt();
							return true;
						}
						case ENodeType::FLOAT:
						{
							outputStream << json->asFloat();
							return true;
						}
						case ENodeType::STRING:
						{
							outputStream << json->asString();
							return true;
						}
						case ENodeType::ARRAY:
						{
							outputStream << json->arraySize();

							for( Int32 i = 0; i < json->arraySize(); ++i )
							{
								if( !serializeJSonNode( outputStream, json->getElement( i ), error ) )
								{
									return false;
								}
							}

							return true;
						}
						case ENodeType::OBJECT:
						{
							outputStream << json->fieldsCount();

							for( auto it = json->firstField(); it != json->endField(); it++  )
							{
								outputStream << it->key;

								if( !serializeJSonNode( outputStream, it->value, error ) )
								{
									return false;
								}
							}

							return true;
						}
						case ENodeType::STUB:
						default:
						{
							error = L"unknown json node type";
							return false;
						}
					}
				}
				else
				{
					error = L"null json node found";
					return false;
				}
			}
		};

		// todo: stream to intermediate buffer and then copy
		String tempError;
		Bool result = Serializer::serializeJSonNode( outputStream, json, tempError );

		if( !result && error )
		{
			*error = tempError;
		}

		return result;
	}
}