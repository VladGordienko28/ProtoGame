//-----------------------------------------------------------------------------
//	Atomic.h: A thread-safe counter
//	Created by Vlad Gordienko, 2018
//-----------------------------------------------------------------------------

namespace flu
{
namespace concurrency
{
	/**
	 *	An atomic integer value
	 */
	class Atomic: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<Atomic>;

		virtual Int32 increment() = 0;
		virtual Int32 decrement() = 0;
		virtual Int32 add( Int32 amount ) = 0;
		virtual Int32 subtract( Int32 amount ) = 0;
		virtual Int32 setValue( Int32 newValue ) = 0;
		virtual Int32 getValue() const = 0;

		static Atomic* create();
		static Atomic* create( Int32 value );

	protected:
		Atomic() = default;
	};

} // namespace concurrency
} // namespace flu