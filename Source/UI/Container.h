//-----------------------------------------------------------------------------
//	Container.h: An abstract UI container
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
	/**
	 *	An abstrct UI container
	 */
	class Container: public Element
	{
	public:


		// really, really bad!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		void addChild( Element* child )
		{
			m_children.addUnique( child );
		}
		void removeChild( Element* child )
		{
			m_children.removeUnique( child, true );
		}
		Bool isChild( Element* child ) const;

	protected:
		Array<Element*> m_children;

		Thickness m_padding;


	};
}
}