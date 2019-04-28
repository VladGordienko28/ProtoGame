//-----------------------------------------------------------------------------
//	JSon.h: JSon object
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu 
{
	/**
	 *	A JSon
	 */
	class JSon: public ReferenceCount
	{
	public:
		using Ptr = SharedPtr<JSon>;

		enum class EMissingPolicy
		{
			USE_NULL,
			USE_STUB
		};

		virtual Bool asBool( Bool _default = false ) const { return _default; }
		virtual Int32 asInt( Int32 _default = 0 ) const { return _default; }
		virtual Float asFloat( Float _default = 0.f ) const { return _default; }
		virtual String asString( String _default = L"" ) const { return _default; }

		virtual void addField( String name, JSon::Ptr field ) {  }
		virtual Bool hasField( String name ) const { return false; }	
		virtual void removeField( String name ) {  }
		virtual Int32 fieldsCount() const { return 0; };
		virtual Map<String, JSon::Ptr>::Iterator firstField() { return nullptr; };
		virtual Map<String, JSon::Ptr>::Iterator endField() { return nullptr; }

		virtual JSon::Ptr getField( String name, EMissingPolicy policy = EMissingPolicy::USE_STUB ) const
		{ 			
			return policy == EMissingPolicy::USE_NULL ? nullptr : JSon::createStubNode(); 
		}

		JSon::Ptr operator[]( String name ) const
		{
			return getField( name, EMissingPolicy::USE_NULL );
		}

		JSon::Ptr operator[]( Char* name ) const
		{
			return getField( name, EMissingPolicy::USE_NULL );
		}

		virtual Int32 arraySize() const { return 0; }
		virtual Int32 insertElement( JSon::Ptr element ) { return 0; }

		virtual JSon::Ptr getElement( Int32 i, EMissingPolicy policy = EMissingPolicy::USE_STUB )
		{
			return policy == EMissingPolicy::USE_NULL ? nullptr : JSon::createStubNode();
		}

		JSon::Ptr operator[]( Int32 i )
		{
			return getElement( i, EMissingPolicy::USE_NULL );
		}

		Bool dotgetBool( const Char* name, Bool _default = false ) const;
		Int32 dotgetInt( const Char* name, Int32 _default = 0 ) const;
		Float dotgetFloat( const Char* name, Float _default = 0.f ) const;
		String dotgetString( const Char* name, String _default = L"" ) const;
		JSon::Ptr dotgetObject( const Char* name, EMissingPolicy policy = EMissingPolicy::USE_STUB ) const;
		JSon::Ptr dotgetArray( const Char* name, EMissingPolicy policy = EMissingPolicy::USE_STUB ) const;

		Bool isBool() const { return getNodeType() == ENodeType::BOOL; }
		Bool isInt() const { return getNodeType() == ENodeType::INT; }
		Bool isFloat() const { return getNodeType() == ENodeType::FLOAT; }
		Bool isString() const { return getNodeType() == ENodeType::STRING; }
		Bool isObject() const { return getNodeType() == ENodeType::OBJECT; }
		Bool isArray() const { return getNodeType() == ENodeType::ARRAY; }

	protected:
		enum class ENodeType
		{
			STUB = 0,
			BOOL,
			INT,
			FLOAT,
			STRING,
			OBJECT,
			ARRAY,
			MAX
		};
		
		JSon();
		JSon( const JSon& other ) = delete;
		JSon( JSon&& other ) = delete;

		virtual ENodeType getNodeType() const = 0;

		JSon::Ptr dotgetNode( const Char* name, EMissingPolicy policy = EMissingPolicy::USE_NULL ) const;

	public:
		static JSon::Ptr createBoolNode( Bool inValue );
		static JSon::Ptr createIntNode( Int32 inValue );
		static JSon::Ptr createFloatNode( Float inValue );
		static JSon::Ptr createStringNode( String inValue );
		static JSon::Ptr createObjectNode();
		static JSon::Ptr createArrayNode();
		static JSon::Ptr createStubNode();

		static JSon::Ptr loadFromStream( IInputStream::Ptr inputStream, String* error = nullptr );
		static Bool saveToStream( IOutputStream::Ptr outputStream, JSon::Ptr json, String* error = nullptr );

		static JSon::Ptr loadFromText( Text::Ptr text, String* error = nullptr );
		static Text::Ptr saveToText( JSon::Ptr json, String* error = nullptr );
	};

} // namespace flu