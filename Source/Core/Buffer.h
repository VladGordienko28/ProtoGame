//-----------------------------------------------------------------------------
//	Buffer.h: A temporal storage of data
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 *	A special buffer for data writing
	 */
	class BufferWriter: public IOutputStream
	{
	public:
		BufferWriter()
			:	m_position( 0 )
		{
		}

		~BufferWriter()
		{
			m_data.empty();
		}

		void* data()
		{
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
			if( m_position + numBytes > SizeT( m_data.size() ) )
			{
				m_data.setSize( static_cast<Int32>( m_position + numBytes ) );
			}

			void* result = &m_data[static_cast<Int32>( m_position )];
			m_position += numBytes;

			return result;
		}

		void writeData( const void* buffer, SizeT numBytes ) override
		{
			if( m_position + numBytes > SizeT( m_data.size() ) )
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

	private:
		Array<UInt8> m_data;
		SizeT m_position;
	};
}