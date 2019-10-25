//-----------------------------------------------------------------------------
//	SoundSystem.h: A sound system
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace aud
{
	/**
	 *	A Sound system
	 */
	class SoundSystem final: public res::IResourceSystem
	{
	public:
		SoundSystem( aud::Device* device );
		~SoundSystem();

		res::Resource* createResource( String resourceName, res::ResourceId resourceId,
			const res::CompiledResource& compiledResource ) override;

		Bool allowHotReloading() const override;
		void reloadResource( res::ResourceId resourceId, const res::CompiledResource& compiledResource ) override;

		Bool hasResource( res::ResourceId resourceId ) const override;
		res::Resource* getResource( res::ResourceId resourceId ) const override;

	private:
		Map<res::ResourceId, Sound*> m_sounds;

		aud::Device* m_device;		
		
		SoundSystem() = delete;

		void destroySound( Sound* sound );
	};
}
}