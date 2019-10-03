//-----------------------------------------------------------------------------
//	System.h: A Image system
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace img
{
	/**
	 *	An Image system
	 */
	class System final: public res::IResourceSystem
	{
	public:
		System( rend::Device* device );
		~System();

		res::Resource* createResource( String resourceName, res::ResourceId resourceId,
			const res::CompiledResource& compiledResource ) override;

		Bool allowHotReloading() const override;
		void reloadResource( res::ResourceId resourceId, const res::CompiledResource& compiledResource ) override;

		Bool hasResource( res::ResourceId resourceId ) const override;
		res::Resource* getResource( res::ResourceId resourceId ) const override;

	private:
		Map<res::ResourceId, Image*> m_images;

		rend::Device* m_device;		
		
		System() = delete;

		void destroyImage( Image* image );
	};
}
}