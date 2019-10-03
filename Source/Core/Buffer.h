//-----------------------------------------------------------------------------
//	Buffer.h: A temporal storage of data
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 *	A special buffer for data writing
	 */
	//todo: split into two classes
	class BufferWriter: public IOutputStream
	{
	public:
		// own data
		BufferWriter()
			:	m_position( 0 ),
				m_data( &m_OwnData )
		{
		}

		// custom data
		BufferWriter( Array<UInt8>* inBuffer )
			:	m_position( 0 ),
				m_data( inBuffer )
		{
			assert( inBuffer );
			assert( inBuffer->size() == 0 );
		}

		~BufferWriter()
		{
			m_OwnData.empty();
		}

		void* data()
		{
			return &(*m_data)[0];
		}

		SizeT size() const override
		{
			return m_data->size();
		}

		SizeT tell() const override
		{
			return m_position;
		}

		void* reserveData( SizeT numBytes ) override
		{
			if( m_position + numBytes > SizeT( m_data->size() ) )
			{
				m_data->setSize( static_cast<Int32>( m_position + numBytes ) );
			}

			void* result = &m_data[static_cast<Int32>( m_position )];
			m_position += numBytes;

			return result;
		}

		void writeData( const void* buffer, SizeT numBytes ) override
		{
			if( m_position + numBytes > SizeT( m_data->size() ) )
			{
				m_data->setSize( static_cast<Int32>( m_position + numBytes ) );
			}

			mem::copy( &(*m_data)[m_position], buffer, numBytes );
			m_position += numBytes;
		}

		void seek( SizeT offset ) override
		{
			assert( offset > 0 && offset < SizeT( m_data->size() ) );
			m_position = offset;
		}

	private:
		Array<UInt8>* m_data;
		Array<UInt8> m_OwnData;
		SizeT m_position;
	};

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
			assert( newPosition >= 0 && newPosition < m_data.size() );
			m_position = newPosition;
		}

		SizeT tell() override
		{
			return m_position;
		}

		Bool isEof() const override
		{
			return m_position >= m_data.size();
		}

	private:
		const Array<UInt8>& m_data;
		SizeT m_position;

		BufferReader() = delete;
	};
}