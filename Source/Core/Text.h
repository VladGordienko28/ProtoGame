//-----------------------------------------------------------------------------
//	Text.h: An abstract text reading/writing interface
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu
{
	/**
	 *	A bunch of text
	 */
	class Text: public ReferenceCount
	{
	public:
		using Ptr = SharedPtr<Text>;

		Text();
		Text( const Char* other );
		Text( String other );
		Text( const Array<String>& other );
		~Text();

		void appendLine( String newLine );
		void appendLine( const Char* newLine );
		void empty();
		String toString() const;

		String& operator[]( Int32 line )
		{
			return m_lines[line];
		}

		const String& operator[]( Int32 line ) const
		{
			return m_lines[line];
		}

		Int32 size() const
		{
			return m_lines.size();
		}

		Bool isEmpty() const
		{
			return m_lines.size() == 0;
		}

	private:
		Array<String> m_lines;

		Text( const Text& other ) = delete;
		Text( Text&& other ) = delete;

		void internalAppend( const Char* newLine );
	};

} // namespace flu