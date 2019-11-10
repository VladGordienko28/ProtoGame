//-----------------------------------------------------------------------------
//	Visitor.h: An abstract UI tree visitor
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
	/**
	 *	An abstract visitor which helps with UI
	 *	tree traverse
	 */
	class IVisitor
	{
	public:
		virtual ~IVisitor()
		{
		}

		virtual void visit( Element* element ) = 0;

		virtual void enterContainer( Container* container ) {};
		virtual void leaveContainer( Container* container ) {};
	};
}
}