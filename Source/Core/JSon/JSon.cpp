//-----------------------------------------------------------------------------
//	JSon.cpp: JSon implementation
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

#include "Core/Core.h"

namespace flu 
{
	class JSonBoolValue: public JSon
	{
	public:
		JSonBoolValue( Bool inValue )
			:	m_value( inValue )
		{
		}

		Bool asBool( Bool _default ) const override
		{
			return m_value;
		}

		Int32 asInt( Int32 _default ) const override
		{
			return m_value ? 1 : 0;
		}

		Float asFloat( Float _default ) const override
		{
			return m_value ? 1.f : 0.f;
		}

		String asString( String _default ) const override
		{
			return m_value ? L"true" : L"false";
		}

	private:
		Bool m_value;

		ENodeType getNodeType() const override
		{
			return ENodeType::BOOL;
		}
	};


	class JSonIntValue: public JSon
	{
	public:
		JSonIntValue( Int32 inValue )
			:	m_value( inValue )
		{
		}

		Bool asBool( Bool _default ) const override
		{
			return m_value != 0;
		}

		Int32 asInt( Int32 _default ) const override
		{
			return m_value;
		}

		Float asFloat( Float _default ) const override
		{
			return static_cast<Float>( m_value );
		}

		String asString( String _default ) const override
		{
			return String::fromInteger( m_value );
		}

	private:
		Int32 m_value;

		ENodeType getNodeType() const override
		{
			return ENodeType::INT;
		}
	};


	class JSonFloatValue: public JSon
	{
	public:
		JSonFloatValue( Float inValue )
			:	m_value( inValue )
		{
		}

		Bool asBool( Bool _default ) const override
		{
			return m_value != 0.f;
		}

		Int32 asInt( Int32 _default ) const override
		{
			return static_cast<Int32>( m_value );
		}

		Float asFloat( Float _default ) const override
		{
			return m_value;
		}

		String asString( String _default ) const override
		{
			return String::fromFloat( m_value );
		}

	private:
		Float m_value;

		ENodeType getNodeType() const override
		{
			return ENodeType::FLOAT;
		}
	};


	class JSonStringValue: public JSon
	{
	public:
		JSonStringValue( String inValue )
			:	m_value( inValue )
		{
		}

		Bool asBool( Bool _default ) const override
		{
			return m_value.len() != 0;
		}

		Int32 asInt( Int32 _default ) const override
		{
			Int32 result;
			m_value.toInteger( result, _default );
			return result;
		}

		Float asFloat( Float _default ) const override
		{
			Float result;
			m_value.toFloat( result, _default );
			return result;
		}

		String asString( String _default ) const override
		{
			return m_value;
		}

	private:
		String m_value;

		ENodeType getNodeType() const override
		{
			return ENodeType::STRING;
		}
	};


	class JSonObjectValue: public JSon
	{
	public:
		JSonObjectValue()
		{
		}

		Bool asBool( Bool _default ) const override
		{
			return m_fields.size() != 0;
		}

		Int32 asInt( Int32 _default ) const override
		{
			return _default;
		}

		Float asFloat( Float _default ) const override
		{
			return _default;
		}

		String asString( String _default ) const override
		{
			String result = L"{ ";

			for( auto& it : m_fields )
			{
				result += String::format( L"%s = %s, ", it.key, it.value );
			}

			return result + L" }";
		}

		void addField( String name, JSon::Ptr field ) override
		{
			assert( field.hasObject() );
			m_fields.put( name, field );
		}

		Bool hasField( String name ) const override
		{ 
			return m_fields.hasKey( name ); 
		}

		void removeField( String name ) override
		{
			m_fields.remove( name );
		}

		Int32 fieldsCount() const override
		{ 
			return m_fields.size();
		}

		JSon::Ptr getField( String name, EMissingPolicy policy ) const override 
		{
			const JSon::Ptr* member = m_fields.get( name );

			if( member )
			{
				return *member;
			}
			else
			{
				return policy == EMissingPolicy::USE_NULL ? nullptr : JSon::createStubNode();
			}
		}

		Map<String, JSon::Ptr>::Iterator firstField() override
		{ 
			return m_fields.begin();
		};

		Map<String, JSon::Ptr>::Iterator endField() override
		{
			return m_fields.end();
		}

	private:
		Map<String, JSon::Ptr> m_fields;

		ENodeType getNodeType() const override
		{
			return ENodeType::OBJECT;
		}
	};


