//-----------------------------------------------------------------------------
//	System.h: A FFX system
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace ffx
{		
	/**
	 *	A FFX system
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
		Map<res::ResourceId, Effect*> m_effects;

		rend::Device* m_device;

		System() = delete;

		void destroyEffect( Effect* effect );
	};
}
}