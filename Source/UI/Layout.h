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

		String getName() const { return m_name; }

		JSon::Ptr getLayout() const;

	private:
		String m_name; // todo: move to res::Resource
		JSon::Ptr m_layout;

		Layout() = delete;
		Layout( String name );

		Bool create( const res::CompiledResource& compiledResource );
		void destroy();

		friend class System;
	};
}
}
