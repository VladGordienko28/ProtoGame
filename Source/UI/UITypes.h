//-----------------------------------------------------------------------------
//	UITypes.h: A base UI system types
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
	/**
	 *	An element horizontal alignment
	 */
	enum class EHorizAlign
	{
		Left,
		Right,
		Center,
		Stretch
	};

	/**
	 *	An element vertical alignment
	 */
	enum class EVertAlign
	{
		Top,
		Bottom,
		Center,
		Stretch
	};

	/**
	 *	A direction
	 */
	enum class EDirection
	{
		Up,
		Left,
		Down,
		Right
	};

	/**
	 *	A position
	 */
	struct Position
	{
	public:
		Float x = 0.f;
		Float y = 0.f;

		// todo: implement methods and operators
	};

	/**
	 *	A size
	 */
	struct Size
	{
	public:
		Float width = 0.f;
		Float height = 0.f;

		// todo: implement methods and operators
	};

	/**
	 *	A thickness
	 */
	struct Thickness
	{
	public:
		Float top = 0.f;
		Float left = 0.f;
		Float bottom = 0.f;
		Float right = 0.f;

		// todo: implement methods and operators
	};

	/**
	 *	Forward declaration
	 */
	class Element;
		class Container;
			class UserLayout;
			class Root;
}
}