//-----------------------------------------------------------------------------
//	System.h: A Font system
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace fnt
{
	/**
	 *	An Font system
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
		Map<res::ResourceId, Font*> m_fonts;

		void destroyFont( Font* font );
	};
}
}