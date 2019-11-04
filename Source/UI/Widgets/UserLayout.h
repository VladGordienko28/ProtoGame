//-----------------------------------------------------------------------------
//	UserLayout.h: An element which contains widgets from the user's layout
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
	/**
	 *	An user layout
	 */
	class UserLayout: public Container
	{
	public:
		UserLayout( String name, Root* root );
		~UserLayout();

		Bool load( String resourceName );
		Bool unload();

	private:
		Layout::Ptr m_layout;

		void recreate();
		void destroyChildren();
	};
}
}