	class JSonArrayValue: public JSon
	{
	public:
		JSonArrayValue()
		{
		}

		Bool asBool( Bool _default ) const override
		{
			return m_elements.size() != 0;
		}

		Int32 asInt( Int32 _default ) const override
		{
			return _default;
		}

		Float asFloat( Float _default ) const override
		{
			return _default;
		}

		String asString( String _default ) const override
		{
			String result = L"[ ";

			for( Int32 i = 0; i < m_elements.size(); ++i )
			{
				result += m_elements[i]->asString();
				
				if( i != m_elements.size() - 1 )
				{
					result += L",";
				}
			}

			return result + L" ]";
		}

		Int32 arraySize() const 
		{ 
			return m_elements.size(); 
		}

		Int32 insertElement( JSon::Ptr element ) override
		{ 
			assert( element.hasObject() );
			return m_elements.push( element );
		}

		JSon::Ptr getElement( Int32 i, EMissingPolicy policy ) override
		{ 
			if( i >= 0 && i < m_elements.size() )
			{
				return m_elements[i];
			}
			else
			{
				return policy == EMissingPolicy::USE_NULL ? nullptr : JSon::createStubNode();
			}
		}

	private:
		Array<JSon::Ptr> m_elements;

		ENodeType getNodeType() const override
		{
			return ENodeType::ARRAY;
		}
	};

	class JSonStubValue: public JSon
	{
	public:
		JSonStubValue()
		{
		}

		~JSonStubValue()
		{
		}

	private:
		ENodeType getNodeType() const override
		{
			return ENodeType::STUB;
		}
	};

	JSon::Ptr JSon::createBoolNode( Bool inValue )
	{
		return new JSonBoolValue( inValue );
	}

	JSon::Ptr JSon::createIntNode( Int32 inValue )
	{
		return new JSonIntValue( inValue );
	}

	JSon::Ptr JSon::createFloatNode( Float inValue )
	{
		return new JSonFloatValue( inValue );
	}

	JSon::Ptr JSon::createStringNode( String inValue )
	{
		return new JSonStringValue( inValue );
	}

	JSon::Ptr JSon::createObjectNode()
	{
		return new JSonObjectValue();
	}

	JSon::Ptr JSon::createArrayNode()
	{
		return new JSonArrayValue();
	}

	JSon::Ptr JSon::createStubNode()
	{
		return new JSonStubValue();
	}

	JSon::JSon()
	{
	}

	Bool JSon::dotgetBool( const Char* name, Bool _default ) const
	{
		JSon::Ptr node = dotgetNode( name, EMissingPolicy::USE_NULL );
		return node.hasObject() ? node->asBool( _default ) : _default;
	}

	Int32 JSon::dotgetInt( const Char* name, Int32 _default ) const
	{
		JSon::Ptr node = dotgetNode( name, EMissingPolicy::USE_NULL );
		return node.hasObject() ? node->asInt( _default ) : _default;
	}

	Float JSon::dotgetFloat( const Char* name, Float _default ) const
	{
		JSon::Ptr node = dotgetNode( name, EMissingPolicy::USE_NULL );
		return node.hasObject() ? node->asFloat( _default ) : _default;
	}

	String JSon::dotgetString( const Char* name, String _default ) const
	{
		JSon::Ptr node = dotgetNode( name, EMissingPolicy::USE_NULL );
		return node.hasObject() ? node->asString( _default ) : _default;
	}

	JSon::Ptr JSon::dotgetObject( const Char* name, EMissingPolicy policy ) const
	{
		return dotgetNode( name, policy );
	}

	JSon::Ptr JSon::dotgetArray( const Char* name, EMissingPolicy policy ) const
	{
		return dotgetNode( name, policy );
	}

	JSon::Ptr JSon::dotgetNode( const Char* name, EMissingPolicy policy ) const
	{
		if( name && *name != '\0' && this->isObject() )
		{
			const Char* dotPosition = cstr::findChar( name, '.' );

			if( dotPosition == nullptr )
			{
				return getField( name, policy );
			}
			else
			{
				String fieldName( name, dotPosition );
				JSon::Ptr nextObject = getField( fieldName, EMissingPolicy::USE_NULL );

				if( nextObject.hasObject() )
				{
					return nextObject->dotgetNode( dotPosition + 1, policy );
				}
			}		
		}

		return policy == EMissingPolicy::USE_STUB ? JSon::createStubNode() : nullptr;
	}
}