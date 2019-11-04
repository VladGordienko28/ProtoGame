//-----------------------------------------------------------------------------
//	Layout.h: An UI layout resource
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
	/**
	 *	A layout resource
	 */
	class Layout: public res::Resource
	{
	public:
		DECLARE_RESOURCE( Layout, System, Compiler );

		~Layout();

		JSon::Ptr getLayout() const;

		using Callback = void( UserLayout::* )();

		void addRecreateCallback( UserLayout* userLayout, Callback callback );
		void removeRecreateCallback( UserLayout* userLayout );

	private:
		struct Listener
		{
			UserLayout* userLayout;
			Callback callback;
		};

		Array<Listener> m_listeners;

		JSon::Ptr m_layout;

		Layout() = delete;
		Layout( String name );

		Bool create( const res::CompiledResource& compiledResource );
		void destroy();

		friend class System;
	};
}
}
