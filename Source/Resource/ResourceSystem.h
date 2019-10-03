//-----------------------------------------------------------------------------
//	ResourceSystem.h: An abstract resource system
//	Created by Vlad Gordienko, 2019
//-----------------------------------------------------------------------------

namespace flu
{
namespace res
{
	/**
	 *	An abstract resource system
	 */
	class IResourceSystem: public NonCopyable
	{
	public:
		using UPtr = UniquePtr<IResourceSystem>;

		virtual Resource* createResource( String resourceName, ResourceId resourceId,
			const CompiledResource& compiledResource ) = 0;

		virtual Bool allowHotReloading() const = 0;
		virtual void reloadResource( ResourceId resourceId, const CompiledResource& compiledResource ) = 0;

		virtual Bool hasResource( ResourceId resourceId ) const = 0;
		virtual Resource* getResource( ResourceId resourceId ) const = 0;
	};

	using ResourceSystemList = StaticArray<IResourceSystem::UPtr, Resource::NUM_TYPES>;
}
}