//-----------------------------------------------------------------------------
//	TextWriter.h: A simple utility class for text writing
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 *	An useful text writer class
	 */
	template<typename STRING_TYPE, typename TEXT_TYPE> class TextWriterBase final
	{
	public:
		using THIS_TYPE = TextWriterBase<STRING_TYPE, TEXT_TYPE>;

		TextWriterBase()
		{
		}

		~TextWriterBase()
		{
		}

		typename TEXT_TYPE::Ptr getText() const
		{
			return new TEXT_TYPE( m_text );
		}

		STRING_TYPE getAsString() const
		{
			return m_text;
		}

	private:
		STRING_TYPE m_text;

		TextWriterBase( const THIS_TYPE& ) = delete;
		TextWriterBase( THIS_TYPE&& ) = delete;
		THIS_TYPE& operator=( const THIS_TYPE& ) = delete;

	public:
		friend THIS_TYPE& operator<<( THIS_TYPE& writer, STRING_TYPE str )
		{
			writer.m_text += str;
			return writer;
		}

		friend THIS_TYPE& operator<<( THIS_TYPE& writer, const typename STRING_TYPE::CHAR_TYPE* str )
		{
			writer.m_text += str;
			return writer;
		}

		friend THIS_TYPE& operator<<( THIS_TYPE& writer, typename STRING_TYPE::CHAR_TYPE ch )
		{
			typename STRING_TYPE::CHAR_TYPE buffer[2] = { ch, 0 };
			writer.m_text += buffer;
			return writer;
		}
	};

	/**
	 *	Define classes
	 */
	using TextWriter = TextWriterBase<String, Text>;
}