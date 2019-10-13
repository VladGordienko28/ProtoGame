//-----------------------------------------------------------------------------
//	Stream.h: An abstract stream interface
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 *	An input stream interface.
	 */
	class IInputStream: public ReferenceCount
	{
	public:
		using Ptr = SharedPtr<IInputStream>;

		virtual ~IInputStream() = default;

		virtual SizeT readData( void* buffer, SizeT size ) = 0;
		virtual SizeT totalSize() = 0;
		virtual void seek( SizeT newPosition ) = 0;
		virtual SizeT tell() = 0;
		virtual Bool isEof() const = 0;

		Bool hasError() const
		{
			return m_hasError;
		}

	protected:
		Bool m_hasError;
	};

	/**
	 *	An output stream interface.
	 */
	class IOutputStream: public ReferenceCount
	{
	public:
		using Ptr = SharedPtr<IOutputStream>;

		virtual ~IOutputStream() = default;

		virtual SizeT size() const = 0;
		virtual SizeT tell() const = 0;
		virtual void* reserveData( SizeT numBytes ) = 0;
		virtual void writeData( const void* buffer, SizeT numBytes ) = 0;
		virtual void seek( SizeT offset ) = 0;

		// todo:
		//virtual void copyFrom( IInputStream::Ptr inputStream, SizeT numBytes ) = 0;
	};

	/**
	 *	Helper output operators
	 */
	inline IOutputStream& operator<<( IOutputStream& stream, Int8 x )
	{
		stream.writeData( &x, sizeof( Int8 ) );
		return stream;
	}
	inline IOutputStream& operator<<( IOutputStream& stream, UInt8 x )
	{
		stream.writeData( &x, sizeof( UInt8 ) );
		return stream;
	}
	inline IOutputStream& operator<<( IOutputStream& stream, Int16 x )
	{
		stream.writeData( &x, sizeof( Int16 ) );
		return stream;
	}
	inline IOutputStream& operator<<( IOutputStream& stream, UInt16 x )
	{
		stream.writeData( &x, sizeof( UInt16 ) );
		return stream;
	}
	inline IOutputStream& operator<<( IOutputStream& stream, Int32 x )
	{
		stream.writeData( &x, sizeof( Int32 ) );
		return stream;
	}
	inline IOutputStream& operator<<( IOutputStream& stream, UInt32 x )
	{
		stream.writeData( &x, sizeof( UInt32 ) );
		return stream;
	}
	inline IOutputStream& operator<<( IOutputStream& stream, Int64 x )
	{
		stream.writeData( &x, sizeof( Int64 ) );
		return stream;
	}
	inline IOutputStream& operator<<( IOutputStream& stream, UInt64 x )
	{
		stream.writeData( &x, sizeof( UInt64 ) );
		return stream;
	}
	inline IOutputStream& operator<<( IOutputStream& stream, Float x )
	{
		stream.writeData( &x, sizeof( Float ) );
		return stream;
	}
	inline IOutputStream& operator<<( IOutputStream& stream, Double x )
	{
		stream.writeData( &x, sizeof( Double ) );
		return stream;
	}
	inline IOutputStream& operator<<( IOutputStream& stream, Bool x )
	{
		stream.writeData( &x, sizeof( Bool ) );
		return stream;
	}
	inline IOutputStream& operator<<( IOutputStream& stream, AnsiChar x )
	{
		stream.writeData( &x, sizeof( AnsiChar ) );
		return stream;
	}
	inline IOutputStream& operator<<( IOutputStream& stream, WideChar x )
	{
		stream.writeData( &x, sizeof( WideChar ) );
		return stream;
	}
	inline IOutputStream& operator<<( IOutputStream& stream, const AnsiChar* x )
	{
		if( SizeT numBytes = ( cstr::length( x ) + 1 ) * sizeof( AnsiChar ) )
		{
			stream.writeData( x, numBytes );		
		}
		return stream;
	}
	inline IOutputStream& operator<<( IOutputStream& stream, const WideChar* x )
	{
		if( SizeT numBytes = ( cstr::length( x ) + 1 ) * sizeof( WideChar ) )
		{
			stream.writeData( x, numBytes );		
		}
		return stream;
	}
	template<class T, SizeT S> inline IOutputStream& operator<<( IOutputStream& stream, const T(&x)[S] )
	{
		for( const auto& it : x )
		{
			stream << it;
		}
		return stream;
	}

	/**
	 *	Helper input operators
	 */
	inline IInputStream& operator>>( IInputStream& stream, Int8& x )
	{
		stream.readData( &x, sizeof( Int8 ) );
		return stream;
	}
	inline IInputStream& operator>>( IInputStream& stream, UInt8& x )
	{
		stream.readData( &x, sizeof( UInt8 ) );
		return stream;
	}
	inline IInputStream& operator>>( IInputStream& stream, Int16& x )
	{
		stream.readData( &x, sizeof( Int16 ) );
		return stream;
	}
	inline IInputStream& operator>>( IInputStream& stream, UInt16& x )
	{
		stream.readData( &x, sizeof( UInt16 ) );
		return stream;
	}
	inline IInputStream& operator>>( IInputStream& stream, Int32& x )
	{
		stream.readData( &x, sizeof( Int32 ) );
		return stream;
	}
	inline IInputStream& operator>>( IInputStream& stream, UInt32& x )
	{
		stream.readData( &x, sizeof( UInt32 ) );
		return stream;
	}
	inline IInputStream& operator>>( IInputStream& stream, Int64& x )
	{
		stream.readData( &x, sizeof( Int64 ) );
		return stream;
	}
	inline IInputStream& operator>>( IInputStream& stream, UInt64& x )
	{
		stream.readData( &x, sizeof( UInt64 ) );
		return stream;
	}
	inline IInputStream& operator>>( IInputStream& stream, Float& x )
	{
		stream.readData( &x, sizeof( Float ) );
		return stream;
	}
	inline IInputStream& operator>>( IInputStream& stream, Double& x )
	{
		stream.readData( &x, sizeof( Double ) );
		return stream;
	}
	inline IInputStream& operator>>( IInputStream& stream, Bool& x )
	{
		stream.readData( &x, sizeof( Bool ) );
		return stream;
	}
	inline IInputStream& operator>>( IInputStream& stream, AnsiChar& x )
	{
		stream.readData( &x, sizeof( AnsiChar ) );
		return stream;
	}
	inline IInputStream& operator>>( IInputStream& stream, WideChar& x )
	{
		stream.readData( &x, sizeof( WideChar ) );
		return stream;
	}
	template<class T, SizeT S> inline IInputStream& operator>>( IInputStream& stream, T(&x)[S] )
	{
		for( auto& it : x )
		{
			stream >> it;
		}
		return stream;
	}

	/**
	 *	Special macro for enum streaming
	 */
	#define ENUM_FOR_STREAM( enumType ) \
		inline IOutputStream& operator<<( IOutputStream& stream, enumType x )\
		{\
			UInt8 temp = static_cast<UInt8>( x );\
			stream.writeData( &temp, sizeof( UInt8 ) );\
			return stream;\
		}\
		inline IInputStream& operator>>( IInputStream& stream, enumType& x )\
		{\
			UInt8 temp;\
			stream.readData( &temp, sizeof( UInt8 ) );\
			x = static_cast<enumType>( temp );\
			return stream;\
		}
}