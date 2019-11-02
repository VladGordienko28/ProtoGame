//-----------------------------------------------------------------------------
//	Element.h: An abstract UI element
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
	/**
	 *	An abstrct UI element
	 */
	class Element: public NonCopyable
	{
	public:
		Element(){} // todo: not empty but...
		virtual ~Element(){}



	public:// protected ///////////////////////////////////////////////////////////
		// relation
		Container* m_owner;
		Root* m_root;

		// layout
		Size m_size;
		Thickness m_margin;
		EHorizAlign m_horizAlign;
		EVertAlign m_vertAlign;

		// render
		Float m_opacity; // todo: maybe move to sprite

	};
}
}