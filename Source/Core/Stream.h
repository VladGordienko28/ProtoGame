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
	};

	/**
	 *	An output stream interface.
	 */
	class IOutputStream: public ReferenceCount
	{
	public:
		using Ptr = SharedPtr<IOutputStream>;

		virtual ~IOutputStream() = default;

		virtual void writeData( const void* buffer, SizeT size ) = 0;
		virtual void seek( SizeT newPosition ) = 0;
		virtual SizeT tell() = 0;
	};
}