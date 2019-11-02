//-----------------------------------------------------------------------------
//	System.h: A UI layout system
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ui
{
	/**
	 *	An UI layout system
	 */
	class System final: public res::IResourceSystem
	{
	public:
		System();
		~System();

		res::Resource* createResource( String resourceName, res::ResourceId resourceId,
			const res::CompiledResource& compiledResource ) override;

		Bool allowHotReloading() const override;
		void reloadResource( res::ResourceId resourceId, const res::CompiledResource& compiledResource ) override;

		Bool hasResource( res::ResourceId resourceId ) const override;
		res::Resource* getResource( res::ResourceId resourceId ) const override;

	private:
		Map<res::ResourceId, Layout*> m_layouts;

		void destroyLayout( Layout* layout );
	};
}
}