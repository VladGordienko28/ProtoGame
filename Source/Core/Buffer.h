//-----------------------------------------------------------------------------
//	Buffer.h: A temporal storage of data
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 *	A special class for data reading
	 */
	class BufferReader: public IInputStream
	{
	public:
		BufferReader( const Array<UInt8>& data )
			:	m_data( data ),
				m_position( 0 )
		{
			m_hasError = false;
		}

		SizeT readData( void* buffer, SizeT size ) override
		{
			assert( buffer );

			const SizeT bytesToRead = min( size, m_data.size() - m_position );

			if( bytesToRead == size )
			{
				mem::copy( buffer, &m_data[m_position], size );
				m_position += size;			
			}
			else
			{
				m_hasError = true;
			}

			return bytesToRead;
		}

		SizeT totalSize() override
		{
			return m_data.size();
		}

		void seek( SizeT newPosition ) override
		{
			assert( newPosition >= 0 && newPosition < static_cast<SizeT>( m_data.size() ) );
			m_position = newPosition;
		}

		SizeT tell() override
		{
			return m_position;
		}

		Bool isEof() const override
		{
			return m_position >= static_cast<SizeT>( m_data.size() );
		}

	private:
		const Array<UInt8>& m_data;
		SizeT m_position;

		BufferReader() = delete;
	};

	/**
	 *	A base buffer writer
	 */
	template<typename ContainerType> class BufferWriterBase: public IOutputStream
	{
	public:
		void* getData()
		{
			assert( m_data.size() > 0 );
			return &m_data[0];
		}

		SizeT size() const override
		{
			return m_data.size();
		}

		SizeT tell() const override
		{
			return m_position;
		}

		void* reserveData( SizeT numBytes ) override
		{
			if( m_position + numBytes > static_cast<SizeT>( m_data.size() ) )
			{
				m_data.setSize( static_cast<Int32>( m_position + numBytes ) );
			}

			void* result = &m_data[static_cast<Int32>( m_position )];
			m_position += numBytes;

			return result;
		}

		void writeData( const void* buffer, SizeT numBytes ) override
		{
			if( m_position + numBytes > static_cast<SizeT>( m_data.size() ) )
			{
				m_data.setSize( static_cast<Int32>( m_position + numBytes ) );
			}

			mem::copy( &m_data[m_position], buffer, numBytes );
			m_position += numBytes;
		}

		void seek( SizeT offset ) override
		{
			assert( offset > 0 && offset < SizeT( m_data.size() ) );
			m_position = offset;
		}

	protected:
		ContainerType m_data;
		SizeT m_position;

		BufferWriterBase()
			:	m_position( 0 ),
				m_data()
		{
		}

		BufferWriterBase( ContainerType& userData )
			:	m_position( 0 ),
				m_data( userData )
		{
		}
	};

	/**
	 *	A buffer which write data to own buffer
	 */
	class OwningBufferWriter: public BufferWriterBase<Array<UInt8>>
	{
	public:
		OwningBufferWriter()
			:	BufferWriterBase<Array<UInt8>>()
		{
		}

		~OwningBufferWriter()
		{
			m_data.empty();
		}
	};

	/**
	 *	A buffer which write data to user specified buffer
	 */
	class UserBufferWriter: public BufferWriterBase<Array<UInt8>&>
	{
	public:
		UserBufferWriter( Array<UInt8>& userData )
			:	BufferWriterBase<Array<UInt8>&>( userData )
		{
		}

		~UserBufferWriter()
		{
		}

	private:
		UserBufferWriter() = delete;
		UserBufferWriter( UserBufferWriter& ) = delete;
		UserBufferWriter( UserBufferWriter&& ) = delete;
	};
}